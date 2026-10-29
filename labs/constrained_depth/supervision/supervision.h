#ifndef CMASTERY_CONSTRAINED_SUPERVISION_H
#define CMASTERY_CONSTRAINED_SUPERVISION_H
#include <stdatomic.h>
#include <stdint.h>

enum cs_status
{
    CS_OK,
    CS_WAIT,
    CS_BUSY,
    CS_INVALID,
    CS_EXHAUSTED,
    CS_RETRY,
    CS_SAFE,
    CS_UNIMPLEMENTED
};

#define CS_TASKS 4u

struct cs_world
{
    atomic_uint sequence, epoch[CS_TASKS];
    unsigned count, boot, last[CS_TASKS];
};
/* Fresh stable storage, exclusive init, one supervisor, concurrent progress
 * calls. Epoch advances ONLY after a committed useful transaction. */
enum cs_status cs_init(struct cs_world *, unsigned tasks,
                       unsigned nonzero_boot);
enum cs_status cs_progress(struct cs_world *, unsigned task);
/* One bounded coherent-read attempt. WAIT/BUSY do not alter last[]. */
enum cs_status cs_window(struct cs_world *);

struct cs_bitmap
{
    atomic_uint pending;
    unsigned required, seen;
};
enum cs_status cs_bitmap_init(struct cs_bitmap *, unsigned tasks);
enum cs_status cs_bitmap_progress(struct cs_bitmap *, unsigned task);
enum cs_status cs_bitmap_window(struct cs_bitmap *);
#ifdef CS_TEST_HOOK
/* Host-only deterministic interleaving instrumentation, excluded from target. */
void cs_bitmap_hook(struct cs_bitmap *);
void cs_snapshot_hook(struct cs_world *);
#endif
enum cs_reason
{
    CS_HUNG = 1,
    CS_OVERLOAD,
    CS_BOOT_FAILED,
    CS_CORRUPT
};

struct cs_recovery
{
    uint32_t magic, attempts, reason, terminal, inverse[4];
};
enum cs_status cs_recovery_init(struct cs_recovery *);
/* Validity is accidental corruption detection, not authentication. At most3
 * retry permissions per initialized record; corrupt record defaults to SAFE. */
enum cs_status cs_failure(struct cs_recovery *, enum cs_reason);

struct cs_power
{
    uint64_t millivolts, microamps, microseconds;
};
/* Sum is exact femtojoules; divide by completed transactions for a rational
 * per-transaction value. No physical observation is inferred from inputs. */
enum cs_status cs_energy(const struct cs_power *, unsigned,
                         uint64_t *femtojoules);
#endif
