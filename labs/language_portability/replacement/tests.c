#include "replacement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"LP_CHECK %s:%d %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
typedef struct {void *live[2];unsigned fail,calls,releases,order,fault;LeOwner *observed;unsigned empty;} Track;
static Track t;
static void cleanup(void){free(t.live[0]);free(t.live[1]);t=(Track){0};}
static void *acquire(void *c,unsigned id,size_t n){Track *s=c;s->calls++;if(s->fail==id)return NULL;void *p=malloc(n);if(!p)abort();s->live[id-1]=p;return p;}
static void release(void *c,unsigned id,void *p,size_t n){Track *s=c;(void)n;if(id<1 || id>2 || s->live[id-1]!=p){s->fault++;return;}s->releases++;s->order=s->order*10+id;if(s->observed)s->empty+=s->observed->data==NULL && s->observed->size==0 && s->observed->id==0;s->live[id-1]=NULL;free(p);}
static LeOps ops(void){return (LeOps){&t,acquire,release};}
static int failure(void){LeOwner out={0};t.fail=2;CHECK(le_run(ops(),0,&out)==LE_NOMEM && t.calls==2 && t.releases==1 && t.order==1 && !out.data);puts("LP failure validated");cleanup();return 0;}
static int order(void){LeOwner out={0};CHECK(le_run(ops(),0,&out)==LE_OK && t.order==21 && t.releases==2 && !out.data);puts("LP order validated");cleanup();return 0;}
static int transfer(void){for(unsigned id=1;id<=2;id++){LeOwner out={0};CHECK(le_run(ops(),id,&out)==LE_OK);/* Observe independent release ledger before using transferred pointer. */CHECK(t.releases==1 && t.order==3-id);CHECK(out.data && out.id==id && out.data[0]==(id==1?1:17));t.observed=&out;le_drop(&out);CHECK(!out.data && !out.size && !out.id && t.empty==1 && t.releases==2);le_drop(&out);CHECK(t.releases==2 && !t.fault);cleanup();}puts("LP transfer validated");return 0;}
static int oracle(void){unsigned fail,tr;while(scanf("%u %u",&fail,&tr)==2){LeOwner out={0};t.fail=fail;LeStatus s=le_run(ops(),tr,&out);unsigned id=out.id;printf("%d %u %u %u %u",s,t.calls,t.releases,t.order,id);le_drop(&out);printf(" %u %u %u\n",t.releases,t.order,t.fault);cleanup();}return 0;}
int main(int argc,char **argv){CHECK(argc==2);atexit(cleanup);if(!strcmp(argv[1],"failure"))return failure();if(!strcmp(argv[1],"order"))return order();if(!strcmp(argv[1],"transfer"))return transfer();if(!strcmp(argv[1],"oracle"))return oracle();if(!strcmp(argv[1],"metrics")){printf("{\"owner_bytes\":%zu,\"resource_bytes\":24,\"maximum_acquisitions\":2}\n",sizeof(LeOwner));return 0;}return 2;}
