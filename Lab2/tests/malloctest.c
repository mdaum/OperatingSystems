#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main() {
  //test one allocation at max size
  void *a = malloc(2048);
  printf("2048: %p\n", a);
  //test one allocation at min size
  void *b = malloc(1);
  printf("1: %p\n", b);
  //test one allocation at arbitrary size
  void *c = malloc(432);
  printf("432: %p\n", c);
  //test one allocation at level boundary
  void *d = malloc(128);
  printf("128 %p\n", d);
  //test creating new superblock
  void *e = malloc(2048);
  printf("new object: %p old object: %p\n", e, a);
  return (errno);
}
