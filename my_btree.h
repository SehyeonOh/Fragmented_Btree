#include <iostream>
//#include "my_interface.h"
#include "my_page.h"
//#include "my_manager.h"
#include "my_fragment.h"
#define HEIGHT_MAX 2
class Segment;
class Manager;
class Btree;

//Logically N-ary tree
struct Vertex{
  class Segment Seg;
  //child is vertex which is lower hierarchy
  struct Vertex * child;
  //sibling is vertex which is same hierarchy(Linked list)
  struct Vertex * sibling;
};

//Segmented B-tree class
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


