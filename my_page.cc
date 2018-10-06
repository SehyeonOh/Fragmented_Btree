#include "my_btree.h"
#define OA_offset 8
#define get2byte(x)   ((x)[0]<<8 | (x)[1])
#define put2byte(p,v) ((p)[0] = (u8)((v)>>8), (p)[1] = (u8)(v))
u32 get4byte(const u8 *p){
  return ((unsigned)p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
}
void put4byte(unsigned u8 *p, u32 v){
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

  //Offset array
  //[CHILDPGNO][DATA][CHILDPGNO]
  //4         ,2    ,4
  put4byte(&Data[OA_offset],pgno_counter);
  pgno_counter++;

  //Metadata portion, (page elem) 
  nFree = PAGE_SIZE - 8 - 4; 
  pgno = MyPgno;

  //Matureness
  IsMature = 0;
  
}
MemPage::~MemPage(void){
  //THINK : Should I need to write?
  free(Data);
}
int MemPage::Insert(const u8 * record, const u32 size, Pgno& L_pgno){
  //return value means successness.
  //0 means success.
  //1 means there is no space to put the record.
  //-1 means the key is already in the page.(Invalid attempt)
  //
  //TODO : Think about the case that mature page has usable space by deleting record.
  //I have no nice idea to deal with that case. So, just leave it now.
  //When some key is deleted, then left child's range absorbs that key.
  int rc;
  u32 Key = get4byte(record);
  u16 Idx;
  rc = SearchKey(Key,Idx,L_pgno,1);

  if(rc){
    //The key is not found.
    if(IsMature){
      //Is mature case.
      //should set L_pgno
      return 1;
    } else if(nFree < size + 4 + 8){
      //Is overflow case.
      //should set L_pgno
      IsMature = 1;
      return 1;
    } else {
      //Insert the record in this page.
      //No need to set L_pgno
      u16 record_addr = top - size - 4;
      //Adjust Offset array
      u16 OA_addr = OA_offset + 4 + Idx * 8;
      u16 OA_end = OA_offset + 4 + nCell * 8;
      memmove(&Data[OA_addr+8],&Data[OA_addr],OA_end - OA_addr);
      put2byte(&Data[OA_addr],record_addr);
      Data[OA_addr+3] = 0;
      //Insert record
      memcpy(&Data[record_addr+4],record,size);
      put4byte(&Data[record_addr],size);
      //modify metadata portion
      nCell++;
      put2byte(Data,nCell);
      nFree -= (size + 4 + 8);
      top = record_addr;
      put2byte(&Data[2],top);

      return 0;
    }
  } else {
    //The key is already in the page.
    //Invalid case.
    return -1;
  }
}
int MemPage::Delete(){

}
int MemPage::Update(){

}
int MemPage::SearchKey(const u32 Key, u16& Idx, Pgno& child_pgno, const u8& need){
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
    u16 cursor = OA_offset + 4 + i * 8;
    u16 key_addr = get2byte(&Data[cursor]) + 4;
    u32 c_key = get4byte(&Data[key_addr]);
    if(c_key == Key){
    //Found case
      Idx = i;
      if(need == 1){
        //store left child pgno
        child_pgno = get4byte(&Data[cursor-4]);
      }
      if(Data[cursor+3]){
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
      if(need == 1){
        //child pgno is needed
        child_pgno = get4byte(&Data[cursor-4]);
      }
      return 1;
    }
  }
  //Not found case
  Idx = nCell;
  if(need == 1){
    //child pgno is needed
    child_pgno = get4byte(&Data[OA_offset + nCell * 8]);
  }
  return 1;
}
int MemPage::RangeSearch(){

}

int MemPage::WritePage(){

}
int MemPage::ReadPage(){

}
