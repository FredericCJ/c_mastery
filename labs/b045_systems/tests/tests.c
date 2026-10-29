#include "systems.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DEPTH_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static int pipeline(void){
 struct s_instruction p[]={{S_LOAD,1,0,0,0},{S_ALU,2,1,0,0},{S_BRANCH,0,2,0,1},{S_ALU,3,2,0,0}};
 unsigned issue[16]={0},cycles=99;CHECK(!s_pipeline(p,4,1,issue,&cycles));
 CHECK(issue[0]==1&&issue[1]==3&&issue[2]==4&&issue[3]==7&&cycles==11);
 CHECK(!s_pipeline(p,4,0,issue,&cycles));CHECK(issue[1]==4&&issue[2]==7&&issue[3]==10&&cycles==14);
 p[0].destination=0;CHECK(!s_pipeline(p,4,1,issue,&cycles)&&issue[1]==2);
 p[3].left=8;cycles=99;issue[0]=99;CHECK(s_pipeline(p,4,1,issue,&cycles)==S_INVALID&&cycles==99&&issue[0]==99);
 CHECK(!s_pipeline(0,0,1,issue,&cycles)&&cycles==0);return 0;
}
static int i2c(void){
 struct s_si2c s={0x80,0,S_I2C_DATA};unsigned release=9;CHECK(!s_i2c_drive(&s,&release)&&release==1);
 CHECK(s_i2c_sample(&s,0,0)==S_PENDING&&s.bit==0&&s.state==S_I2C_DATA);
 CHECK(!s_i2c_sample(&s,1,0)&&s.state==S_I2C_LOST&&s.bit==0);
 CHECK(!s_i2c_drive(&s,&release)&&release==1);
 for(unsigned value=0;value<256;value++){
  struct s_si2c t={value,0,S_I2C_DATA};for(unsigned i=0;i<8;i++){
   CHECK(!s_i2c_drive(&t,&release));CHECK(release==((value>>(7-i))&1));
   CHECK(!s_i2c_sample(&t,1,release));
  }
  CHECK(t.state==S_I2C_ACK);CHECK(!s_i2c_drive(&t,&release)&&release==1);
  CHECK(!s_i2c_sample(&t,1,0)&&t.state==S_I2C_DONE);
  CHECK(s_i2c_sample(&t,1,0)==S_CONFLICT);
 }
 struct s_si2c t={0,8,S_I2C_ACK};CHECK(!s_i2c_sample(&t,1,1)&&t.state==S_I2C_NACK);return 0;
}
static int checksum(void){
 unsigned char bytes[]={1,2,3,4,5};
 for(size_t a=0;a<=5;a++)for(size_t b=a;b<=5;b++){
  struct s_checksum s={0};uint16_t out=0;CHECK(!s_checksum_add(&s,bytes,a));
  CHECK(!s_checksum_add(&s,bytes+a,b-a));CHECK(!s_checksum_add(&s,bytes+b,5-b));
  CHECK(!s_checksum_finish(&s,&out)&&out==0xf6f9);
  uint16_t again=0;CHECK(!s_checksum_finish(&s,&again)&&again==out);
 }
 struct s_checksum s={0};uint16_t out=0;CHECK(!s_checksum_finish(&s,&out)&&out==65535);
 unsigned char all[]={255,255,255,255};CHECK(!s_checksum_add(&s,all,4));CHECK(!s_checksum_finish(&s,&out)&&out==0);return 0;
}
static int paired(void){
 uint32_t a[]={100,100,100,100,100},b[]={90,91,92,93,10000};int64_t m=99;unsigned wins=99;
 CHECK(!s_paired(a,b,5,&m,&wins)&&m==-8&&wins==4);
 CHECK(!s_paired(a,b,4,&m,&wins)&&m==-9&&wins==4);
 uint32_t x=UINT32_MAX,y=0;CHECK(!s_paired(&x,&y,1,&m,&wins)&&m==-(int64_t)UINT32_MAX);
 CHECK(s_paired(a,b,0,&m,&wins)==S_INVALID&&m==-(int64_t)UINT32_MAX);return 0;
}
static int failure(void *ctx,const unsigned char*p,size_t n){
 if(ctx&&n==1)return S_UNRESOLVED;
 unsigned seen=0;
 for(size_t i=0;i<n;i++){if(p[i]==4)seen=1;if(seen&&p[i]==2)return S_FAIL;}return S_PASS;
}
static int reduce(void){
 unsigned char p[]={7,4,9,9,2,8},out[32]={99};size_t n=99;unsigned calls=99;
 CHECK(!s_reduce(p,6,failure,0,100,out,&n,&calls)&&n==2&&out[0]==4&&out[1]==2&&calls<=100);
 CHECK(failure(0,out,n)==S_FAIL);for(size_t i=0;i<n;i++){unsigned char one=out[1-i];CHECK(failure(0,&one,1)!=S_FAIL);}
 n=99;calls=99;out[0]=99;CHECK(s_reduce(p,6,failure,0,1,out,&n,&calls)==S_RANGE&&n==99&&calls==99&&out[0]==99);
 unsigned unresolved_mode=1;
 CHECK(s_reduce(p,6,failure,&unresolved_mode,100,out,&n,&calls)==S_PENDING&&n==99&&calls==99&&out[0]==99);
 CHECK(s_reduce(0,0,failure,0,100,out,&n,&calls)==S_CONFLICT);return 0;
}
static int replace(void){
 unsigned char p[]={1,2,3,4,1,2,5,1,2,3,4,5};struct s_replacement r;
 CHECK(!s_replace(p,12,3,S_FIFO,&r)&&r.faults==9);
 CHECK(!s_replace(p,12,4,S_FIFO,&r)&&r.faults==10);
 CHECK(!s_replace(p,12,3,S_OPTIMAL,&r)&&r.faults==7);
 CHECK(!s_replace(p,12,4,S_OPTIMAL,&r)&&r.faults==6);
 CHECK(!s_replace(p,12,3,S_LRU,&r)&&r.faults==10);
 CHECK(!s_replace(0,0,1,S_FIFO,&r)&&!r.faults);
 r.faults=99;CHECK(s_replace(p,12,0,S_FIFO,&r)==S_INVALID&&r.faults==99);return 0;
}
static int wait(void){
 struct s_wait s={0};CHECK(!s_wait_event(&s,1,S_LOCK));CHECK(!s_wait_event(&s,1,S_WAIT));
 CHECK(s.waiting&&!s.owner&&!s.ready);CHECK(!s_wait_event(&s,2,S_LOCK));
 CHECK(!s_wait_event(&s,2,S_PUBLISH));CHECK(!s_wait_event(&s,2,S_SIGNAL));CHECK(s.ready&&!s.waiting);
 CHECK(!s_wait_event(&s,2,S_UNLOCK));CHECK(!s_wait_event(&s,1,S_LOCK));CHECK(!s_wait_event(&s,1,S_WAIT)&&s.owner==1&&s.ready);
 /* A spurious wake changes eligibility, not predicate truth. */
 struct s_wait t={0};CHECK(!s_wait_event(&t,1,S_LOCK));CHECK(!s_wait_event(&t,1,S_WAIT));
 CHECK(!s_wait_event(&t,2,S_LOCK));CHECK(!s_wait_event(&t,2,S_SIGNAL)&&t.ready&&!t.predicate);
 CHECK(!s_wait_event(&t,2,S_UNLOCK));CHECK(!s_wait_event(&t,1,S_LOCK));CHECK(!s_wait_event(&t,1,S_WAIT)&&t.waiting&&!t.ready);return 0;
}
static int shoot(void){
 struct s_shootdown s={0};CHECK(!s_shoot_begin(&s,3));uint32_t gen=s.generation;
 CHECK(!s_shoot_ack(&s,0,gen));
 unsigned frame_label=1; /* 1=OLD synthetic owner,2=NEW synthetic owner. */
 int reclaimed=s_shoot_reclaim(&s);
 if(s.reclaimed)frame_label=2;
 /* CPU1's deliberately retained synthetic translation observes this frame. */
 printf("shootdown observation: delayed CPU sees synthetic label %u\n",frame_label);
 CHECK(frame_label==1&&reclaimed==S_PENDING&&!s.reclaimed);
 CHECK(s_shoot_ack(&s,1,gen-1)==S_CONFLICT&&s.pending==2);
 CHECK(!s_shoot_ack(&s,1,gen));CHECK(!s_shoot_reclaim(&s)&&s.reclaimed);
 CHECK(s_shoot_ack(&s,0,gen)==S_CONFLICT);CHECK(!s_shoot_begin(&s,0));CHECK(!s_shoot_reclaim(&s));
 s.generation=UINT32_MAX;CHECK(s_shoot_begin(&s,1)==S_CONFLICT);return 0;
}
static int peer(void){
 struct s_peer p={1,0,0,0,1};uint32_t epoch=99,seq=99;CHECK(!s_peer_submit(&p,&epoch,&seq)&&epoch==1&&seq==1);
 CHECK(s_peer_restart(&p,0)==S_PENDING&&p.outstanding);
 CHECK(!s_peer_restart(&p,1)&&p.epoch==2&&p.uncertain&&!p.outstanding&&!p.ready);
 CHECK(s_peer_ready(&p,1)==S_CONFLICT);CHECK(!s_peer_ready(&p,2));CHECK(!s_peer_submit(&p,&epoch,&seq)&&seq==1);
 CHECK(s_peer_ack(&p,1,1)==S_CONFLICT&&p.outstanding);CHECK(!s_peer_ack(&p,2,1)&&!p.outstanding);
 CHECK(s_peer_ack(&p,2,1)==S_CONFLICT);p.epoch=UINT32_MAX;CHECK(s_peer_restart(&p,1)==S_CONFLICT);return 0;
}
int main(int n,char**a){if(n!=2)return 2;const struct{const char*name;int(*test)(void);}cases[]={
 {"pipeline",pipeline},{"i2c",i2c},{"checksum",checksum},{"paired",paired},{"reduce",reduce},{"replace",replace},{"wait",wait},{"shoot",shoot},{"peer",peer}};
 for(size_t i=0;i<sizeof cases/sizeof*cases;i++){if(!strcmp(a[1],cases[i].name))return cases[i].test();}
 return 2;}
