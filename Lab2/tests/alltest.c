#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

 /*
 * Maxwell Daum and James Barbour
 * Honor Code: We did not give or recieve any unpermitted information on this assignment. 
 * All code (execpt for boilerplate) is our own.
 */

int main() {
  int h, i, j;
  for (h = 0; h < 2; ++h) {
    for (i = 0; i < 8; ++i) {
      char *ptrs[1000];
      for (j = 2 << (i+4); j <= 16348; j += 2 << (i+4)) {
        ptrs[j / (2 << (i+4))] = (char*)malloc(2 << (i+4));
        snprintf(ptrs[j / (2 << (i+4))], 6, "Hello");
        printf("%d: %p %s\n", i, ptrs[j / (2 << (i+4))], ptrs[j / (2 << (i+4))]);
      }
      for (j = 2 << (i+4); j <= 16348; j += 2 << (i+4)) {
        free(ptrs[j / (2 << (i+4))]);
        printf("%d: %p %p\n", i, ptrs[j / (2 << (i+4))], ptrs[j / (2 << (i+4))]);
      }
      //free(ptrs);
    }
  }
  return (errno);
}
