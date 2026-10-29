#include "transfers.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SX_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static void revoke(struct sx_map*m,void*v){unsigned page=*(unsigned*)v;(void)sx_remap(m,page,1,1,0,88);}
int main(void){struct sx_map m={0};struct sx_ledger l={0};struct sx_alias a,b;struct sx_hotplug w={0};struct sx_copy_result result;unsigned page=1;size_t n=99;unsigned char p[4]={1,2,3,4};
 for(unsigned i=0;i<4;i++)CHECK(sx_remap(&m,i,1,1,1,55)==B5_OK);
 CHECK(sx_copy(&m,14,p,4,revoke,&page)==B5_CONFLICT&&m.pages[0].bytes[14]==55&&m.pages[1].bytes[0]==88);
 CHECK(sx_partial(&m,14,p,4,&result)==B5_OK&&result.copied==2&&result.status==B5_CONFLICT&&m.pages[0].bytes[14]==1);
 CHECK(sx_allocate(&l,1,10,0,1,&a)==B4_OK&&l.ledger.live==10);
 CHECK(sx_resize(&l,a,0,20,0,&b)==B4_OK&&b.generation==a.generation&&l.ledger.live==10);
 CHECK(sx_resize(&l,a,0,20,1,&b)==B4_OK&&l.ledger.live==20);
 CHECK(sx_lookup(&l,a,0,&n)==B4_STATE&&n==99);
 CHECK(sx_transfer(&l,b,0,1)==B4_OK&&l.attributed[0]==0&&l.attributed[1]==20&&l.ledger.live==20);
 CHECK(sx_release(&l,b,1,0)==B4_STATE&&l.ledger.live==20);
 CHECK(sx_release(&l,b,1,1)==B4_OK&&l.ledger.live==0);
 CHECK(sx_release(&l,b,1,1)==B4_STATE);
 CHECK(sx_online(&w,0)==S_OK&&sx_online(&w,1)==S_OK&&sx_begin(&w,3)==S_OK);
 CHECK(sx_ack(&w,0,1,1)==S_OK&&sx_offline_request(&w,1,1)==S_OK);
 CHECK(sx_reclaim(&w)==S_PENDING&&w.request.pending==2&&w.online==3);
 CHECK(sx_offline_quiescent(&w,1,1)==S_OK&&sx_reclaim(&w)==S_OK&&w.online==1);
 CHECK(sx_online(&w,1)==S_OK&&w.incarnation[1]==2&&sx_begin(&w,3)==S_OK);
 CHECK(sx_ack(&w,1,1,2)==S_CONFLICT&&sx_offline_quiescent(&w,1,1)==S_CONFLICT);
 puts("SX_PASS version_copy partial_count allocation_transfer hotplug_quiescence");return 0;}
