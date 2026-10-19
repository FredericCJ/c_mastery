# T20 architecture mechanism contract, version 1

This required implementation successor supplements all thirty original T20
criteria in PR-T20-01 through PR-T20-04 and their exact `-DELAYED` variants.
Their objective-specific prompts, graduated hints and four-level rubrics remain
in the project supplements. This contract supplies the concrete representation,
operations, failure model and experiments those prompts require. Historical
hours and accepted receipts are unchanged. Family selection is justified by
the mechanism: construction, boundaries, diagnosis, failure, verification,
cost, portability and composition dominate; scheduling and process-containment
designs are explicitly analyses, not simulated measurements.

## Prerequisites and artifacts

Retain the original unit prerequisites for checked arithmetic, byte cursors,
object lifetime, function-pointer contexts and the first-principles ring.
The actual composition uses `f_checked_size` and `f_take` from
`labs/foundations/include/foundation.h`, `CDRing` from
`labs/containers_depth/containers.h`, `SFStream` from
`labs/api_depth/stream/stream.h`, and the T19 registry adapter from
`labs/api_depth/abstractions/registry.h`. Submit your own compatible assessed
producer implementations or explicitly identified earlier learner artifacts.
Assessor providers are not distributed as learner solutions. The board adapter
consumes the separately declared F439 receive-driver contract; its host bus
model is not a real UART. The generated source-bound receipt pins exact consumed
sources. A component name alone does not establish composition.

## Environment and common rules

ISO C17 remains the core baseline. Core algorithms use fixed caller storage,
integer types, loops and callbacks; no algorithm/container library is permitted.
Exact-width integer types must exist. The core header imports only standard
integer/size declarations. Linux adapters explicitly require POSIX descriptors,
`read`, positional I/O, `fsync`, pipes and child-process lifecycle. Host Python,
compilers, sanitizers and object tools are verification dependencies, not target
runtime dependencies. Vendor comparison permits the pinned GPIO HAL only for
the comparison branch; implement the minimal driver yourself.

Every object is live, correctly initialized, stable in address and owned by one
serialized caller unless the particular test specifies an external mutex.
Inputs, outputs and control structures are disjoint, except `ar_double` permits
input/output overlap through staging. No function can validate arbitrary pointer
liveness. Callbacks return normally, do not reenter or mutate their owner, and
honor the declared storage/progress contracts. Allocator callbacks return actual
storage of the requested size or null. Deallocation and notification removal
complete synchronously and cannot fail; notification installation can fail.
No callback can outlive its retained context. These are programmer obligations;
ISO C does not implement ownership, linear types, race freedom or cancellation.
Diagnostics, runtime guards, model arguments and finite experiments enforce
different subsets. Deliberately returning a wrong count is the contained fault
model; arbitrary memory corruption is outside it.

The chosen ARM target is STM32F439/Cortex-M4, with its own register/ABI rules.
GPIO comparison assumes the HSI reset profile, an exclusively configured free
PA2 teaching pin, enabled clock, no concurrent configuration and no conflicting
UART assignment. PA2 GPIO and PA2 USART are alternative experiments, not a
simultaneous board configuration. Cross-link closure is required. No flashing,
board attachment, operational equipment or host security changes are included.

## Formats, storage and pure core — T20.01 and T20.03

`ArRecord` contains one `int32_t`. Format1 is signed16, big-endian two's-complement
wire bytes followed by their XOR; format2 uses signed32 and four data bytes plus
XOR. This wire representation is specified independently of the C object
representation. XOR detects selected accidental corruptions and supplies no
authenticity. Encoders require exactly3 or5 output bytes, reject out-of-range
format1 values before publishing, and never copy structure padding. Decoders
publish a value only after full validation. `ArReader` owns up to5 partial bytes;
`ar_feed` accepts a chunk no larger than the remaining record. Incomplete input
commits parser progress and returns `AR_PENDING`; a complete invalid record
preserves the parser state from before that call. A completed record resets
`used` to zero. A cancelled partial record is explicitly discarded before a new
record retry; the transport must not concatenate its abandoned prefix with the
retry. End of input with a partial record is truncation, not an empty record.

`ar_double` stages at most8 values, rejects signed overflow before multiplication
and preserves all output on rejection. `[1,3,5]` produces `[2,6,10]`. Empty input
requires no pointers. `ArStorage.get(context,index,out)` uses logical indices;
the two concrete test stores reverse physical order without changing clients.
The pipeline gathers all records before publishing transformed output.
The changed stateful transform explicitly stores the previous value and a
presence bit; its two-element window sum commits state/output together.

Produce a before/after change-impact table for wire-format and physical-storage
changes. Explain the hidden decision, owning module and changed clients. A
zero-copy view adds a lifetime edge even when source includes remain acyclic.
`ArBorrow` retains 1..8 immutable caller bytes until complete or acknowledged
cancellation. Chunk delivery exposes a committed prefix. A cancellation request
stops new chunks but retains the pointer until `ar_borrow_ack`; retry starts a
new record with offset zero. Caller reuse is legal only after this terminal
point. Compare with the actual `SFStream` copied frame, whose input can be reused
after successful start. Its `accepted` result is total committed offset, not
the number written by the latest pump call.

## Mechanism and policy — T20.02

`ArQueue` is a bounded array oracle. `ArRingPolicy` composes the unchanged actual
`CDRing`, stores slot indices in the ring and keeps typed payloads in stable
slots. It cannot be moved after initialization. Both expose capacities0..AR_CAP,
default4. Push policy0 rejects newest unchanged; policy1 evicts oldest; policy2
admits only when the incoming priority is strictly higher than the minimum
present priority, evicting the earliest minimum. Remaining identities stay in
FIFO order. Accepted ownership transfers to the queue; evicted ownership is
returned through `ArEviction`; rejected input remains caller-owned. Pop transfers
one identity out. Empty/full do not modify output. No payload destructor is
called implicitly. Priority admission does not guarantee fairness: sustained
higher-priority arrivals can prevent a low-priority item from ever being admitted.

Run the same conformance properties for compile-time one/two-byte write policies
and runtime function-pointer selection. Record exact executable sizes, pointer
and context storage, indirect calls and failure traces. The same correctness
contract permits different partial-progress traces; compare complete bytes and
declared prefix semantics, not identical invocation counts.

`ArTask` snapshots allocator, monotonic millisecond clock, write and notification
callbacks with their context and version. Begin allocates/copies1..8 bytes,
installs a notification and commits only after success. Installation failure
frees temporary storage. Changing the caller's policy record cannot replace the
captured callbacks of an active operation. Their context remains live until
terminal cleanup. A second begin while active is rejected. Poll rejects a
backward clock before transport invocation or task mutation. Clock wrap requires
a new inactive epoch; elapsed-time wraparound is not silently accepted.

A write result contains count and status. Negative counts, count beyond the
remaining request, or unknown status are contract faults, diagnosed before
index arithmetic. Ordinary profile0 rejects positive progress plus failure;
changed profile1 accepts that pair and preserves the committed prefix. Zero
progress is pending. All terminal paths uninstall notification and release
storage once. Cancel is two-phase: request disables writes, acknowledgement
permits cleanup. No finite completion bound exists without an eventual provider
acknowledgment. Subscription/context quiescence is a semantic contract, not a
property of a function-pointer signature.

## Semantic adapters and effects — T20.04

Input ports distinguish positive bytes, temporary lack of progress, permanent
end and failure. `ar_linux_read` maps nonblocking `EAGAIN` and interrupted reads
to pending, zero return to end, and other errors to failure; one invocation is
bounded and performs no hidden retry loop. Actual temporary files and a local
nonblocking pipe feed the same parser. `ar_f439_read` composes the unchanged
F439 SR/DR receive driver with a bounded byte count; hardware error can accompany
a valid prefix. The synthetic bus checks register-read order, and the ARM build
checks linkage. Neither supplies physical UART evidence.

`ArEffects` turns a checked doubling decision into an immutable request ID/value
pair. A pending operation refuses another decision. Retry returns that same
pair; ambiguous/failure feedback does not allocate a new ID or undo external
work. Successful acknowledgment clears pending state. Request IDs retire rather
than wrap. The effect reducer and file adapter are separately tested together.

`ar_outbox` is a deliberately narrow idempotent single-operation file effect:
one already-created, exclusively owned descriptor, initially empty; one nonzero
request ID and one int32 value. The descriptor is seekable, read/write and not
append mode; unsupported access/append flags reject before file mutation.
Its12-byte record is `AR01`, big-endian ID and
big-endian value. Successful positional writes followed by successful `fsync`
precede the acknowledgment. The same ID/value retry reads the committed record
and synchronizes it again before success; a different request or value is
rejected. This repeat synchronization matters after an earlier sync failure.
A short existing read is conservatively a recovery failure, not proof of
physical corruption. A torn or extra
record is an explicit recovery failure, never silently overwritten. Losing the
acknowledgment is injected after the file commit; a child exits and the parent
retries the same operation. This is not exactly-once execution of an unrelated
device effect. A volatile reset/truncate loses deduplication evidence. Actual
power-cut durability, filesystem recovery and parent-directory persistence
remain distinct experiments. Errors can leave a torn record and cannot promise
transactional external rollback.

## Driver, board and service responsibilities — T20.05

The service compares milli-units to a milli-unit threshold and emits a logical
indicator state. Unit conversion is explicit checked centi-to-milli multiply10.
Board configuration maps logical state to active-high/active-low pins; two
opposite configurations coexist in one service binary. The minimal driver owns
MODER/OTYPER/OSPEEDR/PUPDR sequencing and BSRR set/reset writes. The caller owns
RCC initialization and exclusive configuration. Runtime writes to separate pins
use the BSRR contract; same-pin competing requests still require an owner/order.
Write-one-to-clear status uses a direct mask, never generic read/modify/write.

Compare actual HAL GPIO v1.8.3 using device headers2.6.10 and CMSIS core5.9.0
against the minimal selected push-pull/low-speed/no-pull branch. RAM-object tests
compare resulting configuration fields and BSRR write values; they do not
simulate peripheral side effects. Cross-compile actual HAL source and audit
selected entry-point dependency closure. The HAL supports many additional
modes; attributing their full implementation to the selected linked image would
be misleading. Configuration RMW operations are not protected against two
concurrent initializers. No hidden allocator, scheduler, interrupt handler or
polling loop may be inferred from a common library name: inspect actual selected
source and link output. Two-client transfer requires exclusive initialization,
stable disjoint pin ownership and serialization where shared registers require it.

## Execution and cooperating machines — T20.06 and T20.07

Use a discrete one-CPU model with arrivals0,0,1 and two CPU ticks per job. State
whether newly arrived work joins before or after the current tick's rotation.
Compare polling with the same FIFO policy, FIFO event service and a quantum-one
rotation; two workers on one CPU do not double execution capacity. The changed
model adds one compute tick, three asynchronous wait ticks, then one final
compute tick. Analyze overlap without calling model ticks measured latency.

For queue traces, enqueue then service2 with capacity4. Retain IDs, accepted,
rejected, serviced and pending counts for `[3,1,4]` and `[5,0,5]`; then move
service before arrivals and recompute. Derive backlog/response under burst4 and
at most2 arrivals per subsequent interval; explicitly state interval phase,
FIFO ordering, bounded callback cost and no hidden blocking. An unbounded
callback invalidates a finite response bound while bounded queue storage can
remain true. Required resource accounting includes inline capacity, retained
payloads, callback contexts and maximum deferred events.

`ArMachine` represents empty, acquired and submitted states. Acquisition assigns
a nonzero never-reused request within an incarnation; the request counter and
incarnation retire at UINT32_MAX. Submitted ownership remains until exactly one
valid terminal event. Cancel while acquired releases synchronously; cancel while
submitted sets intent. Two ordinary event slots plus one reserved cancellation
acknowledgment slot prevent full-work-queue starvation of the cleanup control
event. Step gives that acknowledgment priority; late duplicates are stale and
cannot release twice. Restart requires no active owner and changes incarnation;
queued old events remain identifiable as stale. `releases` is a bounded test
counter: begin a new owner before it would wrap. A provider that never supplies
an acknowledgment can still prevent progress; reservation alone is not fairness.

Compare product states and a hierarchy that factors common terminal cleanup.
Show disjoint completions commute, but competing admission to one free slot
does not. The multi-producer experiment wraps all queue access with one POSIX
mutex and joins both workers before destruction. This supplies a legal C
synchronization protocol for that execution; plain serialized callback proofs
are not a data-race proof for an unsynchronized port.

## Configuration, containment and decisions — T20.08 through T20.10

Build capacity is2..4. Runtime fields are version1, limit0..build capacity,
policy0..2, and timeout1..1000 milliseconds. Priority policy requires limit>=2.
Fields1..4 are required exactly once; unknown required keys below128 reject;
unknown optional keys>=128 are ignored. At most16 fields are admitted. Validation
stages a complete configuration and never partially activates it. Its canonical
8 bytes are version, limit, policy, reserved zero, then big-endian timeout.
Raw encoding identity remains distinct from normalized semantic identity.
AR_FEATURE changes public struct layout and is deliberately ABI-incompatible;
consumers must agree on the feature/version or exchange only the canonical wire
representation. Compile both capacities and both feature layouts.

Embed source digest, actual compiler-version digest and semantic configuration
in the generated build header. Hash the final artifact externally to avoid
self-reference. Retain compiler commands, generated inputs and result binding.
Relocate the build and compare both semantic identity and actual binary digest.
A stale binary does not count for a new configuration merely because its old
tests passed. Source path provenance and semantic configuration are different
identities; byte reproducibility depends on actual flags and embedded paths.

Supervisor event0 is success,1 timeout,2 terminal fault. Two consecutive timeouts
degrade; success clears the streak; fault stops. A failed bounded logger
increments a saturating loss counter and never changes the primary decision or
recurses. Recovery attempts are limited to two over this owner's entire lifetime.
Initialize dependency A then B; failure of B releases A and preserves stopped
mode. Success owns both resources and returns normal; the adapter owner must
later release them. No attempt reinitializes already-live successful resources.

Malformed count/status validation contains only the stated callback-value model.
It does not contain arbitrary same-address-space writes, dangling contexts or
undefined behavior. Design the original helper-process alternative with bounded
copied IPC, incarnation-tagged replies, monotonic timeout, termination/reaping
and no shared memory. If shared memory is introduced, prove all users quiesce
before unmapping. Process creation alone is not a privilege sandbox.

Submit two concrete decisions: synchronous borrow versus queued copy under
immediate caller reuse, and bounded service versus overload/unavailable adapter.
List correctness requirements before costs. Restore the old arrival workload
but tighten recovery: determine which old conclusion is invalidated. Every
requirement maps to a named assertion, artifact and its original rubric; a
passing always-true test is no evidence. Shared tests need separate assertions
for retention, capacity, cancellation and recovery.

## Deliverables, verification and assessment

Submit public implementations replacing `learner.c`, `ring_learner.c` and
`board_learner.c` and `effects_learner.c`; compile each public test against your implementation and
earlier learner producers. Supply dependency and runtime ownership graphs,
change-impact tables, fixed-capacity and stack calculations, exact failure
snapshots, numerical units, semantic-port tables, ordinary and changed traces,
replayable property failures, cost/disassembly records, configuration manifests
and decision records. Preserve the original criterion IDs and learner delayed
interval of7 days. Author execution is never a learner's delayed assessment.

The source runner `tools/run_architecture_followthrough.py` uses GCC/Clang debug,
optimized/NDEBUG and AddressSanitizer/UndefinedBehaviorSanitizer profiles. Tests
include121 small core sequences, seeded full-range samples with an independently
computed int64/Python oracle,12,288 policy histories, source-bound dependency
and ARM closure checks, intentionally flawed variants and compiled unsolved
learner rejection. A negative result must hit `AR_ASSERT`, not merely fail to
compile or terminate unexpectedly. The property runner retains a minimized
integer counterexample and replay. Pass requires each original rubric's own
ordinary and changed-condition evidence, not only a family-wide green label.

Level0 has missing/incorrect artifacts; level1 passes examples without explaining
ownership or model limits; level2 supplies the invariant/failure/verification
obligations and diagnoses the relevant mutant; level3 independently reconstructs
the changed contract, transfers earlier artifacts and supplies a new counterexample
to an overstated guarantee. Use each original objective's more specific rubric
in addition. Hints remain graduated in the original supplements: identify the
boundary first, write the invariant next, then inspect the first violating
transition. Do not open separate assessor implementation/analysis during an
independent attempt. The reference implementation, exact model analyses and
criterion bindings stay under assessor/research authoring paths.

## Security and validation status

All inputs are synthetic. Executable targets are original curriculum code,
private temporary files, local pipes and a dedicated child process. Harmless
success observations are incorrect count rejection, preserved sentinels,
bounded file bytes or duplicate-release prevention. No crash, sanitizer
observation or model overwrite is called code execution or privilege escalation.
Repairs are checked by the same counterexample plus boundary regressions.
Remaining limits include callback trust, exclusive descriptor/configuration
ownership, no physical timing, no power-loss test and no arbitrary-memory
isolation. No operational system is targeted.

Content is authored. Executed configurations, failures and current source/output
hashes are recorded separately in
`research/v5/expansion/legacy_architecture/validation.json`. Physical observation,
external review and independent learner mastery are not supplied by that report.
Finite tests, model enumeration and link closure do not prove absolute mastery,
all hardware behavior or correctness under every changed assumption.
