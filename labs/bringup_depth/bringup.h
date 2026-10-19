#ifndef BU_BRINGUP_H
#define BU_BRINGUP_H
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
_Static_assert(CHAR_BIT == 8 && UINT_MAX == UINT32_MAX,
               "Selected byte-addressed target with32-bit unsigned required");
/* See CONTRACTS.md. Addresses are modeled numbers, never host pointers. */
typedef enum {
  BU_OK,
  BU_INVALID,
  BU_UNSUPPORTED,
  BU_TIMEOUT,
  BU_FAILED,
  BU_AMBIGUOUS
} BuResult;
typedef struct {
  uint32_t base, size;
} BuRange;
typedef struct {
  uint32_t load, run, size;
} BuCopy;
typedef struct {
  uint32_t stack, reset;
} BuVectors;
int bu_contains(BuRange outer, BuRange inner);
int bu_overlap(BuRange a, BuRange b);
BuResult bu_vectors(const unsigned char *bytes, size_t n, unsigned encoding,
                    uint32_t base, BuRange code, BuRange stack, BuVectors *out);
BuResult bu_memory(unsigned char *ram, size_t nr, const unsigned char *rom,
                   size_t nf, const BuCopy *copy, size_t nc,
                   const BuRange *zero, size_t nz, BuRange retained);
int bu_f439_dma(BuRange bytes);
typedef struct {
  unsigned phase, polls, limit, bypass, dependencies, epoch;
} BuClock;
BuResult bu_clock_start(BuClock *c, unsigned limit, unsigned bypass);
BuResult bu_clock_step(BuClock *c, unsigned oscillator_ready,
                       unsigned pll_ready);
BuResult bu_clock_reconfigure(BuClock *c, unsigned bypass);
BuResult bu_clock_failure(BuClock *c);
typedef int (*BuHookFn)(void *);
typedef struct {
  unsigned dependencies, services;
  BuHookFn call;
  void *context;
} BuHook;
BuResult bu_hooks(const BuHook *hooks, unsigned n, unsigned services,
                  unsigned *completed);
/* 24-byte serialized record; at most eight payload bytes. No packed struct. */
#define BU_RECORD 24u
typedef struct {
  unsigned char version, length, generation, payload[8];
} BuRecord;
BuResult bu_decode(const unsigned char *bytes, size_t n, BuRecord *out);
BuResult bu_store(unsigned char *bytes, size_t n, const BuRecord *value,
                  unsigned writes_allowed, unsigned *writes);
BuResult bu_select(const unsigned char slots[2][BU_RECORD], unsigned *selected);
BuResult bu_migrate(const BuRecord *old, BuRecord *current);
typedef struct {
  uint32_t stack, vector, entry;
  unsigned privileged, thread_mode, masks_closed, pending_clear, dma_active;
  unsigned clock_known, cache_clean, peer_quiescent;
} BuHandoff;
BuResult bu_handoff(const BuHandoff *h, BuRange code, BuRange stack);
#endif
