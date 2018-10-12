#ifndef MY_FRAGMENT_H
#define MY_FRAGMENT_H
#include "my_page.h"
#include <limits.h>

class Fragment{
  public:
    Fragment(MemPage* your_root,const u32& LB,const u32& UB);
    ~Fragment(void);

    //Insert
    //return value means
    //0 means success.
    //1 means need to move to below vertex.
    //-1 means invalid attempt.(redundent key)
    int Insert(const u32& Key, const u8* Value, const u16& size, my_arg& arg);

    //Delete
    //return values means
    //0 means success
    //1 means the key will be inherited to below vertex.
    //-1 means Invalid attempt(Not found) 
    int Delete(const u32& Key, my_arg& arg);

    //Update
    //return value means
    //0 means update is done.
    //1 means delete is done, insert on below vertex is left.(Key is inherited.)
    //-1 means Invalid (NOTFOUND).
    int Update(const u32& Key, const u8* Value, const u16& size, my_arg& arg);

    //Search
    //return values means
    //NULL means NotFound
    //Otherwise, record address is returned.
    u8* Search(const u32& Key, my_arg& arg)const;
    int RangeSearch();

    int RangeCheck(const u32& Key)const{
      return (LowerB < Key && Key < UpperB);
    }

    void InheritKey(void){
      UpperB++;
    }

    u32 ChildPgno(const u16& i)const{
      return Children[i]->GetPgno();
    }
    void IncrementUB(void){
      UpperB++;
    }
    u32 GetLowerB(void)const{
      return LowerB;
    }
    u32 GetUpperB(void)const{
      return UpperB;
    }
    //for debugging
    u32 GetRootPgno(void) const{
      return Root->GetPgno();
    }


    //for debugging
    void printFragment(void)const;
  private:
    MemPage* Root;
    //This order is same as offset array in Root page.
    MemPage** Children;
    u32 nChild;
    //The range of this segment is LowerB < x < UpperB
    //UpperB can be incremented whenthe UpperB is inherited from parent page.
    u32 LowerB;
    u32 UpperB;

    Vertex* parent;
};

#endif
