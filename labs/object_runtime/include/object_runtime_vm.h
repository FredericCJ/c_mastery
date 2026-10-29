#ifndef CMASTERY_OBJECT_RUNTIME_VM_H
#define CMASTERY_OBJECT_RUNTIME_VM_H
#include "object_runtime.h"
#include "cvm.h"
#include "algo.h"

/* Pair-field offsets 0,1 and representable one-past 2; no raw heap address. */
struct orv_derived { struct cvm_value base; uint8_t offset; };
struct or_vm {
 struct cvm_cvm *vm;
 algo_pool registrations;
 struct orv_derived *roots[16];
 uint8_t paused, foreign;
};
struct orv_stats {
 unsigned slots, root_copies, frames, live, moved, work;
 uint8_t forwarding[16];
};
typedef enum or_status (*orv_foreign_fn)(struct or_vm *,struct orv_derived *,struct orv_derived *,void *,struct cvm_value *);
void orv_init(struct or_vm *,struct cvm_cvm *);
enum or_status orv_pause(struct or_vm *,int paused);
enum or_status orv_register(struct or_vm *,struct orv_derived *,uint64_t *ticket);
enum or_status orv_unregister(struct or_vm *,uint64_t ticket);
enum or_status orv_collect(struct or_vm *,unsigned budget,unsigned fault_cut,struct orv_stats *);
enum or_status orv_pair(struct or_vm *,struct orv_derived *,struct orv_derived *,int force_collection,
                  unsigned fault_cut,struct cvm_value *);
enum or_status orv_call(struct or_vm *,struct cvm_value,struct cvm_value,orv_foreign_fn,void *,struct cvm_value *);
#endif
