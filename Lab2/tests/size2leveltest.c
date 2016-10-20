#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "../th_alloc.c"

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
