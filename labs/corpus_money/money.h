#ifndef CORPUS_MONEY_H
#define CORPUS_MONEY_H
#include <stdint.h>
#include <stddef.h>
#define CM_MAX 32u
typedef struct { int64_t minor; char currency[3]; uint8_t scale; } CmMoney;
typedef enum { CM_OK, CM_INVALID, CM_OVERFLOW, CM_FUNDS, CM_ABORTED } CmStatus;
typedef enum { CM_ZERO, CM_FLOOR, CM_EVEN } CmRound;
CmStatus cm_make(int64_t, const char currency[3], unsigned scale, CmMoney *);
CmStatus cm_add(CmMoney, CmMoney, CmMoney *);
CmStatus cm_ratio(CmMoney, uint32_t numerator, uint32_t denominator, CmRound, CmMoney *);
CmStatus cm_allocate(CmMoney, const uint32_t *weights, size_t count, CmMoney *shares);
/* Atomic publication is logical and single-threaded, not C atomic or durable. */
CmStatus cm_settle(CmMoney *accounts, size_t count, size_t from,
                   const size_t *destinations, const uint32_t *weights,
                   size_t recipients, CmMoney amount, size_t abort_after);
#endif
