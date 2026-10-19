#ifndef AP_RECLAMATION_H
#define AP_RECLAMATION_H
#include <limits.h>
#include <stdatomic.h>
#include <stdint.h>
_Static_assert(UINT_MAX == UINT32_MAX,
               "this queue profile requires32-bit unsigned");
#define AP_NONE UINT32_MAX
#define AP_NODES 32u
#define AP_PARTICIPANTS 8u
enum { AP_OK, AP_RETRY, AP_FULL, AP_INVALID, AP_UNIMPLEMENTED };
enum { AP_LOADED, AP_VALIDATED, AP_RETIRE_SNAPSHOT, AP_LINKED };
typedef struct {
  uint32_t sequence, inverse;
} APValue;
typedef struct {
  atomic_uint next, state;
  APValue value;
} APNode;
typedef void (*APHook)(void *, unsigned point, unsigned participant,
                       unsigned node);
typedef struct APQueue {
  atomic_uint head, tail, hazard[AP_PARTICIPANTS][2],
      registration[AP_PARTICIPANTS];
  APNode node[AP_NODES];
  unsigned participants, generation_limit;
  APHook hook;
  void *hook_context;
} APQueue;
typedef struct {
  const APQueue *owner;
  unsigned participant, generation;
} APLease;
int ap_init(APQueue *, unsigned participants, unsigned generation_limit, APHook,
            void *);
int ap_register(APQueue *, APLease *);
/* Owner or externally joined successor only: no in-flight operation/borrow. */
int ap_leave(APQueue *, APLease);
int ap_push(APQueue *, APLease, APValue);
int ap_pop(APQueue *, APLease, APValue *);
unsigned ap_collect(APQueue *);
#endif
