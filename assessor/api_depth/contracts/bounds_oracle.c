#include "bounds.h"
#include <stdio.h>
int main(int argc,char **argv) {
  if(argc!=2) return 4;
  FILE *input=fopen(argv[1],"r");if(!input) return 5;
  unsigned start,delay,elapsed;unsigned long cases=0;
  while(fscanf(input,"%u %u %u",&start,&delay,&elapsed)==3) {
    struct ac_deadline d;int expired=77;
    enum ac_status status=ac_deadline(start,(struct ac_wait){AC_FINITE,delay},&d);
    if(status==AC_OK) status=ac_expired(d,(start+elapsed)%256,&expired);
    printf("%u %d\n",(unsigned)status,expired);++cases;
  }
  int ok=feof(input)&&!ferror(input);if(fclose(input)) return 6;
  if(!ok) return 2;
  return cases?0:3;
}
