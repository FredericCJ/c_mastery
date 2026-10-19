#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
unsigned tf_teaching_bounds(const unsigned char *,size_t);
int main(int argc,char **argv){
 if(argc!=2)return 2;
 FILE *f=fopen(argv[1],"rb");if(!f)return 2;
 if(fseek(f,0,SEEK_END))return 2;
 long length=ftell(f);if(length<0||length>16)return 2;
 rewind(f);size_t n=(size_t)length;unsigned char *p=malloc(n?n:1);if(!p)return 2;
 if(fread(p,1,n,f)!=n||fclose(f)){free(p);return 2;}
 unsigned result=tf_teaching_bounds(p,n);free(p);printf("TF_TRIAGE_RESULT:%u\n",result);return 0;
}
