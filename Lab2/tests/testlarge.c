#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main() {
  char *e = malloc(5000);
  e[0] = 'a';
  e[4990] = '\0';
  printf("%s", e);
  free(e);
  printf("%s", e);

  return (errno);
}
