# Required protocol transfers and actual component composition

These are executable continuations of X-T71-001–008, their original32 objective
criteria and T71-G01–G08. They do not replace the anchor or the seven-day delayed
assessment. The parent [protocol contract](../CONTRACTS.md) supplies C17 lifetime,
callback, containment and rejected-output rules. The
[modern cost contract](../../../curriculum/v5/exercises/modern-cost-contract.md)
supplies all13 required cost dimensions. No additional language/library may
implement the studied mechanism. New bodies remain learner stubs; reference
implementations and technical analyses stay under `assessor/protocols/followthrough`.

## Common environment, prerequisites and assessment

Use the original prerequisite artifacts identified in each X brief. Build with
GCC and Clang at O0/O3 and with AddressSanitizer/UndefinedBehaviorSanitizer. Only
the actual pthread competing-consumer adapter uses pthreads; it also runs both
ThreadSanitizer profiles. The single-core Cortex-M4 saved-mask port is separately
compiled with declared byte-loop helpers, not executed on a board. The fresh
objects and their addresses remain live and stable until every alias, queued
notification and hardware loan is discharged. Initialization is not permission
to reuse storage with surviving aliases. Copyable C wrappers do not confer move
semantics. Safe Rust comparison uses an actual E0382 diagnostic; that does not
cover unsafe Rust/foreign interfaces or prove the C implementation safe.

Each continuation requires D1 a mechanism derivation and a minimal wrong
explanation/counterexample; D2 implementation and ownership/state proof; D3
independent expected traces and failing controls; D4 changed-assumption transfer,
integration evidence and the13-dimension cost review. Every original `.a` criterion
assesses D1, `.b` D2, `.c` D3 and `.d` D4; no shared test automatically passes all
four. Level0 has a false mechanism/ownership claim; level1 only completes the
ordinary path; level2 meets that objective's concrete obligations below; level3
independently derives and defends the original delayed variant at least seven
days later without copying the author solution. A failing ownership, state or
containment obligation is not averaged away. Author execution is never a learner
result. The separate analysis contains assessor reasoning and delayed probes.

## T71-G01 / X-T71-001: an owning recovery capability

Implement the PFDriver/PFRecovery operations in `protocol_follow.h`. The adapter
actually calls the repository's PTDriver, FwDma and FwWatchdog implementations.
PFDriver owns at most one allocated byte buffer; successful admission clears the
caller's sole owner pointer and publishes a recovery capability containing the
stable owner address, loan epoch and DMA ticket. Length is1–64. A deliberately
failed start **after** DMA submission retains that allocation and enters recovery.
The release callback takes the original allocation exactly once; it is bounded,
non-reentrant and may neither fail nor access it afterward.

A reset request marks recovery and sends cancellation without clearing the old
loan. Observing the new reset register as idle is insufficient: recovery requires
a trusted quiescence observation for the **old** loan epoch as well as the actual
FwDma idle result. A stale epoch/ticket rejects while the owner is live. Only
successful recovery increments the hardware epoch. At UINT32_MAX, quiesce and
release, then retire the driver; never wrap or retain a leaked allocation merely
to avoid wrap. A new READY transition uses `pf_driver_ready`, not reinitialization.
DMA or protocol generation exhaustion rejects before taking a new allocation.
Successful ordinary completion also requires matching epoch/idle and feeds the
actual watchdog once. Failed-start completion still needs recovery.

D1 distinguishes C copyability, runtime freshness and dangling-pointer validity.
D2 proves that no release precedes both quiescence obligations. D3 enumerates
start success/failure, repeated incomplete quiescence, reset, duplicate/stale
capabilities, a second loan and epoch exhaustion. Expected witness: new-epoch
idle leaves frees0; old-epoch idle permits frees1; a duplicate old ticket cannot
free loan2. D4 composes asynchronous completion with the later event adapter.
Hints: draw the allocation interval; preserve the old epoch across reset; only
then decide which operation may consume the recovery capability. Preserve
**X-T71-001-DELAYED** and its reset/stale-alias task; no generation check makes a
dangling owner pointer readable.

## T71-G02 / X-T71-002: sum-mode builder and safe replacement

PFBuilder contains the existing address/length/timeout builder plus one mode.
POLL requires no callback. DMA requires a callback and address/length multiples
of16. Original address0–255/multiple4, length1–64, timeout1–1000 and
address+length<=256 constraints still apply. Mode is one-shot. Finish requires
all fields and mode and publishes PFConfig only after every cross-field check.
`pf_builder_replace` admits only an already present field. Stage the replacement
and, when complete, validate the entire new configuration before publication.
A rejected replacement leaves all old fields and presence bits unchanged.

D1 derives the product/sum state space rather than assuming a presence bit proves
semantic validity. D2 implements transactional replacement. D3 compares all8192
address/length/mode combinations and explicit invalid callback/mode cases; a
complete DMA builder at address16,length16 rejects replacing address by20 with
byte-for-byte preservation, then accepts32. D4 compares extra wrapper types with
the checked sum and preserves **X-T71-002-DELAYED**. Hints: keep construction
presence separate from validity; validate the whole proposed value; commit once.
No allocation or failure cleanup is relevant to this fixed-value mechanism.

## T71-G03 / X-T71-003: rejection, cancellation and uncertain acknowledgment

PFSession is an original serialized client/server model with one outstanding
signed32 addition. Start with next1, phase0 and total0. SEND consumes the next
nonzero, nonexhausted sequence and retains its value; APPLY performs the checked
addition and records `(last sequence,last value,total)` together; ACK retires the
pending request and advances next. Wrong sequence/value and overflow reject
unchanged. RETRY resends the same pending operation. CRASH prevents progress;
RECONNECT retains the modeled durable deduplication tuple and retries an uncertain
request. APPLY of an already recorded equal sequence/value acknowledges without
repeating the effect. CANCEL is terminal and forbids all future progress.

The tuple is assumed atomic/durable **only inside this explicit model**. No file,
network or distributed exactly-once implementation is implied. After a real
server failure, a client cannot infer from a missing ACK whether an effect took
place; removing the durable tuple invalidates deduplication. D1 supplies that
indistinguishability argument and a counterexample. D2 specifies linearization
and failure semantics. D3 executes crash after APPLY/before ACK with0–4 retries:
total stays7, then a new request-3 yields4. Wrong ACK and overflow preserve state.
D4 preserves **X-T71-003-DELAYED**, chooses replay/cancel policy and compares the
bounded C runtime automaton with the actual stated scope of Sesh's binary session
typing/failure contract. A transport using arbitrary other synchronization cannot
inherit Sesh's deadlock-freedom claim. Hints: separate effect from ACK; identify
the durable tuple; then decide how cancellation affects an uncertain effect.

## T71-G04 / X-T71-004: entry posting, guard fallback and local transition

PFHsm extends the original six-state machine with a four-event copied queue.
Its current step computes/admit-checks the entire trace before any entry callback.
Callbacks receive entered states, may post copied events, and cannot synchronously
dispatch again. Posted events remain queued until a later run-to-completion step.
The published machine already identifies the admitted target during callbacks;
this is the selected original subset, not a claim of full QP/UML equivalence.
A callback failure leaves that committed transition/trace and any earlier effects
intact and returns IO (basic guarantee). Queue-full is an explicit callback policy.

PF_GUARDED is handled internally as PING when its leaf guard succeeds. If false
at IDLE/BUSY, it propagates to ONLINE's FAIL transition. PF_LOCAL targets IDLE
from the ONLINE region: local1 exits the old leaf and enters IDLE while preserving
ONLINE; local0 additionally exits/reenters ONLINE. A rejected capacity check changes
nothing. D1 derives source/target/LCA and fallback. D2 preserves immutable current
event and queues entry-posted START. D3 checks exact traces and attempted reentry;
local BUSY→IDLE produces `[exit BUSY,enter IDLE]`, whereas the external region
transition adds exit/enter ONLINE. D4 preserves **X-T71-004-DELAYED** and revisits
action ordering under a different framework contract. Hints: determine the main
source before finding the LCA; evaluate guards before effects; defer dispatch.

## T71-G05 / X-T71-005: interrupt admission and competing consumers

PFActive puts both queue operations **and the entire machine dispatch/removal**
under one supplied lock. Lock callbacks do not block recursively and own the
serialization authority. The hosted backend uses one pthread mutex, two producers
making4000 attempts each and two competing consumers. Full admission rejects the
new event; an unhandled head remains until trusted `pf_active_discard` removes it
and increments a checked discard counter. Dispatch failure cannot silently pop.
Entry callbacks are not used by this competing-consumer adapter; introducing them
requires the explicit nonrecursive posting policy from G04.

`irq_port.c` supplies saved-PRIMASK enter/restore plus a memory barrier for a
single-core Cortex-M4 context. One persistent mask slot is safe only with
non-reentrant calls and no NMI/fault entrant. It is not an SMP lock or a pthread
mutex usable in a real interrupt handler. D1 exposes why two consumers break the
old single-consumer premise. D2 serializes the full transition, not just head
increments. D3 checks handled=accepted, accepted+dropped=8000, pings=handled,
blocked-head preservation and counted administrative discard under both race
detectors. D4 preserves **X-T71-005-DELAYED** and supplies target priority/latency
evidence separately; compile observations do not measure interrupt latency.
Hints: lock the machine invariant; choose a full-queue policy; distinguish the
pending head from a malformed event that has actually been discarded.

## T71-G06 / X-T71-006: owning two-child framed decoder

PFDecoder parses one original frame `[length,payload...,checksum]`, length0–8,
checksum the modulo256 sum of payload bytes. It owns one allocation (at least
one byte even for an empty payload), persistent header-child/body-child state,
checksum, progress and an absolute uint64 deadline. It consumes at most one byte
per step. Missing input consumes nothing. Allocation failure leaves the header
unconsumed and permits a retry. Bad checksum releases and terminates; cancellation
at any suspension releases exactly once. On success the sole buffer owner moves
to the output pointer and the frame no longer releases it. Repeated DONE does not
republish ownership. No automatic local is assumed to survive suspension.

Times must be monotonic. `now>=deadline` wins before input acceptance, including
when the final checksum is available at the same call. D1 derives the child-frame
layout and timeout arbitration. D2 preserves owner/output canaries and handles
allocation failure. D3 tests every cancellation prefix for length0–8, unavailable
input at every boundary, incorrect checksum, repeated cancel and a length1 frame
whose third byte arrives exactly at the deadline. D4 preserves
**X-T71-006-DELAYED** and compares explicit child frames with actual T72 stackful
fibers. Hints: give each potentially waiting child persistent state; decide who
owns partial output; write the timeout ordering before writing the state switch.

## T71-G07 / X-T71-007: reactive guard and shared immutable subtrees

PFNode describes a binary sequence/selector/leaf DAG of1–8 nodes, with forward
child indices. Expand execution occurrences into at most16 PFOccurrence frames
**before** callbacks; a graph with more paths rejects without publication. Shared
immutable nodes may appear in different occurrences, each with its own cursor
and active bit. The callback receives both occurrence and node identity. A memory
sequence retains completed-child progress while an action is RUNNING. The reactive
guard is reevaluated before each tick; false aborts every active occurrence once,
resets progress and returns FAILURE. Completed external effects are not undone.
Cancel twice cannot repeat aborts. A malformed callback result preserves its active
occurrence for explicit cleanup, returns INVALID and does not publish a result;
earlier callback effects still occurred. Topology is immutable while frames live.

D1 explains why a mutable cursor stored on the shared node is wrong. D2 derives
occurrence expansion and bounded recursion/stack. D3 uses two edges to the same
leaf: occurrence1 succeeds, occurrence2 remains RUNNING, repeated ticks skip1,
guard failure aborts2 only. It also rejects cycles and an exponentially expanded
DAG exceeding16 frames. D4 preserves **X-T71-007-DELAYED** and compares expansion
cost with a dynamic activation stack under an explicit admission bound. Hints:
distinguish node identity from activation identity; retain basic failure effects;
abort before starting a new reactive path.

## T71-G08 / X-T71-008 and T71-INTEGRATION: queued epoch-qualified driver

`system_follow.h` composes the actual producers listed in ADAPTERS.md. PFSystem
has three execution-model tasks, a four-message copied queue, four branded
message slots, the owning driver and a16-result history ring. Init performs
GPIO status, timer configuration and one UART header write before readying DMA;
partial init is a basic hardware-effect guarantee, not transactional rollback.
The object cannot move because tasks, brands and driver tokens reference it.

Two serialized producer IDs post COMPLETE/RESET/QUIET messages carrying a copied
recovery capability and observed epoch/idle. The copied queue transports only an
internal slot index; a branded slot remains allocated and immutable until the
consumer processes it. External hardware events must be epoch-qualified before
posting. Queue/pool exhaustion rejects without publishing a brand. Consumer task2
looks up the brand, applies the actual driver operation, releases only the message
slot and records the result. A PENDING result consumes that message but retains
the **driver buffer**; a later actual quiescence event must be posted. An exhausted
history ring counts lost diagnostic entries without undoing completed effects.

Arm one deadline for a matching live DMA loan using a monotonic uint64 tick and
timeout1–1000. Reject clock reversal and addition overflow without arming. The
configured timer's elapsed-tick adapter supplies `pf_system_tick`; its timestamp
is an explicit trusted model input, not a physical clock reading. Before expiry,
no reset is posted. At or after expiry, enqueue an epoch-qualified RESET through
the actual copied queue. A full queue leaves the deadline armed for a retry;
enqueue success disarms it. Processing timeout requests cancellation but never
certifies quiescence or releases the old allocation. A delayed timeout for an
already completed/recovered loan becomes a harmless stale-capability rejection.
Measure the actual target tick source separately before a hardware timing claim.

D1 separates the message slot, copied queue element and asynchronous allocation.
D2 makes all producer imports real source-level calls. D3 checks the expected
history PENDING,PENDING,STATE,PENDING,OK across reset/new-epoch/old-epoch observations,
fills all four slots, and proves queued stale tokens cannot release a new loan.
D4 retains **X-T71-008-DELAYED**: `cache_follow.h` supplies the additional selected
TX contract. The caller owns an aligned32/64-byte allocation, including all line
padding, and a verified32-bit bus mapping for those same lines. Payload is1 through
the owned extent. No unrelated dirty byte, writable alias or competing DMA may
share a line. `pf_cached_start` rejects partial/misaligned extents, composes the
actual B06 whole-line clean/completion policy, and only then transfers ownership
to the actual DMA/typestate producer. Checking alignment alone cannot prove the
mapping or exclusive allocation. A failed asynchronous start still owns the
cleaned allocation; release still needs old-epoch quiescence. This is TX source
ownership, not the different RX invalidation contract. Run
`python3 tools/run_protocols_cache_transfer.py` for source-bound positive,
two-mutant and stub checks. Selected H745 MMIO code is separately cross-compiled
in B06; physical observations remain required. Hints: identify three distinct owners; release the message even when
the DMA remains pending; preserve the old epoch through reset.

## Validation, source boundaries and containment

The required R2 alternatives also live in alternatives.h/.c. PAEmpty,
PAAddress, PALength and PATimeout implement a fixed address→length→timeout
construction route using the actual earlier setters; finish accepts PATimeout.
It admits exactly the same completed configurations as the checked builder,
although it intentionally orders construction. A raw struct copy/fabrication
can bypass this route; argument-type diagnostics are not move or ownership
enforcement. Invalid setters preserve the destination. Compare field/boundary
products with the original builder and compile wrong_builder_stage.c expecting
an incompatible argument diagnostic. The alternative's PT_BEGIN/WAIT/END sum
keeps count/sum in PTThread, consumes at most one value per call and handles
stable DONE in a wrapper before entering the completed macro. Overflow restores
the entry pc as well as sum/count. The dual-endpoint driver runs actual client
and server transitions for remaining0..8 and rejects another BYE at both ends.
tools/run_t71_t72_alternatives.py records these controls and keeps intentional
PT_WAIT case-label fall-through separate from errors that must remain diagnosed.
Hints: distinguish completed configuration sets from construction order;
automatic locals are recreated on entry; advance both endpoints exactly once.

tools/run_protocols_baseline_depth.py separately executes retained anchors,
their original two mutants, a Python transition oracle against actual C, and
additional setter permutations, retirement, nested-tree and queue-wrap cases.
It does not alter any earlier accepted receipt.

active_import.h also connects the actual execution.tasks and sync.copied_queue
producers to PTMachine. PATaskActive owns four modeled task records, the copied
queue and at most one pending head event. Receiving from the physical queue
transfers ownership into that pending slot; a failed dispatch retains it ahead
of later events. The aggregate queue count plus pending slot never exceeds4.
Both competing consumers serialize the entire receive/dispatch/commit under
the supplied lock. An explicit administrative discard consumes the pending
event and increments a diagnostic count; no automatic dropping occurs.
Producer IDs0/1 and consumer IDs2/3 must exist in the retained scheduler model.
All operations use immediate deadlines and do not block while holding the lock.

The supplied pthread adapter exercises2000 PING events with two producers and
two consumers, failed-head/full-queue admission, insufficient trace capacity,
and successful reattempt by the other consumer. Its queue fields are inspected
only under the same lock to enforce aggregate capacity; this is an explicit
versioned representation dependency, not a portable opaque-queue peek API.
The saved-PRIMASK port previously audited is the separate UP interrupt lock;
no hosted pthread schedule proves interrupt timing or real scheduler dispatch.
Run tools/run_protocols_active_imports.py, including two semantic controls and
both compiler TSan profiles. The actual task sources remain a model here.

Run `python3 tools/run_protocols_followthrough.py`. Each mechanism has two
separately compiled semantic mutants and a compiling learner stub. Require the
named PF_CHECK/PE_CHECK/PS_CHECK rejection, not just compilation failure. The
actual producer campaign proves reference-artifact composition only; a learner
must import their own accepted revisions and reproduce it. The original delayed
IDs and every objective rubric remain required. Current evidence and exact
source pins are in `research/v5/modern/protocols_followthrough_validation.json`;
its existence alone is not acceptance—check passed flags and current hashes.

Inspected follow-through records are in
`research/v5/modern/protocols_followthrough_resources.json`: bounded Rust ownership,
Sesh0.1.1, QP/C8.1.5 RTC/entry/exit/guard/local-transition, and Dunkels stackless
continuation passages. Original model choices above are explicit and do not
inherit stronger language/framework guarantees. Exact published C17 remains a
source gap; N1570 remains the identified C11 draft baseline comparison.

Adversarial targets are these original in-process state machines and synthetic
registers. Attacker-controlled inputs are tags, sequence replay, malformed frames
and event floods; attacker control excludes forging live allocator pointers,
trusted old-epoch quiescence or changing the backend. Harmless witnesses are
duplicate totals, prematurely incremented free counters, lost events or wrong
parse acceptance. Repair and regression controls target those properties. No
crash or sanitizer result is called code execution; no firmware is flashed and
no external service or host security setting is changed. Finite host schedules,
generated code and model cuts establish neither physical behavior nor absolute
mastery, external review or independent learner performance.
