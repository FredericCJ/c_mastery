#include "cmk2.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;} } while(0)
static void w32(unsigned char *p,unsigned v){for(unsigned i=0;i<4;i++)p[i]=(unsigned char)(v>>(i*8));}
static void checksum(unsigned char *p,unsigned n){unsigned sum=0;p[9]=0;for(unsigned i=0;i<n;i++)sum+=p[i];p[9]=(unsigned char)(0-sum);}
static int madt(void){
 unsigned char p[128]={0};memcpy(p,"APIC",4);w32(p+4,76);w32(p+36,0xfee00000);
 for(unsigned i=0;i<4;i++){p[44+i*8]=0;p[45+i*8]=8;p[47+i*8]=(unsigned char)(i*3);w32(p+48+i*8,1);}checksum(p,76);
 struct cmk2_topology t={0};CHECK(!cmk2_madt_parse(p,76,&t));CHECK(t.count==4&&t.ids[3]==9&&t.lapic==0xfee00000);
 for(unsigned n=0;n<76;n++)CHECK(cmk2_madt_parse(p,n,&t));
 p[45]=0;checksum(p,76);CHECK(cmk2_madt_parse(p,76,&t));p[45]=8;p[55]=p[47];checksum(p,76);CHECK(cmk2_madt_parse(p,76,&t));
 p[55]=3;checksum(p,76);p[10]^=1;CHECK(cmk2_madt_parse(p,76,&t));p[10]^=1;
 p[44]=9;checksum(p,76);CHECK(cmk2_madt_parse(p,76,&t));
 return 0;
}
static int channel(void){
 struct cmk2_channel q={0};struct cmk2_message m;
 CHECK(cmk2_channel_receive(&q,&m)==CMK2_EMPTY);
 for(unsigned round=0;round<20;round++){
  for(unsigned i=0;i<8;i++)CHECK(cmk2_channel_send(&q,(struct cmk2_message){round*8+i,i^123})==0);
  CHECK(cmk2_channel_send(&q,(struct cmk2_message){999,0})==CMK2_FULL);
  for(unsigned i=0;i<8;i++){CHECK(!cmk2_channel_receive(&q,&m));CHECK(m.sequence==round*8+i&&m.value==(i^123));}
 }
 CHECK(!cmk2_channel_send(&q,(struct cmk2_message){42,77}));cmk2_channel_close(&q);
 CHECK(cmk2_channel_send(&q,m)==CMK2_CLOSED);CHECK(!cmk2_channel_receive(&q,&m)&&m.sequence==42);CHECK(cmk2_channel_receive(&q,&m)==CMK2_CLOSED);
 CHECK(cmk2_channel_receive(&q,0)==CMK2_INVALID);q.count=9;CHECK(cmk2_channel_send(&q,m)==CMK2_INVALID);
 return 0;
}
int main(int argc,char **argv){if(argc!=2)return 2;if(!strcmp(argv[1],"madt"))return madt();if(!strcmp(argv[1],"channel"))return channel();return 2;}
