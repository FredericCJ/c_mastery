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
struct ag_domain;
struct ag_lease {
  const struct ag_domain *owner;
  unsigned participant, generation;
};
struct ag_domain {
  cmx_epoch retained;
  unsigned registered[4], generation[4], depth[4], section[4], nested;
};
void ag_init(struct ag_domain *, unsigned nested);
int ag_register(struct ag_domain *, struct ag_lease *);
int ag_enter(struct ag_domain *, struct ag_lease);
int ag_leave(struct ag_domain *, struct ag_lease);
int ag_depart(struct ag_domain *, struct ag_lease);
int ag_advance(struct ag_domain *);
int ag_reclaimable(const struct ag_domain *, unsigned retired_epoch);
struct ag_version {
  unsigned index, generation;
};
struct ag_borrow {
  struct ag_lease lease;
  unsigned section;
  struct ag_version version;
};
struct ag_object {
  unsigned active, generation, retired, retired_epoch, refs;
  unsigned char bytes[32];
};
struct ag_store;
typedef void (*ag_callback_fn)(void *, struct ag_store *, struct api_handle,
                           const unsigned char[32]);
struct ag_capture {
  struct ag_store *owner;
  struct ag_version version;
  ag_callback_fn callback;
  void *context;
  unsigned occupied;
};
struct ag_store {
  struct ag_domain domain;
  struct ag_object objects[AG_VERSIONS];
  unsigned current, writer;
  struct api_registry callbacks;
  struct ag_capture capture[4];
  unsigned drops, invocations;
};
void ag_store_init(struct ag_store *, unsigned nested, unsigned char initial);
int ag_writer_acquire(struct ag_store *, unsigned writer);
int ag_writer_release(struct ag_store *, unsigned writer);
int ag_publish(struct ag_store *, unsigned writer, unsigned char value);
int ag_read(struct ag_store *, struct ag_lease, struct ag_borrow *);
int ag_copy(struct ag_store *, struct ag_borrow, unsigned char out[32]);
int ag_read_end(struct ag_store *, struct ag_borrow);
unsigned ag_collect(struct ag_store *);
enum api_status ag_capture(struct ag_store *, struct ag_borrow, ag_callback_fn, void *, struct api_handle *);
enum api_status ag_dispatch(struct ag_store *);
enum api_status ag_remove(struct ag_store *, struct api_handle);
enum api_status ag_destroy_callbacks(struct ag_store *);
#endif
