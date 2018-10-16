#ifndef MY_MANAGER_H
#define MY_MANAGER_H
#include "my_fragment.h"
//Logically N-ary tree
class Vertex{
  public:
Vertex(MemPage * your_root,const u32& LB,const u32& UB);
    ~Vertex(void);
    Vertex* GetChild(void)const{ return child; }
    void SetChild(Vertex* vtx){ child = vtx; }
    Vertex* GetSibling(void)const{ return sibling; }
    void SetSibling(Vertex* vtx){ sibling = vtx; }
  Fragment* Frag;
  private:
  //child is vertex which is lower hierarchy
  Vertex * child;
  //sibling is vertex which is same hierarchy(Linked list)
  Vertex * sibling;
};

//Manager structure
class Manager{
  public:
    Manager(int fd);
    ~Manager(void);

    //Insert
    //return value means
    //0 means success
    //-1 means invalid attempt.(redundant key)
    int Insert(const u32& Key, const u8* Value, const u16& size);

    //Delete
    //return value means
    //0 means success
    //-1 means invalid attempt.(NotFound)
    int Delete(const u32& Key);

    //Update
    //return value means
    //0 means success
    //-1 means invalid attempt.(NotFound)
    int Update(const u32& Key, const u8* Value, const u16& size);

    //InheritAndInsert
    int InheritAndInsert(Vertex* vtx,const u32& Key, const u8* Value, const u16& size, my_arg& arg);
    //Search
    //return values means
    //NULL means NotFound
    //Otherwise, record address is returned.
    //size stores the size of record.
    u8* Search(const u32& Key, u16& size)const;

    int RangeSearch();   

    Vertex* FindVertex(const u32& Key)const;
    Vertex* InheritKey(const u32& Key,Vertex* From);

    void BeginTxn(void){
      begin_flag = 1;
    }

    void CommitTxn(void){
      //Write all dirty pages
      for (std::set<MemPage*>::iterator it=write_set.begin(); it!=write_set.end(); ++it){
        MemPage* page = *it;
        page->WritePage(fd);
      }
      //Clear the dirty page lists
      write_set.clear();
      //Force pages to be flushed to SSD.
      fsync(fd);
      begin_flag = 0;
    }

    //debug
    void printManager(void)const;
    void Visualize(void)const;
  private:
    void SeparateBtree();   
    //Starting point of manager structure
    Vertex* Top;
    //current pointing vertex
//    struct Vertex* cursor;
    //write_set
    //for taking dirty pages until commit.
    std::set<MemPage*> write_set;
    //begin_flag is used for distinguishing
    //begin ... txn ... commit and single statement.
    bool begin_flag;
    int fd;

};
#endif
