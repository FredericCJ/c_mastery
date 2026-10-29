#ifndef OWNERSHIP_AUTHORITY_BOUNDARY_H
#define OWNERSHIP_AUTHORITY_BOUNDARY_H
#include "ownership.h"
#include <stddef.h>
#include <stdint.h>

enum
{
    OAF_READ = 1,
    OAF_WRITE = 2,
    OAF_WIRE_SIZE = 16
};

enum oaf_status
{
    OAF_OK,
    OAF_INVALID,
    OAF_DENIED,
    OAF_STALE,
    OAF_FULL
};

/* Trusted, externally serialized, live and noncopied authority. Init is only
   for fresh storage. Arena and root outlive every view. Native C fields are
   not protected against arbitrary same-process writes. */
struct oaf_root
{
    struct ow_tenant_arena *arena;
    uint32_t epoch;
    unsigned tenant, rights, retired;
};

struct oaf_view
{
    struct oaf_root *root;
    uint32_t epoch, offset, count;
    unsigned rights;
};

/* Every field is untrusted. The caller/server selects the trusted parent
   out of band; a request never supplies a trusted native pointer. */
struct oaf_request
{
    uint32_t epoch, offset, count;
    unsigned tenant, rights;
};
enum oaf_status oaf_init(struct oaf_root *, struct ow_tenant_arena *,
                         unsigned tenant, unsigned rights);
enum oaf_status oaf_view(struct oaf_root *, struct oaf_view *);
enum oaf_status oaf_subview(const struct oaf_view *, uint32_t offset,
                            uint32_t count, unsigned rights, struct oaf_view *);
enum oaf_status oaf_revoke(struct oaf_root *);
enum oaf_status oaf_encode(struct oaf_request,
                           unsigned char out[OAF_WIRE_SIZE]);
enum oaf_status oaf_decode(const unsigned char *, size_t, struct oaf_request *);
enum oaf_status oaf_write(const struct oaf_view *, struct oaf_request, int32_t);
enum oaf_status oaf_read(const struct oaf_view *, struct oaf_request,
                         int32_t out[8]);
enum oaf_status oaf_wire_write(const struct oaf_view *, const unsigned char *,
                               size_t, int32_t);
enum oaf_status oaf_wire_read(const struct oaf_view *, const unsigned char *,
                              size_t, int32_t out[8]);
#endif
