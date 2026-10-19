#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
static void *start(void *p){return p;}
typedef int (*Create)(pthread_t *,const pthread_attr_t *,void *(*)(void *),void *);
typedef int (*Join)(pthread_t,void **);
_Static_assert(_Generic(&pthread_create,Create:1,default:0),"POSIX_CREATE_SIGNATURE");
_Static_assert(_Generic(&pthread_join,Join:1,default:0),"POSIX_JOIN_SIGNATURE");
#ifdef TL_BAD_SIGNATURE
static int wrong(void *p){(void)p;return 0;}
_Static_assert(_Generic(&wrong,void *(*)(void *):1,default:0),"POSIX_REJECT_INTEGER_RESULT");
#endif
int main(void){Create create=pthread_create;Join join=pthread_join;
    printf("POSIX_THREADS create-link=%d join-link=%d result=pointer status=direct-error handle-bytes=%zu\n",create!=NULL,join!=NULL,sizeof(pthread_t));
    return start(NULL)!=NULL;
}
