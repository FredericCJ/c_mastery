#include "api_depth.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc,char **argv){
 unsigned n=argc==2?(unsigned)strtoul(argv[1],0,10):2;
 if(n>9)return 200;
 struct api_buffer p={{0},0,1};unsigned char data[9]={0};enum api_status s=api_append(&p,data,n);
 if(n>1&&s==API_FULL&&p.length!=0){puts("SAME_FAILURE_REJECTED_APPEND_LENGTH");return 17;}
 if((n<=1&&s!=API_OK)||(n>1&&s!=API_FULL))return 200;
 return 0;
}
