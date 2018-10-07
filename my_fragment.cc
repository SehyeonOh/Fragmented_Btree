
Fragment::Fragment(MemPage* your_root, u32 LB, u32 UB){
  Root = your_root;
  LowerB = LB;
  UpperB = UB;
  nChild = your_root->nCell + 1;
  Children = new (MemPage *)[nChild];
  for(int i = 0; i < nChild; i++){
    Children[i] = NULL;
  }
}
Segmemt::~Fragment(void){
  //delete all page
  for(int i = 0; i < nChild; i++){
    if(!Children[i]){
      free(Children[i]);
    }
  }
  free(Root);
}
int Fragment::Insert(const u32& Key, const u8* Value, const u16 size){
  //Find target page and insert it.
  //return value means
  //0 means success.
  //1 means need to move to below vertex.
  //-1 means invalid attempt.(Over the range.)
  if(Key < LowerB || Key > UpperB)
    return -1;
  //TODO: Logical pgno may be useless. There is no need of page table.
  //Fragment class can the role of page table.
  u16 child;
  int rc;
  //First try the Root.
  rc = Root->Insert(Key,Value,size, child);
  if(rc<=0){
    //success or invaild attempt
    return rc;
  } else {
    //If the child exist, insert it on that page.
    u16 new_child;
    if(Children[child]){
      rc = Children[child]->Insert(Key,Value,size,new_child);
      return rc;
    } else {
      Children[child] = new MemPage(pgno_next,Root->pgno);
      rc = Children[child]->Insert(Key,Value,size,new_child);
      return rc;
    }
    
  }

}
int Fragment::Delete(const u32& Key){
  //return values means
  //0 means success
  //1 means the key was inherited to below vertex.
  //-1 means not found( Invalid attempt ) 
  if(Key < LowerB || Key > UpperB)
    return -1;
  u16 child;
  int rc;
  rc = Root->Delete(Key,child);
  if(rc<=0){
    return rc;
  } else {
    u16 new_child;
    if(Children[child]){
      rc = Children[child]->Delete(Key,new_child);
      return rc;
    } else {
      return -1;
    }
  }
}
int Fragment::Update(const u32& Key, const u8* Value, const u16 size){
  //return value means
  //0 means delete and insert are done.
  //1 means delete is done, insert on below vertex is left.(Key is inherited.)
  //2 means none of them is done, the key was inherited.
  //-1 means Invalid update. (NOTFOUND).
  if(Key < LowerB || Key > UpperB)
    return -1;
  u16 child;
  int rc;
  rc = Root->Update(Key,Value,size,child);
  if(rc<=0){
    return rc;
  } else if(rc == 1) {
    u16 new_child;
    if(Children[child]){
      rc = Children[child]->Insert(Key,Value,size,child);
      return rc;
    } else {
      Children[child] = new MemPage(pgno_next,Root->pgno);
      rc = Children[child]->Insert(Key,Value,size,child);
      return rc;
    }
  } else{
    //rc == 2
    u16 new_child;
    if(Children[child]){
      rc = Children[child]->Update(Key,Value,size,new_child);
      //The key can be inherited.
      return rc;
    } else {
      return -1;
    }
  }

}
int Fragment::SearchKey();
int Fragment::RangeSearch();
