#include "models.h"
#include <stdio.h>
#include <string.h>
static void sequence(const unsigned *ops,unsigned n,unsigned family,unsigned identity){
 TmQueue q={0};tm_init(&q);printf("BEGIN %u %u %u\n",family,identity,n);
 for(unsigned i=0;i<n;i++){
  unsigned char out=99;TmQueue old=q;TaStatus status=ops[i]<2?tm_push(&q,(unsigned char)ops[i]):ops[i]==2?tm_pop(&q,&out):tm_clear(&q);
  int preserved=status==TA_OK||memcmp(&q,&old,sizeof q)==0;
  printf("Q %u %u %u %u %zu %d\n",i,ops[i],(unsigned)status,(unsigned)out,ta_count(&q.core),preserved);
 }
}
int main(void){
 unsigned ops[64],identity=0;
 for(unsigned radix=3;radix<=4;radix++)for(unsigned n=0,total=1;n<=6;n++,total*=radix)for(unsigned word=0;word<total;word++){
  unsigned code=word;for(unsigned i=0;i<n;i++){ops[i]=code%radix;code/=radix;}sequence(ops,n,radix,identity++);
 }
 for(unsigned seed=1;seed<=32;seed++){
  uint32_t state=seed;for(unsigned i=0;i<64;i++){state=state*UINT32_C(1664525)+UINT32_C(1013904223);ops[i]=(state>>16)%4;}sequence(ops,64,5,seed);
 }
 unsigned char wire[12];for(unsigned count=0;count<=5;count++)for(size_t n=0;n<=12;n++)for(unsigned opcode=0;opcode<2;opcode++){
  for(unsigned i=0;i<12;i++)wire[i]=(unsigned char)(i*17+3);
  wire[0]=(unsigned char)opcode;wire[1]=(unsigned char)count;TmPacket a={0},b={0};unsigned sa=tm_decode(wire,n,&a),sb=tm_reference(wire,n,0,&b);
  printf("D %u %zu %u %u %u %u %u",count,n,opcode,sa,sb,a.count,b.count);for(unsigned i=0;i<4;i++)printf(" %u %u",a.values[i],b.values[i]);puts("");
 }
 for(int first=-3;first<=3;first++)for(int second=-3;second<=3;second++)for(int third=-3;third<=3;third++)for(unsigned split=0;split<=3;split++)for(int sat=0;sat<2;sat++){
  int32_t input[3]={first,second,third};int64_t output[3];TmFir f={0};unsigned a=tm_fir(&f,input,split,sat,output),b=tm_fir(&f,input+split,3-split,sat,output+split);
  printf("F %d %d %d %u %d %u %u %lld %lld %lld\n",first,second,third,split,sat,a,b,(long long)output[0],(long long)output[1],(long long)output[2]);
 }
 return 0;
}
