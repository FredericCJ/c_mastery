#ifndef AG_GRACE_H
#define AG_GRACE_H
#include "api_depth.h"
#include "cmx.h"
#include <limits.h>
#define AG_READERS 4u
#define AG_VERSIONS 17u
#define AG_NONE UINT_MAX
enum {
  AG_OK,
  AG_INVALID,
  AG_BUSY,
  AG_FULL,
  AG_PENDING,
  AG_EXHAUSTED,
  AG_UNIMPLEMENTED
};
typedef struct AGDomain AGDomain;
typedef struct {
  const AGDomain *owner;
  unsigned participant, generation;
} AGLease;
struct AGDomain {
  cmx_epoch retained;
  unsigned registered[4], generation[4], depth[4], section[4], nested;
};
void ag_init(AGDomain *, unsigned nested);
int ag_register(AGDomain *, AGLease *);
int ag_enter(AGDomain *, AGLease);
int ag_leave(AGDomain *, AGLease);
int ag_depart(AGDomain *, AGLease);
int ag_advance(AGDomain *);
int ag_reclaimable(const AGDomain *, unsigned retired_epoch);
typedef struct {
  unsigned index, generation;
} AGVersion;
typedef struct {
  AGLease lease;
  unsigned section;
  AGVersion version;
} AGBorrow;
typedef struct {
  unsigned active, generation, retired, retired_epoch, refs;
  unsigned char bytes[32];
} AGObject;
typedef struct AGStore AGStore;
typedef void (*AGCallback)(void *, AGStore *, ApiHandle,
                           const unsigned char[32]);
typedef struct {
  AGStore *owner;
  AGVersion version;
  AGCallback callback;
  void *context;
  unsigned occupied;
} AGCapture;
struct AGStore {
  AGDomain domain;
  AGObject objects[AG_VERSIONS];
  unsigned current, writer;
  ApiRegistry callbacks;
  AGCapture capture[4];
  unsigned drops, invocations;
};
void ag_store_init(AGStore *, unsigned nested, unsigned char initial);
int ag_writer_acquire(AGStore *, unsigned writer);
int ag_writer_release(AGStore *, unsigned writer);
int ag_publish(AGStore *, unsigned writer, unsigned char value);
int ag_read(AGStore *, AGLease, AGBorrow *);
int ag_copy(AGStore *, AGBorrow, unsigned char out[32]);
int ag_read_end(AGStore *, AGBorrow);
unsigned ag_collect(AGStore *);
ApiStatus ag_capture(AGStore *, AGBorrow, AGCallback, void *, ApiHandle *);
ApiStatus ag_dispatch(AGStore *);
ApiStatus ag_remove(AGStore *, ApiHandle);
ApiStatus ag_destroy_callbacks(AGStore *);
#endif
