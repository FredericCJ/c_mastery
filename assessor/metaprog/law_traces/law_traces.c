#include "law_traces.h"
#ifndef MPT_MUTATION
#define MPT_MUTATION 0
#endif
_Static_assert('A'==65 && 'Z'==90 && 'a'==97 && ' '==32,"ASCII execution profile");
int mpt_assign(struct mpt_owner *out,struct mpt_view view,struct mpt_allocator allocator){
    unsigned char *p;size_t i;struct mpt_owner staged;
    if(!out||!allocator.allocate||!allocator.release||(!view.bytes&&view.length))return MPT_INVALID;
    if(view.length>8)return MPT_RANGE;
    p=allocator.allocate(allocator.context,view.length?view.length:1);
    if(!p){if(MPT_MUTATION==1)mpt_drop(out);return MPT_NOMEM;}
    for(i=0;i<view.length;++i)p[i]=view.bytes[i];
    staged=(struct mpt_owner){p,view.length,view.cache,allocator};mpt_drop(out);*out=staged;return MPT_OK;
}
void mpt_drop(struct mpt_owner *owner){
    struct mpt_allocator a;unsigned char *p;if(!owner)return;p=owner->bytes;a=owner->allocator;*owner=(struct mpt_owner){0};if(p)a.release(a.context,p);
}
struct mpt_view mpt_view(const struct mpt_owner *owner){return owner?(struct mpt_view){owner->bytes,owner->length,owner->cache}:(struct mpt_view){NULL,0,0};}
static int canonical(struct mpt_view view,unsigned mode,unsigned char out[8],size_t *length){
    size_t i,n=0;unsigned pending=0;
    if(mode>MPT_ASCII_TEXT||(!view.bytes&&view.length))return MPT_INVALID;
    if(view.length>8)return MPT_RANGE;
    for(i=0;i<view.length;++i){unsigned char c=view.bytes[i];
        if(mode==MPT_ASCII_TEXT){
            if(c<32||c>126)return MPT_RANGE;
            if(c==32){if(n)pending=1;continue;}
            if(pending){out[n++]=32;pending=0;}
            if(c>='A'&&c<='Z'&&MPT_MUTATION!=2)c=(unsigned char)(c+('a'-'A'));
        }
        out[n++]=c;
    }
    *length=n;return MPT_OK;
}
int mpt_equal(struct mpt_view a,struct mpt_view b,unsigned mode,unsigned *out){
    unsigned char x[8],y[8];size_t nx,ny,i;int status;unsigned same=1;if(!out)return MPT_INVALID;
    status=canonical(a,mode,x,&nx);if(status!=MPT_OK)return status;status=canonical(b,mode,y,&ny);if(status!=MPT_OK)return status;
    if(nx!=ny)same=0;else for(i=0;i<nx;++i)if(x[i]!=y[i])same=0;
    *out=same;return MPT_OK;
}
int mpt_hash(struct mpt_view view,unsigned mode,uint64_t *out){
    unsigned char bytes[8];size_t n,i;uint64_t h=UINT64_C(14695981039346656037);int status;if(!out)return MPT_INVALID;status=canonical(view,mode,bytes,&n);if(status!=MPT_OK)return status;
    h=(h^(unsigned char)n)*UINT64_C(1099511628211);
    for(i=0;i<n;++i){if(MPT_MUTATION==4&&bytes[i]==0)break;h=(h^bytes[i])*UINT64_C(1099511628211);}
    if(MPT_MUTATION==3)h=(h^view.cache)*UINT64_C(1099511628211);
    *out=h;return MPT_OK;
}
