#include "page.h"
class Fragment{
  public:
    Fragment();
    ~Fragment();
    int Insert();
    int Delete();
    int Update();
    int SearchKey();
    int RangeSearch();
  private:
    MemPage* Root;
    //This order is same as offset array in Root page.
    MemPage** Children;
    u32 nChild;
    //The range of this segment is LowerB < x < UpperB
    //UpperB can be incremented whenthe UpperB is inherited from parent page.
    u32 LowerB;
    u32 UpperB;
};


