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
  void *m = malloc(1000);
  void *n = malloc(1000);
  void *o = malloc(1000); //end block 5
  void *p = malloc(1000);
  void *q = malloc(1000);
  void *r = malloc(1000); //end block 6
  void *s = malloc(1000);
  void *t = malloc(1000);
  void *u = malloc(1000); //end block 7
  void *v = malloc(1000);
  void *w = malloc(1000);
  void *x = malloc(1000); //end block 8
  free(a);
  free(b);
  free(c);
  free(d);
  free(e);
  free(g);
  free(f);
  free(h);
  free(i); //whole superblocks should now be free...should now hit the loop...
  free(j);
  free(k);
  free(l);
  free(m);
  free(n);
  free(o);
  free(p);
  free(q);
  free(r);
  free(s);
  free(t);
  free(u);
  free(v);
  free(w);
  free(x);

  return (errno);
}
