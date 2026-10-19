/* Learner scaffold: implement the four contracts; do not copy assessor code. */
#include "format.h"
enum lf_status lf_prefix(lf_formatter f, void *ctx, char *d, size_t n,
                         struct lf_result *r) {
  (void)f;
  (void)ctx;
  (void)d;
  (void)n;
  (void)r;
  return LF_NOT_IMPLEMENTED;
}
enum lf_status lf_transaction(lf_formatter f, void *ctx, char *d, size_t n,
                              struct lf_result *r) {
  (void)f;
  (void)ctx;
  (void)d;
  (void)n;
  (void)r;
  return LF_NOT_IMPLEMENTED;
}
enum lf_status lf_parse_policy(const char *s, enum lf_policy *p) {
  (void)s;
  (void)p;
  return LF_NOT_IMPLEMENTED;
}
enum lf_status lf_typed(enum lf_policy p, struct lf_value v, char *d, size_t n,
                        struct lf_result *r) {
  (void)p;
  (void)v;
  (void)d;
  (void)n;
  (void)r;
  return LF_NOT_IMPLEMENTED;
}
