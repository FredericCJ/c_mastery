#include "async_store.h"
#include <errno.h>
#include <stdio.h>
int __wrap_pthread_create(pthread_t *a, const pthread_attr_t *b,
                          void *(*c)(void *), void *d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return EAGAIN;
}
static struct vg_vs s;
static struct vv_work work;
int main(void) {
  struct cvm_program p = {0};
  p.count = 1;
  p.length = 8;
  p.functions[0] = (struct cvm_function){0, 8, 0, 0};
  p.code[0] = OP_INT;
  p.code[1] = 5;
  p.code[5] = OP_HOST;
  p.code[6] = 3;
  p.code[7] = OP_RET;
  struct cvm_value out = cvm_int(91);
  store db = {0};
  struct vg_vh host = {&db, 8, VH_VERSIONED5, 0, 1};
  struct vg_va a;
  if (vs_start(&s, &p, VS_CHECKED_V1, 3, 0, 1, 8, &work) != VS_RUNNING ||
      vs_step(&s, &out) != VS_RUNNING || vs_step(&s, &out) != VS_PENDING ||
      va_init(&a) != VA_READY)
    return 1;
  if (va_submit(&a, &s, &host) != CVM_HOST_ERROR || a.active || a.joined ||
      vs_step(&s, &out) != CVM_HOST_ERROR || out.integer != 91 ||
      va_destroy(&a) != VA_READY)
    return 1;
  puts("VA_CREATE_FAILURE_PASS no_worker_no_borrow_retained");
  return 0;
}
