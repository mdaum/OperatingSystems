#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main() {
  int h, i, j;
  for (h = 0; h < 2; ++h) {
    for (i = 0; i < 7; ++i) {
      char** ptrs = malloc(2048);
      for (j = 2 << (i+4); j <= 16348; j += 2 << (i+4)) {
        ptrs[j / (2 << (i+4))] = malloc(2 << (i+4));
        snprintf(ptrs[j / (2 << (i+4))], 6, "Hello");
        printf("%d: %p %s\n", i, ptrs[j / (2 << (i+4))], ptrs[j / (2 << (i+4))]);
      }
      for (j = 2 << (i+4); j <= 16348; j += 2 << (i+4)) {
        free(ptrs[j / (2 << (i+4))]);
        printf("%d: %p %p\n", i, ptrs[j / (2 << (i+4))], ptrs[j / (2 << (i+4))]);
      }
      free(ptrs);
    }
  }
  return (errno);
}
