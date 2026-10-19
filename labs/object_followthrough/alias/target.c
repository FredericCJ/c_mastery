#include "alias.h"
static FDSlot slot=FD_SLOT_INITIALIZER;
static OsWideSlot wide;
static unsigned char bytes[32],wire[8]={0,0,0,7,0,0,0,11};
static FDValue value;
static FDPool pool;
static FDHandle handle;
static OsContainer owner={1,17};
static int output;
static void *none(void *c,size_t s,size_t a){(void)c;(void)s;(void)a;return NULL;}
static void release(void *c,void *p){(void)c;(void)p;}
static int init(void *c,FDValue *p){(void)c;p->value=7;p->serial=11;return 0;}
static void drop(void *c,FDValue *p){(void)c;(void)p;}
static int wide_drop(void *c,OsWide *p){(void)c;(void)p;return 0;}
static void *(*volatile allocation_call)(void *,size_t,size_t)=none;
volatile int os_target_result;
void _start(void)
{
    OsAllocator a={{NULL,none,release},sizeof(OsWide),64};const FDValue *view=NULL;const OsWide *borrow=NULL;int result=os_acquire(&slot,a);
    result+=(int)fd_slot_initialize(&slot,init,drop,NULL);result+=(int)fd_slot_publish(&slot,&view);fd_slot_reset(&slot);
    result+=os_wide_acquire(&wide,a,17,wide_drop,NULL);result+=os_wide_borrow(&wide,&borrow);result+=os_wide_return(&wide,&borrow);result+=os_wide_reset(&wide);
    result+=os_wire(wire,8,&value);result+=(int)os_host_representation(7);os_transform(bytes+16,bytes,4,0x55);os_ordinary(bytes+16,bytes,4,0x55);os_nested(bytes+16,bytes,4,0x55);os_shared(bytes+16,bytes,bytes,4);
    result+=os_partition(bytes,32,16,0,4,0x55,0);result+=(int)os_move(bytes,32,2,0,4);fd_pool_init(&pool);result+=(int)fd_pool_acquire(&pool,value,&handle);result+=(int)fd_pool_get(&pool,handle,&value);result+=(int)fd_pool_release(&pool,handle);result+=os_member(&owner,&owner.member,&output);
    /* Clone is rooted but only invoked if the callback returns actual allocated storage. */
    {void *allocated=allocation_call(a.allocator.context,sizeof(FDValue),_Alignof(FDValue));if(allocated){result+=os_clone(allocated,&value);a.allocator.deallocate(a.allocator.context,allocated);}}
#ifdef OS_TARGET_HELPER
    {extern void *malloc(size_t);void *p=malloc(4);if(p)result++;}
#endif
    os_target_result=result;for(;;){}
}
