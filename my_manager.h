#ifndef MY_MANAGER_H
#define MY_MANAGER_H
#include "my_fragment.h"
//Logically N-ary tree
class Vertex{
  public:
    Vertex();
    ~Vertex();
    Fragment* GetFrag(void){ return Frag; }
    Vertex* GetChild(void){ return child; }
    void SetChild(Vertex* vtx){ child = vtx; }
    Vertex* GetSibling(void){ return sibling; }
    void SetSibling(Vertex* vtx){ sibling = vtx; }
  private:
  Fragment* Frag;
  //child is vertex which is lower hierarchy
  Vertex * child;
  //sibling is vertex which is same hierarchy(Linked list)
  Vertex * sibling;
};

//Manager structure
class Manager{
  public:
    Manager();
    ~Manager();
    int Insert();
    int Delete();
    int Update();
    int SearchKey();
    int RangeSearch();   
    Vertex* FindVertex(const u32& Key);
    Vertex* InheritKey(const u32& Key,Vertex* From);
  private:
    void SeparateBtree();   
    //Starting point of manager structure
    Vertex* Top;
    //current pointing vertex
//    struct Vertex* cursor;

};
#endif
