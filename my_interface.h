#include <iostream>

class KV{
  public:
    KV(){};
    ~KV(){};
    virtual int Insert()=0;

    virtual int Delete()=0;

    virtual int Update()=0;

    virtual int SearchKey()=0;

    virtual int RangeSearch()=0;

    void Scan(void); 

    int LoadFile(char *);
    
  private:
    int my_Write();
    int my_Read();
    int my_Sync();


};

void KV::Scan(void)

int LoadFile(char * filename)

int KV::my_Write()
int KV::my_Read()
int KV::my_Sync()
