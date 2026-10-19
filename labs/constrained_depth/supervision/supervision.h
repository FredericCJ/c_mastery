#ifndef CMASTERY_CONSTRAINED_SUPERVISION_H
#define CMASTERY_CONSTRAINED_SUPERVISION_H
#include <stdatomic.h>
#include <stdint.h>
enum cs_status { CS_OK, CS_WAIT, CS_BUSY, CS_INVALID, CS_EXHAUSTED, CS_RETRY, CS_SAFE, CS_UNIMPLEMENTED };
#define CS_TASKS 4u
typedef struct {atomic_uint sequence,epoch[CS_TASKS];unsigned count,boot,last[CS_TASKS];} CSWorld;
/* Fresh stable storage, exclusive init, one supervisor, concurrent progress
 * calls. Epoch advances ONLY after a committed useful transaction. */
enum cs_status cs_init(CSWorld *,unsigned tasks,unsigned nonzero_boot);
enum cs_status cs_progress(CSWorld *,unsigned task);
/* One bounded coherent-read attempt. WAIT/BUSY do not alter last[]. */
enum cs_status cs_window(CSWorld *);
typedef struct {atomic_uint pending;unsigned required,seen;} CSBitmap;
enum cs_status cs_bitmap_init(CSBitmap *,unsigned tasks);
enum cs_status cs_bitmap_progress(CSBitmap *,unsigned task);
enum cs_status cs_bitmap_window(CSBitmap *);
#ifdef CS_TEST_HOOK
/* Host-only deterministic interleaving instrumentation, excluded from target. */
void cs_bitmap_hook(CSBitmap *);
void cs_snapshot_hook(CSWorld *);
#endif
enum cs_reason { CS_HUNG=1, CS_OVERLOAD, CS_BOOT_FAILED, CS_CORRUPT };
typedef struct {uint32_t magic,attempts,reason,terminal,inverse[4];} CSRecovery;
enum cs_status cs_recovery_init(CSRecovery *);
/* Validity is accidental corruption detection, not authentication. At most3
 * retry permissions per initialized record; corrupt record defaults to SAFE. */
enum cs_status cs_failure(CSRecovery *,enum cs_reason);
typedef struct {uint64_t millivolts,microamps,microseconds;} CSPower;
/* Sum is exact femtojoules; divide by completed transactions for a rational
 * per-transaction value. No physical observation is inferred from inputs. */
enum cs_status cs_energy(const CSPower *,unsigned,uint64_t *femtojoules);
#endif
