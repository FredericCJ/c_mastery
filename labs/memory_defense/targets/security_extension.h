#ifndef CM_SECURITY_EXTENSION_H
#define CM_SECURITY_EXTENSION_H
#include <stddef.h>
#include <stdint.h>
typedef struct {uint32_t generation;unsigned live,owner;unsigned value;} MDEObject;
typedef struct {uint32_t generation;unsigned owner;unsigned value;} MDEEvent;
/* All pointers designate live distinct control/storage objects. Serialized
 * transitions; the generation must never wrap or be reset under queued events. */
int mde_deliver(MDEObject *,MDEEvent);
int mde_retire(MDEObject *);
int mde_recreate(MDEObject *,unsigned,unsigned);
/* Binary logging produces exactly4*n uppercase hex-escape bytes plus NUL,
 * checks n <= (capacity-1)/4 before multiplication; output unchanged on reject. */
int mde_log(const unsigned char *,size_t,char *,size_t);
typedef struct MDEOwner MDEOwner;
struct MDEOwner {unsigned state;void *first,*second;void (*release)(void *,void *);void *context;};
/* state0 closed/empty; state1 acquired. May destroy partially constructed
 * first/second. Publish closed and detach both pointers before callbacks.
 * Callbacks may reenter destroy, but may not free control or mutate it. */
int mde_destroy(MDEOwner *);
#endif
