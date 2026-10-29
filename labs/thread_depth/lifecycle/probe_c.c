#include <stdio.h>
#if defined(__STDC_NO_THREADS__) && __STDC_NO_THREADS__ == 1
int main(void){puts("C_THREADS_UNAVAILABLE_BY_MACRO");return 77;}
#else
#include <threads.h>
static int start(void *p){(void)p;return 3;}
typedef int (*tw_create_fn)(thrd_t *,thrd_start_t,void *);
typedef int (*tw_join_fn)(thrd_t,int *);
_Static_assert(_Generic(&thrd_create,tw_create_fn:1,default:0),"C_CREATE_SIGNATURE");
_Static_assert(_Generic(&thrd_join,tw_join_fn:1,default:0),"C_JOIN_SIGNATURE");
#ifdef TL_BAD_SIGNATURE
static void *wrong(void *p){return p;}
_Static_assert(_Generic(&wrong,thrd_start_t:1,default:0),"C_REJECT_POINTER_RESULT");
#endif
int main(void){thrd_start_t f=start;tw_create_fn create=thrd_create;tw_join_fn join=thrd_join;
    printf("C_THREADS header=available create-link=%d join-link=%d result=int thrd_success=%d thrd_error=%d thrd_nomem=%d handle-bytes=%zu version=%ld\n",create!=NULL,join!=NULL,thrd_success,thrd_error,thrd_nomem,sizeof(thrd_t),(long)__STDC_VERSION__);
    return f(NULL)!=3;
}
#endif
