# Messaging cost and progress investigation

This shared required exercise supplies the experimental work for the `.d`
objectives of T73.01 through T73.10, `T73-cost` and `T73-fairness`. Its inputs are
the learner's completed corresponding cores and successor artifacts, the
public `cost_driver.c`, `cost_reference.c`, `cost_check.py`, and the registered
select contract. It does not permit importing assessor implementations into a
learner workspace. Each original exercise retains its own rubric and delayed
variant. Submit one evidence table with **separate rows per objective**; sharing
a measurement driver does not automatically satisfy all those criteria.

The host profile is Linux C17/POSIX with an explicitly marked GNU-only stack
attribute probe. GCC and Clang are compiled at `-O0` and `-O3`, without link-time
optimization. The comparison alternatives are supplied verification tools,
not dependencies allowed to replace the mechanism being learned. Compile the
learner artifact and alternatives in separate translation units under the same
flags. Preserve strict warnings. Sanitizers run in separate correctness
campaigns and must not be represented as uninstrumented timing observations.

## Concrete workloads and outputs

The driver takes a mode and an operation count in 8..100000 and emits JSON.
`cost_check.py` checks 26 modes at count128. The measurement runner uses
count1000, seven measured repetitions and a separate process warmup by default.
Retain raw `CLOCK_MONOTONIC` nanosecond samples, exact commands, environment,
clock-observer samples, symbol sizes, link map, disassembly and compiler
per-function stack-usage output. The runner never changes affinity, scheduler,
host security, physical hardware or deployed services.

| Objective | Required concrete experiment and equal contract |
| --- | --- |
| T73.01.d | `thread_stack`: eight actual nursery children report their live stack/guard attributes, wait for cooperative cancellation, then join. Record spawn-to-ready and cancel-to-join intervals and the inline nursery/tree metadata sizes. An attribute size is a virtual stack reservation, not resident pages or a stack high-water mark. |
| T73.02.d | `select_shared` versus `select_registered`: two real producers each submit N distinct unsigned values to capacity4 queues; one selector drains exactly2N, preserving each channel identity. Both use nonblocking sends with explicit FULL retry, blocking select, close/drain, no rendezvous and no payload allocator. Retain retry counts and each participant's latency distribution. The registry adds per-channel locks and O(W) notification work; it does not promise improved parallelism. |
| T73.03.d | `operation_owned`: N actual inline backend/completion cycles transfer unique synthetic resource descriptors through `struct mfo_operation`, invoke receivers and destroy each exactly once. Report control size, dispatch sequence, resource count and latency. Synthetic constant-time destructors bound this workload only. |
| T73.04.d | `actor_owned`: N actual owned mailbox admission/step cycles invoke the handler and destroy each descriptor exactly once. Explain the five stable records versus four queued messages and the active-message lifetime. No allocating payload or cross-thread actor safety is implied. |
| T73.05.d | `supervisor_tree`: a live 4x4 tree repeatedly replaces the final two children of one branch, outside its restart window. Each iteration must contain four ordered events and two constructor calls; initialization/final teardown add16 each. Separate callback cost and metadata scans from the abstract restart policy. Failure cuts remain assessed by `supervisor_tree` correctness tests. |
| T73.06.d | `deque_sc`, `deque_weak`, `deque_mutex`: one owner pushes N distinct unsigned values into capacity8; two thieves consume all exactly once. Record successful work distribution and retries, actual atomic lock-free queries, emitted atomic instructions and fixed storage. The mutex alternative has a different progress class; equal value/ownership behavior on this workload does not make the progress guarantees equal. |
| T73.07.d | `transaction` versus `transaction_mutex`: two threads each commit N paired increments; both cells finish at2N and every admitted transaction observes a coherent pair. Transaction latency includes conflict/retry through commit. Timestamp gaps are permitted and reported. Neither benchmark permits nontransactional payload access. |
| T73.08.d | `snapshot_pins` versus `snapshot_copy`: publish `(i,~i)`, acquire a historical snapshot, publish a changed pair, read the original pair eight times, then release. Both promise the same historical two-field value. Record the pinned version high-water count, control size and caller copy size. Larger payloads, many active snapshots and allocation are changed workloads, not silently extrapolated results. |
| T73.09.d | `sequence_sc`, `sequence_weak`, `sequence_mutex`: one writer publishes `(i,~i)` while one reader makes N bounded one-attempt snapshot calls. Successful pairs must be coherent; refusal preserves output. Count refusals separately and include all attempt latencies. The mutex reference uses one try-lock, not an unbounded retry hidden inside the comparison. |
| T73.10.d | `stream_item` versus `stream_batch16`: one producer submits N values to the actual capacity4 executor stream; callbacks deliver every value once. Requests are issued per item or in batches up to16. Record exact request counts, enqueue latency and completion interval. Retain context until worker join and handle release in both cases. A larger burst or dropped item invalidates the comparison. |

The reference pair and mutex deque are small first-principles alternatives in
separate source, not a systems library implementation. Sample storage and
formatted output belong to the host observer. Output is emitted after timing;
clock reads and assertion checks remain in the timed region and are disclosed.
The no-op observer records paired clock-call overhead separately. Do not
subtract one median from every sample and label the result an exact mechanism
cost. Process warmup does not mean that every subsequent object is cache-warm.

## Thirteen-dimension evidence table

For each mapped objective state the following, using `COST_REVIEW.md` and its
shared 13-dimension contract. A justified limitation is required where a value
cannot be concluded from this host experiment; an empty cell is not zero cost.

1. **Representation:** `sizeof`, alignment, inline records, atomic/control
   fields and ABI. State that pointer/enum/mutex representations are platform
   observations, not portable C layouts.
2. **Reachable storage:** distinguish inline capacity, payload owners, retired
   arrays, pinned versions, live callback contexts and thread stacks. Show the
   lifetime expression; e.g. the registered list adds W stack nodes while W
   selects remain live, and stalled shared owners can extend allocated lifetime.
3. **Lifetime:** identify every transfer, pin, registration, join and external
   admission gate. Cancellation request is not quiescence or destruction.
4. **Mutation and aliasing:** name the protecting mutex/atomic order and the
   caller precondition for unique descriptors and valid borrowed outputs.
5. **Allocation:** distinguish allocation-free inline operations from actual
   `struct mfl_allocator` control/array/clone allocations tested in the lifetime and
   version campaigns. Trace all failure cuts; this timing workload does not
   measure arbitrary allocator behavior.
6. **Movement/copying:** count unsigned copies, the two-field snapshot copy,
   moved owner descriptors and payload copies. A descriptor move is not a deep
   clone and does not itself establish unique ownership in ISO C.
7. **Destruction:** count actual callbacks, final reference release and joins.
   Include rollback cleanup and delayed reclamation; external callback runtime
   has no finite bound unless the callback contract supplies one.
8. **Dispatch:** identify function-pointer calls, switches, conditional atomics
   and direct calls; inspect generated instructions and compiler optimization.
9. **Concurrency:** retain attempt counts, contention distribution, lock order,
   progress arguments and any reader failure budget. Distinguish ISO C atomics
   from POSIX scheduling and the observed ABI.
10. **Failure:** state FULL/BUSY/CONFLICT/cancellation/overflow and unchanged-output
    or owner-retention rules. A faster alternative that skips them fails.
11. **Compilation:** preserve tool versions, source hashes, flags, source unit
    boundaries and generated control flow. Explain `-O0`/`-O3` changes rather
    than treating either as a language guarantee.
12. **Binary/stack:** retain symbol/map sizes, `.su` output and live stack/guard
    attributes. Do not add per-function sizes into a fictitious whole-program
    stack maximum; pthread internals, recursion and callback frames need their
    own call/lifetime model.
13. **Boundary:** identify syscall/library/compiler/ABI boundaries, caller
    discipline and unchecked assumptions. The GNU stack query is a host
    verification extension; it is not an ISO C or freestanding target dependency.

## Progress falsification and assessment

The `fairness` mode replays N actual atomic steal prefixes: thief A reads a
candidate, thief B completes `mfw_steal`, then A loses its compare/exchange. Both
take steps; B succeeds N times and A never succeeds. This is a controlled finite
interleaving, not a native scheduler measurement or a memory-model exploration.
Generalize the schedule until finite-counter retirement, and explain why a
succession of fresh, quiescently initialized instances can repeat it. Per-call
bounded refusal and global work completion do not guarantee per-client success.

For condition waits, use the inspected POSIX scheduling passages: waiter wake
and mutex acquisition order depend on scheduling policy, and a return requires
predicate re-evaluation. State the weakest assumptions under which queued work
eventually runs. No scheduler fairness or finite upper latency bound is supplied
by this C API. A callback that does not return prevents executor completion;
the paused-callback stream test checks retention while delayed, not a universal
cancellation deadline. Measured percentiles describe these samples only.

Pass requires all equal-work and semantic tests, all raw evidence and13 rows,
an explicit comparison of runtime checks versus caller discipline, and a
counterexample to at least one overstated safety or progress claim. Superficial
completion presents timings without the contract or repeats "lock-free is
faster." Understanding explains failures, emitted code, observer effect and
retained resources. Independent mastery additionally changes a workload or ABI,
predicts what must change, and defends the revised result in a separate attempt.

Graduated hints: first compare exact accepted/committed counts; next isolate
metadata, payload and callback costs; finally construct a schedule where one
participant repeatedly loses without any C data race. The supplied delayed
transfer has two executable choices. `deque_sc4`, `deque_weak4` and
`deque_mutex4` keep the same total N produced values and capacity8 while four
thieves compete; report all five participants rather than dividing aggregate
time by an invented equal share. `snapshot_pins4` and `snapshot_copy4` retain
four distinct historical pairs simultaneously, publish a fifth current value,
read each historical pair eight times and release pins in non-LIFO order. The
pin alternative must reject a fifth active pin without changing its output;
the copy alternative does not acquire that extra pin. Count five live versions
versus four caller copies and re-evaluate the representation/lifetime argument.
These are concrete changes to contention and retained state, not constant-only
substitutions. Independently assess one unseen trace in either changed model.
Larger copied payloads, bounded callback CPU work and controlled affinity are
additional investigations, not evidence inferred from these supplied modes.

Author execution status and exact limitations are recorded in
`research/v5/modern/messaging_cost_validation.json` when that report exists and
its pins match. Author evidence is neither a learner assessment nor physical
hardware validation. Security scope is original synthetic local processes;
there are no external targets, real credentials or host-policy changes.
