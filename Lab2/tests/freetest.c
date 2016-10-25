#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main() {
  //test one allocation at max size
  void *a = malloc(2048);
  printf("orig 2048: %p\n", a);
  //free it
  free(a);
  //reallocate at max level
  void *b = malloc(2048);
  printf("new 2048:  %p\n", b);
  return (errno);
}
