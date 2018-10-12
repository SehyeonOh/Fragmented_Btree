#include "my_manager.h"
#include <stdlib.h>
#include <algorithm>
using namespace std;

int main(){
  bool hash[LONG_MAX];
  for(uint32_t i = 0; i < LONG_MAX; i++){
    hash[i] = 0;
  }
  uint32_t key_seq[LONG_MAX];
  int len_seq[LONG_MAX];
  srand(0);
  for(uint32_t i = 0; i < LONG_MAX; i++){
    key_seq[i] = i;
    len_seq[i] = i;
  }
  random_shuffle(key_seq,&key_seq[LONG_MAX-1])
  random_shuffle(len_seq,&len_seq[LONG_MAX-1])
  Manager * mng = new Manager();
  printf("Initial\n");

  mng->printManager();
  u8 buf[100];
  memcpy(buf, "test_record",12);
  printf("start insert\n\n");
  fflush(stdout);
  int rc;
  for(int j = 0; j < 207; j++){
    int k = (j + 1) * 207;
    rc = mng->Insert(k,buf,12);
    printf("(%d) rc : %d\n",k,rc);
  }
  for(int j = 0; j < 205; j++){
    for(int k = 1; k < 207; k++){
      rc = mng->Insert((j)*207 +k,buf,12);
      printf("(%d) rc : %d\n",j * 207 + k, rc);
    }
  }
  mng->printManager();
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("start update\n");
  memcpy(buf, "larger_update",14);
  for(int j = 0; j < 207; j++){
    int k = (j + 1) * 207;
    rc = mng->Delete(k);
//    rc = mng->Update(k,buf,14);
    printf("(%d) rc : %d\n",k,rc);
  }
//  for(int j = 0; j < 205; j++){
//    for(int k = 1; k < 207; k++){
////      rc = frag->Delete((j)*207 +k);
//      rc = frag->Update((j)*207 +k,buf,14);
//      printf("(%d) rc : %d\n",j * 207 + k, rc);
//    }
//  }
  mng->printManager();
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
