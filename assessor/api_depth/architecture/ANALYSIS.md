# T20 assessor technical analysis

Withheld author implementation and analysis, not independent review. Read the
original thirty criterion prompts and their exact delayed tasks before using
these notes. The implementations under this directory are one bounded design,
not uniquely correct architecture. The source-bound campaign records what ran.

## T20.01: responsibility and change

The old pipeline decomposition would let every stage interpret wire fields and
physical order. Here `core.c` alone transforms logical values; encoder/decoder
own signed-wire conversion; storage callbacks own physical order; adapters own
progress and environment. Format1 to2 changes encoder/parser version and size,
not `ar_double`, storage values or indicator decisions. Reversing the storage
array changes only `get`; the two tests deliberately disagree on physical order
while agreeing on logical outputs. A real change-impact table must identify
these actual functions rather than count directory names.

The borrowed path adds storage→operation lifetime dependence, although neither
type mentions a platform. `ArBorrow.data` must remain readable/unchanged while
active, including cancel-requested. Clearing it at request time is premature.
The copied SFStream path changes that bound: one start copies payload into66
inline bytes and subsequent writes need no caller pointer. The parent's pointer
can be reused but the stream owner still cannot be freed while a handle is used.

Compiler dependency output proves a narrow header property only. The deliberately
leaked FILE/GPIO declarations fail because the core interface neither declares
nor includes those types. A repair that merely adds those headers would compile
but violate the architectural boundary. Runtime callback relationships can be
cyclic without cyclic includes; reentry/lifetime rules are separately necessary.
The actual registry plugin is registered before execution, self-removes during
dispatch, and its context release occurs after its callback returns. No recursive
dispatch or generic safe unload is inferred.

## T20.02: policy, mechanism and retained context

Reject-newest `[A,B]+C` keepsA/B and leavesC with its caller. Drop-oldest returnsA
and keepsB/C. The priority transfer returns the earliest minimum only for a
strictly larger incoming priority; ties reject. Payload release accounting must
use identity, not only count. `ArQueue` is an independent compact-array oracle;
`ArRingPolicy` maps stable typed slots to the actual CDRing indices. Its bounded
rebuild for an interior eviction costs O(capacity), but preserves the ring
implementation and FIFO order of survivors. Claiming O(1) priority admission
would be false.12,288 operation histories compare status, returned identities,
priorities and counts, including physical wrap.

The allocation/notification test first fails allocation, then fails subscription
after allocation, and verifies exactly one temporary release. On success the
task snapshots callback/context/version; changing the caller's ArOps record
does not alter the pending operation. That is version pinning with a lifetime
obligation, not a runtime pointer update. The unit counterexample for a wrong
write count is a one-byte remaining request and returned count2. Negative1 is
the smallest negative count. Unsupported status1 violates the write-result
subset without an out-of-bounds access. Fault cleanup may change terminal state
but cannot advance the committed offset on a malformed result.

Compile-time selection makes one direct branch available for optimization;
runtime selection retains function-pointer/context storage and an indirect call.
Inspect each compiler's actual assembly before claiming either was inlined or
devirtualized. Partial write1 versus write2 conforms to the same prefix contract
without equal call counts. No result implies universal speed improvement.

## T20.03: pure transitions and irreversible effects

Overflow admission precedes signed multiplication. Staging makes overlapping
input/output safe for this one API; it does not extend alias permission to other
control/output pairs. The explicit window state is deterministic even though
it evolves. Copying that state permits replay; an implicit static previous value
would make two clients interfere.

`ArEffects` commits a request ID/value as data. The side-effect adapter can commit
its file before the reducer learns success. Rolling back only ArEffects would
not roll back that file. Ambiguous feedback therefore keeps the same pending
request, and retry does not generate another ID. The journal permits one logical
operation, explicitly rejecting a second distinct request; it is not an unbounded
transaction log. Its exactly-once-looking observation is idempotent setting of
that file record, not deduplication of an unrelated actuator, email or network
service. After a failed fsync, a matching readable record still needs successful
fsync before acknowledgment. The source inspection triggered this important
repair and its regression.

The small core domain has1+3+9+27+81=121 sequences. That arithmetic says nothing
about int32 extrema, so separate int64/Python oracles sample full-range values,
preserve rejection sentinels and shrink the actual no-doubling mutant toward
zero. A successful sample is not an exhaustive signed-range proof. The range
argument itself uses INT32_MIN/2 and INT32_MAX/2 before evaluation.

## T20.04: adapters with matching meanings

The file and nonblocking pipe are actual Linux executions. Empty open pipe is
pending; after writer close it is end. A signature returning zero for both
would lose a necessary state. The selected UART driver handles SR/DR ordering;
its adapter admits at most8 bytes and can return an error after a valid prefix.
The register-bus model is not physical USART operation. Target MMIO wrappers,
cold-profile guard and UART initialization are actually linked, but no reset
vector or flashing is supplied by this T20 adapter experiment.

Format1 value513 is bytes02,01,03. Both1+2 and2+1 chunkings must finish at the
same logical record. The parser retains partial prefix only while that record
remains active. On cancellation/retry, silently keeping the old partial bytes
can assemble a false record; the stated retry starts a new parser record. A
deferred borrow cannot be dropped into a synchronous-copy interface merely
because both accept `(pointer,length)`.

Short writes are forced one byte at a time; each of twelve cut positions is
injected. A zero-byte failed attempt can retry from an empty journal. A nonzero
torn prefix refuses recovery rather than guessing an intended operation.
`O_APPEND` is rejected because Linux can ignore pwrite's supplied offset in
that mode. A pipe/descriptor error and malformed existing record are different
causes even if the selected API coarsens both to AR_FAILED; record errno or
recovery context in the learner dossier. Volatile truncation deliberately loses
deduplication memory. No physical-power claim follows from child termination.

## T20.05: selected GPIO comparison

Threshold belongs to the logical service; conversion belongs to a value adapter;
active-low mapping belongs to board configuration; BSRR and W1C belong to the
driver. The actual HAL source handles many more modes than the selected minimal
implementation. Compare the same push-pull, low-speed, no-pull output branch,
including clock enable/read-back/barrier and exclusive initialization. HAL's
selected GPIO functions have no heap acquisition or scheduling call; this is a
source/link result for these functions, not all HAL modules. No interrupt is
enabled for the selected output mode, and the unused EXTI callback is not counted
as an active dependency after section garbage collection.

BSRR emits one set/reset mask. HAL toggle reads ODR first, so a higher-level
concurrent toggle operation is not made atomic just by its final BSRR write.
Initialization read/modify/write touches shared configuration words and needs
one configuration owner. Two independent pins can use serialized calls in the
host model; genuinely concurrent clients need a target synchronization design.
The direct W1C mask1 leaves pending bit2 set from initial3; RMW with3 writes both
ones and clears both. RAM-object vendor tests compare register values written,
not electrical effects or read-only/write-only behavior. No glitch-free startup,
electrical timing, reset trace or pin waveform is claimed.

## T20.06: scheduling assumptions

FIFO/polling with identical FIFO policy and costs yields completions2,4,6. Two
worker slots sharing one CPU with quantum-one rotation yield3,4,6; the third job
waits for a worker slot instead of secretly becoming a third worker. The revised
one-compute/three-wait/one-compute model explicitly compares one worker5,10,15,
two workers5,6,11 and three asynchronous slots5,6,7 under the stated ready order.
The extra overlap has an extra retained-operation resource cost.

Enqueue-before-service gives original processed2,2,2, drop0,0,0, pending1,0,2;
overload processed2,2,2, drop1,0,1, pending2,0,2. Service-before-arrival gives
original processed0,2,2 and pending3,2,4, overload processed0,2,2 and pending4,2,4.
The same final pending count does not imply the same latency/identity history.
After burst4 and one service2, backlog2; arrivals<=2 per interval preserve that
post-service bound. Arrival3 makes pre-service occupancy5 exceed capacity4.
The discrete response bound requires a phase convention and bounded callbacks.
With no callback-duration bound, no finite completion bound follows from FIFO.

## T20.07: composition and quiescence

The selected product has three meaningful ownership states: empty, acquired,
submitted. Cancellation intent is orthogonal only while submitted. Refactoring
terminal cleanup into a parent state can reduce repetition but cannot permit
acquired-with-no-owner or released-yet-submitted states. Acknowledgment's dedicated
slot resolves the full ordinary queue counterexample; it does not force an
uncooperative provider to acknowledge. The priority-step mutant processes a
queued completion first and fails the specified cancellation-ack ordering.

The request counter never resets at restart; incarnation separately distinguishes
the child generation. Stale queued events are rejected after restart/new acquire.
Both terminal orderings retain a single release. Independent machines commute;
one free admission slot with A/B does not. The pthread variant locks shared
mutation and joins before destruction; no unsynchronized field is read until
join. This is a concrete legal execution, not proof of all OS interleavings or
a general wait-free protocol.

## T20.08: identity and ABI

Canonical bytes are independent of field ordering and structure padding.
Optional unknown fields are ignored by policy; required unknown fields reject.
Do not hash raw C object bytes and call the result semantic identity. The feature
variant intentionally changes `sizeof(ArConfig)` and requires a compatible ABI
or explicit wire adapter. C17 does not enforce that independently built consumers
used identical macros. Source/tool/generated-header IDs and final artifact hash
serve distinct provenance roles; none is a correctness or authenticity proof.

The old generated-header/binary pair is retained, the header changes, and the old
result is rejected against the new expected identity. A newly compiled binary
then reports the new identity. Relocation comparisons report actual equality,
not an assumption of reproducible bytes on every compiler/build flag.

## T20.09: bounded faults and recovery

The exact supervisor sequence is normal,normal,degraded,normal,stopped with final
streak0. Logger exhaustion changes only its loss counter. Recovery from stopped
or degraded owns A before B; B failure releases A, does not falsely activate B,
and leaves the mode unchanged. Success must not be restarted while its resources
remain live. Two attempts bound this instance, not a sliding window with hidden
resets. Ownership transfer to a new supervisor would need another explicit bound.

The returned-value defect model can be validated; arbitrary shared-address-space
overwrites cannot. The bounded Python value-overwrite example is explicitly a
design counterexample, not an exploit or native memory-safety claim. The requested
process alternative uses a copied12-byte message, incarnation, monotonic deadline
and terminal wait/reap before reclamation. A real sandbox would additionally
constrain inherited capabilities, which this design does not claim to implement.

Positive progress with terminal failure is legal only in the revised profile.
The prefix is kept and the remaining bytes are not replayed automatically.
Duplicate terminal polling returns the stored terminal result without another
release. Cleanup notification removal is infallible/quiescent by contract;
if that premise changes, this implementation is insufficient and must retain
storage until a separate removal acknowledgment.

## T20.10: evidence and changed decisions

A synchronous borrow with bounded completion and no retention can meet immediate
caller reuse. An asynchronous retained borrow cannot. The alternatives are
copying n bytes into n extra bytes, delayed caller reuse, or transferring the
buffer's ownership. Their equivalent success/error/completion contracts precede
cost comparison. No finite buffer admits unbounded sustained overload with no
loss and an unavailable consumer.

The intentionally ineffective test links the no-doubling mutant yet exits zero
without invoking it. This positive process result is marked rejected evidence;
the actual semantic test and property oracle expose it. Distinct assertions for
capacity/identity, retention/pinning, cancel/release and recovery/rollback are
mapped to their own original objectives. A graph edge is not an assertion.
When old arrivals return but restart acknowledgment becomes mandatory, the old
latency proof still lacks the new recovery premise. Historical evidence stays
valid for its original conditions and cannot be silently relabeled.

## Evidence limits and assessment

Require all original ordinary and delayed criteria, not a score from the count
of green commands. The canonical bindings identify exact command selectors and
separate C execution, target cross-build, finite model and design analysis.
External review, independent learner transfer and physical board observation
remain unperformed. Hint escalation and level0..3 distinctions remain in each
original supplement. A level3 answer must reconstruct the relevant invariant
under its changed assumptions and explain a counterexample to an excessive
safety, portability, latency or equivalence claim.
