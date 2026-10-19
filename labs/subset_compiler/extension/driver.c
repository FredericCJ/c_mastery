#include "extension.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc,char**argv){char source[16384],output[131072];CEProgram p;CEDiagnostic d;FILE*f;size_t n;unsigned fuel=10000;uint32_t value=99;if(argc!=3)return 2;f=fopen(argv[2],"rb");if(!f)return 2;n=fread(source,1,sizeof source,f);if(ferror(f)||!feof(f)){fclose(f);return 2;}fclose(f);if(!ce_compile(source,n,&p,&d)){fprintf(stderr,"REJECT %u %zu %zu\n",d.reason,d.begin,d.end);return 1;}if(!strcmp(argv[1],"x86")||!strcmp(argv[1],"thumb")){if(!ce_emit(&p,!strcmp(argv[1],"thumb"),output,sizeof output))return 3;fputs(output,stdout);return 0;}if(!ce_run(&p,(uint32_t)strtoul(argv[1],0,0),&fuel,&value)){puts("RUNTIME_REJECT");return 4;}printf("%u\n",value);return 0;}
