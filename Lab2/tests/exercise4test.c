#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main() {
  //make 4 full superblocks at level 5
  void *a = malloc(1000);
  void *b = malloc(1000); 
  void *c = malloc(1000); //end block 1
  void *d = malloc(1000); 
  void *e = malloc(1000);
  void *f = malloc(1000); //end block 2
  void *g = malloc(1000);
  void *h = malloc(1000);
  void *i = malloc(1000); //end block 3
  void *j = malloc(1000);
  void *k = malloc(1000);
  void *l = malloc(1000); //end block 4
  free(a);
  free(b);
  free(c);
  free(d);
  free(e);
  free(g);
  free(f);
  free(h);
  free(i); //whole superblocks should now be free...should now hit the loop...
	
  return (errno);
}
