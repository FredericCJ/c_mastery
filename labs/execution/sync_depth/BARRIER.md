# Reusable barrier, registered phases and tree publication

## Identity and objectives

Required successor for PR-T51-10 / T51.10.a–c, including each original
`AX-PR-T51-10-T51.10.{a,b,c}-DELAYED` task. Exercise families: derivation,
construction, diagnosis, boundaries, verification and changed-condition transfer.
Preserve the original fixture arrivals0,1,2 then2,0,1 and historical evidence.
This contract supplies executable depth for generation saturation, coordinated
membership withdrawal and a four-thread tree barrier; it does not replace the
original common wait protocol.

## Prerequisites and sources

Import completed PR-T51-01 CMX common waits/task identities and PR-T51-09 terminal
result discipline. Read actual `cmx_barrier_wait`, timeout/cancel abandonment,
ready/wake operations and generation representation before constructing the
adapter. Source register `research/v5/expansion/legacy_concurrency_barrier/resources.json`
records those bounded course passages, POSIX Issue8 barrier wait/init/destroy and
memory synchronization passages. The registered-phase and tree compositions
are original course analyses; do not attribute them to an uninspected paper.

## Environment and permitted dependencies

Core uses ISO C17, actual CMX, fixed caller storage and no allocation/library
synchronization. Target execution uses a Cortex-M4 soft-float ABI and disposable
QEMU `mps2-an386`; audit emitted code, undefined symbols and explicit original
memory helpers. The hosted native adapter separately uses pthread mutex/condition
operations and real threads. It does not call pthread_barrier_wait to implement
the mechanism. POSIX provides synchronization guarantees beyond ISO C; C11/C17
data-race rules still apply. C23 does not make an ordinary counter a barrier.
No physical measurements or native CMX task context-switch are implied.

## Problem and supplied inputs

Complete public `barrier.c` and `barrier_native.c`, using the supplied headers,
test drivers, M4 harness and explicit-import runners. Learner sources remain
unsolved; reference implementations/analysis are withheld. For three parties,
first two arrivals block and third releases round0; the next order2,0,1 releases
round1. A fast participant may enter round1 before peers collect round0, but an
old ticket cannot collect its new result. All36 pairs of arrival permutations
must satisfy the independently stated expected round counter and crossing rule.
Duplicate arrival, zero parties, early release and output sentinels are explicit
cases, not inferred from a stress checksum.

## Representation and ownership

`CDBarrier` exclusively owns a real `cmx_barrier` and its participating CMX wait
operations. `members` is a nonempty mask of at most16 task slots; `registered`
tracks live registrations for the current uint32 configuration epoch. Each
registration snapshots its actual task generation. All members must register
before any arrival. Each member has at most one pending or completed-uncollected
ticket `(member,round)`. Task/kernel/control storage stays live and stable; no
unrelated CMX operation may overwrite a task result before collection. Close,
result drain and registration release precede task exit or object reuse.

Initialization accepts fresh/quiescent storage only. Output objects are writable
and disjoint from live inputs/control. C copies do not create a second registration
or an owned lifetime; release the single registered handle, and invalidate copies
by the next configuration epoch. All external handle users must quiesce before
destroy/reinitialization; checking a freed owner pointer is not memory safety.

## Core operations and invariants

`cdb_arrive` admits only the current member, once per round, with all registrations
present and no configuration change staged. It calls actual CMX wait. BLOCKED/OK
produce a ticket; other results preserve the output and admit no arrival. A
deadline already due may return TIMEOUT before admission and does not itself
break the cohort. Once admitted, selected timeout breaks that generation: the
initiator gets TIMEOUT, peers BROKEN. Cancel similarly gives initiator CLOSED,
peers BROKEN. Future arrivals fail BROKEN until explicit quiescent reset. A
completed success wins over later cancel. `result` is one-shot; errors preserve
the completed-round output; an old round/configuration ticket is STALE.

Prove: a successful round cannot cross before all current parties arrived;
registered identities and pending slots cannot duplicate one participant;
every admitted ticket gets one terminal result; no next-round ticket consumes a
previous-round wake. The raw arrived count can reset before all result holders
collect, so it alone does not prove quiescence. CMX scanning of up to16 tasks is
separate from O(1) arrival-counter arithmetic. Adapter settle/validation scans
bounded slots; report that cost honestly.

## Failure, reset and finite counters

`stage(next_mask)` is allowed only with no pending or uncollected results and no
arrivals. It freezes new arrivals. The coordinator then obtains releases of all
old registrations, including withdrawn participants. `apply` refuses while even
one old registration remains, validates new tasks, increments configuration epoch,
sets the new party mask and clears broken state. Re-register the new cohort.
This separately coordinated phase changes3→2 by withdrawal and2→3 by admission;
it does not decrement parties in the middle of a wait. It is a changed invariant,
not the old fixed-set proof with a different constant.

Choose a generation limit1..255. Successful or broken generations monotonically
consume that budget. At the limit refuse new arrival/reset with CDB_RETIRED;
do not wrap or reset to0 while an old ticket could compare equal. A three-round
test implements the reduced-width delayed task. Configuration epoch likewise
saturates at UINT32_MAX. Close/release/destroy remain legal at exhaustion.
Destroy requires staged close, no registrations, arrivals or results. No automatic
timeout for missing parties is promised; use an admitted finite deadline or
explicit cancellation, then drain the whole cohort.

## Native publication and tree construction

Implement one pthread mutex around the whole actual CMX/adapter transition and
a condition variable for sleeping native callers. Wait in a predicate loop;
the last arrival broadcasts while holding the mutex. The mutex's release/acquire
chain carries every pre-barrier ordinary write to every post-barrier reader.
Callers have fixed distinct native participant IDs; no concurrent use of one ID,
asynchronous cancellation, callback reentry or concurrent destruction. Join all
threads before destroy. Unexpected pthread failures are port-fatal with possibly
retained state; stop/inspect, do not claim transactional retry or safe disposal.

The four-thread tree has two two-party leaf barriers and a two-party root for
one leader per leaf. Each tree wait performs leaf gather, leader root gather,
then leaf release; the nonleader cannot return after the first leaf phase.
Derive transitive publication from both leaf groups through root and back through
release. Keep these phases separate for reuse: arrivals from a later logical
round must not discharge an earlier release. Storage is three native barrier
nodes in this concrete topology. Leaders traverse three waits, nonleaders two;
derive the balanced-tree generalization's O(P) storage/O(log P) levels as an
analysis, not a claim that arbitrary P was executed or that this implementation
has measured lower latency.

For both central and tree run32 rounds of ordinary shared writes and reads,
delaying a different participant each round. Each thread writes its own slot,
waits, reads all four slots, then waits again before any next-round overwrite.
Expect512 checked plain reads. Central:256 arrivals/64 generations. Tree:256
arrivals at each leaf/128 leaf generations,128 root arrivals/64 root generations.
These are real counted operations and bounded publication observations, not a
weak-memory model proof. Timing delay is a schedule perturbation, not a deadline
guarantee. The second barrier is essential to avoid a valid C data race during
payload reuse even when the first barrier works.

## Required deliverables and verification

Deliver completed core/native code, transition diagram, per-pointer last-use
table, membership-change protocol, generation saturation proof, publication graph,
tree cost derivation, exact command logs and raw expected/observed traces. Reuse
actual CMX imports; synthetic substitutes do not satisfy this composition.
Run four core cases under GCC/Clang debug/optimized/ASan+UBSan and actual M4
GCC O0/O3/Clang O3, then central/tree native tests additionally under both TSan
compilers. Explicitly reject old-ticket acceptance, early release, ignored cancel,
reset-before-drain and wrapping-generation mutants. Each unsolved consumer or
producer must compile then fail its intended assertion; timeout/crash/sanitizer
diagnostic is not an intended semantic pass. No execution status is inferred
from this paragraph: current source-bound campaigns record what actually ran.

## Per-objective rubric and graduated hints

T51.10.a: round identity, duplicate/zero/one-party boundaries, late collection and
reduced-width saturation. T51.10.b: initiator versus peer error, future BROKEN,
drain/registration quiescence and3→2→3 membership. T51.10.c: ordinary-write
publication, reuse fence, tree gather/release, finite trace and cost limits.
Score0: missing/unsafe;1: ordinary fixture only;2: explicit invariants plus all
relevant boundaries/evidence and limits;3: independently derive the changed
contract and diagnose a new interleaving after the original minimum7-day delay.
Author executions do not award a learner that level.

Hints: (1) arrived0 does not mean no old users; (2) a ticket contains both
configuration and round; (3) broken state belongs to the cohort, not one waiter;
(4) registration changes require an admission freeze; (5) draw both directions
of a tree's publication edges; (6) protect next-round payload reuse separately.
Separate assessor analysis is under `assessor/legacy_concurrency_sync/`.

## Comparison, security and validation limits

POSIX pthread barriers choose one unspecified serial thread and reset after the
required arrival count. This adapter instead exposes explicit CMX results,
fixed registrations, broken generations, finite retirement and coordinated
membership changes. Do not claim API equivalence. POSIX destruction/reinitializing
with blocked waiters is undefined; this teaching API refuses known outstanding
references but still depends on external lifetime discipline and exclusion.

Targets are original local programs and disposable QEMU with synthetic task IDs,
no services, deployed systems or board flashing. Early release and stale-round
acceptance demonstrate synchronization-property violations only. Repair uses
generation/registration/state and publication edges; noncooperative missing
participants, port errors and invalid external lifetimes remain explicit limits.
Independent review, actual delayed learner attempts and physical observations
are separate from authored, executed and source-validated reference support.
