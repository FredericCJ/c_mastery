#include "fault.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){printf("DF_CHECK:%d\n",__LINE__);return 1;}}while(0)
struct du_hook {struct df_capture*c;struct df_input*input;unsigned pass;};
static void nested(void*p){struct du_hook*h=p;struct df_record out={.version=77};unsigned secondary=77;h->pass=df_read(h->c,&out,&secondary)==DF_BUSY&&out.version==77&&secondary==77&&df_capture(h->c,h->input,0,0)==DF_AGAIN;}
static int deep_frame(void){unsigned char live[160];memset(live,0xa5,sizeof live);memset(live+64,0,96);size_t used=0;return df_watermark(live,sizeof live,0xa5,&used)==DF_OK&&used==96&&sizeof live==160;}
int main(void){
 unsigned char image[32]={1},other[32]={2},code[]={8,96};uint32_t words[]={0x1234,0x20001000,2,3,12,0x101,0x100,0x01000000};
 struct df_input input={.cfsr=0x8200,.hfsr=0x40000000,.bfar=0x20001000,.mmfar=0xdeadbeef,.exc_return=0xfffffffd,.stack_address=0x20002000,.words=words,.available=8,.readable=1,.aligned=1,.image=image};struct df_capture c;df_init(&c);struct du_hook h={&c,&input,0};
 CHECK(df_capture(&c,&input,nested,&h)==DF_OK&&h.pass);struct df_record r;unsigned secondary;CHECK(df_read(&c,&r,&secondary)==DF_OK&&secondary==1&&r.valid==(DF_STATUS|DF_BUS_ADDRESS|DF_FRAME|DF_IMAGE)&&r.bfar==0x20001000&&r.mmfar==0);
 struct df_flags flags;CHECK(df_flags(&r,&flags)==DF_OK&&flags.precise_bus&&flags.address_valid&&flags.forced&&!flags.imprecise);
 struct df_access access={.pc=77};CHECK(df_access(&r,image,0x100,code,2,&access)==DF_OK&&access.pc==0x100&&access.effective_address==0x20001000&&access.operand==0x1234&&access.write&&access.precise);
 access.pc=77;CHECK(df_access(&r,other,0x100,code,2,&access)==DF_IDENTITY&&access.pc==77);r.cfsr|=0x400;CHECK(df_access(&r,image,0x100,code,2,&access)==DF_OK&&!access.precise);
 input.cfsr=0;CHECK(df_capture(&c,&input,0,0)==DF_AGAIN);CHECK(df_read(&c,&r,&secondary)==DF_OK&&r.cfsr==0x8200);
 /* Explicit failed-peripheral model: rich logging first reaches lockup before
  * capture; fixed capture first remains inspectable after the same stop. */
 unsigned refault;struct df_capture rich,fixed;df_init(&rich);df_init(&fixed);
 CHECK(df_refault(0,0,0,&refault)==DF_OK&&refault==DF_LOCKUP);CHECK(df_read(&rich,&r,&secondary)==DF_BUSY);
 CHECK(df_capture(&fixed,&input,0,0)==DF_OK);CHECK(df_refault(0,0,0,&refault)==DF_OK&&refault==DF_LOCKUP);CHECK(df_read(&fixed,&r,&secondary)==DF_OK&&r.cfsr==0);
 df_init(&c);CHECK(df_capture(&c,&input,0,0)==DF_OK&&df_read(&c,&r,&secondary)==DF_OK&&!(r.valid&DF_BUS_ADDRESS)&&r.bfar==0);CHECK(df_flags(&r,&flags)==DF_OK&&!flags.address_valid);
 input.words=0;input.readable=0;df_init(&c);CHECK(df_capture(&c,&input,0,0)==DF_OK&&df_read(&c,&r,&secondary)==DF_OK&&!(r.valid&DF_FRAME));
 input.words=words;input.readable=1;input.available=7;df_init(&c);CHECK(df_capture(&c,&input,0,0)==DF_OK&&df_read(&c,&r,&secondary)==DF_OK&&!(r.valid&DF_FRAME));
 input.available=8;input.exc_return=0xffffffed;df_init(&c);CHECK(df_capture(&c,&input,0,0)==DF_OK&&df_read(&c,&r,&secondary)==DF_OK&&!(r.valid&DF_FRAME));
 input.exc_return=0xfffffffd;input.cfsr=0x800;df_init(&c);CHECK(df_capture(&c,&input,0,0)==DF_OK&&df_read(&c,&r,&secondary)==DF_OK&&!(r.valid&DF_FRAME));
 input.cfsr=0;words[6]|=1;df_init(&c);CHECK(df_capture(&c,&input,0,0)==DF_OK&&df_read(&c,&r,&secondary)==DF_OK&&!(r.valid&DF_FRAME));words[6]&=~UINT32_C(1);
 words[7]|=1u<<9;df_init(&c);CHECK(df_capture(&c,&input,0,0)==DF_OK&&df_read(&c,&r,&secondary)==DF_OK&&!(r.valid&DF_FRAME));words[7]&=~(1u<<9);
 input.cfsr=0;input.stack_address|=4;df_init(&c);CHECK(df_capture(&c,&input,0,0)==DF_OK&&df_read(&c,&r,&secondary)==DF_OK&&!(r.valid&DF_FRAME));r.version=2;CHECK(df_flags(&r,&flags)==DF_UNSUPPORTED);
 unsigned outcome=99;CHECK(df_refault(0,0,0,&outcome)==DF_OK&&outcome==DF_LOCKUP);CHECK(df_refault(0,1,0,&outcome)==DF_OK&&outcome==DF_IGNORED);CHECK(df_refault(1,0,0,&outcome)==DF_OK&&outcome==DF_PENDING);CHECK(df_refault(2,0,1,&outcome)==DF_OK&&outcome==DF_LOCKUP);CHECK(df_refault(2,0,0,&outcome)==DF_OK&&outcome==DF_IGNORED);CHECK(df_refault(3,1,0,&outcome)==DF_OK&&outcome==DF_LOCKUP);
 unsigned char stack[256],canary[4];memset(stack,0xa5,sizeof stack);memset(canary,0x5a,sizeof canary);size_t used=999;unsigned intact=99;CHECK(df_watermark(stack,256,0xa5,&used)==DF_OK&&used==0);memset(stack+160,0,96);CHECK(df_watermark(stack,256,0xa5,&used)==DF_OK&&used==96);
 /* A live160-byte frame can retain the fill value in its lower64 bytes. */
 CHECK(deep_frame());CHECK(df_canary(canary,4,0x5a,&intact)==DF_OK&&intact);canary[3]=0;CHECK(df_canary(canary,4,0x5a,&intact)==DF_OK&&!intact);
 struct df_live_frame frames[]={{32,0,0,0},{64,0,0,0},{32,1,0,0}};struct df_budget b;CHECK(df_budget(frames,3,256,0,0,&b)==DF_OK&&b.required==160&&b.fits&&256-used==160);
 struct df_live_frame fp[]={{160,0,0,0},{0,1,1,1},{0,1,1,1}};CHECK(df_budget(fp,3,256,0,0,&b)==DF_OK&&b.required==376&&!b.fits);
 b.required=77;CHECK(df_budget(frames,3,256,1,0,&b)==DF_UNKNOWN&&b.required==77);CHECK(df_budget(frames,3,256,0,1,&b)==DF_UNKNOWN&&b.required==77);frames[0].software=UINT32_MAX;CHECK(df_budget(frames,3,256,0,0,&b)==DF_RANGE&&b.required==77);
 puts("DF_PASS:validity first-fault image-identity precise-imprecise stack-model FP-budget opaque-bound");return 0;
}
