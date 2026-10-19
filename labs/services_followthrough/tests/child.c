#include "abi.h"
#if SF_CASE>=21 && SF_CASE<=23
#include "../store/service.h"
#endif
#if SF_CASE==19 || SF_CASE==20
#include "wire.h"
#endif
static volatile unsigned image_word;
static void diagnostic(uint64_t n){char out[16];for(unsigned i=0;i<16;i++)out[i]="0123456789abcdef"[(n>>(60-4*i))&15];cmk_write(out,16);cmk_write("\n",1);}
static uint64_t queue;
static void done(unsigned n){cmk_call(CMK_EXIT,n,0,0);for(;;){}}
static void checked(int value,unsigned line){if(!value){char out[4];for(unsigned i=0;i<4;i++)out[i]="0123456789abcdef"[(line>>(12-4*i))&15];cmk_write("SF_CHILD_CHECK:",15);cmk_write(out,4);cmk_write("\n",1);done(91);}}
#define check(x) checked(!!(x),__LINE__)
static struct sf_bootstrap bootstrap(void){struct sf_bootstrap b;for(unsigned i=0;i<100000;i++){int64_t r=cmk_call(SF_BOOTSTRAP,(uintptr_t)&b,0,0);if(!r)return b;check(r==SF_FULL);cmk_call(CMK_YIELD,0,0,0);}done(91);return b;}
static int64_t receive(void){unsigned char data;struct sf_receive s={(uintptr_t)&data,1,0};int64_t r;do{r=cmk_call(SF_RECEIVE,queue,(uintptr_t)&s,0);}while(r==SF_RETRY);return r;}
static void waiter(uintptr_t ignored){(void)ignored;cmk2_thread_exit(receive()==SF_CLOSED?0:91);}
void _start(void){image_word=1;check(image_word==1);
#if SF_CASE==12
 struct sf_bootstrap b=bootstrap();queue=b.request;uint64_t threads[3];
 for(unsigned i=0;i<3;i++){
  void *s=cmk_alloc(4);
  check(s!=0);
  int64_t t=cmk_call(SF_TYPED_THREAD,(uintptr_t)waiter,(uintptr_t)s+16384,0);
  if(t<0){diagnostic((uint64_t)t);diagnostic((uint64_t)cmk_call(SF_INFO,4,0,0));}
  check(t>0);
  threads[i]=(uint64_t)t;
 }
 check(receive()==SF_CLOSED);for(unsigned i=0;i<3;i++)check(cmk_call(SF_TYPED_JOIN,threads[i],0,0)==0);done(7);
#elif SF_CASE==13
 check(cmk_call(CMK_BLOCK_COUNT,65537,0,0)<0);check(!cmk_call(CMK2_SET_AFFINITY,2,0,0));for(;;)image_word++;
#elif SF_CASE==14
 struct sf_bootstrap b=bootstrap();
 if(b.role==1){for(unsigned i=0;i<100000;i++){int64_t r=cmk_call(SF_INFO,3,0,0);check(r==0||r==1);if(r==1)done(7);cmk_call(CMK_YIELD,0,0,0);}done(91);}
 if(b.role==3)check(cmk_call(CMK_BLOCK_COUNT,65537,0,0)>0);
 if(b.role==4){const char name[]="spawn";check(cmk_call(CMK_BLOCK_COUNT,65537,0,0)<0);check(cmk_call(SF_TYPED_SPAWN,(uintptr_t)name,5,1)==CMK2_EPERM);}
 done(7);
#elif SF_CASE==16
 struct sf_bootstrap b=bootstrap();
 if(b.role==1){while(cmk_call(SF_INFO,3,0,0)!=1)cmk_call(CMK_YIELD,0,0,0);done(7);}
 if(b.role==2){volatile unsigned char *p=(volatile unsigned char *)(uintptr_t)cmk_call(CMK2_RESERVE_ZERO,1,0,0);check((uintptr_t)p>=CMK_USER_BASE);check(!cmk_call(SF_FAULT_CONTROL,0,0,0));*p=1;}
 if(b.role==3){volatile unsigned char *p=(volatile unsigned char *)(uintptr_t)CMK_USER_LIMIT;*p=1;}
 if(b.role==4){volatile unsigned char *p=(volatile unsigned char *)(uintptr_t)UINT64_C(0x100000);*p=1;}
 if(b.role==5){unsigned char *p=cmk_alloc(1);check(p!=0);p[0]=0xc3;((void (*)(void))(uintptr_t)p)();}
 done(91);
#elif SF_CASE==18
 struct sf_bootstrap b=bootstrap();
 if(!b.role){const char name[]="child";uint64_t child=(uint64_t)cmk_call(SF_TYPED_SPAWN,(uintptr_t)name,5,1);check((int64_t)child>0);struct sf_grant g={child,1,0};int64_t mail=cmk_call(SF_GRANT,b.request,(uintptr_t)&g,0);g.rights=2;int64_t gate=cmk_call(SF_GRANT,b.reply,(uintptr_t)&g,0);check(mail>0&&gate>0);struct sf_bootstrap next={(uint64_t)mail,(uint64_t)gate,1,1,0};check(!cmk_call(SF_BIND,child,(uintptr_t)&next,0));for(;;)cmk_call(CMK_YIELD,0,0,0);}
 void *page=cmk_alloc(1);check(page!=0);struct cmk2_block_request request={0,0,(uintptr_t)page};int64_t ticket=cmk_call(CMK2_BLOCK_SUBMIT,65537,(uintptr_t)&request,0);check(ticket>0);uint64_t self=(uint64_t)cmk_call(SF_INFO,0,0,0);struct sf_span span={(uintptr_t)&self,sizeof self};check(!cmk_call(SF_SEND,b.request,(uintptr_t)&span,0));queue=b.reply;check(receive()==SF_CLOSED);done(0);
#elif SF_CASE==19
 struct sf_bootstrap b=bootstrap();struct sfv_wire w={0};w.capacity=SFV_RESULT;w.request.session=1;w.request.id=1;w.request.fuel=7;w.request.gc=0;w.request.length=3;struct sf_span span={(uintptr_t)&w,sizeof w};check(!cmk_call(SF_SEND,b.request,(uintptr_t)&span,0));struct sfv_response response;struct sf_receive out={(uintptr_t)&response,sizeof response,0};int64_t r;do{r=cmk_call(SF_RECEIVE,b.reply,(uintptr_t)&out,0);}while(r==SF_RETRY);check(r==sizeof response&&response.status==CVM_VERIFY&&response.id==1);
 CvmProgram p={0};p.count=1;p.length=3;p.code[0]=OP_JMP;p.functions[0].length=3;size_t length=0;check(!cvm_encode(&p,w.request.image,sizeof w.request.image,&length));w.request.length=(uint32_t)length;check(!cmk_call(SF_SEND,b.request,(uintptr_t)&span,0));do{r=cmk_call(SF_RECEIVE,b.reply,(uintptr_t)&out,0);}while(r==SF_RETRY);check(r==sizeof response&&response.status==CVM_FUEL&&!response.length&&response.id==1);done(7);
#elif SF_CASE==20
 struct sf_bootstrap b=bootstrap();uint64_t reply=(uint64_t)cmk_call(SF_CHANNEL,0,0,0);check((int64_t)reply>0);struct sf_grant g={b.session,1,0};uint64_t handles[2];handles[0]=(uint64_t)cmk_call(SF_GRANT,reply,(uintptr_t)&g,0);g.recipient=SF_PROCESS_TAG+65537;g.rights=3;handles[1]=(uint64_t)cmk_call(SF_GRANT,reply,(uintptr_t)&g,0);check((int64_t)handles[0]>0&&(int64_t)handles[1]>0);struct sf_span message={(uintptr_t)handles,sizeof handles};check(!cmk_call(SF_SEND,b.reply,(uintptr_t)&message,0));queue=reply;check(receive()==1);check(!cmk_call(SF_CHANNEL_CLOSE,reply,0,0));
 struct sfv_wire w={0};w.capacity=SFV_RESULT;w.request.session=1;w.request.id=1;w.request.fuel=7;CvmProgram p={0};p.count=1;p.length=6;p.code[0]=OP_INT;p.code[1]=42;p.code[5]=OP_RET;p.functions[0].length=6;size_t length=0;check(!cvm_encode(&p,w.request.image,sizeof w.request.image,&length));w.request.length=(uint32_t)length;message=(struct sf_span){(uintptr_t)&w,sizeof w};check(!cmk_call(SF_SEND,b.request,(uintptr_t)&message,0));done(7);
#elif SF_CASE==21
 struct sf_bootstrap b=bootstrap();struct sfs_request q={0};q.session=1;q.id=1;q.operation=SFS_PUT;q.client=2;q.epoch=1;q.klen=5;q.vlen=4;q.key[0]='r';q.key[1]='i';q.key[2]='g';q.key[3]='h';q.key[4]='t';q.value[0]=9;struct sf_span span={(uintptr_t)&q,sizeof q};check(!cmk_call(SF_SEND,b.request,(uintptr_t)&span,0));struct sfs_reply reply;struct sf_receive out={(uintptr_t)&reply,sizeof reply,0};int64_t r;do{r=cmk_call(SF_RECEIVE,b.reply,(uintptr_t)&out,0);}while(r==SF_RETRY);check(r==sizeof reply&&!reply.status&&reply.session==1&&reply.id==1);done(7);
#else
 done(7);
#endif
}
