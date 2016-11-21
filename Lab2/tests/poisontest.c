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
  char *e = malloc(2048);
  e[0] = 'a';
  e[2040] = '\0';
  printf("%s", e);
  free(e);
  printf("%s", e);

  return (errno);
}
