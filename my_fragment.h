#ifndef MY_FRAGMENT_H
#define MY_FRAGMENT_H
#include "my_page.h"
#include <limits.h>

class Fragment{
  public:
    Fragment(MemPage* your_root, u32 LB, u32 UB);
    ~Fragment(void);

    //Insert
    //return value means
    //0 means success.
    //1 means need to move to below vertex.
    //-1 means invalid attempt.(Over the range.)
    int Insert(const u32& Key, const u8* Value, const u16 size, my_arg& arg);

    //Delete
    //return values means
    //0 means success
    //1 means the key was inherited to below vertex.
    //-1 means not found( Invalid attempt ) 
    int Delete(const u32& Key);

    //Update
    //return value means
    //0 means delete and insert are done.
    //1 means delete is done, insert on below vertex is left.(Key is inherited.)
    //2 means none of them is done, the key was inherited.
    //-1 means Invalid update. (NOTFOUND).
    int Update(const u32& Key, const u8* Value, const u16 size);

    int SearchKey();
    int RangeSearch();


    //for debugging
    void printFragment(void);
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

#endif
