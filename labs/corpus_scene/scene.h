#ifndef CORPUS_SCENE_H
#define CORPUS_SCENE_H
#include <stdint.h>
#define CS_N 16u
enum cs_status {
  CS_OK,
  CS_INVALID,
  CS_FULL,
  CS_STALE,
  CS_RANGE,
  CS_BUSY
};
struct cs_matrix {
  int32_t m[3][4];
};
struct cs_node {
  struct cs_matrix local, world;
  uint32_t generation;
  unsigned parent, live, dirty;
};
struct cs_scene {
  struct cs_node node[CS_N];
};
struct cs_handle {
  const struct cs_scene *owner;
  unsigned slot;
  uint32_t generation;
};
/* All controls exclusively owned/live/noncopied; pointers distinct. Init only
   fresh storage. Root parent is CS_N; null parent handle selects root. */
void cs_init(struct cs_scene *scene);
enum cs_status cs_add(struct cs_scene *scene, const struct cs_handle *parent, const struct cs_matrix *local,
                struct cs_handle *out);
enum cs_status cs_set(struct cs_scene *scene, struct cs_handle handle, const struct cs_matrix *local);
enum cs_status cs_world(struct cs_scene *scene, struct cs_handle handle, struct cs_matrix *out,
                  unsigned *multiplications);
enum cs_status cs_reparent(struct cs_scene *scene, struct cs_handle handle, const struct cs_handle *parent,
                     unsigned preserve_world);
enum cs_status cs_remove(struct cs_scene *scene, struct cs_handle handle);

#define NV_POLYS 8u
#define NV_VERTS 6u
#define NV_POINTS 10u
struct nv_point {
  int32_t x, y;
};
struct nv_polygon {
  unsigned n;
  struct nv_point p[NV_VERTS];
};
struct nv_path {
  unsigned n;
  struct nv_point p[NV_POINTS];
};
struct nv_scratch {
  struct nv_point left[NV_POINTS], right[NV_POINTS];
  struct nv_path staged;
};
struct nv_session {
  uint32_t generation;
  unsigned active;
  struct nv_path path;
};
struct nv_ticket {
  const struct nv_session *owner;
  uint32_t generation;
};
/* Input is an ordered convex corridor, not arbitrary unordered mesh. All
   integer coordinates [-64,64]. Positive-area CCW polygons; point agent and
   closed edges. Distinct actual typed scratch. Every refusal preserves caller
   path/control. */
enum cs_status nv_path(const struct nv_polygon *corridor, unsigned n, struct nv_point start,
                 struct nv_point end, struct nv_scratch *scratch, unsigned portal_budget,
                 unsigned point_budget, struct nv_path *out);
void nv_init(struct nv_session *session);
enum cs_status nv_begin(struct nv_session *session, const struct nv_polygon *corridor, unsigned n,
                  struct nv_point start, struct nv_point end, struct nv_scratch *scratch,
                  unsigned portal_budget, unsigned point_budget, struct nv_ticket *out);
enum cs_status nv_copy(const struct nv_session *session, struct nv_ticket ticket, struct nv_path *out);
enum cs_status nv_cancel(struct nv_session *session, struct nv_ticket ticket);
#endif
