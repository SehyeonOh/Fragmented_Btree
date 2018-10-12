#include "my_page.h"
#define get2byte(x)   ((x)[0]<<8 | (x)[1])
#define put2byte(p,v) ((p)[0] = (u8)((v)>>8), (p)[1] = (u8)(v))
u32 pgno_next = 1;
//This variable is for allocating page number.
u32 get4byte(const u8 *p){
  return ((unsigned)p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
}
void put4byte(u8 *p, u32 v){
  p[0] = (u8)(v>>24);
  p[1] = (u8)(v>>16);
  p[2] = (u8)(v>>8);
  p[3] = (u8)v;
}
//Pgno should take appropriate last bit.
//Before construct page
MemPage::MemPage(const u32& root_pgno, const u16& nthChild){
  Data = (u8 *)malloc(PAGE_SIZE);
  //nCell
  nCell = 0;
  //top of record content area
  top = PAGE_SIZE;
  put2byte(&Data[2],top);
  //Parent pgno
  put4byte(&Data[4],root_pgno);
  //nth child
  put2byte(&Data[8], nthChild);

  //Metadata portion, (page elem) 
  nFree = PAGE_SIZE - METADATA_SIZE; 
  pgno = pgno_next++;

  //Matureness
  IsMature = 0;
  my_vtx = NULL;
}
MemPage::MemPage(const int& fd, const u32& Pgno, u8& type){
  //Construct from disk image
  Data = (u8 *)malloc(PAGE_SIZE);
  //pgno
  pgno = Pgno;
  //Get page from ssd
  ReadPage(fd);
  //nCell
  nCell = get2byte(Data);
  //top
  top = get2byte(&Data[2]);
  //nFree
  nFree = top - 2 * nCell - METADATA_SIZE;
  //distinguish Root or child type.
  type = Data[7]&1;
}
MemPage::~MemPage(void){
  //THINK : Should I need to write?
  free(Data);
}
int MemPage::Insert(const u32& Key, const u8 * Value, const u16& size, const my_arg& arg){
  //return value means successness.
  //0 means success.
  //1 means need to check child page(intra frag) or below vertex(inter frag)
  //TODO : Think about the case that mature page has usable space by deleting record.
  //I have no nice idea to deal with that case. So, just leave it now.
  int rc;
  u16 Idx;
  u16 record_size = size + 2 + KEY_SIZE;
  Idx = arg.Idx;

  //The key is not found.
  if(IsMature){
    //Is mature case.
    //should set child number
    return 1;
  } else if(nFree < 2 + record_size){
    //Is overflow case.
    //should set child number
    IsMature = 1;
    return 1;
  } else {
    //Insert the record in this page.
    //No need to set child number
    u16 record_addr = top - record_size;
    //Last bit for deletion mark
    u16 stored = record_addr << 1;
    //Adjust Offset array
    u16 OA_addr = OA_offset + Idx * 2;
    u16 OA_end = OA_offset + nCell * 2;
    memmove(&Data[OA_addr+2],&Data[OA_addr],OA_end - OA_addr);
    put2byte(&Data[OA_addr],stored);
    //Insert record
    put4byte(&Data[record_addr],Key);
    put2byte(&Data[record_addr+KEY_SIZE],size);
    memcpy(&Data[record_addr+KEY_SIZE+2],Value,size);
    //modify metadata portion
    nCell++;
    put2byte(Data,nCell);
    nFree -= (record_size + 2);
    top = record_addr;
    put2byte(&Data[2],top);

    return 0;
  }
}
int MemPage::Delete(const u32& Key, const my_arg& arg){
  //return value means
  //0 means that the record and offset elem is deleted.
  //1 means that the record is deleted only.( for distinguishing inheritance. )
  int rc;
  u16 Idx;
  Idx = arg.Idx;
  //found
  //Offset array
  u16 OA_addr = OA_offset + Idx * 2;
  //record
  u16 record_addr = get2byte(&Data[OA_addr]) >> 1;
  u16 record_size = get2byte(&Data[record_addr+KEY_SIZE]);
  if(IsMature){
    rc = 1;
    //Do not remove the key in the offset array. just mark it as 'delete'.
    //Adjust offset which is changed by compaction.
    record_size += 2;
    u16 new_addr = record_addr + record_size;
    new_addr = new_addr << 1;
    put2byte(&Data[OA_addr],new_addr);
    //Deletion mark.
    Data[OA_addr+1] |= 1;
    //record remains only
    //[ 4 bytes Key, 4 bytes Left child's logical pgno ]
  } else {
    rc = 0;
    //Offset array
    u16 OA_end = OA_offset + nCell * 2;
    //record
    record_size += (KEY_SIZE + 2);
    //Sort Offset array
    //TOREMENBER: One logical page number is removed.
    //            It can affact on size of page table.
    memmove(&Data[OA_addr],&Data[OA_addr+2],OA_end - OA_addr);
    //Modify metadata portion
    nCell--;
    put2byte(Data,nCell);
    //2 bytes from offset array
    nFree += (2);
  }
  //Adjust Offset array
  for(int i = 0; i < nCell; i++){
    u16 addr = OA_offset + i * 2;
    u16 offset = get2byte(&Data[addr]);
    u16 deletion = Data[addr+1] & 1;
    offset = offset >> 1;
    if(offset < record_addr){
      offset = (offset + record_size) << 1;
      offset |= deletion;
      put2byte(&Data[addr], offset );
    }
  }
  //Compaction on record content area
  memmove(&Data[top+record_size],&Data[top],record_addr 
      + IsMature * (KEY_SIZE) - top);
  nFree += (record_size);
  top += record_size;
  put2byte(&Data[2],top);
  return rc;

}

int MemPage::Update(const u32& Key, const u8 * Value, const u16& size, const my_arg& arg){
//return value means success.
//0 means update is completed.
//1 means delete is completed, but insert is not possible.
  int rc;
  u16 Idx;
  Idx = arg.Idx;
    //found
    //Delete first
    u16 old_nFree = nFree;
    //Offset array
    u16 OA_addr = OA_offset + Idx * 2;
    u16 OA_end = OA_offset + nCell * 2;
    //record
    u16 old_record_addr = get2byte(&Data[OA_addr]) >> 1;
    u16 old_value_size = get2byte(&Data[old_record_addr+KEY_SIZE]);
    u16 old_record_size = old_value_size;
    old_record_size += (KEY_SIZE + 2);
    //Adjust Offset array
    for(int i = 0; i < nCell; i++){
      u16 addr = OA_offset + i * 2;
      u16 offset = get2byte(&Data[addr]);
      u16 deletion = Data[addr+1]& 1;
      offset = offset >> 1;
      if(offset < old_record_addr)
        put2byte(&Data[addr], ((offset + old_record_size)<<1|deletion));
    }
    //Compaction on record content area
    memmove(&Data[top+old_record_size],&Data[top],old_record_addr - top);
    nFree += (old_record_size);
    top += old_record_size;
    //Second, insert
    if(old_nFree + old_value_size < size){
      //Cannot insert into this page
      //the insertion will be occurred on child page.
      //Adjust Offset array
      u16 new_addr = top - KEY_SIZE;
      u16 r_addr = new_addr;
      new_addr = new_addr << 1;
      put2byte(&Data[OA_addr],new_addr);
      Data[OA_addr+1] |= 1;
      //Re-insert [Key, Child pgno]
      put4byte(&Data[r_addr],Key);
      nFree -= (KEY_SIZE);
      top = r_addr;
      IsMature = 1;
      put2byte(&Data[2],top);
      rc = 1;
    } else {
      //Can insert into this page
      u16 record_size = size + 2 + KEY_SIZE;
      u16 record_addr = top - record_size;
      //Last bit for deletion mark
      u16 stored = record_addr << 1;
      //Adjust Offset array
      OA_end = OA_offset + nCell * 2;
      //No need to move it.
//      memmove(&Data[OA_addr+2],&Data[OA_addr],OA_end - OA_addr);
      put2byte(&Data[OA_addr],stored);
      //Insert record
      put4byte(&Data[record_addr],Key);
      put2byte(&Data[record_addr+KEY_SIZE],size);
      memcpy(&Data[record_addr+KEY_SIZE+2],Value,size);
      //modify metadata portion
      nFree -= (record_size);
      top = record_addr;
      put2byte(&Data[2],top);
      rc = 0;
    }
    return rc;
}
int MemPage::SearchKey(const u32& Key, my_arg& arg)const{
  //return value means be found or not.
  //0 means be found.
  //1 means NOTFOUND or DELETED.
  u32 backup_UB = arg.UB;
  for(int i = 0; i < nCell; i++){
    u16 cursor = OA_offset + i * 2;
    u16 record_addr = get2byte(&Data[cursor]);
    //Last bit is for deletion mark.
    u16 deletion = record_addr & 1;
    //Remove last bit.
    record_addr = record_addr >> 1;
    u32 c_key = get4byte(&Data[record_addr]);
    arg.UB = c_key;
//    printf("c_key : %lu vs %lu\n",c_key, Key);
    if(c_key == Key){
    //Found case
      arg.Idx = i;
//      if(arg.LB >= arg.UB)
//    printf("FOUND WRONG!\n");
//      printf("(search) %dth : %lu ~ %lu\n",arg.Idx, arg.LB, arg.UB);
      if(deletion){
        //last byte is 1 means deleted.
        //Same as not found.
        return 1;
      } else {
        //last byte is 0 means remains.
        return 0;
      }
    } else if(c_key > Key){
      //Not found case
      arg.Idx = i;
//      if(arg.LB >= arg.UB)
//    printf("NOTFOUND WRONG!\n");
//      printf("(search) %dth : %lu ~ %lu\n",arg.Idx, arg.LB, arg.UB);
      return 1;
    }
    arg.LB = c_key;
  }
  //Not found case
  arg.Idx = nCell;
  arg.UB = backup_UB;
//  if(arg.LB >= arg.UB)
//    printf("WRONG!\n");
//  printf("(search) Last %dth : %lu ~ %lu\n",arg.Idx, arg.LB, arg.UB);
  return 1;
}

u8* MemPage::GetRecord(const u16& Idx)const{
  u16 offset = OA_offset + Idx * 2;
  u16 record_addr = get2byte(&Data[offset])>>1;
  u8* record = &Data[record_addr+4];
  //[2byte size, (size) byte value]
  return record;
}

//int MemPage::RangeSearch(u32& Lower, u32& Upper, u8& direction)
int MemPage::RangeSearch(){
  //TODO: Search and specify how to implement.
  //direction means... 
}

void MemPage::printPage(void){
  printf("pgno : %lu, root pgno : %lu, nCell : %u, nFree : %u, Top : %u, IsMature : %d------------------\n", 
      pgno,get4byte(&Data[4]),
           nCell,nFree,top,IsMature);
  for(int i = 0; i < nCell; i++){
    u16 cursor = OA_offset + i * 2;
    u16 record_addr = get2byte(&Data[cursor]);
    //Last bit is for deletion mark.
    u16 deletion = record_addr & 1;
    //Remove last bit.
    record_addr = record_addr >> 1;
    u32 c_key = get4byte(&Data[record_addr]);
    printf("%d th(%u) : (Key %lu), (loc %u); ",
        i, deletion, get4byte(&Data[record_addr]), 
           record_addr);
  }
  printf("\n");
}
