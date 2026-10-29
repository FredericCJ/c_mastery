#ifndef CORPUS_MONEY_H
#define CORPUS_MONEY_H
#include <stdint.h>
#include <stddef.h>
#define CM_MAX 32u

struct cm_money
{
    int64_t minor;
    char currency[3];
    uint8_t scale;
};

enum cm_status
{
    CM_OK,
    CM_INVALID,
    CM_OVERFLOW,
    CM_FUNDS,
    CM_ABORTED
};

enum cm_round
{
    CM_ZERO,
    CM_FLOOR,
    CM_EVEN
};
enum cm_status cm_make(int64_t, const char currency[3], unsigned scale,
                       struct cm_money *);
enum cm_status cm_add(struct cm_money, struct cm_money, struct cm_money *);
enum cm_status cm_ratio(struct cm_money, uint32_t numerator,
                        uint32_t denominator, enum cm_round, struct cm_money *);
enum cm_status cm_allocate(struct cm_money, const uint32_t *weights,
                           size_t count, struct cm_money *shares);
/* Atomic publication is logical and single-threaded, not C atomic or durable. */
enum cm_status cm_settle(struct cm_money *accounts, size_t count, size_t from,
                         const size_t *destinations, const uint32_t *weights,
                         size_t recipients, struct cm_money amount,
                         size_t abort_after);
#endif
