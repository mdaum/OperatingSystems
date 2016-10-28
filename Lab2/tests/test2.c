#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main() {
  void *a = malloc(2048);
  printf("Hello %p\n", a);
  void *b = malloc(2048);
  printf("Hello %p\n", b);
  void *c = malloc(2048);
  printf("Hello %p\n", c);
  void *d = malloc(2048);
  printf("Hello %p\n", d);
  void *e = malloc(2048);
  printf("Hello %p\n", e);
  void *f = malloc(2048);
  printf("Hello %p\n", f);
  void *g = malloc(2048);
  printf("Hello %p\n", g);
  void *h = malloc(2048);
  printf("Hello %p\n", h);
  puts("FREE 1");
  free(a);
  free(b);
  free(c);
  free(d);
  free(e);
  free(f);
  free(g);
  free(h);
  puts("REALLOCATING");
  a = malloc(2048);
  printf("Hello %p\n", a);
  b = malloc(2048);
  printf("Hello %p\n", b);
  c = malloc(2048);
  printf("Hello %p\n", c);
  d = malloc(2048);
  printf("Hello %p\n", d);
  e = malloc(2048);
  printf("Hello %p\n", e);
  f = malloc(2048);
  printf("Hello %p\n", f);
  g = malloc(2048);
  printf("Hello %p\n", g);
  h = malloc(2048);
  printf("Hello %p\n", h);
  puts("FREE 2");
  free(d);
  free(a);
  free(h);
  free(b);
  free(f);
  free(c);
  free(e);
  free(g);

  return (errno);
}
