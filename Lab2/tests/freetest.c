#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main() {
  //test one allocation at max size
  void *a = malloc(2048);
  free(a);
  printf("Test 1: %d\n",a==malloc(2048));

  //test one allocation at min size
  void *b = malloc(1);
  free(b);
  printf("Test 2: %d\n", b==malloc(5));

  //test one allocation at arbitrary size
  void *c = malloc(432);
  free(c);
  printf("Test 3: %d\n", c==malloc(512));

  //test one allocation at level boundary
  void *d = malloc(128);
  void *e = malloc (127);
  free(d);
  free(e);
  void *f = malloc(100);
  free(f);
  malloc(99);
  printf("Test 4: %d\n", malloc(99)==d);

  //test creating new superblock
  void *g = malloc(20);
  void *h = malloc (30);
  void *i = malloc(10);
  free(i);
  free(h);
  free(g);
  printf("Test 5: %d\n", malloc(5)==g);
  return (errno);
}
