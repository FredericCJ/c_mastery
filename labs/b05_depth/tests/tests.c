#include "b05.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DEPTH_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static int verify(void){
 unsigned peak=99;struct b5_instruction good[]={{B5_PUSH,0},{B5_PUSH,0},{B5_ADD,0},{B5_HALT,0}};
 CHECK(!b5_verify(good,4,&peak)&&peak==2);
 struct b5_instruction join[]={{B5_PUSH,0},{B5_BRANCH,4},{B5_PUSH,0},{B5_JUMP,4},{B5_PUSH,0},{B5_HALT,0}};
 peak=99;CHECK(b5_verify(join,6,&peak)==B5_CONFLICT&&peak==99);
 struct b5_instruction loop[]={{B5_JUMP,0}};CHECK(!b5_verify(loop,1,&peak)&&peak==0);
 struct b5_instruction unreachable[]={{B5_PUSH,0},{B5_HALT,0},{99,0}};
 CHECK(b5_verify(unreachable,3,&peak)==B5_INVALID);
 struct b5_instruction under[]={{B5_POP,0}};CHECK(b5_verify(under,1,&peak)==B5_CONFLICT);return 0;
}
static int recover(void){
 struct b5_redo r[3]={{6,12,0,1},{7,18,0,1},{8,23,0,1}};
 for(unsigned i=0;i<3;i++)r[i].checksum=b5_checksum(r[i].generation,r[i].value);
 for(size_t n=0;n<=3;n++){uint32_t out=99;CHECK(!b5_recover(5,10,r,n,&out));CHECK(out==(n?r[n-1].value:10));}
 uint32_t out=99;r[1].checksum^=1;CHECK(b5_recover(5,10,r,3,&out)==B5_CONFLICT&&out==99);r[1].checksum^=1;
 r[1].commit=0;CHECK(!b5_recover(5,10,r,3,&out)&&out==12);r[1].commit=1;
 CHECK(!b5_recover(7,18,r,3,&out)&&out==23);
 r[1].generation=9;r[1].checksum=b5_checksum(9,18);out=99;
 CHECK(b5_recover(5,10,r,3,&out)==B5_CONFLICT&&out==99);
 /* Original weak-checksum teaching target: paired corruption turns a new
  * record into apparently checkpointed history without changing checksum. */
 struct b5_redo weak={9,200,0,1};weak.checksum=b5_checksum(9,200);
 CHECK(!b5_recover(8,100,&weak,1,&out)&&out==200);
 weak.generation^=1;weak.value^=1;
 CHECK(weak.checksum==b5_checksum(weak.generation,weak.value));
 CHECK(!b5_recover(8,100,&weak,1,&out)&&out==100);
 return 0;
}
static int copy(void){
 struct b5_page p[4]={0};unsigned char input[64];for(unsigned i=0;i<64;i++)input[i]=(unsigned char)(i+1);
 for(unsigned i=0;i<4;i++)p[i].present=p[i].user=p[i].writable=1;
 CHECK(!b5_copy_to_user(p,14,input,4)&&p[0].bytes[14]==1&&p[1].bytes[1]==4);
 p[1].writable=0;unsigned char old=p[0].bytes[15];
 CHECK(b5_copy_to_user(p,15,input,2)==B5_CONFLICT&&p[0].bytes[15]==old);
 CHECK(!b5_copy_to_user(p,64,0,0));CHECK(b5_copy_to_user(p,64,input,1)==B5_RANGE);
 CHECK(b5_copy_to_user(p,SIZE_MAX,input,2)==B5_RANGE);
 p[1].writable=1;p[1].user=0;CHECK(b5_copy_to_user(p,16,input,1)==B5_CONFLICT);return 0;
}
static int timer(void){
 struct b5_timer t={0};int expired=99;CHECK(!b5_timer_arm(&t,UINT32_MAX-2,5));
 CHECK(!b5_timer_poll(&t,1,&expired)&&!expired&&t.remaining==1);
 CHECK(!b5_timer_poll(&t,2,&expired)&&expired&&t.remaining==0);
 CHECK(!b5_timer_poll(&t,3,&expired)&&expired);
 CHECK(b5_timer_poll(&t,2,&expired)==B5_RANGE&&t.last==3);
 CHECK(b5_timer_arm(&t,0,UINT32_C(0x80000000))==B5_RANGE&&t.last==3);
 CHECK(!b5_timer_arm(&t,10,0));CHECK(!b5_timer_poll(&t,10,&expired)&&expired);return 0;
}
static int response(void){
 struct b5_task h[]={{2,5,0},{1,7,1}};uint32_t out=99;
 CHECK(!b5_response(3,1,30,h,2,&out)&&out==10);
 out=99;CHECK(b5_response(3,1,9,h,2,&out)==B5_RANGE&&out==99);
 CHECK(!b5_response(3,1,10,0,0,&out)&&out==4);
 h[0].period=0;CHECK(b5_response(3,1,30,h,2,&out)==B5_INVALID);return 0;
}
static int grace(void){
 struct b5_grace g={0};uint32_t first,newcomer;int done=99;
 CHECK(!b5_read_enter(&g,0,&first));CHECK(!b5_grace_begin(&g));
 CHECK(!b5_read_enter(&g,1,&newcomer));CHECK(!b5_grace_done(&g,&done)&&!done);
 CHECK(!b5_read_exit(&g,0,first));CHECK(!b5_grace_done(&g,&done)&&done);
 CHECK(g.active[1]==newcomer);CHECK(b5_read_exit(&g,1,first)==B5_CONFLICT);
 CHECK(!b5_read_exit(&g,1,newcomer));CHECK(!b5_grace_begin(&g));
 CHECK(!b5_read_enter(&g,0,&newcomer));CHECK(!b5_grace_done(&g,&done)&&done);
 CHECK(!b5_read_exit(&g,0,newcomer));g.next_ticket=UINT32_MAX;
 CHECK(b5_read_enter(&g,0,&newcomer)==B5_RANGE);return 0;
}
static int request(void){
 struct b5_request r={1,B5_IDLE,0,0};CHECK(!b5_request(&r,B5_SUBMIT,1,0));
 CHECK(!b5_request(&r,B5_CANCEL,1,0));CHECK(r.state==B5_CANCEL_REQUESTED&&!r.terminal_count);
 CHECK(b5_request(&r,B5_REAP,1,0)==B5_CONFLICT);
 CHECK(!b5_request(&r,B5_COMPLETE,1,2)&&r.terminal_count==1);
 CHECK(b5_request(&r,B5_COMPLETE,1,1)==B5_CONFLICT&&r.result==2);
 CHECK(!b5_request(&r,B5_REAP,1,0)&&r.generation==2);
 CHECK(b5_request(&r,B5_COMPLETE,1,0)==B5_CONFLICT);
 CHECK(!b5_request(&r,B5_SUBMIT,2,0));CHECK(!b5_request(&r,B5_COMPLETE,2,0));
 CHECK(b5_request(&r,B5_CANCEL,2,0)==B5_CONFLICT);
 r.generation=UINT32_MAX;CHECK(!b5_request(&r,B5_REAP,UINT32_MAX,0)&&r.state==B5_RETIRED);return 0;
}
int main(int n,char**a){if(n!=2)return 2;
 const struct{const char*n;int(*f)(void);}c[]={{"verify",verify},{"recover",recover},{"copy",copy},{"timer",timer},{"response",response},{"grace",grace},{"request",request}};
 for(size_t i=0;i<sizeof c/sizeof*c;i++) { if(!strcmp(a[1],c[i].n)) return c[i].f(); }
 return 2;}
