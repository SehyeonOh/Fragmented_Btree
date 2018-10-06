#include "my_btree.h"

//For convenience
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef uint32_t Pgno;

//This variable is for allocating page number.
Pgno pgno_counter = 1;

class MemPage{

  /* Page format (10/5)
   *  It is slotted paged structure.
   *
   *  1, Matadata portion, 8 bytes
   *  0 - 1, 2 bytes : number of records
   *  2 - 3, 2 bytes : Start of record content area
   *  4 - 7, 4 bytes : parent page number for recover manager structure
   *
   *  2, Offset Array portion, 2 bytes per record
   *  Odd number th element is child pgno      4 bytes
   *  Even number th element is record offset  4 bytes 
   *  (latter element : front 2 bytes are used for offset.
   *                    last byte is used for distinguishing deleteness.)
   *                    0 means remains. 1 means deleted.
   *  The pattern is page, data, page, data, ... , data, page.
   *                               ( temporary.. child pgno would be logical. There should be page table.)
   *  First and Last is child pgno.
   *  child pgno is invalid until the page is full.
   *
   *  3, Free space between offset array and record content area.
   *  4, Record content area
   *     Record format
   *    [ 4 bytes size, [ 4 bytes Key : Record ] ]
   *
   */

  public:
    MemPage(u32 MyPgno, u32 ParentPgno){
    ~MemPage(void);
    //Insert record(Key, value).
    //return value means successness.
    //0 means success.
    //1 means there is no space to put the record.
    //-1 means the key is already in the page.(Invalid attempt)
    //
    //TODO : Think about the case that mature page has usable space by deleting record.
    //I have no nice idea to deal with that case. So, just leave it now.
    //When some key is deleted, then left child's range absorbs that key.
    //
    int Insert(const u8 * record, const u32 size, Pgno& L_pgno);
    int Delete();
    int Update();

    //Search Key
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
    int SearchKey(const u32 Key, u16& Idx, Pgno& child_pgno, const u8& need);
    int RangeSearch();

    int WritePage(void);
    int ReadPage();

    //For debugging
    void printPage(void);

    u8 IsMature(void){ return IsMature; }

  private:
   /*
   *  1, Matadata portion, 8 bytes
   *  0 - 1, 2 bytes : number of records
   *  2 - 3, 2 bytes : Start of record content area
   *  4 - 7, 4 bytes : parent page number for recover manager structure
   */
  //Page size
  u8* Data;
  Pgno pgno;
  u16 nCell;
  u16 nFree;
  u16 top;
  //Distinguish the page is mature or not.
  //Maturation depends on whether the page can has child page or not.
  //A page can be mature when it no longer take more record.
  //One assumption is needed : All record is always smaller than the page size.
  //0 means not mature, 1 means mature.
  u8 IsMature;
};

