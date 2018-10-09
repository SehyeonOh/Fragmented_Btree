//#include "my_interface.h"
#include "my_manager.h"
#define HEIGHT_MAX 2
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


