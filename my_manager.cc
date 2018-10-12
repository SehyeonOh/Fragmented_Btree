#include "my_manager.h"
#include <sys/stat.h>
#include <queue>
#include <set>
#define get2byte(x)   ((x)[0]<<8 | (x)[1])
Vertex::Vertex(MemPage * your_root,const u32& LB,const u32& UB){
  Frag = new Fragment(your_root, LB, UB);
  child = NULL;
  sibling = NULL;
}
Vertex::~Vertex(void){
  //DFS
  if(child)
    delete(child);
  if(sibling)
    delete(sibling);
  delete(Frag);
}
Manager::Manager(int FD){
  //get fd
  fd = FD;
  struct stat buf;
  int rc = fstat(FD,&buf);
  u32 file_size = buf.st_size;
  if(buf.st_size >= PAGE_SIZE){
    //construct structure by reading the file
    //1, Create MemPage pointers first.
    u32 N_page = file_size / PAGE_SIZE;
    MemPage** pg_list = new MemPage*[N_page];
    MemPage* ptr;
    //2, Get page one by one
    for(u32 i = 0; i < N_page; i++){
      ptr = pg_list[i];
      u8 type;
      ptr = new MemPage(fd,i,type);
      //Last bit is already removed.
      u32 Root_pgno = ptr->GetRootPgno();
      u16 nThChild = ptr->GetnThChild();
      if(type){
        //Root page of some fragment
        //Create Vertex and Set the hierarchy of vertex(child or parent)
        u32 LB, UB;
        Vertex* parent = NULL;
        if(!Root_pgno){
          //Top vertex
          //has no parent vertex
          LB = 0;
          UB = KEY_MAX;
        } else {
          //the other vertices
          //has parent
          //find parent vertex
          parent = pg_list[Root_pgno]->GetVertex();
          //find boundary

        }
        Vertex* new_vtx = new Vertex(ptr,LB,UB);
        //For easy recovery
        ptr->SetShortCut(new_vtx);
      } else {
        //Child page of some fragment
      }
    }
  }else {
    //Initial constructor
    MemPage* root = new MemPage(0,0);
    Top = new Vertex(root, 0, KEY_MAX);
  }
  begin_flag = 0;
}
Manager::~Manager(void){
  delete(Top);
}
int Manager::Insert(const u32& Key, const u8* Value, const u16& size){
  //return value means
  //0 means success
  //-1 means invalid attempt.(redundant key)
  my_arg arg;
  arg.write_set = &write_set;
  int rc;
  Vertex* vtx = FindVertex(Key);
  arg.LB = vtx->Frag->GetLowerB();
  arg.UB = vtx->Frag->GetUpperB();
//  printf("%lu < .. < %lu\n",arg.LB, arg.UB);
  rc = vtx->Frag->Insert(Key,Value,size,arg);
  if(rc <= 0){
    //return 0 or -1
    //Correctly done, or Invalid(redundant key)
    if(!rc && !begin_flag){
      CommitTxn();
    }
    return rc;
  } 
  //New fragment creation
  if(arg.UB == Key) {
    //Inherit?
    arg.UB++;
  }
  MemPage* new_page = new MemPage(arg.Pgno);
  Vertex * new_vtx = new Vertex(new_page,arg.LB,arg.UB);
//  printf("new frag\n");
//  printf("%lu < .. < %lu\n",arg.LB, arg.UB);
  rc = new_vtx->Frag->Insert(Key,Value,size,arg);
  if(rc){
    //If not correctly done,
    //ERROR
    printf("ERROR-----------------------------------------------\n");
    exit(0);
  }
  Vertex* lm = vtx->GetChild();
  vtx->SetChild(new_vtx);
  new_vtx->SetSibling(lm);
  if(!begin_flag){
    CommitTxn();
  }
  return 0;
}
int Manager::Delete(const u32& Key){
  //return value means
  //0 means success
  //-1 means invalid attempt.(NotFound)
  my_arg arg;
  arg.write_set = &write_set;
  int rc;
  Vertex* vtx = FindVertex(Key);
  arg.LB = vtx->Frag->GetLowerB();
  arg.UB = vtx->Frag->GetUpperB();
  rc = vtx->Frag->Delete(Key,arg);
  if(rc<=0){
    //return 0 or -1
    //Correctly done, or Invalid(NotFound)
    if(!rc && !begin_flag){
      CommitTxn();
    }
    return rc;
  } 
  //Key inheritance
  Vertex * bt_vtx = InheritKey(Key,vtx);
  if(bt_vtx == vtx){
    //There is no child which has the key as the upper bound.
    //inheritance is not needed.
    //the
  } else {
    //The upper bound is already incremented in InheritKey method.
    //Nothing to do remains.
  }
  if(!begin_flag){
    CommitTxn();
  }
  return 0;
}
int Manager::Update(const u32& Key, const u8* Value, const u16& size){
  //return value means
  //0 means success
  //-1 means invalid attempt.(NotFound)
  my_arg arg;
  arg.write_set = &write_set;
  int rc;
  Vertex* vtx = FindVertex(Key);
  arg.LB = vtx->Frag->GetLowerB();
  arg.UB = vtx->Frag->GetUpperB();
  rc = vtx->Frag->Update(Key,Value,size,arg);
  if(rc<=0){
    //return 0 or -1
    //Correctly done, or Invalid(NotFound)
    if(!rc && !begin_flag){
      CommitTxn();
    }
    return rc;
  } 
  //Key inheritance and insert the key
  Vertex * bt_vtx = InheritKey(Key,vtx);
  if(bt_vtx == vtx){
    //There is no child which has the key as the upper bound.
    //Need to create child vertex which takes the inheritance and the (key,value).
    if(arg.UB != Key){
      printf("Something wrong!!!!!!!!!!!!!\n");
    }
    arg.UB++;
    MemPage* new_page = new MemPage(arg.Pgno);
    Vertex * new_vtx = new Vertex(new_page,arg.LB,arg.UB);
    rc = new_vtx->Frag->Insert(Key,Value,size,arg);
    if(rc){
      //If not correctly done,
      //ERROR
      printf("ERROR-----------------------------------------------\n");
      exit(0);
    }
    Vertex* lm = vtx->GetChild();
    vtx->SetChild(new_vtx);
    new_vtx->SetSibling(lm);
  } else {
    //The upper bound is already incremented in InheritKey method.
    //Insert the Key in bt_vtx
    rc = bt_vtx->Frag->Insert(Key,Value,size,arg);
    if(rc == -1){
      printf("ERROR-----------------------------------------------\n");
      exit(0);
    }
    if(rc<=0){
      //return 0
      //Correctly done
      if(!rc && !begin_flag){
        CommitTxn();
      }
      return rc;
    } 
    arg.UB++;
    MemPage* new_page = new MemPage(arg.Pgno);
    Vertex * new_vtx = new Vertex(new_page,arg.LB,arg.UB);
    rc = new_vtx->Frag->Insert(Key,Value,size,arg);
    if(rc){
      //If not correctly done,
      //ERROR
      printf("ERROR-----------------------------------------------\n");
      exit(0);
    }
    Vertex* lm = vtx->GetChild();
    vtx->SetChild(new_vtx);
    new_vtx->SetSibling(lm);
  }
  if(!begin_flag){
    CommitTxn();
  }
  return 0;
}

u8* Manager::Search(const u32& Key, u16& size)const{
  //return values means
  //NULL means NotFound
  //Otherwise, record address is returned.
  //size stores the size of record.
  my_arg arg;
  u8* Record = NULL;
  Vertex* vtx = FindVertex(Key);
  arg.LB = vtx->Frag->GetLowerB();
  arg.UB = vtx->Frag->GetUpperB();
  Record = vtx->Frag->Search(Key,arg);
  if(!Record){
//  printf("%lu < .. < %lu\n",arg.LB, arg.UB);
//    printf("Key is gone. %lu\n", Key);
    size = 0;
    return NULL;
  }
  size = get2byte(Record);
  return (Record+2);
}

int Manager::RangeSearch(){
}

Vertex* Manager::FindVertex(const u32& Key)const{
  //Vertex* parent = Top;
  //Vertex* prev_parent;
  Vertex* cursor;
  Vertex* vtx = Top;
  while(vtx){
    if(cursor = vtx->GetChild()){
      while(cursor){
        if(cursor->Frag->RangeCheck(Key)){
          vtx = cursor;
          break;
        }
        cursor = cursor->GetSibling();
      }
      if(!cursor){
        break;
      }
    } else {
      break;
    }
  }
  return vtx;
}
Vertex* Manager::InheritKey(const u32& Key,Vertex* From){
  //Vertex* parent = Top;
  //Vertex* prev_parent;
  Vertex* cursor;
  Vertex* vtx = From;
  while(vtx){
    
    if(cursor = vtx->GetChild()){
      while(cursor){
        if(cursor->Frag->RangeCheck(Key-1)){
          vtx = cursor;
          vtx->Frag->IncrementUB();
          break;
        }
        cursor = cursor->GetSibling();
      }
      if(!cursor){
        break;
      }
    } else {
      break;
    }
  }
  return vtx;
}

void Manager::printManager(void)const{
  //BFS
  std::queue<Vertex*> my_queue;
  my_queue.push(Top);
  Vertex* vtx;
  Vertex* cursor;
  while(!my_queue.empty()){
    vtx = my_queue.front();
    my_queue.pop();
    vtx->Frag->printFragment();
    if(cursor = vtx->GetChild()){
      while(cursor){
        my_queue.push(cursor);
        cursor = cursor->GetSibling();
      }
    }
  }
}
void Manager::Visualize(void)const{
  //BFS
  std::queue<Vertex*> my_queue;
  my_queue.push(Top);
  Vertex* vtx;
  Vertex* cursor;
  while(!my_queue.empty()){
    vtx = my_queue.front();
    my_queue.pop();
    if(cursor = vtx->GetChild()){
      printf("Parent : %lu\n",vtx->Frag->GetRootPgno());
      printf("Child : ");
      while(cursor){
        my_queue.push(cursor);
        if(vtx->Frag->GetLowerB()>cursor->Frag->GetLowerB())
            printf("(LBError)");
        if(vtx->Frag->GetUpperB()+1==cursor->Frag->GetUpperB())
          printf("(Inherit");
        else if(vtx->Frag->GetUpperB()+1<cursor->Frag->GetUpperB())
          printf("(UBError)");
        printf("%lu ", cursor->Frag->GetRootPgno());
        cursor = cursor->GetSibling();
      }
      printf("\n");
    }
  }
}
