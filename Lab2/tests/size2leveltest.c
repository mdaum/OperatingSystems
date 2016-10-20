#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

static inline int size2level (ssize_t size) {
  /* Your code here.
   *    * Convert the size to the correct power of two. 
   *       * Recall that the 0th entry in levels is really 2^5, 
   *          * the second level represents 2^6, etc.
   *             */
  if (size <= 32) return 0;
  int i = -5;
  if (size % 2 != 0) ++i;
  while(size >>= 1) ++i; 
  return i;
}

int main(){
  int zero0=size2level(30);
  int zero1=size2level(32);
  int one0=size2level(33);
  int one1=size2level(64);
  int five = size2level(2000);
  int six=size2level(2048);
  if(!zero0&&!zero1&&one0&&one1&&five==5&&six==6)puts("size2leveltest PASS");
  else puts("size2leveltest FAIL");
  return 0;
}
