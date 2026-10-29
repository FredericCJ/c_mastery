#include "variant.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
int main(void){char op;
  while(scanf(" %c",&op)==1){
    if(op=='L'){uint32_t maximum,cap,out=77;char bytes[128];if(scanf("%"SCNu32" %"SCNu32" %127s",&maximum,&cap,bytes)!=3)return 2;int r=nv_length((const unsigned char *)bytes,strlen(bytes),maximum,cap,&out);printf("%d %"PRIu32"\n",r,out);}
    else if(op=='D'){uint32_t d,s;char out[30];size_t n=77;memset(out,'!',sizeof out);if(scanf("%"SCNu32" %"SCNu32,&d,&s)!=2)return 2;int r=nv_date(1,d,s,out,sizeof out,&n);printf("%d %zu ",r,n);if(!r)fputs(out,stdout);else putchar(out[0]);putchar('\n');}
    else if(op=='S'){unsigned length,a,b;unsigned char bytes[128];if(scanf("%u %u %u",&length,&a,&b)!=3 || length>128 || a>b || b>length)return 2;for(unsigned i=0;i<length;i++){unsigned v;if(scanf("%u",&v)!=1 || v>255)return 2;bytes[i]=(unsigned char)v;}struct nv_sum x,y,z;int r=nv_sum(bytes,a,&x)||nv_sum(bytes+a,b-a,&y)||nv_sum(bytes+b,length-b,&z)||nv_combine(x,y,&x)||nv_combine(x,z,&x);printf("%d %u %u\n",r,(unsigned)(uint16_t)~x.sum,x.odd);}
    else return 2;
  }return ferror(stdin)?2:0;
}
