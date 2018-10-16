#include "my_page.h"
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
    //Root page constructor
    //new page allocation
MemPage::MemPage(const u32& parent_root_pgno, const u32& LB, const u32& UB){
  Data = (u8 *)malloc(PAGE_SIZE);
  //nCell
  nCell = 0;
  //top of record content area
  top = PAGE_SIZE;
  put2byte(&Data[2],top);
  //nFree
  nFree = PAGE_SIZE - ROOT_METADATA_SIZE; 
  put2byte(&Data[4],nFree);
  //Parent pgno
  put4byte(&Data[6],parent_root_pgno);
  //Lower bound and Upper bound
  put4byte(&Data[10], LB);
  put4byte(&Data[14], UB);

  pgno = pgno_next++;

  my_vtx = NULL;
}


    //Pgno should take appropriate last bit.
    //Child page constructor 
    //new page allocation
MemPage::MemPage(const u32& my_root_pgno, const u16& nthChild){
  Data = (u8 *)malloc(PAGE_SIZE);
  //nCell
  nCell = 0;
  //top of record content area
  top = PAGE_SIZE;
  put2byte(&Data[2],top);
  //nFree
  nFree = PAGE_SIZE - METADATA_SIZE; 
  put2byte(&Data[4],nFree);
  //Parent pgno
  put4byte(&Data[6],my_root_pgno);
  //nth child
  put2byte(&Data[10], nthChild);

  pgno = pgno_next++;

  my_vtx = NULL;
}
MemPage::MemPage(const int& fd, load_arg& arg, bool& type){
  //Construct from disk image
  Data = (u8 *)malloc(PAGE_SIZE);
  //pgno
  pgno = arg.root.Pgno;
  //Get page from ssd
  ReadPage(fd);
  //distinguish Root or child type.
  type = Data[9]&1;
  //nCell
  nCell = get2byte(Data);
  //top
  top = get2byte(&Data[2]);
  //nFree
  nFree = get2byte(&Data[4]);
  if(type){
    // child page
    arg.child.Pgno = get4byte(&Data[6]);
    arg.child.nthchild = get2byte(&Data[10]);
  } else {
    // root page
    arg.root.Pgno = get4byte(&Data[6]);
    arg.root.LB = get4byte(&Data[10]);
    arg.root.UB = get4byte(&Data[14]);
  }
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
  u16 my_OA_offset = get_OA_offset(Data);

  //The key is not found.
  if(nFree < 2 + record_size){
    //Is overflow case.
    //should set child number
    return 1;
  } else {
    //Insert the record in this page.
    //No need to set child number
    u16 record_addr = top - record_size;
    //Last bit for deletion mark
    u16 stored = record_addr << 1;
    //Adjust Offset array
    u16 OA_addr = my_OA_offset + Idx * 2;
    u16 OA_end = my_OA_offset + nCell * 2;
    memmove(&Data[OA_addr+2],&Data[OA_addr],OA_end - OA_addr);
    put2byte(&Data[OA_addr],stored);
    //Insert record
    put4byte(&Data[record_addr],Key);
    put2byte(&Data[record_addr+KEY_SIZE],size);
    memcpy(&Data[record_addr+KEY_SIZE+2],Value,size);
    //modify metadata portion
    nCell++;
    put2byte(Data,nCell);
    top = record_addr;
    put2byte(&Data[2],top);
    nFree -= (record_size + 2);
    put2byte(&Data[4],nFree);
    return 0;
  }
}
int MemPage::DeleteMark(const u32& Key, const my_arg& arg){
  //return value means
  //0 means correctly done. 
  int rc;
  u16 Idx;
  Idx = arg.Idx;
  u16 my_OA_offset = ROOT_OA_offset;
  //found
  //Offset array
  u16 OA_addr = my_OA_offset + Idx * 2;
  //record
  u16 record_addr = get2byte(&Data[OA_addr]) >> 1;
  u16 record_size = get2byte(&Data[record_addr+KEY_SIZE]) + 2;
  //Do not remove the key in the offset array. just mark it as 'delete'.
  //Deletion mark.
  Data[OA_addr+1] |= 1;
  //No compaction. So top is not modified.
  nFree += (record_size);
  put2byte(&Data[4],nFree);
  return 0;
}
int MemPage::Delete(const u32& Key, const my_arg& arg){
  //return value means
  //0 means that the record and offset elem is deleted.
  int rc;
  u16 Idx;
  Idx = arg.Idx;
  u16 my_OA_offset = get_OA_offset(Data);
  //found
  //Offset array
  u16 OA_addr = my_OA_offset + Idx * 2;
  //record
  u16 record_addr = get2byte(&Data[OA_addr]) >> 1;
  u16 record_size = get2byte(&Data[record_addr+KEY_SIZE]);
  rc = 0;
  //Offset array
  u16 OA_end = my_OA_offset + nCell * 2;
  //record
  record_size += (KEY_SIZE + 2);
  //Sort Offset array
  memmove(&Data[OA_addr],&Data[OA_addr+2],OA_end - OA_addr);
  //Modify metadata portion
  nCell--;
  put2byte(Data,nCell);
  //2 bytes from offset array
  nFree += (2);
  nFree += (record_size);
  put2byte(&Data[4],nFree);
  return 0;
}

int MemPage::Update(const u32& Key, const u8 * Value, const u16& size, const my_arg& arg){
//return value means success.
//0 means update is completed.
//1 means delete is completed, but insert is not possible.(Inheritance)
  int rc;
  u16 Idx;
  Idx = arg.Idx;
  bool type = Data[9]&1;
  //1 is child, 0 is root.
  u16 my_OA_offset = get_OA_offset(Data);
  //found
  //Delete first
  //Contiguous free region size
  if(nFree >= size + 2 + 2 + KEY_SIZE){
    //Delete
    Delete(Key,arg);
    u16 freeSpace = top - my_OA_offset - 2 * nCell;
    if(freeSpace < size + 2 + 2 + KEY_SIZE){
      //Compaction is needed
      Compaction();
    }
    //Insert
    Insert(Key,Value,size,arg);
    return 0;
  }
  //Cannot insert new data.
  //The inheritance is needed.
  //delete and inheritance
  if(!type){
    //Root page
    DeleteMark(Key,arg);
  } else {
    //Child page
    Delete(Key,arg);
  }
  //Key inheritance
  //go to child
  return 1;
}
int MemPage::SearchKey(const u32& Key, my_arg& arg)const{
  //return value means be found or not.
  //0 means be found.
  //1 means NOTFOUND or DELETED.
  u16 my_OA_offset = get_OA_offset(Data);
  u32 backup_UB = arg.UB;
  for(int i = 0; i < nCell; i++){
    u16 cursor = my_OA_offset + i * 2;
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

void MemPage::Compaction(void){
  u8* new_Data = (u8*)malloc(PAGE_SIZE);
  u16 my_OA_offset = get_OA_offset(Data);
  u16 OA_cur;
  u16 record_cur = PAGE_SIZE;
  for(u16 i = 0; i < nCell; i++){
    OA_cur = my_OA_offset + i * 2;
    u16 record_addr = get2byte(&Data[OA_cur]);
    bool deletion = record_addr & 1;
    record_addr = record_addr >> 1;
    u16 record_size = KEY_SIZE;
    if(!deletion)
      record_size += 2 + get2byte(&Data[record_addr+4]);
    record_cur = record_cur - record_size;
    //put record
    memcpy(&new_Data[record_cur],&Data[record_addr],record_size);
    //put Offset array
    put2byte(&new_Data[OA_cur], (record_cur << 1) | (u16)deletion);
  }
  //copy metadata slot
  memcpy(new_Data,Data,my_OA_offset);
  //modify top. only top is modified.
  top = record_cur;
  put2byte(&new_Data[2],top);
  if(top != nFree){
    printf("WRONG!!\n");
    exit(0);
  }
  free(Data);
  //Swap Data to new_Data
  Data = new_Data;
}

u8* MemPage::GetRecord(const u16& Idx)const{
  u16 my_OA_offset = get_OA_offset(Data);
  u16 offset = my_OA_offset + Idx * 2;
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
  u16 my_OA_offset = get_OA_offset(Data);
  printf("pgno : %lu, root pgno : %lu, nCell : %u, nFree : %u, Top : %u------------------\n", 
      pgno,get4byte(&Data[4]),
           nCell,nFree,top);
  for(int i = 0; i < nCell; i++){
    u16 cursor = my_OA_offset + i * 2;
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
