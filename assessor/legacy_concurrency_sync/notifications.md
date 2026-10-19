# Notifications technical analysis (withheld assessor material)

The crucial decomposition is stored credit plus one reserved successful claim.
An IRQ post to an empty blocked recipient consumes one internally in CMX and
queues the surplus. Counting only task.notifications loses that handoff; counting
it twice violates accepted=consumed+cancelled+stored+claimed. `settle` reads the
actual selected terminal result before post/result/cancel/close decisions.
Timeout-before-post therefore has claimed0 and queues the new batch for a later
operation; success-before-cancel retains claimed1. An external task operation
before collection could overwrite CMX result, hence exclusive task protocol is
a real prerequisite, not a consequence of the handle representation.

Up-to-N is linearized at result collection: subtract one reserved claim from N,
then remove min(remaining requested,stored) under the same exclusion. This is
constant work and preserves at least one returned occurrence after success.
Consume-all uses UINT32_MAX; stored plus claim may exceed that representable
result by one. The remainder is retained, not wrapped. Close has room for its
cancelled addition because that stored subset is already included in accepted.
Post conservatively reserves both accepted/rejected accounting and a next fault
sequence, so finite counters never trap cleanup. No unseen future event can be
guaranteed admitted by a bounded representation.

FULL leaves the whole batch with the caller, while the explicit acknowledged fault
channel accounts for a refused attempt. Sequence plus pending prevents overwrite.
Backpressured attempts are not added to refused again. Clamp is a different API:
it needs an accepted-count result and responsibility for its remainder. The old
fixture is preserved as that contrast; the reference does not rewrite it.

The1024 histories use independent integer counts for post1/post3/take1/consume-all;
they cover exactly five steps at capacity8, not every count/schedule. Separate
boundaries cover UINT32_MAX and constructed reachable uint64 exhaustion. Mutation
controls distinguish clamp, surplus loss, stale identity, Boolean collapse,
up-to/all confusion and missing fault publication. All stubs remain unsolved.

PRIMASK protects single-core shared core state; the wrapper preserves saved mask
and enforces a stated IRQ priority policy. BASEPRI holds lower-priority PendSV
while allowing IRQ0. Four real exceptions demonstrate burst count ownership and
coalesced scheduling. SysTick is a24-bit virtual timer in QEMU; its observed deltas
include instrumentation and model scheduling. Neither these nor pthread samples
are physical execution-time bounds. Target link audits exclude implicit helpers
except original memory routines explicitly compiled into the image.

The hosted extension uses pthread exclusion, not volatile fields or individually
atomic loads. Two producers on one allowed CPU and consumer on another exercise
cross-CPU visibility with mutex synchronization. This is still one serialized
logical CMX state machine. The atomic load/store counterexample is intentionally
race-free and deterministically forced: both loads see0; each store1; fetch_add
instead yields2. TSan need not complain about a semantic lost update. No hardware
weak-order coverage or lock-free reclamation theorem is claimed.

Assessor score3 requires explaining a changed condition before implementation and
diagnosing an unfamiliar failure after the original delay. Source-bound author
campaigns establish executable reference support only. Treat wrong markers,
failed compilation, signals, sanitizer reports or absent affinity as failed
evidence. Native unexpected port failures may occur after mutation; this adapter
requires stop/inspect rather than replay on ambiguous effect.
