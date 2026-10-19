#include "service.h"
#include <string.h>
static struct sfv_service service;
static struct sfv_request request;
#define VCHECK(x) do{if(!(x))return __LINE__;}while(0)
static CvmProgram program(unsigned kind,int32_t value){
 CvmProgram p={0};p.count=1;
 if(kind==1){p.code[0]=OP_JMP;p.length=3;}
 else {p.code[p.length++]=OP_INT;for(unsigned i=0;i<4;i++)p.code[p.length++]=(uint8_t)((uint32_t)value>>(8*i));
  if(kind>=2){p.code[p.length++]=OP_INT;p.code[p.length++]=2;p.length+=3;p.code[p.length++]=OP_PAIR;}
  if(kind==3){for(unsigned i=0;i<10;i++){p.code[p.length++]=OP_INT;p.code[p.length++]=(uint8_t)i;p.length+=3;p.code[p.length++]=OP_PAIR;}}
  p.code[p.length++]=OP_RET;
 }p.functions[0].length=p.length;return p;
}
static int make(unsigned kind,int32_t value,uint64_t session,uint64_t id){CvmProgram p=program(kind,value);memset(&request,0,sizeof request);request.session=session;request.id=id;request.fuel=100;request.gc=100;size_t n=0;int r=cvm_encode(&p,request.image,sizeof request.image,&n);request.length=(uint32_t)n;return r;}
static int finish(unsigned c){int r=SFV_RUNNING;for(unsigned i=0;i<101&&r==SFV_RUNNING;i++)r=sfv_step(&service,c);return r;}
int sfv_tests(void){
 memset(&service,0,sizeof service);VCHECK(!sfv_open(&service,0,1)&&!sfv_open(&service,1,1));
 VCHECK(!make(0,42,1,1));request.image[0]^=1;VCHECK(sfv_submit(&service,0,&request)==CVM_VERIFY&&service.slots[0].high==0);request.image[0]^=1;
 VCHECK(sfv_cancel(&service,0,1,1)==SFV_STALE);VCHECK(sfv_submit(&service,0,&request)==SFV_RUNNING);request.image[0]^=1;VCHECK(sfv_submit(&service,0,&request)==SFV_CONFLICT);VCHECK(!finish(0));
 unsigned char out[SFV_RESULT];memset(out,0xa5,sizeof out);size_t length=99;unsigned status=99;VCHECK(sfv_reply(&service,0,1,1,out,4,&length,&status)==SFV_CAPACITY&&length==99&&status==99&&out[0]==0xa5);
 VCHECK(!sfv_reply(&service,0,1,1,out,sizeof out,&length,&status)&&!status&&length==5&&out[0]==CVM_INT&&out[1]==42);
 VCHECK(!make(1,0,1,2));request.fuel=7;VCHECK(sfv_submit(&service,0,&request)==SFV_RUNNING&&finish(0)==CVM_FUEL);VCHECK(!sfv_reply(&service,0,1,2,out,sizeof out,&length,&status)&&status==CVM_FUEL&&!length);
 VCHECK(!make(2,17,1,3));VCHECK(sfv_submit(&service,0,&request)==SFV_RUNNING&&!finish(0));VCHECK(!make(2,19,1,1));VCHECK(sfv_submit(&service,1,&request)==SFV_RUNNING&&!finish(1));
 struct sfv_handle a,b;CvmValue v=cvm_int(99);VCHECK(!sfv_handle(&service,0,&a)&&!sfv_handle(&service,1,&b));VCHECK(a.value.integer==b.value.integer&&a.value.generation==b.value.generation);VCHECK(sfv_field(&service,1,&a,0,&v)==SFV_FOREIGN&&v.integer==99);VCHECK(!sfv_field(&service,1,&b,0,&v)&&v.integer==19);
 VCHECK(sfv_cancel(&service,0,1,3)==SFV_CANCELLED&&sfv_reply(&service,0,1,3,out,sizeof out,&length,&status)==SFV_CANCELLED);VCHECK(!sfv_reply(&service,1,1,1,out,sizeof out,&length,&status));
 for(unsigned cut=0;cut<=2;cut++){VCHECK(!make(0,23,1,4+cut));VCHECK(sfv_submit(&service,0,&request)==SFV_RUNNING);for(unsigned i=0;i<cut;i++)(void)sfv_step(&service,0);VCHECK(sfv_cancel(&service,0,1,4+cut)==SFV_CANCELLED&&sfv_reply(&service,0,1,4+cut,out,sizeof out,&length,&status)==SFV_CANCELLED);}
 VCHECK(!make(3,1,1,7));VCHECK(sfv_submit(&service,0,&request)==SFV_RUNNING&&finish(0)==SFV_CAPACITY);VCHECK(!sfv_reply(&service,0,1,7,out,sizeof out,&length,&status)&&status==SFV_CAPACITY&&!length);
 VCHECK(!make(0,1,1,UINT64_MAX));VCHECK(sfv_submit(&service,0,&request)==SFV_RUNNING&&!finish(0));VCHECK(!sfv_reply(&service,0,1,UINT64_MAX,out,sizeof out,&length,&status));request.id=1;VCHECK(sfv_submit(&service,0,&request)==SFV_STALE);request.id=0;VCHECK(sfv_submit(&service,0,&request)==SFV_ARGUMENT);
 VCHECK(!sfv_open(&service,0,2));VCHECK(sfv_reply(&service,0,1,UINT64_MAX,out,sizeof out,&length,&status)==SFV_STALE&&sfv_field(&service,0,&a,0,&v)==SFV_FOREIGN);VCHECK(sfv_open(&service,0,1)==SFV_STALE);
 /*512 independently calculated signed integer results and wire encodings. */
 for(unsigned i=0;i<512;i++){int32_t want=(int32_t)i-256;VCHECK(!make(0,want,2,i+1));VCHECK(sfv_submit(&service,0,&request)==SFV_RUNNING&&!finish(0));VCHECK(!sfv_reply(&service,0,2,i+1,out,sizeof out,&length,&status)&&!status&&length==5);uint32_t got=(uint32_t)out[1]|(uint32_t)out[2]<<8|(uint32_t)out[3]<<16|(uint32_t)out[4]<<24;VCHECK(got==(uint32_t)want);}
 return 0;
}
#ifdef SFV_MAIN
#include <stdio.h>
int main(void){int r=sfv_tests();if(r)printf("SFV_CHECK:%d\n",r);else puts("PASS sfv:512-results cancellation capacity identity fuel verifier");return !!r;}
#endif
