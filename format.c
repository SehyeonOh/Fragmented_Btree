#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main(){
  struct timespec t1_st,t1_ed,t2_st,t2_ed;
  long long time1, time2;
  //initialization
  printf("1st case is \n");
  clock_gettime(CLOCK_MONOTONIC, &t1_st);
  //test_case 1
  clock_gettime(CLOCK_MONOTONIC, &t1_ed);
  sleep(1); // No meaning
  printf("2nd case is \n");
  clock_gettime(CLOCK_MONOTONIC, &t2_st);
  //test_case 2
  clock_gettime(CLOCK_MONOTONIC, &t2_ed);

  //check correctness
  time1 = 1000000000LLU*(t1_ed.tv_sec - t1_st.tv_sec);
  time1 += (t1_ed.tv_nsec - t1_st.tv_nsec);
  time2 = 1000000000LLU*(t2_ed.tv_sec - t2_st.tv_sec);
  time2 += (t2_ed.tv_nsec - t2_st.tv_nsec);
  
  printf("1st case time : %d\n", time1);
  printf("2nd case time : %d\n\n", time2);
  if(time1 < time2)
    printf("1st case is better than 2nd one.\n");
  else
    printf("2nd case is better than 1st one.\n");
  return 0;
}
