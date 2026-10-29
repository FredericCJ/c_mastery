#ifndef CMASTERY_BUILD_CACHE_H
#define CMASTERY_BUILD_CACHE_H
#include "metaprog.h"

enum
{
    MPB_OK,
    MPB_INVALID,
    MPB_CYCLE,
    MPB_FULL,
    MPB_BUSY,
    MPB_DONE,
    MPB_FAILED,
    MPB_STALE,
    MPB_EXHAUSTED,
    MPB_IO,
    MPB_UNCERTAIN,
    MPB_MISS
};

enum
{
    MPB_SOURCE,
    MPB_TOOL,
    MPB_CONFIG,
    MPB_ENVIRONMENT,
    MPB_INCLUDE,
    MPB_SCHEMA
};

struct mpb_bytes
{
    unsigned char bytes[8];
    size_t length;
};

struct mpb_node
{
    unsigned id, dependencies;
    int32_t revision;
    struct mpb_bytes inputs[6];
};

struct mpb_entry
{
    uint64_t key;
    unsigned char input[512];
    size_t input_length;
    char artifact[256];
    size_t artifact_length;
};

struct mpb_plan
{
    struct mpb_entry entries[8];
    unsigned order[8], dependencies[8];
    size_t count;
};

int mpb_plan(const struct mpb_node *nodes, size_t count,
             unsigned force_collision, struct mpb_plan *output);
int mpb_match(const struct mpb_entry *expected, const struct mpb_entry *cached);
int mpb_encode(const struct mpb_entry *entry, unsigned id, unsigned char *bytes,
               size_t capacity, size_t *used);
struct mpb_schedule;

struct mpb_ticket
{
    const struct mpb_schedule *owner;
    uint64_t epoch;
    unsigned id;
};

struct mpb_schedule
{
    const struct mpb_plan *plan;
    uint64_t epoch;
    unsigned pending, running, done, failed;
};

int mpb_begin(struct mpb_schedule *schedule, const struct mpb_plan *plan);
int mpb_claim(struct mpb_schedule *schedule, struct mpb_ticket *ticket);
int mpb_finish(struct mpb_schedule *schedule, struct mpb_ticket ticket,
               const struct mpb_entry *produced);
int mpb_publish(const struct mpb_schedule *schedule, struct mpb_plan *output);
#endif
