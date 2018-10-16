#include "my_fragment.h"


Fragment::Fragment(MemPage* your_root,const u32& LB,const u32& UB){
  Root = your_root;
  LowerB = LB;
  UpperB = UB;
  nChild = 0;
  Children = NULL;
}
Fragment::~Fragment(void){
  //delete all page
  for(int i = 0; i < nChild; i++){
    if(!Children[i]){
      delete(Children[i]);
    }
  }
  delete(Children);
  delete(Root);
}
int Fragment::Insert(const u32& Key, const u8* Value, const u16& size, my_arg& arg){
  //Find target page and insert it.
  //return value means
  //0 means success.
  //1 means need to move to below vertex.
  //-1 means invalid attempt.(redundent key)
  //TODO: Logical pgno may be useless. There is no need of page table.
  //Fragment class can the role of page table.
//  printf("(INSERT) LB : %lu UB : %lu\n",LowerB, UpperB);
  u16 child;
  int rc;
  //First search the Root.
  rc = Root->SearchKey(Key,arg);
  child = arg.Idx;
  if(!rc){
    //the key is already in the page.
    //Invalid(redundent key)
    return -1;
  } 
  if(!Children){
    //Matureness is replaced existence of Children pointer.
    //If Children is not initialized,
    //the key is not found.
    rc = Root->Insert(Key,Value,size, arg);
    if(!rc){
      //Insertion is successfully done.
      arg.write_set->insert(Root);
      return 0;
    } 
    //Initialize Children first.
    nChild = Root->GetnCell()+1;
    Children = new MemPage*[nChild];
    for(int i = 0; i < nChild; i++){
      Children[i] = NULL;
    }
  }
  //go to child page
  //  printf("(frag) goto child page\n");
  //Check existance of the target child
  if(!Children[child]){
    //If the target child is not created yet,
    //Create it and insert.
    Children[child] = new MemPage(Root->GetPgno(),child);
    arg.Idx = 0;
    rc = Children[child]->Insert(Key,Value,size,arg);
    arg.write_set->insert(Children[child]);
    //return 0;
    return 0;
  } 
  //If the target child is already created,
  //Check it
  rc = Children[child]->SearchKey(Key,arg);
  if(!rc){
    //the key is already in the page.
    //Invalid(redundent key)
    return -1;
  } 
  //the key is not in the page.
  arg.Pgno = Children[child]->GetPgno();
  rc = Children[child]->Insert(Key,Value,size,arg);
  if(rc){
    //return 1
    //1 means overflow. So insert into below vertex.
    return 1;
  }
  //return 0 
  arg.write_set->insert(Children[child]);
  return 0;
}
int Fragment::Delete(const u32& Key, my_arg& arg){
  //return values means
  //0 means success
  //1 means the key will be inherited to below vertex.
  //-1 means Invalid attempt(Not found) 
//  printf("(DELETE) LB : %lu UB : %lu\n",LowerB, UpperB);
  u16 child;
  int rc;
  rc = Root->SearchKey(Key,arg);
  child = arg.Idx;
  if(!rc){
    //found
    if(Children){
      Root->DeleteMark(Key,arg);
    } else {
      Root->Delete(Key,arg);
    }
    arg.write_set->insert(Root);
    //TODO : Check drop page or not.
    return 0;
  } 
  //Not found
  //go to child page
  if(!Children){
    //If there is no children,
    //Invalid (Not found)
    return -1;
  }
  //Check the target child
  if(!Children[child]){
    //If the target child does not exist,
    //Invalid (Not found)
    return -1;
  }
  //the target child exists.
  rc = Children[child]->SearchKey(Key,arg);
  if(rc){
    //Not found
    //Invalid (Not found)
    return -1;
  }
  //found
  rc = Children[child]->Delete(Key,arg);
  //return 0
  arg.write_set->insert(Children[child]);
  //TODO: Check drop page or not.
  return 0;
}
int Fragment::Update(const u32& Key, const u8* Value, const u16& size, my_arg& arg){
  //return value means
  //0 means update is done.
  //1 means delete is done, insert on below vertex is left.(Key is inherited.)
  //-1 means Invalid (NOTFOUND).
  u16 child;
  int rc;
  rc = Root->SearchKey(Key,arg);
  child = arg.Idx;
  if(rc){
    //Not found
    //go to child
    if(!Children){
      //If children does not exist,
      //Invalid (Not found)
      return -1;
    }
    //children exists.
    if(!Children[child]){
      //If the target child does not exist,
      //Invalid (Not found)
      return -1;
    }
    //Check the child page
    rc = Children[child]->SearchKey(Key,arg);
    if(rc){
      //Not found
      //Invalid (Not found)
      return -1;
    }
    //Found in child page
    arg.Pgno = Children[child]->GetPgno();
    rc = Children[child]->Update(Key,Value,size,arg);
  } else {
    //Found
    rc = Root->Update(Key,Value,size,arg);
    if(!rc){
      //Update is done
      arg.write_set->insert(Root);
      return 0;
    }
    //go to child(Insert)
    if(!Children){
      //Initialize Children first.
      nChild = Root->GetnCell()+1;
      Children = new MemPage*[nChild];
      for(int i = 0; i < nChild; i++){
        Children[i] = NULL;
      }
    }
    //children exists.
    if(!Children[child]){
      //If the target child is not created yet,
      //Create it and insert.
      Children[child] = new MemPage(Root->GetPgno(),child);
      arg.Idx = 0;
      rc = Children[child]->Insert(Key,Value,size,arg);
      //return 0;
      arg.write_set->insert(Children[child]);
      return 0;
    } 
    //If the target child is already created,
    //Check it
    rc = Children[child]->SearchKey(Key,arg);
    if(!rc){
      //the key is already in the page.
      //Invalid(redundent key)
      return -1;
    } 
    //the key is not in the page.
    arg.Pgno = Children[child]->GetPgno();
    rc = Children[child]->Insert(Key,Value,size,arg);
  }
  //return 0 or 1
  if(rc){
    //return 1
    //1 means go to below(Insert). The key is inherited.
    return 1;
  }
  //return 0 
  arg.write_set->insert(Children[child]);
  return 0;
}
u8* Fragment::Search(const u32& Key,my_arg& arg)const{
  //return values means
  //NULL means NotFound
  //Otherwise, record address is returned.
  u16 child;
  int rc;
  rc = Root->SearchKey(Key,arg);
  child = arg.Idx;
  if(!rc){
    //Found
    u8* Record = Root->GetRecord(arg.Idx);
    return Record;
  }
  //Not found, check child
  //go to child page
  if(!Children){
    //If there is no children,
    //(Not found)
    return NULL;
  }
  //Check the target child
  if(!Children[child]){
    //If the target child does not exist,
    //(Not found)
    return NULL;
  }
  //the target child exists.
  rc = Children[child]->SearchKey(Key,arg);
  if(rc){
    //Not found
    //(Not found)
    return NULL;
  }
  //found
  u8* Record = Children[child]->GetRecord(arg.Idx);
  //return 0 or 1
  //1 means the key will be inherited.
  return Record;

}
int Fragment::RangeSearch(){

}

void Fragment::printFragment(void)const{
  printf("Fragement information\n");
  printf("nChild : %lu, Range : (%lu < key < %lu)\n", nChild, LowerB, UpperB);
  printf("ROOT PAGE---\n");
  Root->printPage();
  printf("CHILD PAGES-------------\n");
  for(int i = 0; i < nChild; i++){
    if(Children){
      if(Children[i]){
        printf("%dth child page------------------------\n", i);
        Children[i]->printPage();

      } else {
        printf("%dth child is not exist!---------------,\n", i);
      }
    } else {
      printf("There is no child!!!------------------------------\n");
    }
  }
  printf("-------------------------------------------\n");
}
