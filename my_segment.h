#include "my_btree.h"

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


