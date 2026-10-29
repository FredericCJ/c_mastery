#include "resize.h"
#include <stdio.h>
#include <stdlib.h>
struct lp_real {unsigned attempts,zero,release,fail;};
static void *resize(void *v,void *p,size_t before,size_t after){
    struct lp_real *r=v;(void)before;r->attempts++;
    if(!after){r->zero++;return NULL;}
    if(r->fail){r->fail=0;return NULL;}
    return realloc(p,after);
}
static void release(void *v,void *p,size_t n){struct lp_real *r=v;(void)n;r->release++;free(p);}
int main(void){
    struct lp_real real={0};struct lr_buffer b;struct lr_allocator a={&real,resize,release};int failed=0;
    if(lr_init(&b,a)!=LR_OK)return 1;
    if(lr_resize(&b,4)!=LR_OK)return 2;
    for(size_t i=0;i<4;i++)b.data[i]=(unsigned char)(i+31);
    unsigned char *saved=b.data;real.fail=1;
    if(lr_resize(&b,8)!=LR_NOMEM || b.data!=saved || b.size!=4)failed=1;
    if(!failed && (lr_resize(&b,8)!=LR_OK || b.size!=8))failed=1;
    /* saved becomes indeterminate after success and is never evaluated again. */
    if(!failed)for(size_t i=0;i<8;i++)if(b.data[i]!=(i<4?i+31:0))failed=1;
    if(!failed && (lr_resize(&b,2)!=LR_OK || b.data[0]!=31 || b.data[1]!=32))failed=1;
    if(lr_resize(&b,0)!=LR_INVALID || real.zero)failed=1;
    lr_release(&b);if(real.release!=1 || b.data || b.size)failed=1;
    if(!failed)puts("LP_REALLOC nonzero-only failure-preserves grow-shrink explicit-release");
    return failed;
}
