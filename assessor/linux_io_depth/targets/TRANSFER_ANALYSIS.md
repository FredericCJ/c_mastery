# T62 transfer assessor analysis

IO5 maintains cursor equal to the accepted stream prefix. The temporary vector
is rebuilt from that cursor after every positive result; negative results cannot
subtract already transferred bytes. A terminal error is compatible with prior
progress. Callback result injection validates the adapter transition, while real
writev partial progress validates the syscall interaction separately. Neither
shows that a whole application request is atomic.

IO6 must maintain user-space readiness after a finite visit; a new kernel edge is
not promised for the unread suffix. Alternating next ensures B is selected within
two ready visits under the stated finite-work model. It does not bound scheduler
descheduling, page faults, interrupt load or blocking code accidentally introduced
by a learner. Last progress and operation start are distinct origins: resetting
both lets a slow peer extend total lifetime indefinitely. Expiry wins a simultaneous
progress event by the contract's explicit ordering.

IO7's mutex covers each complete head/count/value transition. Condition waits
atomically release the lock and recheck the predicate after reacquiring. Signals
alone are not stored permissions. Closing wakes both classes and retains queued
ownership for draining. Capacity3 deliberately defeats `(head+count)&(capacity-1)`.
The actual issuer is single-threaded even though its input is concurrent. Scaling
to multiple io_uring issuers requires a new publication proof, not removal of
this queue mutex. Withheld variant: make queue payloads borrowed buffers and
cancel an accepted operation before its terminal CQE; identify the earliest valid
release point and account for rejected queue admission separately.

IO8 a successful MORE CQE is an event, not the terminal lifetime event. Timeout
and target each own an identity; permit either ordering while rejecting duplicates.
The100ms duration makes the first-event observation practical but is not an API
timing guarantee. If a slow test environment times out before the event, report
the actual failed assumption and rerun with a documented larger deadline; never
accept a missing MORE as proof that this mechanism was exercised.

IO9: the three congruences are independent; memory address alignment cannot
justify offset or length. The direct-path selection is an application policy
based on observed statx fields, not an assertion that every misaligned Linux I/O
fails. The manual permits different filesystem behavior, including fallback.
File synchronization covers data/required metadata; the directory operation
covers the created name under the selected filesystem contract. Raw timings
include both, and may be dominated by synchronization rather than copy costs.
Withheld transfer: an admitted direct write returns a short prefix not divisible
by the required offset alignment. A naïve aligned remainder loop is no longer
admissible. Require explicit error/progress reporting or a separately specified
recovery path; do not round the offset backward and duplicate bytes.

IO10: the temporary file's fsync precedes rename. Source directory synchronization
concerns removal of tmp; destination synchronization concerns replacement of
target. With distinct directories neither substitutes for the other. An error
after rename cannot be represented as unchanged output, because another opener
can already observe the replacement. The phase/result contract exposes this
uncertainty and avoids a destructive rollback fantasy. The missing-destination-
sync mutant is rejected by actual call instrumentation: relying solely on one
crash replay can miss a broken protocol because unrelated writeback may happen
to persist the name. The original probe's wrapper records real inode identities
and forwards to real fsync; it does not emulate persistence.

The guest kill occurs after serial output of each marker. CPU scheduling and
device activity may continue between the operation and host kill; record that
window, do not call it an instruction-exact hardware power cut. Ext4 journal replay
and QEMU backing-file behavior are selected implementation evidence. A full
research analysis should vary filesystem, mount mode, block flush behavior and
cut points within operations, with source-grounded expected admissible states.
