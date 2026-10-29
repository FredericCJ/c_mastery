#include "models.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TM_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 struct tm_queue q={0};tm_init(&q);CHECK(tm_push(&q,7)==TA_OK&&tm_push(&q,8)==TA_OK);struct tm_queue old=q;
 CHECK(tm_push(&q,9)==TA_FULL&&memcmp(&q,&old,sizeof q)==0);
 struct tm_borrow borrow;unsigned char out=99;CHECK(tm_borrow(&q,&borrow)==TA_OK&&tm_read(borrow,&out)==TA_OK&&out==7);
 CHECK(tm_pop(&q,&out)==TA_OK&&out==7);CHECK(tm_push(&q,9)==TA_OK);CHECK(tm_pop(&q,&out)==TA_OK);CHECK(tm_push(&q,10)==TA_OK);CHECK(tm_pop(&q,&out)==TA_OK);CHECK(tm_push(&q,11)==TA_OK);
 out=99;CHECK(tm_read(borrow,&out)==TA_INVALID&&out==99);
 CHECK(tm_borrow(&q,&borrow)==TA_OK);uint64_t revision=q.revision;CHECK(tm_pop(&q,&out)==TA_OK&&q.revision==revision+1);
 CHECK(tm_clear(&q)==TA_OK&&ta_count(&q.core)==0);CHECK(tm_read(borrow,&out)==TA_INVALID);
 tm_init(&q);CHECK(tm_push(&q,12)==TA_OK);q.revision=UINT64_MAX;old=q;CHECK(tm_clear(&q)==TA_INVALID&&memcmp(&q,&old,sizeof q)==0);CHECK(tm_borrow(&q,&borrow)==TA_OK&&tm_read(borrow,&out)==TA_OK&&out==12);
 const unsigned char wire[6]={1,2,1,2,0xfe,0xdc};struct tm_packet p={0},r={0};
 CHECK(tm_decode(wire,6,&p)==TM_OK&&p.count==2&&p.values[0]==258&&p.values[1]==65244);
 CHECK(tm_reference(wire,6,0,&r)==TM_OK&&memcmp(&p,&r,sizeof p)==0);
 unsigned char noncanonical[6]={1,130,1,2,0xfe,0xdc};old=q;
 CHECK(tm_decode(noncanonical,6,&p)==TM_COUNT);CHECK(tm_reference(noncanonical,6,1,&r)==TM_OK&&r.values[0]==258);
 struct tm_packet saved=p;CHECK(tm_decode(wire,5,&p)==TM_SHORT&&memcmp(&p,&saved,sizeof p)==0);
 for(size_t split=0;split<=3;split++){
  int32_t input[3]={1,2,3},double_input[3]={2,4,6};int64_t a[3],b[3],c[3];struct tm_fir f={0},g={0},h={0};
  CHECK(tm_fir(&f,input,3,0,a)==TM_OK);CHECK(a[0]==1&&a[1]==3&&a[2]==5);
  CHECK(tm_fir(&g,double_input,3,0,b)==TM_OK);for(unsigned i=0;i<3;i++)CHECK(b[i]==2*a[i]);
  CHECK(tm_fir(&h,input,split,0,c)==TM_OK&&tm_fir(&h,input+split,3-split,0,c+split)==TM_OK);CHECK(memcmp(a,c,sizeof a)==0);
  f.previous=g.previous=0;CHECK(tm_fir(&f,input,3,1,a)==TM_OK&&tm_fir(&g,double_input,3,1,b)==TM_OK);CHECK(a[2]==4&&b[2]==4&&b[2]!=2*a[2]);
 }
 struct tm_fir f={7},saved_f=f;int32_t invalid[2]={1,1000001};int64_t outputs[2]={77,88};CHECK(tm_fir(&f,invalid,2,0,outputs)==TM_INVALID&&f.previous==saved_f.previous&&outputs[0]==77&&outputs[1]==88);
 puts("TM_PASS:borrow,clear,refusal,canonical,differential,FIR,chunk,saturation");return 0;
}
