#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "../th_alloc.c"

int main() {
  void *x = malloc_internal(2048);
  printf("Hello %p\n", x);
  return (errno);
}
