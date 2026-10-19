# Assessor: target-specific erase and reclamation

The expected F439 trace clears W1C error bits, unlocks only if locked, selects
PSIZE=10, issues SER/SNB/STRT or PG then one aligned32-bit write, waits BSY0,
restores controls, refreshes FLASH_ACR caches and verifies memory. EOPIE stays0;
requiring EOP would hang an otherwise completed operation. For H745 SER is
bit2, START bit7, SNB10:8; BER bit3 must stay0. PSIZE is preserved and the
selected source requires a qualified supply. No callback may restore CR or
read flash while the operation is queued/busy.

The operation is one-way until terminal. Hardware failure permits controller
cleanup after quiescence but does not establish cell reuse. The readback scan
visits32words/call, taking1024calls for128KiB; work is O(sector bytes) and
operation storage is constant. The synchronous completion model is separate
from the real asynchronous target: read/write callbacks are actual peripheral
operations, not arbitrary side-effect-free mock values.

In reclamation, the induction invariant is an intact validated record outside
the sector currently being erased/programmed. After inactive erase, old is
unchanged. During body/commit programming, old is unchanged. After exact
new-record readback and CRC confirmation, new is valid before old erase.
Failed old erase leaves new valid. Equal sequences with distinct valid content
are corruption; choosing by address would conceal it. CRC collision resistance
is not cryptographic authenticity. The test's inject-after-program corruption
must prevent old erase even though the low-level word readbacks once passed.

The mutant suite removes three different obligations: waiting for quiescence,
checking physical readback, and revalidating the new record before destroying
the old. The scaffold must compile and fail a semantic assertion. Delayed
bank2 assessment requires recognizing F439 encodedSNB12..15 forbidden and
logical12 beginning at encoded16, plus selected exact capacity/DB1M profile.
It must not alter accepted source inspection or claim the existing restricted
adapter already implements that variant.

Physical counters, fault-contained ECC snapshots, complete signed boot policy,
MPU/vector setup and reset sequencing are distinct remaining work. Neither the
host cut model nor freestanding target disassembly establishes any of them.
