#ifndef MY_PAGE_H
#define MY_PAGE_H
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PAGE_SIZE 4096
#define KEY_MAX ULONG_MAX
#define OA_offset 8
#define METADATA_SIZE 8
#define KEY_SIZE 4

//For convenience
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef uint32_t Pgno;


//Argument structure
struct my_arg{
  u16 Idx;
  u32 LB;
  u32 UB;
};


class MemPage{

  /* Page format (10/7)
   *  It is slotted paged structure.
   *
   *  1, Matadata portion, 16 bytes
   *   0 -  1 , 2 bytes : Number of records
   *   2 -  3 , 2 bytes : Start of record content area
   *   4 -  7 , 4 bytes : Parent page number for recover manager structure
   *
   *                               ( temporary.. child pgno would be logical. There should be page table.)
   *  child pgno is invalid until the page is full.
   *
   *  2, Offset Array portion, 2 bytes per record
   *     Last bit means deletion or not.
   *                    0 means remains. 1 means deleted.
   *
   *  3, Free space between offset array and record content area.
   *  4, Record content area
   *     Record format
   *    [ 4 bytes (Key), 
   *      2 bytes{Size), n bytes (Value) ]
   *
   */

  public:
    MemPage(u32 MyPgno, u32 ParentPgno);
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
    int Insert(const u32& Key, const u8 * Value, const u16 size, my_arg& arg);

    //Delete
    //return value means foundness.
    //0 means be found.
    //1 means not found.
    //-1 means not found and the page is not mature. (Invalid delete).
    int Delete(const u32& Key, my_arg& arg);
      
      //Update
      //Delete and Insert
      //return value means state.
      //0 means delete and insert are done.
      //1 means delete is done, insert on child is left.
      //2 means none of them are done.
      //-1 means not found and the page is not mature. (Invalid update).
      //When new record size is lower than nFree + the old one regardless matureness,
      //delete and insert can be done on one page.
    int Update(const u32& Key, const u8 * Value, const u16 size, my_arg& arg);

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
    int SearchKey(const u32& Key, my_arg& arg);
    int RangeSearch();

    int WritePage(int fd){
      return pwrite(fd,Data,PAGE_SIZE,(pgno - 1) * PAGE_SIZE);
    }
    int ReadPage(int fd){
      return pread(fd,Data,PAGE_SIZE,(pgno - 1) * PAGE_SIZE);
    }

    //For debugging
    void printPage(void);

    u8 IsMatured(void){ return IsMature; }
    u16 GetnCell(void){ return nCell;}
    u32 GetPgno(void){ return pgno;}

  private:
  /* Page format (10/7)
   *  It is slotted paged structure.
   *
   *  1, Matadata portion, 16 bytes
   *   0 -  1 , 2 bytes : Number of records
   *   2 -  3 , 2 bytes : Start of record content area
   *   4 -  7 , 4 bytes : Parent page number for recover manager structure
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
  //Addable?, Appendable?, immutable?... Nice wording is needed.
  u8 IsMature;
};

#endif
