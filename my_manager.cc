#include "my_manager.h"
#include <queue>
#define KEY_MAX ULONG_MAX-1
Vertex::Vertex(MemPage * your_root, u32 LB, u32 UB){
  Frag = new Fragement(your_root, LB, UB);
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
Manager::Manager(MemPage * your_root){
  Top = new Vertex(your_root, 1, KEY_MAX);
//  cursor = Top;
}
Manager::~Manager(void){
  delete(Top);
}
int Manager::Insert(const u32& Key, const u8* Value, const u16 size){
  //return value means
  //0 means success
  //-1 means invalid attempt.(Same key)
  my_arg arg;
  int rc;
  Vertex* target = FindVertex(Key);
  rc = target->GetFrag()->Insert(Key,Value,size,arg);
  if(rc<=0){
    return rc;
  } else {
    //New fragment creation
    Mempage* new_page = new MemPage(target->GetFrag()->ChildPgno(arg.Idx));
    Vertex * new_vtx = new Vertex(new_page,arg.LB,arg.UB);
    Vertex* lm = target->GetChild();
    target->SetChild(new_vtx);
    new_vtx->SetSibling(lm);
    return 0;
  }
}
int Manager::Delete(const u32& Key){
  //return value means
  //0 means success
  //-1 means invalid attempt.(Same key)
  my_arg arg;
  int rc;
  Vertex* target = FindVertex(Key);
  rc = target->GetFrag()->Delete(Key,arg);
  if(rc<=0){
    return rc;
  } else {

  }
  
}
int Manager::Update(){
}
int Manager::SearchKey(){
}
int Manager::RangeSearch(){
}
Vertex* Manager::FindVertex(const u32& Key){
  //Vertex* parent = Top;
  //Vertex* prev_parent;
  Vertex* cursor;
  Vertex* vtx = Top;
  while(vtx){
    
    if(cursor = vtx->GetChild()){
      while(cursor){
        if(cursor->GetFrag()->RangeCheck()){
          vtx = cursor;
          break;
        }
        cursor = cursor->Sibling();
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
        if(cursor->GetFrag()->RangeCheck(Key-1)){
          vtx = cursor;
          vtx->GetFrag()->IncrementUB();
          break;
        }
        cursor = cursor->Sibling();
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
