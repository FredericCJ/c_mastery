#ifndef BU_BRINGUP_H
#define BU_BRINGUP_H
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
_Static_assert(CHAR_BIT == 8 && UINT_MAX == UINT32_MAX,
               "Selected byte-addressed target with32-bit unsigned required");

/* See CONTRACTS.md. Addresses are modeled numbers, never host pointers. */
enum bu_result
{
    BU_OK,
    BU_INVALID,
    BU_UNSUPPORTED,
    BU_TIMEOUT,
    BU_FAILED,
    BU_AMBIGUOUS
};

struct bu_range
{
    uint32_t base, size;
};

struct bu_copy
{
    uint32_t load, run, size;
};

struct bu_vectors
{
    uint32_t stack, reset;
};

int bu_contains(struct bu_range outer, struct bu_range inner);
int bu_overlap(struct bu_range a, struct bu_range b);
enum bu_result bu_vectors(const unsigned char *bytes, size_t n,
                          unsigned encoding, uint32_t base,
                          struct bu_range code, struct bu_range stack,
                          struct bu_vectors *out);
enum bu_result bu_memory(unsigned char *ram, size_t nr,
                         const unsigned char *rom, size_t nf,
                         const struct bu_copy *copy, size_t nc,
                         const struct bu_range *zero, size_t nz,
                         struct bu_range retained);
int bu_f439_dma(struct bu_range bytes);

struct bu_clock
{
    unsigned phase, polls, limit, bypass, dependencies, epoch;
};
enum bu_result bu_clock_start(struct bu_clock *c, unsigned limit,
                              unsigned bypass);
enum bu_result bu_clock_step(struct bu_clock *c, unsigned oscillator_ready,
                             unsigned pll_ready);
enum bu_result bu_clock_reconfigure(struct bu_clock *c, unsigned bypass);
enum bu_result bu_clock_failure(struct bu_clock *c);
typedef int (*bu_hook_fn)(void *);

struct bu_hook
{
    unsigned dependencies, services;
    bu_hook_fn call;
    void *context;
};
enum bu_result bu_hooks(const struct bu_hook *hooks, unsigned n,
                        unsigned services, unsigned *completed);
/* 24-byte serialized record; at most eight payload bytes. No packed struct. */
#define BU_RECORD 24u

struct bu_record
{
    unsigned char version, length, generation, payload[8];
};
enum bu_result bu_decode(const unsigned char *bytes, size_t n,
                         struct bu_record *out);
enum bu_result bu_store(unsigned char *bytes, size_t n,
                        const struct bu_record *value, unsigned writes_allowed,
                        unsigned *writes);
enum bu_result bu_select(const unsigned char slots[2][BU_RECORD],
                         unsigned *selected);
enum bu_result bu_migrate(const struct bu_record *old,
                          struct bu_record *current);

struct bu_handoff
{
    uint32_t stack, vector, entry;
    unsigned privileged, thread_mode, masks_closed, pending_clear, dma_active;
    unsigned clock_known, cache_clean, peer_quiescent;
};
enum bu_result bu_handoff(const struct bu_handoff *h, struct bu_range code,
                          struct bu_range stack);
#endif
