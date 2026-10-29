#include "declarations.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"LP_CHECK %s:%d %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
static int canonical(void){unsigned char a[2]={32,0};int32_t v=77;CHECK(ld_decode(a,&v)==LD_INVALID && v==77);a[0]=0;a[1]=7;CHECK(ld_decode(a,&v)==LD_OK && v==7);puts("LP canonical validated");return 0;}
static int sign(void){unsigned char a[2]={16,0};int32_t v=0;CHECK(ld_decode(a,&v)==LD_OK && v==-4096);a[0]=31;a[1]=255;CHECK(ld_decode(a,&v)==LD_OK && v==-1);puts("LP sign validated");return 0;}
static int table(void){uint32_t v=77;CHECK(ld_triangle(65536,&v)==LD_INVALID && v==77);CHECK(ld_triangle(65535,&v)==LD_OK && v==2147450880u);CHECK(ld_triangle(0,&v)==LD_OK && v==0);puts("LP table validated");return 0;}
static int wire(void){unsigned char a[2]={165,90};CHECK(ld_encode(4096,a)==LD_INVALID && a[0]==165 && a[1]==90);CHECK(ld_encode(-4096,a)==LD_OK && a[0]==16 && a[1]==0);CHECK(ld_encode(0,a)==LD_OK && !a[0] && !a[1]);puts("LP wire validated");return 0;}
static int oracle(void){char op;int64_t input;while(scanf(" %c %" SCNd64,&op,&input)==2){if(op=='e'){unsigned char out[2]={165,90};enum ld_status s=ld_encode((int32_t)input,out);printf("%d %u %u\n",s,out[0],out[1]);}else if(op=='d'){unsigned char in[2]={(unsigned char)((uint64_t)input>>8),(unsigned char)input};int32_t out=99;enum ld_status s=ld_decode(in,&out);printf("%d %" PRId32 "\n",s,out);}else if(op=='t'){uint32_t out=99;enum ld_status s=ld_triangle((uint32_t)input,&out);printf("%d %" PRIu32 "\n",s,out);}else CHECK(0);}return 0;}
int main(int argc,char **argv){CHECK(argc==2);if(!strcmp(argv[1],"canonical"))return canonical();if(!strcmp(argv[1],"sign"))return sign();if(!strcmp(argv[1],"table"))return table();if(!strcmp(argv[1],"wire"))return wire();if(!strcmp(argv[1],"oracle"))return oracle();if(!strcmp(argv[1],"metrics")){printf("{\"wire_bytes\":2,\"value_bits\":13,\"table_entries\":32,\"triangle_max\":65535}\n");return 0;}return 2;}
