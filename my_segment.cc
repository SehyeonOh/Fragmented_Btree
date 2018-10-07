
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
Segmemt::~Fragment(){
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
  Pgno& L_pgno;
  int rc;
  //First try the Root.
  rc = Root->Insert(Key,Value,size, L_pgno);
  if(rc<=0){
    //success or invaild attempt
    return rc;
  } else {
    
  }

}
int Fragment::Delete();
int Fragment::Update();
int Fragment::SearchKey();
int Fragment::RangeSearch();
