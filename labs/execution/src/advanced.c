#include "cmx_advanced.h"
/* Original implementation required; these stubs do not satisfy gates. */
enum cmx_result cmx_priority_order(const cmx_job *a0, unsigned a1, unsigned a2,
                                   unsigned *a3) {
  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_partition(const cmx_job *a0, unsigned a1, unsigned a2,
                              uint32_t a3, unsigned *a4) {
  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_simulate(const cmx_job *a0, unsigned a1, unsigned a2,
                             const unsigned *a3, uint32_t a4,
                             cmx_sim_result *a5) {
  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_server_init(cmx_server *a0, uint32_t a1, uint32_t a2) {
  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_server_update(cmx_server *a0, uint32_t a1) {
  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_server_charge(cmx_server *a0, uint32_t a1, uint32_t a2) {
  return CMX_UNIMPLEMENTED;
}
void cmx_identity_init(cmx_identity_pool *a0) { *a0 = (cmx_identity_pool){0}; }
uint32_t cmx_identity_take(cmx_identity_pool *a0) { return 0; }
int cmx_identity_valid(const cmx_identity_pool *a0, uint32_t a1) { return 0; }
enum cmx_result cmx_identity_put(cmx_identity_pool *a0, uint32_t a1) {
  return CMX_UNIMPLEMENTED;
}
void cmx_reservation_init(cmx_reservation *a0) { *a0 = (cmx_reservation){0}; }
enum cmx_result cmx_reserve(cmx_reservation *a0, uint32_t *a1) {
  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_publish(cmx_reservation *a0, uint32_t a1, uint32_t a2) {
  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_reserved_pop(cmx_reservation *a0, uint32_t *a1) {
  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_history_check(const cmx_history_op *a0, unsigned a1,
                                  unsigned a2, unsigned *a3) {
  return CMX_UNIMPLEMENTED;
}
void cmx_rcu_init(cmx_rcu *a0, unsigned a1) { *a0 = (cmx_rcu){0}; }
enum cmx_result cmx_rcu_read(cmx_rcu *a0, unsigned a1, unsigned *a2) {
  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_rcu_leave(cmx_rcu *a0, unsigned a1) {
  return CMX_UNIMPLEMENTED;
}
unsigned cmx_rcu_publish(cmx_rcu *a0, unsigned a1) { return 0; }
int cmx_rcu_quiescent(const cmx_rcu *a0) { return 0; }
