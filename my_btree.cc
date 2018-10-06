#include "my_btree.h"
#include "my_page.h"
#define HEIGHT_MAX 2
class Segment;
class Manager;
class Btree;

//Segmented B-tree class
class Segment{
  public:
    Segment();
    ~Segment();
    int Insert();
    int Delete();
    int Update();
    int SearchKey();
    int RangeSearch();
  private:
    struct MemPage* Root;
    //The range of this segment is LowerB < x < UpperB
    u32 LowerB;
    u32 UpperB;
};
//Logically N-ary tree
struct Vertex{
  class Segment Seg;
  //child is vertex which is lower hierarchy
  struct Vertex * child;
  //sibling is vertex which is same hierarchy(Linked list)
  struct Vertex * sibling;
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
    class Segment* FindSegment();
  private:
    void SeparateBtree();   
    //Starting point of manager structure
    struct Vertex* Top;
    //current pointing vertex
    struct Vertex* cursor;

};

class Btree: class KV {
  public:
    Btree();
    ~Btree();

    virtual int Insert();

    virtual int Delete();

    virtual int Update();

    virtual int SearchKey();

    virtual int RangeSearch();

  private:
    class Manager boss;
};


virtual int Btree::Insert(){

}

virtual int Btree::Delete(){

}

virtual int Btree::Update(){

}

virtual int Btree::SearchKey(){

}

virtual int Btree::RangeSearch(){

}
