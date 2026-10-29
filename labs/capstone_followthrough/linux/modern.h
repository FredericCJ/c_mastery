#ifndef CAPSTONE_MODERN_H
#define CAPSTONE_MODERN_H
#include "service.h"
#include "ownership.h"
#include "errors.h"

struct cs_result_sink
{
    cs_sink_fn sink;
    void *context;
};

er_progress cs_result_step(const unsigned char *, size_t, size_t,
                           struct cs_result_sink *);

struct cs_slab
{
    struct ow_shared rights;
    struct cs_wire wire;
    unsigned generation, live, destructions;
};

struct cs_slabs
{
    struct cs_slab slot[4];
};

struct cs_slab_token
{
    unsigned slot, generation, consumer;
};

void cs_slabs_init(struct cs_slabs *);
enum cs_status cs_slabs_share(struct cs_slabs *, const struct cp_message *,
                              struct cs_slab_token[2]);
enum cs_status cs_slabs_view(struct cs_slabs *, struct cs_slab_token,
                             const unsigned char **, size_t *);
enum cs_status cs_slabs_drop(struct cs_slabs *, struct cs_slab_token);

struct cs_receipts
{
    struct cp_message messages[4];
    unsigned epoch, count, effects;
};
enum cs_status cs_receipt(struct cs_receipts *, unsigned,
                          const struct cp_message *);
struct cs_resources;

struct cs_resource_entry
{
    struct cs_resources *owner;
    unsigned id;
};

struct cs_resources
{
    struct ow_scope scope;
    struct cs_resource_entry entries[5];
    void *candidate, *queue;
    int input, output, epoll;
    unsigned acquired, release_ids[5], released, borrows;
    int modern, published, closing, cleanup_error;
};

int cs_resources_open(struct cs_resources *, int, int, int, unsigned, int);
int cs_resources_close(struct cs_resources *);
int cs_resources_borrow(struct cs_resources *);
int cs_resources_ack(struct cs_resources *);
#endif
