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
#define ROOT_OA_offset 18
#define ROOT_METADATA_SIZE 18
#define OA_offset 12
#define METADATA_SIZE 12
#define KEY_SIZE 4
//For convenience
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#define get2byte(x)   ((x)[0]<<8 | (x)[1])
#define put2byte(p,v) ((p)[0] = (u8)((v)>>8), (p)[1] = (u8)(v))
#define get_OA_offset(Data) ((Data[9]&1)? OA_offset : ROOT_OA_offset)
u32 get4byte(const u8*p);
void put4byte(u8*p, u32 v);

class Vertex;

class MemPage;

//Argument structure
struct my_arg{
  u16 Idx;
  u32 LB;
  u32 UB;
  u32 Pgno;
  std::set<MemPage*>* write_set;
};

union load_arg{
  struct{
    u32 Pgno;
    u32 LB;
    u32 UB;
  } root;
  struct{
    u32 Pgno;
    u16 nthchild;
  } child;
};


class MemPage{

  /* Page format (10/16)
   *  It is slotted paged structure.
   *  Root page of each fragement
   *  1, Matadata portion, 18 bytes
   *   0 -  1 , 2 bytes : Number of records(nCell)
   *   2 -  3 , 2 bytes : Start of record content area(top)
   *   4 -  5 , 2 bytes : bytes of actually usable space.(nFree)
   *   6 -  9 , 4 bytes : (Root page) : store parent fragment's root pgno
   *                      (Child page): store Own fragment's root pgno
   *                      Last bit : 0 means root page. 1 means child page.
   *  10 - 13 , 4 bytes : Lower bound of the fragment
   *  14 - 17 , 4 bytes : Upper bound of the fragment
   *
   *  Child page of each fragement
   *  1, Matadata portion, 12 bytes
   *   0 -  1 , 2 bytes : Number of records(nCell)
   *   2 -  3 , 2 bytes : Start of record content area(top)
   *   4 -  5 , 2 bytes : bytes of actually usable space.(nFree)
   *   6 -  9 , 4 bytes : (Root page) : store parent fragment's root pgno
   *                      (Child page): store Own fragment's root pgno
   *                      Last bit : 0 means root page. 1 means child page.
   *  10 - 11 , 2 bytes : Position of the page in the fragment (nth child)
   *
   *
   *  2, Offset Array portion, 2 bytes per record
   *     Last bit means deletion or not.
   *                    0 means remains. 1 means deleted.
   *     This deletion mark is used only on Root page.
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
    //Root page constructor
    //new page allocation
    MemPage(const u32& parent_root_pgno, const u32& LB, const u32& UB);
    //Pgno should take appropriate last bit.
    //Child page constructor 
    //new page allocation
    MemPage(const u32& my_root_pgno, const u16& nthChild);
    //Page loading constructor
    //Load page from file of file descriptor.
    MemPage(const int& fd, load_arg& arg, bool& type);

    ~MemPage(void);

    //InsertRoot
    //return value means successness.
    //0 means success.
    //1 means need to check child page(intra frag) or below vertex(inter frag)
    //TODO : Think about the case that mature page has usable space by deleting record.
    //I have no nice idea to deal with that case. So, just leave it now.
    //When some key is deleted, then left child's range absorbs that key.
    //
    int Insert(const u32& Key, const u8 * Value, const u16& size, const my_arg& arg);

    //DeleteMark
    //return value means
    //0 means that the record is deleted.
    int DeleteMark(const u32& Key, const my_arg& arg);
    //Delete
    //return value means
    //0 means that the record and offset elem are deleted.
    int Delete(const u32& Key, const my_arg& arg);
      
    //Update
    //return value means success.
    //0 means update is completed.
    //1 means delete is completed, but insert is not possible.(Inheritance)
    int Update(const u32& Key, const u8 * Value, const u16& size, const my_arg& arg);

    //SearchKey
    //return value means be found or not.
    //0 means be found.
    //1 means NOTFOUND or DELETED.
    int SearchKey(const u32& Key, my_arg& arg) const;

    void Compaction(void);

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

    u16 GetnCell(void){ return nCell;}
    u32 GetPgno(void){ return pgno;}

    //For convinient recovery,
    void SetShortCut(Vertex* shortcut){
      my_vtx = shortcut;
    }
    Vertex* GetVertex(void){
      return my_vtx;
    }

  private:
  /* Page format (10/15)
   *  It is slotted paged structure.
   *  Root page of each fragement
   *  1, Matadata portion, 18 bytes
   *   0 -  1 , 2 bytes : Number of records(nCell)
   *   2 -  3 , 2 bytes : Start of record content area(top)
   *   4 -  5 , 2 bytes : bytes of actually usable space.(nFree)
   *   6 -  9 , 4 bytes : (Root page) : store parent fragment's root pgno
   *                      (Child page): store Own fragment's root pgno
   *                      Last bit : 0 means root page. 1 means child page.
   *  10 - 13 , 4 bytes : Lower bound of the fragment
   *  14 - 17 , 4 bytes : Upper bound of the fragment
   *
   *  Child page of each fragement
   *  1, Matadata portion, 12 bytes
   *   0 -  1 , 2 bytes : Number of records(nCell)
   *   2 -  3 , 2 bytes : Start of record content area(top)
   *   4 -  5 , 2 bytes : bytes of actually usable space.(nFree)
   *   6 -  9 , 4 bytes : (Root page) : store parent fragment's root pgno
   *                      (Child page): store Own fragment's root pgno
   *                      Last bit : 0 means root page. 1 means child page.
   *  10 - 11 , 2 bytes : Position of the page in the fragment (nth child)
   */
  //Page size
  u8* Data;
  u32 pgno;
  u16 nCell;
  u16 nFree;
  u16 top;

  //For efficient recovery
  //This is only used for recovery. and root of fragment only has this.
  Vertex* my_vtx;
};

#endif
