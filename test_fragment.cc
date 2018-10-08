#include "my_fragment.h"
using namespace std;

int main(){
  MemPage * page = new MemPage(1,0);
  printf("Initial\n");
  page->printPage();

  u32 ub = ULONG_MAX-1;
  Fragment * frag = new Fragment(page,1,ub);
  frag->printFragment();
  u8 buf[100];
  memcpy(buf, "test_record",12);
  printf("start insert\n\n");
  fflush(stdout);
  int i = 1;
  my_arg arg;
  int rc;
  for(int j = 0; j < 207; j++){
    int k = (j + 1) * 207;
    rc = frag->Insert(k,buf,12,arg);
    printf("(%d) rc : %d\n",k,rc);
  }
//  for(int j = 0; j < 205; j++){
//    for(int k = 1; k < 207; k++){
//      rc = frag->Insert((j)*207 +k,buf,12,arg);
//      printf("(%d) rc : %d\n",j * 207 + k, rc);
//    }
//  }
  frag->printFragment();
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("start update\n");
  memcpy(buf, "larger_update",14);
  for(int j = 0; j < 207; j++){
    int k = (j + 1) * 207;
//    rc = frag->Delete(k);
    rc = frag->Update(k,buf,14);
    printf("(%d) rc : %d\n",k,rc);
  }
//  for(int j = 0; j < 205; j++){
//    for(int k = 1; k < 207; k++){
////      rc = frag->Delete((j)*207 +k);
//      rc = frag->Update((j)*207 +k,buf,14);
//      printf("(%d) rc : %d\n",j * 207 + k, rc);
//    }
//  }
  frag->printFragment();
  return 0;
//  printf("start update\n\n");
//  memcpy(buf, "larger_update",14);
//  for( int i = 0; i < 10; i++){
//    int rc;
//    rc = page->Update(i+1, buf, 14, L_pgno);
//    printf("rc : %d\n",rc);
//    page->printPage();
//  }
//  page->printPage();
//  printf("start Delete\n");
//  for( int i = 0; i < 204; i++){
//    int rc;
//    rc = page->Delete(i+1, L_pgno);
//    printf("rc : %d\n",rc);
//    page->printPage();
//  }
//  page->printPage();
  return 0;
}
