#ifndef EF_FOUNDATION_H
#define EF_FOUNDATION_H
#include "com_edu7.h"
#include "edu7_depth.h"

enum
{
    EF_OK,
    EF_INVALID,
    EF_BUSY,
    EF_STALE,
    EF_FULL,
    EF_DROP,
    EF_STUB = 99
};

struct ef_budget
{
    unsigned canonical, session, count, last, transport, network, body, wire;
};

struct ef_route
{
    struct edu_datagram packet;
    const uint8_t *borrow;
    uint32_t generation;
    unsigned active, mode, egress, next;
};

struct ef_framer
{
    struct edu_framer legacy;
    uint8_t bytes[47];
    unsigned length, active, escape, discard;
};

struct ef_wide
{
    struct edu_record base;
    int64_t value;
    uint8_t extra[8], extra_length;
};

int ef_budget(unsigned schema, unsigned extra, struct ef_budget *);
int ef_enqueue(struct ef_route *, const uint8_t *, size_t, unsigned ingress,
               unsigned copy, unsigned version, uint32_t *);
int ef_peek(const struct ef_route *, uint32_t, struct edu_frame *);
int ef_release(struct ef_route *, uint32_t);
int ef_local(const uint8_t *, size_t, unsigned node, struct edu_datagram *);
int ef_encode(const struct edu_record *, uint8_t *, size_t, size_t *);
int ef_wide_encode(const struct ef_wide *, uint8_t *, size_t, size_t *);
int ef_wide_decode(const uint8_t *, size_t, struct ef_wide *);
int ef_frame_encode(const struct edu_frame *, unsigned version, uint8_t *,
                    size_t, size_t *);
int ef_frame_byte(struct ef_framer *, unsigned version, uint8_t, unsigned fault,
                  struct edu_frame *);
#endif
