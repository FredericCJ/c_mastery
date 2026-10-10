/* Independent review regressions. Explicit byte spans must not become C strings. */
#include "cvm.h"
#include <stdio.h>
#include <string.h>
int main(void) {
  const char name[] = "(fn main\0junk () 42)";
  const char value[] = "(fn main () true\0junk)";
  const char *cases[] = {name,value};
  const size_t sizes[] = {sizeof name-1,sizeof value-1};
  for(unsigned i=0;i<2;++i){
    CvmProgram p;memset(&p,0xa5,sizeof p);CvmProgram before=p;
    CvmStatus status=cvm_compile(cases[i],sizes[i],&p);
    printf("explicit-span-%u: %s\n",i,cvm_status_name(status));
    if(status!=CVM_SYNTAX||memcmp(&p,&before,sizeof p))return 1;
  }
  return 0;
}
