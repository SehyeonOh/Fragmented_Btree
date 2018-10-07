#include <iostream>
#include "my_page.h"
using namespace std;

int main(){
  MemPage * page = new MemPage(1,0);
  printf("Initial\n");
  page->printPage();
  u8 buf[100];
  memcpy(buf, "test_record",12);
  u16 L_pgno;
  printf("start insert\n\n");
  int i = 1;
  while(!page->IsMatured()){
    int rc;
    rc = page->Insert(i++, buf, 12, L_pgno);
  }
  page->printPage();
//  printf("start update\n\n");
//  memcpy(buf, "larger_update",14);
//  for( int i = 0; i < 10; i++){
//    int rc;
//    rc = page->Update(i+1, buf, 14, L_pgno);
//    printf("rc : %d\n",rc);
//    page->printPage();
//  }
//  page->printPage();
  printf("start Delete\n");
  for( int i = 0; i < 204; i++){
    int rc;
    rc = page->Delete(i+1, L_pgno);
    printf("rc : %d\n",rc);
    page->printPage();
  }
  page->printPage();
  return 0;











  for( int i = 0; i < 10; i++){
    int rc;
    rc = page->Insert(i+1, buf, 12, L_pgno);
    printf("rc : %d\n",rc);
  }
  page->printPage();

  free(page);

  return 0;

}
