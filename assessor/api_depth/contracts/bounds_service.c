#include "bounds.h"
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv) {
  if(argc!=2) return 64;
  unsigned value=3,eval=0;AcStatus r;
  if(!strcmp(argv[1],"capacity")) r=ac_checked_add(2,4,&value,&eval);
  else if(!strcmp(argv[1],"external")) {
    unsigned char p[3]={5,4,0};r=ac_admit_wire(p,3,&value);
  } else if(!strcmp(argv[1],"internal")) {
    value=5;r=ac_checked_add(0,4,&value,&eval);
  } else return 64;
  printf("status=%u value=%u;owned process fault boundary\n",(unsigned)r,value);
  return r==AC_INTERNAL?70:0;
}
