#include "security_extension.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define C(x) do{if(!(x)){fprintf(stderr,"B06_SECURITY_CHECK:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
struct md_context {struct mde_owner *owner;unsigned calls;void *order[2];};
static void release(void *p,void *v){struct md_context *c=p;C(c->owner->state==0&&!c->owner->first&&!c->owner->second);C(!mde_destroy(c->owner));C(c->calls<2);c->order[c->calls++]=v;}
int main(void){struct mde_object o={1,1,8,91};struct mde_event queued={1,8,77};C(mde_retire(&o)&&mde_recreate(&o,8,19));C(!mde_deliver(&o,queued)&&o.value==19);queued.generation=2;queued.owner=9;C(!mde_deliver(&o,queued));queued.owner=8;C(mde_deliver(&o,queued)&&o.value==77);C(mde_retire(&o));o.generation=UINT32_MAX;C(!mde_recreate(&o,8,0)&&!o.live);
 const unsigned char b[]={'%',0,10,0x1B,255};char out[21];memset(out,'?',sizeof out);C(!mde_log(b,5,out,20)&&out[0]=='?');C(mde_log(b,5,out,sizeof out));C(!strcmp(out,"\\x25\\x00\\x0A\\x1B\\xFF"));C(!mde_log(b,SIZE_MAX,out,sizeof out));C(mde_log(NULL,0,out,1)&&out[0]==0);
 int a=1,bv=2;struct mde_owner owner={1,&a,&bv,release,NULL};struct md_context c={&owner,0,{0}};owner.context=&c;C(mde_destroy(&owner)&&c.calls==2&&c.order[0]==&bv&&c.order[1]==&a);C(!mde_destroy(&owner));owner=(struct mde_owner){1,&a,NULL,release,&c};c.calls=0;C(mde_destroy(&owner)&&c.calls==1);puts("stale-event, binary-log, reentrant-destruction PASS");return 0;}
