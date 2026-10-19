#include "cmx_advanced.h"
void cmx_epoch_init(cmx_epoch *d) { (void)d; }
enum cmx_result cmx_epoch_enter(cmx_epoch *d, unsigned p) {
  (void)d;
  (void)p;
  return CMX_INVALID;
}
void cmx_epoch_leave(cmx_epoch *d, unsigned p) {
  (void)d;
  (void)p;
}
int cmx_epoch_advance(cmx_epoch *d) {
  (void)d;
  return 0;
}
void cmx_rcu_init(cmx_rcu *d, unsigned r) {
  (void)d;
  (void)r;
}
enum cmx_result cmx_rcu_read(cmx_rcu *d, unsigned p, unsigned *r) {
  (void)d;
  (void)p;
  (void)r;
  return CMX_INVALID;
}
enum cmx_result cmx_rcu_leave(cmx_rcu *d, unsigned p) {
  (void)d;
  (void)p;
  return CMX_INVALID;
}
unsigned cmx_rcu_publish(cmx_rcu *d, unsigned r) {
  (void)d;
  (void)r;
  return 0;
}
int cmx_rcu_quiescent(const cmx_rcu *d) {
  (void)d;
  return 0;
}
