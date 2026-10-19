#ifndef CORPUS_SCENE_H
#define CORPUS_SCENE_H
#include <stdint.h>
#define CS_N 16u
typedef enum {
  CS_OK,
  CS_INVALID,
  CS_FULL,
  CS_STALE,
  CS_RANGE,
  CS_BUSY
} CsStatus;
typedef struct {
  int32_t m[3][4];
} CsMatrix;
typedef struct {
  CsMatrix local, world;
  uint32_t generation;
  unsigned parent, live, dirty;
} CsNode;
typedef struct {
  CsNode node[CS_N];
} CsScene;
typedef struct {
  const CsScene *owner;
  unsigned slot;
  uint32_t generation;
} CsHandle;
/* All controls exclusively owned/live/noncopied; pointers distinct. Init only
   fresh storage. Root parent is CS_N; null parent handle selects root. */
void cs_init(CsScene *scene);
CsStatus cs_add(CsScene *scene, const CsHandle *parent, const CsMatrix *local,
                CsHandle *out);
CsStatus cs_set(CsScene *scene, CsHandle handle, const CsMatrix *local);
CsStatus cs_world(CsScene *scene, CsHandle handle, CsMatrix *out,
                  unsigned *multiplications);
CsStatus cs_reparent(CsScene *scene, CsHandle handle, const CsHandle *parent,
                     unsigned preserve_world);
CsStatus cs_remove(CsScene *scene, CsHandle handle);

#define NV_POLYS 8u
#define NV_VERTS 6u
#define NV_POINTS 10u
typedef struct {
  int32_t x, y;
} NvPoint;
typedef struct {
  unsigned n;
  NvPoint p[NV_VERTS];
} NvPolygon;
typedef struct {
  unsigned n;
  NvPoint p[NV_POINTS];
} NvPath;
typedef struct {
  NvPoint left[NV_POINTS], right[NV_POINTS];
  NvPath staged;
} NvScratch;
typedef struct {
  uint32_t generation;
  unsigned active;
  NvPath path;
} NvSession;
typedef struct {
  const NvSession *owner;
  uint32_t generation;
} NvTicket;
/* Input is an ordered convex corridor, not arbitrary unordered mesh. All
   integer coordinates [-64,64]. Positive-area CCW polygons; point agent and
   closed edges. Distinct actual typed scratch. Every refusal preserves caller
   path/control. */
CsStatus nv_path(const NvPolygon *corridor, unsigned n, NvPoint start,
                 NvPoint end, NvScratch *scratch, unsigned portal_budget,
                 unsigned point_budget, NvPath *out);
void nv_init(NvSession *session);
CsStatus nv_begin(NvSession *session, const NvPolygon *corridor, unsigned n,
                  NvPoint start, NvPoint end, NvScratch *scratch,
                  unsigned portal_budget, unsigned point_budget, NvTicket *out);
CsStatus nv_copy(const NvSession *session, NvTicket ticket, NvPath *out);
CsStatus nv_cancel(NvSession *session, NvTicket ticket);
#endif
