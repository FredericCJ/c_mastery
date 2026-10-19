#include "clinic.h"
int mc_acquire(McOwner *owner,McAllocator a,int value)
{int *p;if(!owner || owner->value || !a.allocate || !a.release)return -1;p=a.allocate(a.context,sizeof *p);if(!p)return -1;*p=value;owner->value=p;owner->allocation=a;return 0;}
int mc_read(const McOwner *owner,int *out)
{if(!owner || !owner->value || !out)return -1;*out=*owner->value;return 0;}
int mc_snapshot(const McOwner *owner,int *out)
{
#ifdef MC_MUTANT_ESCAPE
    if(!owner || !owner->value || !out)return -1;
    *out=0;return 0;
#else
    return mc_read(owner,out);
#endif
}
int mc_release(McOwner *owner)
{
    int *p;
    if(!owner || !owner->value){
#ifdef MC_MUTANT_RELEASE
        return 0;
#else
        return -1;
#endif
    }
    p=owner->value;owner->value=NULL;++owner->releases;owner->allocation.release(owner->allocation.context,p);return 0;
}
FDStatus mc_move(unsigned char *base,size_t length,size_t dst,size_t src,size_t count)
{return fd_move(base,length,dst,src,count);}
int mc_member(const McContainer *owner,const int *member,int *out)
{
    const unsigned char *base;const int *expected;
    if(!owner || !member || !out)return -1;
    base=(const unsigned char *)owner;expected=(const int *)(const void *)(base+offsetof(McContainer,member));
#ifndef MC_MUTANT_MEMBER
    if(member!=expected)return -1;
#else
    (void)expected;
#endif
    *out=owner->member;return 0;
}
int mc_initial(const McFirst *owner,const int *member,int *out)
{
    const McFirst *recovered;
    if(!owner || !member || !out || member!=&owner->first)return -1;
    recovered=(const McFirst *)(const void *)member;
    if(recovered!=owner)return -1;
    *out=recovered->first;return 0;
}
