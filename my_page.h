#ifndef MY_PAGE_H
#define MY_PAGE_H
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <set>
#define PAGE_SIZE 4096
#define KEY_MAX (ULONG_MAX-1)
#define OA_offset 10
#define METADATA_SIZE 10
#define KEY_SIZE 4

//For convenience
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

class MemPage;

//Argument structure
struct my_arg{
  u16 Idx;
  u32 LB;
  u32 UB;
  u32 Pgno;
  std::set<MemPage*>* write_set;
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
    //Pgno should take appropriate last bit.
    //Before construct page
    MemPage(const u32& root_pgno, const u16& nthChild);
    //Construct from disk image
    MemPage(const int& fd, const u32& Pgno, u8& type);
    ~MemPage(void);

    //return value means successness.
    //0 means success.
    //1 means need to check child page(intra frag) or below vertex(inter frag)
    //TODO : Think about the case that mature page has usable space by deleting record.
    //I have no nice idea to deal with that case. So, just leave it now.
    //When some key is deleted, then left child's range absorbs that key.
    //
    int Insert(const u32& Key, const u8 * Value, const u16& size, const my_arg& arg);

    //Delete
    //return value means
    //0 means that the record and offset elem is deleted.
    //1 means that the record is deleted only.( for distinguishing inheritance. )
    int Delete(const u32& Key, const my_arg& arg);
      
    //Update
    //return value means success.
    //0 means update is completed.
    //1 means delete is completed, but insert is not possible.
    int Update(const u32& Key, const u8 * Value, const u16& size, const my_arg& arg);

    //SearchKey
    //return value means be found or not.
    //0 means be found.
    //1 means NOTFOUND or DELETED.
    int SearchKey(const u32& Key, my_arg& arg) const;

    u8* GetRecord(const u16& Idx)const;
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

    //For convinient recovery,
    void SetShortCut(Vertex* shortcut){
      my_vtx = shortcut;
    }
    Vertex* GetVertex(void){
      return my_vtx;
    }
    u32 GetRootPgno(void){
      return get4byte(&Data[4])>>1;
    }
    u16 GetnThChild(void){
      return get2byte(&Data[8]);
    }

  private:
  /* Page format (10/11)
   *  It is slotted paged structure.
   *
   *  1, Matadata portion, 10 bytes
   *   0 -  1 , 2 bytes : Number of records
   *   2 -  3 , 2 bytes : Start of record content area
   *   4 -  7 , 4 bytes : (Root page) : store parent fragment's root pgno
   *                      (Child page): store Own fragment's root pgno
   *                      Last bit : 0 means root page. 1 means child page.
   *   8 -  9 , 2 bytes : Position of the page in the fragment (nth child)
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

  //For efficient recovery
  //This is only used for recovery. and root of fragment only has this.
  Vertex* my_vtx;
};

#endif
