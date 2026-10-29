#define _DEFAULT_SOURCE
#include "fiber_follow.h"
#include "coroutines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#define C(x) do{if(!(x)){fprintf(stderr,"FF_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
struct co_state{struct cf_fiber child;unsigned allocated,released,stage,bad;uintptr_t child_rsp,outer_rsp;};
static void inner(struct cf_fiber*f,void*v){struct co_state*s=v;unsigned char*owned=malloc(31);if(!owned){s->bad=1;return;}s->allocated++;owned[0]=29;__asm__ volatile("movq %%rsp,%0":"=r"(s->child_rsp));
 if(cf_fiber_suspend(f)!=CO_IO||owned[0]!=29)s->bad=1;free(owned);s->released++;
}
static void outer(struct cf_fiber*f,void*v){struct co_state*s=v;unsigned char*owned=malloc(19);if(!owned){s->bad=1;return;}s->allocated++;owned[0]=17;__asm__ volatile("movq %%rsp,%0":"=r"(s->outer_rsp));
 if(cf_fiber_resume(&s->child)!=CO_PENDING)s->bad=1;s->stage=1;
 if(cf_fiber_suspend(f)!=CO_IO||owned[0]!=17)s->bad=1;
 if(cf_fiber_cancel(&s->child)!=CO_OK||cf_fiber_resume(&s->child)!=CO_DONE)s->bad=1;
 free(owned);s->released++;s->stage=2;
}
static void noop(struct cf_fiber*f,void*v){(void)f;(void)v;}
static unsigned fail_unmap,fail_protect;
int __real_munmap(void*,size_t);
int __wrap_munmap(void*p,size_t n){if(fail_unmap){fail_unmap=0;errno=EIO;return -1;}return __real_munmap(p,n);}
int __real_mprotect(void*,size_t,int);
int __wrap_mprotect(void*p,size_t n,int prot){if(fail_protect){fail_protect=0;errno=EACCES;return -1;}return __real_mprotect(p,n,prot);}
int main(int argc,char**argv){
 if(argc==2&&!strcmp(argv[1],"guard")){
  struct cf_fiber f;C(cf_fiber_create(&f,65536,noop,NULL)==CO_OK);
  pid_t child=fork();C(child>=0);if(!child){*(volatile unsigned char*)f.mapping=1;_exit(0);}int status;C(waitpid(child,&status,0)==child);C(WIFSIGNALED(status)&&WTERMSIG(status)==SIGSEGV);C(cf_fiber_destroy(&f)==CO_OK);puts("FF_GUARD:isolated child SIGSEGV at PROT_NONE mapping");return 0;
 }
 struct cf_fiber failed={0};fail_protect=1;
 C(cf_fiber_create(&failed,65536,noop,NULL)==CO_IO&&!failed.mapping);
 fail_protect=1;fail_unmap=1;
 C(cf_fiber_create(&failed,65536,noop,NULL)==CO_IO&&failed.mapping&&failed.fiber.state==4);
 C(cf_fiber_resume(&failed)==CO_STATE&&cf_fiber_cancel(&failed)==CO_STATE);
 C(cf_fiber_destroy(&failed)==CO_OK&&!failed.mapping);
 struct co_state s={0};struct cf_fiber f;C(cf_fiber_create(&s.child,131072,inner,&s)==CO_OK);C(cf_fiber_create(&f,131072,outer,&s)==CO_OK);
 C(cf_fiber_resume(&f)==CO_PENDING&&s.stage==1&&s.allocated==2&&!s.released&&!s.bad);
 C(s.outer_rsp>=(uintptr_t)f.fiber.context.bottom&&s.outer_rsp<(uintptr_t)f.fiber.context.bottom+f.fiber.context.size);
 C(s.child_rsp>=(uintptr_t)s.child.fiber.context.bottom&&s.child_rsp<(uintptr_t)s.child.fiber.context.bottom+s.child.fiber.context.size);
 C(cf_fiber_destroy(&f)==CO_STATE&&cf_fiber_destroy(&s.child)==CO_STATE);
 C(cf_fiber_cancel(&f)==CO_OK&&f.cancel);C(cf_fiber_resume(&f)==CO_DONE&&s.stage==2&&s.released==2&&!s.bad);
 C(cf_fiber_destroy(&s.child)==CO_OK);void*retained=f.mapping;fail_unmap=1;
 C(cf_fiber_destroy(&f)==CO_IO&&f.mapping==retained&&f.fiber.state==4);
 C(cf_fiber_resume(&f)==CO_STATE&&cf_fiber_destroy(&f)==CO_OK);C(cf_fiber_destroy(&f)==CO_INVALID);
 puts("FF_NESTED:owned automatic cleanup completed before both stack unmaps");return 0;
}
