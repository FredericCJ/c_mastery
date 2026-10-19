# T75 successor cost and enforcement dossier

Use every row of modern-cost-contract.md for each chosen mechanism. This is the
author's bounded worked dossier and experiment protocol, not a learner decision
or independent assessment. `resilience_service_decision.json` discloses that
functional exploration preceded the new timing predictions. The timer decision
and actual list/heap/wheel measurements are separate, already source-bound
experiments. Preserve all results that disagree with a prediction.

`tools/run_resilience_service_cost.py` builds the actual composition with GCC and
Clang at O0/O3. The diagnostic binary retains timer operation counts; the timing
binary disables only those counters with RZT_NO_WORK, keeping all required
checks, copies, ownership, synchronization and durability callbacks. Both run
the same independent9096-history oracle. Ten declared workloads cover nominal,
burst loss, long independent tail with/without hedge, correlated loss with/without
hedge, slow consumer/restart, cancellation, exhausted credit and Retry-After.
Each measurement includes fixture initialization, allocator tracking and
malloc/free. One warmup precedes seven samples of100 independent requests. The
clock brackets the whole batch; no per-operation clock subtraction is applied.
Samples describe host total work, while reply/quiet/notification ticks describe
the virtual workload. They are different quantities. Scheduling, host load,
caches, malloc and timing noise prevent a universal performance ranking.

Both diagnostic and timing configurations also build and execute the actual
owned-pressure native harness and independent121554-history oracle. Capture its
separate map, object disassembly, unresolved symbols and static frame reports;
its five-policy operation comparison is not a timing benchmark. The layout
driver includes the owned queue, two-producer pump and gate, so their storage is
measured rather than inferred from names. Whole-program sizes include harnesses
and imports; they are not isolated abstraction code sizes.

| Dimension | Representation, bound, evidence and enforcer |
| --- | --- |
| Representation | Scope uses checked finite reference/borrow counters and immutable parent identity; bucket carries exact quotient/remainder credit; timers retain16 indexed nodes; retry retains one actual NetStack flight; hedge retains two tickets/owned replies; journal128 records and outbox four events. `layout` prints actual host sizes/alignment. C types describe fields; runtime checks establish valid states. |
| Reachable storage | The controller owns its automatic state, one scope allocation and up to two18-byte response allocations. State retains two NetStacks, one journal/outbox/hub, wheel, bucket and arbiter. Device storage and callbacks are borrowed, distinct and external. `peak_heap_bytes` counts successful requested bytes, excluding allocator metadata; generated frame evidence records automatic storage. |
| Lifetime | Scope creation precedes work; retry/hedge borrow until ACK/reset and response disposal; a winner can be visible before losing owners become quiet. Outbox event ends only after durable ACK. C does not enforce callback context lifetime or no-copy controls; caller rules plus runtime state checks are required. |
| Mutation/aliasing | Controller is serialized; native service/arbiter/pump tests use the declared mutex boundaries. All aliases to live controls obey their contracts. A raw copied pointer can still outlive a scope or bypass admission: neither opaque naming nor an atomic count repairs that misuse. |
| Allocation | Scope creation and at most two reply allocations per controller run; failure1 refuses without output, later failure may follow a durable effect. Count successful allocations, total requested bytes and peak requested live bytes. Native POSIX internals may allocate outside the injected allocator and are not credited as zero heap. |
| Movement/copying | Requests/replies serialize18 octets, transport retains the request, channel copies an event sequence, journal writes64-byte records and ACK device32-byte records in partial chunks. Response owner transfer moves a descriptor; it does not duplicate the18-byte allocation. Fixture byte-write counts exclude reads, CPU cache traffic and internal kernel copies. |
| Destruction | Controller checks at most two attempt owners; no recursive owner chain is used here. General scope destruction walks bounded parent chains. User disposers, fdatasync and POSIX waits can block; no hard time bound follows from the finite owner count. Native blocked-disposer tests verify retained permits rather than assuming immediate cleanup. |
| Dispatch | Original C direct calls compose mechanisms; allocator/device/decoder/disposer invoke function pointers. Inspect actual object disassembly, relocations and unresolved symbols for each compiler/optimization. Inlining may remove direct boundaries, not callback obligations. No claim of language-enforced effects or virtual-method equivalence. |
| Concurrency | Native service protects admission/effects and retains scope/permit through I/O; native arbiter locks publication and defers destruction until callbacks finish. The composition's virtual event order is deliberately serialized. TSan/native schedules and the serial oracle establish different bounded facts, neither all C executions nor weak-memory proofs. |
| Failure | Allocation refusal preserves output but may follow a committed effect; uncertain writes poison controls and require recovery. Duplicate equal intent replays; altered intent conflicts; cancellation cannot erase committed notification. Mutants exercise early wake/lost recovery, adjacent anchors test partial I/O, ACK cuts, late responses and capacity. |
| Compilation | Record full clean build command/time and an actual one-object rebuild plus relink after removing only that generated object in the temporary build. Also record an unchanged build. These are host observations, not compiler complexity bounds. Strict C17 diagnostics and sanitizers have their stated coverage limits. |
| Binary/stack | Retain size, link map, per-function compiler stack-usage files, disassembly and unresolved symbols. A static frame entry is not a whole-call-stack bound: callbacks, library frames, dynamic linker, threads and OS stacks remain additional. This is hosted; no freestanding runtime/helper claim is inferred. |
| Boundary | Explicit18-byte wire format,64/32-byte persistent formats, issuer epoch and finite retained request IDs prevent ABI structs from becoming wire records. Callback ABI and uint64/int/octet availability remain target assumptions. Signed32 wire decode is arithmetic. Restart does not preserve heap pointers; replay reconstructs event identities. |

Count comparisons before discussing time. In long-tail pairs, equal durability
and ownership means the hedged result can arrive sooner while cleanup still
waits for the original. In correlated-loss pairs, count every original/retry/
hedge against the same credit. Restart intentionally changes the delivery
contract to at-least-once; comparing it to volatile exactly-once enqueue as if
they offered equal recovery would be invalid. Full native service throughput,
physical power failure, network latency, other ABIs and independent learner
selection remain separate evidence, not hidden zeros in this table.
