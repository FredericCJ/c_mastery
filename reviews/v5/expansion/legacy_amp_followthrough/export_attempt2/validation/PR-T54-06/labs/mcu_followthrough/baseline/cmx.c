/* Additive warning-clean incomplete baseline. Original source unchanged. */
#include "cmx.h"
enum cmx_result cmx_queue_abort(cmx_kernel *k, cmx_queue *q) {
  (void)k;
  (void)q;

  (void)k;
  (void)q;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_latch_wait(cmx_kernel *k, cmx_latch *l, unsigned t,
                               cmx_deadline d) {
  (void)k;
  (void)l;
  (void)t;
  (void)d;

  return CMX_UNIMPLEMENTED;
}
void cmx_latch_signal(cmx_kernel *k, cmx_latch *l) {
  (void)k;
  (void)l;
}
enum cmx_result cmx_mutex_close(cmx_kernel *k, cmx_mutex *m) {
  (void)k;
  (void)m;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_cond_close(cmx_kernel *k, cmx_cond *c) {
  (void)k;
  (void)c;

  return CMX_UNIMPLEMENTED;
}
void cmx_event_close(cmx_kernel *k, cmx_event *e) {
  (void)k;
  (void)e;
}
enum cmx_result cmx_rw_close(cmx_kernel *k, cmx_rwlock *r) {
  (void)k;
  (void)r;

  return CMX_UNIMPLEMENTED;
}
/* Implement the public contract. A successful build is not acceptance. */
void cmx_init(cmx_kernel *a0, enum cmx_policy a1) {
  (void)a0;
  (void)a1;
 *a0 = (cmx_kernel){0}; }
int cmx_task_add(cmx_kernel *a0, unsigned a1, uint32_t a2, uintptr_t a3,
                 size_t a4) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;

  return CMX_NONE;
}
int cmx_schedule(cmx_kernel *a0) {
  (void)a0;
 return CMX_NONE; }
enum cmx_result cmx_sleep(cmx_kernel *a0, unsigned a1, cmx_deadline a2) {
  (void)a0;
  (void)a1;
  (void)a2;

  return CMX_UNIMPLEMENTED;
}
void cmx_tick(cmx_kernel *a0, uint32_t a1) {
  (void)a0;
  (void)a1;
}
enum cmx_result cmx_cancel(cmx_kernel *a0, unsigned a1) {
  (void)a0;
  (void)a1;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_exit(cmx_kernel *a0, unsigned a1, int a2) {
  (void)a0;
  (void)a1;
  (void)a2;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_join(cmx_kernel *a0, unsigned a1, unsigned a2,
                         cmx_deadline a3) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_reap(cmx_kernel *a0, unsigned a1) {
  (void)a0;
  (void)a1;

  return CMX_UNIMPLEMENTED;
}
void cmx_sem_init(cmx_sem *a0, unsigned a1, unsigned a2) {
  (void)a0;
  (void)a1;
  (void)a2;
 *a0 = (cmx_sem){0}; }
enum cmx_result cmx_sem_take(cmx_kernel *a0, cmx_sem *a1, unsigned a2,
                             cmx_deadline a3) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_sem_give(cmx_kernel *a0, cmx_sem *a1) {
  (void)a0;
  (void)a1;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_sem_close(cmx_kernel *a0, cmx_sem *a1) {
  (void)a0;
  (void)a1;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_mutex_init(cmx_kernel *a0, cmx_mutex *a1, unsigned a2,
                               unsigned a3) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_mutex_lock(cmx_kernel *a0, cmx_mutex *a1, unsigned a2,
                               cmx_deadline a3) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_mutex_unlock(cmx_kernel *a0, cmx_mutex *a1, unsigned a2) {
  (void)a0;
  (void)a1;
  (void)a2;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_cond_wait(cmx_kernel *a0, cmx_cond *a1, cmx_mutex *a2,
                              unsigned a3, cmx_deadline a4) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_cond_signal(cmx_kernel *a0, cmx_cond *a1, unsigned a2) {
  (void)a0;
  (void)a1;
  (void)a2;

  return CMX_UNIMPLEMENTED;
}
void cmx_queue_init(cmx_queue *a0, unsigned a1) {
  (void)a0;
  (void)a1;
 *a0 = (cmx_queue){0}; }
enum cmx_result cmx_queue_send(cmx_kernel *a0, cmx_queue *a1, unsigned a2,
                               uint32_t a3, cmx_deadline a4) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_queue_recv(cmx_kernel *a0, cmx_queue *a1, unsigned a2,
                               cmx_deadline a3) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_queue_close(cmx_kernel *a0, cmx_queue *a1) {
  (void)a0;
  (void)a1;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_event_wait(cmx_kernel *a0, cmx_event *a1, unsigned a2,
                               uint32_t a3, unsigned a4, unsigned a5,
                               cmx_deadline a6) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;
  (void)a6;

  return CMX_UNIMPLEMENTED;
}
void cmx_event_set(cmx_kernel *a0, cmx_event *a1, uint32_t a2) {
  (void)a0;
  (void)a1;
  (void)a2;
}
enum cmx_result cmx_notify(cmx_kernel *a0, unsigned a1, uint32_t a2) {
  (void)a0;
  (void)a1;
  (void)a2;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_notify_take(cmx_kernel *a0, unsigned a1, cmx_deadline a2) {
  (void)a0;
  (void)a1;
  (void)a2;

  return CMX_UNIMPLEMENTED;
}
void cmx_barrier_init(cmx_barrier *a0, unsigned a1) {
  (void)a0;
  (void)a1;
 *a0 = (cmx_barrier){0}; }
enum cmx_result cmx_barrier_wait(cmx_kernel *a0, cmx_barrier *a1, unsigned a2,
                                 cmx_deadline a3) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;

  return CMX_UNIMPLEMENTED;
}
void cmx_once_init(cmx_once *a0) {
  (void)a0;
 *a0 = (cmx_once){0}; }
enum cmx_result cmx_once_enter(cmx_kernel *a0, cmx_once *a1, unsigned a2,
                               cmx_deadline a3) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_once_finish(cmx_kernel *a0, cmx_once *a1, unsigned a2,
                                unsigned a3) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;

  return CMX_UNIMPLEMENTED;
}
void cmx_rw_init(cmx_rwlock *a0) {
  (void)a0;
 *a0 = (cmx_rwlock){0}; }
enum cmx_result cmx_rw_read(cmx_kernel *a0, cmx_rwlock *a1, unsigned a2,
                            cmx_deadline a3) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_rw_write(cmx_kernel *a0, cmx_rwlock *a1, unsigned a2,
                             cmx_deadline a3) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_rw_unlock(cmx_kernel *a0, cmx_rwlock *a1, unsigned a2) {
  (void)a0;
  (void)a1;
  (void)a2;

  return CMX_UNIMPLEMENTED;
}
int cmx_invariant(const cmx_kernel *a0) {
  (void)a0;
 return 0; }
enum cmx_result cmx_response(const cmx_job *a0, unsigned a1, unsigned a2,
                             uint32_t *a3) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_edf_admit(const cmx_job *a0, unsigned a1, uint32_t a2) {
  (void)a0;
  (void)a1;
  (void)a2;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_cbs_init(cmx_cbs *a0, uint32_t a1, uint32_t a2) {
  (void)a0;
  (void)a1;
  (void)a2;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_cbs_release(cmx_cbs *a0, uint32_t a1) {
  (void)a0;
  (void)a1;

  return CMX_UNIMPLEMENTED;
}
enum cmx_result cmx_cbs_charge(cmx_cbs *a0, uint32_t a1) {
  (void)a0;
  (void)a1;

  return CMX_UNIMPLEMENTED;
}
