#include "my_manager.h"
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#define test_size 1000000000LU
using namespace std;
#define divide 100000
int main(){
  struct timespec t1_st,t1_ed,t2_st,t2_ed;
  long long time1, time2;
  printf("Initialize key\n");
  fflush(stdout);
  bool * hash = new bool[test_size];
  for(uint32_t i = 0; i < test_size; i++){
    hash[i] = 0;
  }
  u32* key_seq = new u32[test_size/10];
  int* len_seq = new int[test_size/10];
  srand(0);
  for(uint32_t i = 0; i < test_size/10; i++){
//    len_seq[i] = rand() % 1024 + 16; // %1024
    len_seq[i] = 128;
    u32 key;
    while(1){
      key = (u32)rand();
      key = key % test_size;
      if(hash[key] == 1) continue;
      hash[key] = 1;
      key_seq[i] = key+1;
      break;
    }
  }
  int fd = open("file_output.txt",O_RDWR | O_CREAT, 0666);
  Manager * mng = new Manager(fd);
  int rc;
//  printf("Initial manager\n");
//  fflush(stdout);
//
////  mng->printManager();
  u8 buf[1024];
  for(int i = 0; i < 1024; i++){
    buf[i] = (u8)(rand()& (1<<7 - 1));
  }
  printf("start insert\n\n");
  fflush(stdout);
  clock_gettime(CLOCK_MONOTONIC, &t1_st);
  for(int j = 0; j < test_size/divide; j++){
    rc = mng->Insert(key_seq[j],buf,len_seq[j]);
//    printf("(%lu) rc : %d\n",key_seq[j],rc);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1_ed);
  //check correctness
  time1 = 1000000000LLU*(t1_ed.tv_sec - t1_st.tv_sec);
  time1 += (t1_ed.tv_nsec - t1_st.tv_nsec);
  printf("insert case time : %llu\n", time1);
  printf("performance   : %llu\n", 10000000000000LLU / time1);
//  mng->Visualize();
//  mng->printManager();
//  printf("\n");
//  printf("\n");
//  printf("\n");
//  printf("\n");
//  printf("start search all one by one\n");
//  u32 total = 0;
//  for(int j = 0; j < test_size/divide; j++){
//    u16 size = 0;
//    u8* Record = NULL;
//    Record = mng->Search(key_seq[j],size);
//      static u32 cnt = 0;
//    if(Record){
//      total+= size;
//      printf("%luth size : %lu\n",++cnt, size);
//    }else {
//      printf("%luth NotFound\n", ++cnt);
//    }
////    printf("(%lu) rc : %d\n",key_seq[j],rc);
//  }
//  printf("total record size : %llu\n",total);
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("start update\n");
  memcpy(buf, "larger_update",14);
  clock_gettime(CLOCK_MONOTONIC, &t1_st);
  for(int j = 0; j < test_size/divide; j++){
    rc = mng->Update(key_seq[j],buf,len_seq[test_size/divide + j]);
//    printf("(%lu) rc : %d\n",key_seq[j],rc);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1_ed);
  //check correctness
  time1 = 1000000000LLU*(t1_ed.tv_sec - t1_st.tv_sec);
  time1 += (t1_ed.tv_nsec - t1_st.tv_nsec);
  printf("update case time : %llu\n", time1);
  printf("performance   : %llu\n", 10000000000000LLU / time1);
//  mng->Visualize();
//  mng->printManager();
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("start delete\n");
  memcpy(buf, "larger_update",14);
  clock_gettime(CLOCK_MONOTONIC, &t1_st);
  for(int j = 0; j < test_size/divide; j++){
    rc = mng->Delete(key_seq[j]);
//    rc = mng->Update(k,buf,14);
//    printf("(%lu) rc : %d\n",key_seq[j],rc);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1_ed);
  //check correctness
  time1 = 1000000000LLU*(t1_ed.tv_sec - t1_st.tv_sec);
  time1 += (t1_ed.tv_nsec - t1_st.tv_nsec);
  printf("delete case time : %llu\n", time1);
  printf("performance   : %llu\n", 10000000000000LLU / time1);
//  mng->Visualize();
//  mng->printManager();
  free(mng);
  close(fd);
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
