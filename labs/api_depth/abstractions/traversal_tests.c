#include "traversal.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TRAVERSAL-CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
typedef struct{unsigned values[32],count,total;AtAction action;AtPush *host;AcEpochOwner *owner;unsigned nested,errors;}Visit;
static AtAction visit(void *ctx,unsigned char value) {
 Visit *v=ctx;if(v->count<32)v->values[v->count]=value;++v->count;v->total+=value;
 if(v->nested) {
  AtResult r=at_push(v->host,v->owner,255,visit,v);
  if(r!=AT_END&&r!=AT_FULL)++v->errors;
 }
 return v->action;
}
static AcEpochOwner owner(size_t count) {
 AcEpochOwner o={0};o.live=1;o.bytes.generation=3;o.bytes.length=count;
 for(size_t i=0;i<count;++i)o.bytes.data[i]=(uint8_t)(2*(i+1));
 return o;
}
int main(void) {
 AcEpochOwner o=owner(4);AtCursor cursor;unsigned char value=99;
 CHECK(at_begin(&o,5,&cursor)==AT_ITEM);
 CHECK(at_next(&cursor,&value)==AT_ITEM&&value==2);
 CHECK(at_next(&cursor,&value)==AT_ITEM&&value==4);
 CHECK(at_next(&cursor,&value)==AT_STOP&&value==4&&at_next(&cursor,&value)==AT_STOP);
 CHECK(at_begin(&o,255,&cursor)==AT_ITEM&&at_next(&cursor,&value)==AT_ITEM);
 CHECK(ac_epoch_update(&o,1,7)==AC_OK&&at_next(&cursor,&value)==AT_ITEM&&value==7);
 CHECK(ac_epoch_reset(&o)==AC_OK&&at_next(&cursor,&value)==AT_STALE&&value==7);
 o=owner(1);CHECK(at_begin(&o,255,&cursor)==AT_ITEM);o.live=0;
 CHECK(at_next(&cursor,&value)==AT_STALE&&value==7);
 unsigned pairs=0;
 for(size_t n=0;n<=4;++n)for(unsigned limit=0;limit<=9;++limit) {
  o=owner(n);Visit pull={0},push={0};AtPush host={0};
  CHECK(at_begin(&o,limit,&cursor)==AT_ITEM);AtResult result;
  while((result=at_next(&cursor,&value))==AT_ITEM)(void)visit(&pull,value);
  CHECK(at_push(&host,&o,limit,visit,&push)==result);
  CHECK(pull.count==push.count&&pull.total==push.total&&!memcmp(pull.values,push.values,sizeof pull.values));
  unsigned expected=(unsigned)n<limit/2?(unsigned)n:limit/2;
  CHECK(pull.count==expected&&pull.total==expected*(expected+1));
  CHECK(host.depth==0&&host.max_depth==1&&host.visits==push.count);
  CHECK(at_next(&cursor,&value)==result);++pairs;
 }
 o=owner(4);Visit rejected={0};rejected.action=AT_REMOVE;AtPush host={0};
 CHECK(at_push(&host,&o,255,visit,&rejected)==AT_DENIED&&rejected.count==1&&o.bytes.length==4);
 Visit stop={0};stop.action=AT_STOP_NOW;CHECK(at_push(&host,&o,255,visit,&stop)==AT_STOP&&stop.count==1);
 AtDeferred d={0};Visit deferred={0};
 CHECK(at_pump(&d,1)==AT_INVALID&&at_cancel(&d)==AT_INVALID);
 CHECK(at_defer(&d,&o,255,visit,&deferred)==AT_ITEM);
 CHECK(at_pump(&d,0)==AT_ITEM&&!deferred.count);
 CHECK(at_pump(&d,1)==AT_ITEM&&deferred.count==1);
 CHECK(at_pump(&d,2)==AT_ITEM&&deferred.count==3);
 CHECK(at_cancel(&d)==AT_CANCELLED&&at_pump(&d,9)==AT_CANCELLED&&deferred.count==3&&!d.context);
 CHECK(at_defer(&d,&o,255,visit,&deferred)==AT_ITEM);
 CHECK(ac_epoch_reset(&o)==AC_OK&&at_pump(&d,1)==AT_STALE&&deferred.count==3);
 o=owner(1);Visit nesting={0};nesting.host=&host;nesting.owner=&o;nesting.nested=1;
 CHECK(at_push(&host,&o,255,visit,&nesting)==AT_END&&host.max_depth==3&&host.visits==3&&!nesting.errors);
 AtNode nodes[7]={{1,2,3},{2,4,5},{3,6,7},{4,0,0},{5,0,0},{6,0,0},{7,0,0}};
 const unsigned char expected[]={1,2,4,5,3,6,7};unsigned trees=0;
 for(unsigned depth=0;depth<=3;++depth)for(size_t cap=0;cap<=7;++cap) {
  unsigned char a[7]={0},b[7]={0};size_t na=99,nb=99;
  AtResult ra=at_recursive(nodes,7,1,depth,a,cap,&na),rb=at_continuations(nodes,7,1,depth,b,cap,&nb);
  CHECK(ra==rb&&na==nb&&!memcmp(a,b,na));
  CHECK(ra==(depth==3&&cap==7?AT_END:AT_FULL));
  CHECK(!memcmp(a,expected,na));++trees;
 }
 nodes[3].left=99;unsigned char a[7],b[7];size_t na,nb;
 CHECK(at_recursive(nodes,7,1,3,a,7,&na)==AT_INVALID&&at_continuations(nodes,7,1,3,b,7,&nb)==AT_INVALID&&na==nb);
 nodes[3].left=1;
 CHECK(at_recursive(nodes,7,1,3,a,7,&na)==AT_FULL&&at_continuations(nodes,7,1,3,b,7,&nb)==AT_FULL&&na==nb);
 CHECK(at_recursive(0,0,0,0,0,0,&na)==AT_END&&na==0&&at_continuations(0,0,0,0,0,0,&nb)==AT_END&&nb==0);
 printf("TRAVERSAL-PASS:pull_push_pairs=%u;tree_bounds=%u;depth=3;cursor=%zu;push=%zu;deferred=%zu\n",pairs,trees,sizeof(AtCursor),sizeof(AtPush),sizeof(AtDeferred));return 0;
}
