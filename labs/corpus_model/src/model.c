#include "model.h"
void cm_init(CmModel *m) {
  if (m)
    *m = (CmModel){0};
}
int cm_valid(const CmModel *m) {
  (void)m;
  return CM_INVALID;
}
int cm_apply(CmModel *m, const CmEdit *e, size_t n) {
  (void)m;
  (void)e;
  (void)n;
  return CM_INVALID;
}
int cm_load(CmModel *m, const unsigned char *s, size_t n, unsigned c) {
  (void)m;
  (void)s;
  (void)n;
  (void)c;
  return CM_INVALID;
}
int cm_get(const CmModel *m, unsigned id, int32_t *v) {
  (void)m;
  (void)id;
  (void)v;
  return CM_INVALID;
}
int cm_eval(const CmModel *m, const unsigned char *s, size_t n, int32_t *v) {
  (void)m;
  (void)s;
  (void)n;
  (void)v;
  return CM_INVALID;
}
void cm_session_init(CmSession *s) {
  if (s)
    *s = (CmSession){0};
}
int cm_step(CmSession *s, const unsigned char *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return CM_INVALID;
}
int cm_feed(CmSession *s, const unsigned char *p, size_t n, size_t *used) {
  (void)s;
  (void)p;
  (void)n;
  (void)used;
  return CM_INVALID;
}
int cm_finish(CmSession *s) {
  (void)s;
  return CM_INVALID;
}
int cm_drain(CmSession *s, CmWrite w, void *c) {
  (void)s;
  (void)w;
  (void)c;
  return CM_INVALID;
}
