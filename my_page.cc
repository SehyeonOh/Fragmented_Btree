#include "my_page.h"
#define get2byte(x)   ((x)[0]<<8 | (x)[1])
#define put2byte(p,v) ((p)[0] = (u8)((v)>>8), (p)[1] = (u8)(v))
//This variable is for allocating page number.
Pgno pgno_counter = 1;
u32 get4byte(const u8 *p){
  return ((unsigned)p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
}
void put4byte(u8 *p, u32 v){
  p[0] = (u8)(v>>24);
  p[1] = (u8)(v>>16);
  p[2] = (u8)(v>>8);
  p[3] = (u8)v;
}

MemPage::MemPage(u32 MyPgno, u32 ParentPgno){

  Data = (u8 *)malloc(PAGE_SIZE);
  //nCell
  nCell = 0;
  //top of record content area
  top = PAGE_SIZE;
  put2byte(&Data[2],top);
  //Parent pgno
  put4byte(&Data[4],ParentPgno);

  //Metadata portion, (page elem) 
  nFree = PAGE_SIZE - METADATA_SIZE; 
  pgno = MyPgno;

  //Matureness
  IsMature = 0;
  
}
MemPage::~MemPage(void){
  //THINK : Should I need to write?
  free(Data);
}
int MemPage::Insert(const u32& Key, const u8 * Value, const u16 size, u16& child){
  //return value means successness.
  //0 means success.
  //1 means there is no space to put the record.
  //-1 means the key is already in the page.(Invalid attempt)
  //
  //TODO : Think about the case that mature page has usable space by deleting record.
  //I have no nice idea to deal with that case. So, just leave it now.
  //When some key is deleted, then left child's range absorbs that key.
  int rc;
  u16 Idx;
  u16 record_size = size + 2 + KEY_SIZE;
  rc = SearchKey(Key,Idx);
  child = Idx;

  if(rc){
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
  } else {
    //The key is already in the page.
    if(IsMature){
      u16 OA_addr = OA_offset + Idx * 2;
      u16 deletion = Data[OA_addr+1]&1;
      if(deletion){
        //the key is inherited to child.
        //Should set child number
        return 1;
      } else {
        //Invalid case
        return -1;
      }
      
    } else {
      //Invalid case.
      return -1;
    }
  }
}
int MemPage::Delete(const u32& Key, u16& child){
  //return value means foundness.
  //0 means be found.
  //1 means not found.
  //-1 means not found and the page is not mature. (Invalid delete).
  int rc;
  u16 Idx;
  rc = SearchKey(Key,Idx);
  child = Idx;
  if(rc){
    //Not found
    if(IsMature){
      //The record can be in child page.
      return 1;
    } else {
      //The record with given key is not in this whole structure.
      //Invalid attempt
      return -1;
    }
  } else {
    //found
    //Offset array
    u16 OA_addr = OA_offset + Idx * 2;
    //record
    u16 record_addr = get2byte(&Data[OA_addr]) >> 1;
    u16 record_size = get2byte(&Data[record_addr+KEY_SIZE]);
    if(IsMature){
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
      u16 offset = get2byte(&Data[addr]) >> 1;
      if(offset < record_addr){
        offset = (offset + record_size) << 1;
        put2byte(&Data[addr], offset );
      }
    }
    //Compaction on record content area
    memmove(&Data[top+record_size],&Data[top],record_addr 
                                             + IsMature * (KEY_SIZE) - top);
    nFree += (record_size);
    top += record_size;
    put2byte(&Data[2],top);
    return 0;
  }
}

int MemPage::Update(const u32& Key, const u8 * Value, const u16 size, u16& child){
//Delete and Insert
//return value means state.
//0 means delete and insert are done.
//1 means delete is done, insert on child is left.
//2 means none of them are done.
//-1 means not found and the page is not mature. (Invalid update).
//When new record size is lower than nFree + the old one regardless matureness,
//delete and insert can be done on one page.
  int rc;
  u16 Idx;
  rc = SearchKey(Key,Idx);
  child = Idx;
  if(rc){
    //Not found
    if(IsMature){
      //The record can be in child page.
      return 2;
    } else {
      //The record with given key is not in this whole structure.
      //Invalid attempt
      return -1;
    }
  } else {
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
    if(IsMature){
      //Deletion mark.
      u16 deletion = Data[OA_addr+1]&1;
      if(deletion){
        //The record can be in child page.
        return 2;
      }
      //Sorting Offset array is delayed
//      old_record_size += 2;
      old_record_size += (KEY_SIZE + 2);
      //record remains only
      //[ 4 bytes Key, 4 bytes Left child's logical pgno ]
    } else {
      //record
      old_record_size += (KEY_SIZE + 2);
      //Sorting Offset array is delayed
      //for avoiding calling memmove twice.
      //Modify metadata portion
      //2 bytes from offset array
    }
    //Adjust Offset array
    for(int i = 0; i < nCell; i++){
      u16 addr = OA_offset + i * 2;
      u16 offset = get2byte(&Data[addr]) >> 1;
      if(offset < old_record_addr)
        put2byte(&Data[addr], (offset + old_record_size)<<1);
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
      if(IsMature){
        u16 new_addr = top - KEY_SIZE;
        u16 r_addr = new_addr;
        new_addr = new_addr << 1;
        put2byte(&Data[OA_addr],new_addr);
        Data[OA_addr+1] |= 1;
        //Re-insert [Key, Child pgno]
        put4byte(&Data[r_addr],Key);
        nFree -= (KEY_SIZE);
        top = r_addr;
      } else {
        memmove(&Data[OA_addr],&Data[OA_addr+2],OA_end - (OA_addr + 2));

        nCell--;
        nFree += 2;
        IsMature = 1;
        put2byte(Data,nCell);
      }
      put2byte(&Data[2],top);
      return 1;
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
      
      return 0;
    }
  }

}
int MemPage::SearchKey(const u32& Key, u16& Idx){
  //return value means be found or not.
  //0 means be found.
  //1 means not found.
  //need value means needness of pgno of child page.
  //When the key is found,
  //0 means no need.
  //1 means left child is needed.
  //When the key is not found and the page is mature,
  //0 means no need.
  //1 means need the child.
  //When the key is not found but the page is not mature,
  //child_pgno is not used
  for(int i = 0; i < nCell; i++){
    u16 cursor = OA_offset + i * 2;
    u16 record_addr = get2byte(&Data[cursor]);
    //Last bit is for deletion mark.
    u16 deletion = record_addr & 1;
    //Remove last bit.
    record_addr = record_addr >> 1;
    u32 c_key = get4byte(&Data[record_addr]);
    if(c_key == Key){
    //Found case
      Idx = i;
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
      Idx = i;
      return 1;
    }
  }
  //Not found case
  Idx = nCell;
  return 1;
}
//int MemPage::RangeSearch(u32& Lower, u32& Upper, u8& direction)
int MemPage::RangeSearch(){
  //TODO: Search and specify how to implement.
  //direction means... 
}

void MemPage::printPage(void){
  printf("pgno : %d, parent pgno : %d, nCell : %d, nFree : %d, Top : %d, IsMature : %d------------------\n", 
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
    printf("%d th(%d) : (Key %lu), (loc %u); ",
        i, deletion, get4byte(&Data[record_addr]), 
           record_addr);
  }
  printf("\n");
}
