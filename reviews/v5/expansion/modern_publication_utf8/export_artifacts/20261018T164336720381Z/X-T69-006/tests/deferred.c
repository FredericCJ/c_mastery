#include "dispatch_deferred.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DS_CHECK:retained_closure:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
typedef struct {int value;unsigned calls,destroys,releases;} Environment;
static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t condition=PTHREAD_COND_INITIALIZER;
static unsigned entered,released;
static void destroy(void *p){Environment *e=p;++e->destroys;}
static void release(void *p){Environment *e=p;++e->releases;}
static int body(void *p,int arg,int *out){Environment *e=p;CHECK(!e->destroys && !e->releases);++e->calls;
 if(arg==99){(void)pthread_mutex_lock(&mutex);entered=1;(void)pthread_cond_broadcast(&condition);while(!released)(void)pthread_cond_wait(&condition,&mutex);(void)pthread_mutex_unlock(&mutex);}
 *out=e->value+arg;return arg<0?DS_FAILED:DS_OK;
}
static void done(DSDeferred *c){(void)pthread_mutex_lock(&c->mutex);while(!c->done)(void)pthread_cond_wait(&c->condition,&c->mutex);(void)pthread_mutex_unlock(&c->mutex);}
static void scenario(unsigned choice,int emit){
 DSDeferred c;CHECK(ds_deferred_init(&c)==DS_OK);Environment env={17,0,0,0};OwShared shared={1,1,destroy,release,&env};DSDeferredToken token={0};CHECK(ds_deferred_bind(&c,&shared,body,&token)==DS_OK && shared.strong==2);DSDeferredToken copy=token;
 CHECK(ow_shared_drop(&shared)==OW_OK && shared.strong==1 && !env.destroys);CHECK(ds_deferred_destroy(&c)==DS_BUSY);
 if(choice==0){CHECK(ds_deferred_cancel(token)==DS_OK && !c.active && env.destroys==1 && env.releases==1);}
 else{
  CHECK(ds_deferred_submit(token,choice==3?99:choice==4?-1:3)==DS_OK);CHECK(ds_deferred_submit(copy,3)==DS_BUSY);
  if(choice==1)CHECK(ds_deferred_cancel(token)==DS_BUSY && shared.strong==1);
  else {CHECK(ds_deferred_start(token)==DS_OK);if(choice==3){(void)pthread_mutex_lock(&mutex);while(!entered)(void)pthread_cond_wait(&condition,&mutex);(void)pthread_mutex_unlock(&mutex);CHECK(ds_deferred_cancel(token)==DS_BUSY && shared.strong==1);int untouched=91;CHECK(ds_deferred_drain(token,&untouched)==DS_BUSY && untouched==91);(void)pthread_mutex_lock(&mutex);released=1;(void)pthread_cond_broadcast(&condition);(void)pthread_mutex_unlock(&mutex);}}
  done(&c);int out=91;int rc=ds_deferred_drain(token,&out);CHECK(rc==((choice==1 || choice==3)?DS_STALE:choice==4?DS_FAILED:DS_OK));CHECK(out==(choice==2?20:91));CHECK(!c.active && !c.job && shared.strong==0 && env.destroys==1 && env.releases==1);
 }
 CHECK(ds_deferred_start(copy)==DS_STALE && ds_deferred_cancel(copy)==DS_STALE && env.calls==(choice>1?1u:0u));
 if(emit)printf("D %u %u %u %u %u\n",choice,env.calls,env.destroys,env.releases,c.generation);
 CHECK(ds_deferred_destroy(&c)==DS_OK);
}
int main(int argc,char **argv){if(argc!=2)return 2;int emit=!strcmp(argv[1],"oracle");for(unsigned i=0;i<5;++i)scenario(i,emit);
 DSDeferred c;CHECK(ds_deferred_init(&c)==DS_OK);Environment env={17,0,0,0};OwShared shared={1,1,destroy,release,&env};DSDeferredToken token={&c,99};OwShared dead={0};
 CHECK(ds_deferred_bind(&c,&dead,body,&token)==DS_FAILED && !c.active && token.generation==99);
 shared.strong=UINT_MAX;CHECK(ds_deferred_bind(&c,&shared,body,&token)==DS_FAILED && shared.strong==UINT_MAX && !c.active && token.generation==99);shared.strong=1;
 for(unsigned i=0;i<3;++i){CHECK(ds_deferred_bind(&c,&shared,body,&token)==DS_OK);CHECK(ds_deferred_cancel(token)==DS_OK && shared.strong==1);}
 CHECK(ds_deferred_bind(&c,&shared,body,&token)==DS_FULL && shared.strong==1);CHECK(ow_shared_drop(&shared)==OW_OK && env.destroys==1 && env.releases==1);CHECK(ds_deferred_destroy(&c)==DS_OK);return 0;
}
