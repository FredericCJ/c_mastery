#include "contracts.h"
static uint32_t u32(const unsigned char *p) {return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint64_t u64(const unsigned char *p) {return (uint64_t)u32(p)|(uint64_t)u32(p+4)<<32;}
int kf_boot(const unsigned char *p,size_t n,KfBoot *out) {
    KfBoot b={0};
    if(!p||!out||cmk_boot_parse(p,n,&b.core)!=0)return 0;
    size_t total=u32(p),off=8;unsigned command=0;
    while(off<total) {
        /* The retained parser validated every extent first; keep these checks
         * explicit so this extension never relies on unbounded pointer walks. */
        if(total-off<8)return 0;
        uint32_t type=u32(p+off),size=u32(p+off+4);
        if(size<8||size>total-off)return 0;
        ++b.visited;
        if(type==1) {
            if(command++||size<9)return 0;
            size_t len=0;while(len<size-8&&p[off+8+len])++len;
            if(len==size-8)return 0;
            b.command=p+off+8;b.command_length=len;
        }
        size_t padding=(8u-(size&7u))&7u;
        if(padding>total-off-size)return 0;
        size_t step=(size_t)size+padding;
        off+=step;b.next=off;
    }
    if(off!=total)return 0;
    *out=b;return 1;
}
int kf_frames(cmk_frames *f,const cmk_boot *b,const KfSpan *r,size_t nr) {
    if(!f||!b||(!r&&nr)||b->entry_size<24||b->map_bytes%b->entry_size)return 0;
    for(size_t i=0;i<nr;++i)if(r[i].start>r[i].end)return 0;
    for(size_t off=0;off<b->map_bytes;off+=b->entry_size) {
        uint64_t base=u64(b->map+off),length=u64(b->map+off+8);
        if(length>UINT64_MAX-base)return 0;
    }
    /* No output mutation until all interval arithmetic is known valid. */
    for(unsigned page=0;page<CMK_MAX_FRAMES;++page) {
        uint64_t lo=(uint64_t)page*4096,hi=lo+4096;unsigned usable=0,reserved=page==0;
        for(size_t off=0;off<b->map_bytes;off+=b->entry_size) {
            uint64_t base=u64(b->map+off),end=base+u64(b->map+off+8);uint32_t type=u32(b->map+off+16);
            if(type==1&&base<=lo&&hi<=end)usable=1;
            if(type!=1&&base<hi&&lo<end)reserved=1;
        }
        for(size_t i=0;i<nr;++i)if(r[i].start<r[i].end&&r[i].start<hi&&lo<r[i].end)reserved=1;
        f->state[page]=(unsigned char)(usable&&!reserved);
    }
    return 1;
}
size_t kf_free(const KfPool *p) {size_t n=0;for(unsigned i=0;i<CMK_MAX_FRAMES;++i)n+=p->frames.state[i]==1;return n;}
int kf_allocate(KfPool *p,KfSpace *s,unsigned first,unsigned count,unsigned budget) {
    if(!p||!s||!s->live||!count||count>16||first>=512||count>512-first)return 0;
    for(unsigned i=0;i<count;++i)if(s->pages[first+i].frame)return 0;
    unsigned done=0;
    for(;done<count;++done) {
        uint32_t frame=done<budget?cmk_frame_take(&p->frames):0;
        if(!frame)break;
        if(frame<0x300000||frame>=0x400000){(void)cmk_frame_put(&p->frames,frame);break;}
        unsigned char *bytes=p->bytes[(frame-0x300000)/4096];
        for(unsigned j=0;j<4096;++j)bytes[j]=0;
        s->pages[first+done]=(KfPte){frame,7}; /* present,user,writable; model NX by lack of execute */
    }
    if(done==count)return 1;
    for(unsigned i=0;i<done;++i){/* leak allocated frame */s->pages[first+i]=(KfPte){0};}
    return 0;
}
int kf_reap_user(KfPool *p,KfSpace *s,unsigned slot,const KfCpu *c) {
    if(!p||!s||!c||s->live||slot==c->current||slot==c->cr3_owner||slot==c->tss_owner)return 0;
    /* Old execution stack may still be active. Only user frames are reclaimed;
     * static kernel stacks and page tables are not frame-pool allocations. */
    for(unsigned i=0;i<512;++i)if(s->pages[i].frame) {
        if(cmk_frame_put(&p->frames,s->pages[i].frame)!=0)return 0;
        s->pages[i]=(KfPte){0};
    }
    return 1;
}
int kf_retire_slot(KfSpace *s,unsigned slot,const KfCpu *c) {
    if(!s||!c||s->live||slot==c->current||slot==c->cr3_owner||slot==c->tss_owner||slot==c->stack_owner)return 0;
    for(unsigned i=0;i<512;++i)if(s->pages[i].frame)return 0;
    if(s->generation==UINT32_MAX)s->retired=1;else ++s->generation;
    return 1;
}
