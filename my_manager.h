#include "my_btree.h"

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
