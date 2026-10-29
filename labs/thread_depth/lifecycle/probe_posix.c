#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
static void *start(void *p){return p;}
typedef int (*tw_create_fn)(pthread_t *,const pthread_attr_t *,void *(*)(void *),void *);
typedef int (*tw_join_fn)(pthread_t,void **);
_Static_assert(_Generic(&pthread_create,tw_create_fn:1,default:0),"POSIX_CREATE_SIGNATURE");
_Static_assert(_Generic(&pthread_join,tw_join_fn:1,default:0),"POSIX_JOIN_SIGNATURE");
#ifdef TL_BAD_SIGNATURE
static int wrong(void *p){(void)p;return 0;}
_Static_assert(_Generic(&wrong,void *(*)(void *):1,default:0),"POSIX_REJECT_INTEGER_RESULT");
#endif
int main(void){tw_create_fn create=pthread_create;tw_join_fn join=pthread_join;
    printf("POSIX_THREADS create-link=%d join-link=%d result=pointer status=direct-error handle-bytes=%zu\n",create!=NULL,join!=NULL,sizeof(pthread_t));
    return start(NULL)!=NULL;
}
