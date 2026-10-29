#include "fault_adapter.h"
#include <stdio.h>
#include <stdlib.h>
void *__real_malloc(size_t);
void __real_free(void *);
static unsigned calls,refuse,live;
void *__wrap_malloc(size_t n){void *p;++calls;if(calls==refuse)return NULL;p=__real_malloc(n);if(p)++live;return p;}
void __wrap_free(void *p){if(p)--live;__real_free(p);}
static int equal(void *c,struct mpt_view a,struct mpt_view b){unsigned same;(void)c;return mpt_equal(a,b,MPT_ASCII_TEXT,&same)==MPT_OK?(int)same:-1;}
static uint64_t hash(void *c,struct mpt_view a){uint64_t h=0;(void)c;(void)mpt_hash(a,MPT_BYTES,&h);return h;}
int main(void){
    struct mpt_trace in={{{MPT_SET,0,4},{MPT_SET,1,4},{MPT_DROP,1,0},{MPT_SET,0,3},{MPT_SET,1,2},{MPT_CHECK,0,0}},6,0},out={0};struct mpt_laws laws={equal,hash,NULL};struct mpt_outcome result={0};unsigned attempts=99;
    refuse=1;result.first_step=99;if(mpt_replay(&in,laws,&result)!=MPT_NOMEM||result.first_step!=99||live)return 1;
    calls=0;refuse=5;out.count=11;if(mpt_shrink(&in,laws,&out,&attempts)!=MPT_NOMEM||out.count!=11||attempts!=99||live)return 2;
    calls=0;refuse=0;if(mpt_shrink(&in,laws,&out,&attempts)!=MPT_OK||out.count!=3||live)return 3;
    puts("MPT_ENV actual-malloc-refusal initial-and-candidate no-false-minimum no-retained-owner");return 0;
}
