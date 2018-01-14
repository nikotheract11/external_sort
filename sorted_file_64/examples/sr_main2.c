#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bf.h"
#include "sort_file.h"

#define CALL_OR_DIE(call)     \
  {                           \
    SR_ErrorCode code = call; \
    if (code != SR_OK) {      \
      printf("Error\n");      \
      exit(code);             \
    }                         \
  }

#define RUN_AND_TIME(call)                                              \
  {                                                                     \
    clock_t start = clock();                                            \
    call;                                                               \
    clock_t end = clock();                                              \
    printf("CPU Time: %lf\n", (end - start) / (double)CLOCKS_PER_SEC);  \
  }

int main() {
  BF_Init(LRU);
  CALL_OR_DIE(SR_Init());
  int fd,nfd;

 // SR_OpenFile("unsorted_data.db",&fd);

//  SR_PrintAllEntries(fd);
  printf("Sorting 'unsorted_data.db' file in field 'name' ...");
 CALL_OR_DIE(SR_SortedFile("unsorted_data.db", "sorted_name.db", 1, 3));
//	  SR_PrintAllEntries(fd);
 // printf("Sorting 'unsorted_data.db' file in field 'surname' ...");
 // CALL_OR_DIE(SR_SortedFile("unsorted_data.db", "sorted_surname.db", 2, 33))
  printf("Sorting sorted_surname.db file in 'field' ...");
  //printf("eee=%d\n",getEntries(fd) );
//  CALL_OR_DIE(SR_SortedFile("unsorted_data.db", "sorted_id.db", 0, 9))
  //CALL_OR_DIE(SR_OpenFile("temp",&fd));
 // SR_PrintAllEntries(fd);
  //SR_OpenFile("temp",&nfd);
  //SR_PrintAllEntries(nfd);
  BF_Close();
}
