#ifndef SECURITY_DEPTH_COMMAND_H
#define SECURITY_DEPTH_COMMAND_H
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

enum sc_result
{
    SC_OK,
    SC_LENGTH,
    SC_OPCODE,
    SC_COUNT,
    SC_DENIED,
    SC_BUDGET,
    SC_STALE,
    SC_FULL,
    SC_INVALID,
    SC_EXHAUSTED,
    SC_UNIMPLEMENTED
};

struct sc_authority
{
    unsigned generation, allowed;
};

struct sc_command
{
    unsigned opcode, count, generation;
    uint16_t values[4];
};

struct sc_batch
{
    unsigned count, work;
    struct sc_command commands[4];
};

struct sc_meter
{
    unsigned headers, values, bytes;
};
enum sc_result sc_decode(const unsigned char *, size_t,
                         const struct sc_authority *, int, struct sc_batch *,
                         struct sc_meter *);
enum sc_result sc_revoke(struct sc_authority *);
enum sc_result sc_apply(const struct sc_batch *, const struct sc_authority *,
                        uint32_t *, uint32_t *);
enum sc_result sc_decimal(const unsigned char *, size_t, int, unsigned *,
                          char[2]);
enum sc_result sc_extent(size_t, size_t, size_t, size_t, size_t *);
/* Original intentionally flawed authority target; synthetic counter only. */
enum sc_result sc_flawed_authority(const unsigned char *, size_t, uint32_t *);

struct sc_ticket
{
    unsigned index, generation;
};

struct sc_job
{
    unsigned active, generation, retries;
    struct sc_batch request;
};

struct sc_service
{
    struct sc_job jobs[3];
    unsigned used, count, terminal, released, control_pending, closed;
};

void sc_service_init(struct sc_service *);
enum sc_result sc_submit(struct sc_service *, const struct sc_batch *,
                         struct sc_ticket *);
enum sc_result sc_finish(struct sc_service *, struct sc_ticket, int,
                         const struct sc_authority *, uint32_t *, uint32_t *);
enum sc_result sc_retry(struct sc_service *, struct sc_ticket);
enum sc_result sc_control(struct sc_service *);
enum sc_result sc_control_complete(struct sc_service *);
enum sc_result sc_shutdown(struct sc_service *);

struct sc_resources
{
    int (*acquire)(void *, unsigned);
    void (*release)(void *, unsigned);
    void *context;
};

struct sc_recovery
{
    unsigned state, attempts, owned, log_fallbacks;
};
enum sc_result sc_recover(struct sc_recovery *, struct sc_resources, int);
#endif
