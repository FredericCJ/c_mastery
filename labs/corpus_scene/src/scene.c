#include "scene.h"
void cs_init(struct cs_scene *s) { (void)s; }
enum cs_status cs_add(struct cs_scene *s, const struct cs_handle *p, const struct cs_matrix *m,
                struct cs_handle *out) {
  (void)s;
  (void)p;
  (void)m;
  (void)out;
  return CS_INVALID;
}
enum cs_status cs_set(struct cs_scene *s, struct cs_handle h, const struct cs_matrix *m) {
  (void)s;
  (void)h;
  (void)m;
  return CS_INVALID;
}
enum cs_status cs_world(struct cs_scene *s, struct cs_handle h, struct cs_matrix *out, unsigned *w) {
  (void)s;
  (void)h;
  (void)out;
  (void)w;
  return CS_INVALID;
}
enum cs_status cs_reparent(struct cs_scene *s, struct cs_handle h, const struct cs_handle *p, unsigned k) {
  (void)s;
  (void)h;
  (void)p;
  (void)k;
  return CS_INVALID;
}
enum cs_status cs_remove(struct cs_scene *s, struct cs_handle h) {
  (void)s;
  (void)h;
  return CS_INVALID;
}
enum cs_status nv_path(const struct nv_polygon *c, unsigned n, struct nv_point a, struct nv_point b,
                 struct nv_scratch *s, unsigned p, unsigned q, struct nv_path *out) {
  (void)c;
  (void)n;
  (void)a;
  (void)b;
  (void)s;
  (void)p;
  (void)q;
  (void)out;
  return CS_INVALID;
}
void nv_init(struct nv_session *s) { (void)s; }
enum cs_status nv_begin(struct nv_session *s, const struct nv_polygon *c, unsigned n, struct nv_point a,
                  struct nv_point b, struct nv_scratch *w, unsigned p, unsigned q,
                  struct nv_ticket *out) {
  (void)s;
  (void)c;
  (void)n;
  (void)a;
  (void)b;
  (void)w;
  (void)p;
  (void)q;
  (void)out;
  return CS_INVALID;
}
enum cs_status nv_copy(const struct nv_session *s, struct nv_ticket t, struct nv_path *out) {
  (void)s;
  (void)t;
  (void)out;
  return CS_INVALID;
}
enum cs_status nv_cancel(struct nv_session *s, struct nv_ticket t) {
  (void)s;
  (void)t;
  return CS_INVALID;
}
