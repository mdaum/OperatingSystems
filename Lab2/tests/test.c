#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main() {
  void *a = malloc(1024);
  printf("Hello %p\n", a);
  void *b = malloc(1024);
  printf("Hello %p\n", b);
  void *c = malloc(1024);
  printf("Hello %p\n", c);
  void *d = malloc(1024);
  printf("Hello %p\n", d);
  void *e = malloc(1024);
  printf("Hello %p\n", e);
  void *f = malloc(1024);
  printf("Hello %p\n", f);
  void *g = malloc(1024);
  printf("Hello %p\n", g);
  void *h = malloc(1024);
  printf("Hello %p\n", h);
  free(a);
  free(b);
  free(c);
  free(d);
  free(e);
  free(f);
  free(g);
  free(h);

  return (errno);
}
