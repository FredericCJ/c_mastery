# Histories, resource admission and progress

This required additive packet implements original PR-T26-01 T26.01.a–c and
T26.02.a–c plus their unchanged seven-day variants. Retain their exact tasks,
rubrics and fourteen-field shared assessment contract. This is author material
and executable scaffolding, not accepted learner evidence. SPSC, reclamation
construction and kernel tasks retain their separate contracts.

## Prerequisites and environment

Bring the retained CMX queue interface, T25 atomic publication/reuse arguments,
bounded recursion/state enumeration and the original T26 predictions. Read the
exact inspected passages in
`research/v5/expansion/legacy_advanced_history/resources.json`: Herlihy/Wing
sections2.1–2.2, corrected Michael–Scott pseudocode including its allocator
premises, and the original paper's algorithm/safety/progress discussion. The
paper's CAS and memory-management model is not itself a proof of this C17
implementation; the C11 draft source remains explicitly distinct from final C17.

Core C17 profile C: typed fixed storage, no external history-checker or queue
library, no allocation in the implementation. Python, hosted stdio and sanitizers
are host verification tools. The retained CMX implementation is a real imported
artifact; implement the exported CMX learner skeleton separately. Target
audits select native and Cortex-M4/M0 compiler lowering and never authorize ISR
execution. The allocator diagnostic intentionally changes the dependency
profile; strict target links must reject its unresolved malloc/free boundary.

## History representation and operations

Implement `ah_history` for one initially empty object with1–4 capacity and at
most7 operations. Each operation contains invocation/response event ordinals,
participant0–7, operation INSERT/REMOVE, value and result. Event ordinals are
unique; invocation can be0, response must be later, and UINT32_MAX denotes a
pending response only. A participant cannot have overlapping operations. Reject
invalid dimensions, events, participant and enum fields without changing output.
Input/output/control objects must be live, aligned and disjoint. No callbacks,
retained pointers or shared concurrent checker state exist.

For FIFO, INSERT appends and returns VALUE, or FULL precisely when logical
occupancy equals capacity. REMOVE returns the oldest VALUE, or EMPTY. For SET,
INSERT returns EXISTS if already present, otherwise FULL if capacity is reached,
otherwise inserts VALUE; REMOVE(value) returns VALUE and erases that value or
ABSENT. The SET removal argument changes the operation contract: do not merely
rename a FIFO and ignore duplicates. The history's result/value fields must
match the chosen sequential specification.

`AH_COMPLETE_ONLY` returns INCOMPLETE if any operation is pending. The changed
`AH_EXTEND_PENDING` policy chooses a subset of pending operations to complete,
derives legal results for those included, and discards the other pending calls.
Every completed original call remains mandatory. Appended responses occur after
the recorded prefix; respect all already recorded real-time precedences. Return
VALID with a witness containing selected operation order, selected-pending mask,
results and values, or NONLINEAR if no witness exists. Publish the witness only
on VALID; invalid, incomplete and unsuccessful searches preserve the output.
The search explores at most2^p times7! permutations, with bounded copied object
state and at most7 recursive levels. State stack and code costs; do not present
this exponential assessor as a production unbounded history checker.

## Six concrete packets

**D01a — histories and pending completion.** Check capacity2
push11[0,3],push22[1,2],pop11[4,5], and the nonoverlapping
push11[0,1],push22[2,3],pop22[4,5]. Supply a legal witness or enumerate why none
exists. Add the original pending-enqueue case where a completed removal already
returns its value: dropping all pending calls is wrong. Compare the C checker
to the independent permutations oracle over every selected three-operation
action combination and90 event orders. Preserve invalid-input and witness
publication checks and a falsification control that ignores real-time order.

**D01b — two distinct correctness questions.** Execute the small sequentially
consistent atomic LIFO trace in `tests.c`. All its accesses are defined and
sequentially ordered, but the FIFO checker must reject the return order. The
SET specification may admit the corresponding removal of a named member.
Independently draw the release/acquire reads-from edge protecting an ordinary
payload; removing release or acquire leaves a C data-race candidate even if an
observed value happens to match FIFO. Use a relation model, not native undefined
payload values, as that counterexample. Compare the changed SET duplicate,
absent-member and capacity behavior to its own oracle.

**D01c — candidate points and bounded failure.** Trace initialized private node,
successful next-link CAS, optional/helped Tail advance, Head CAS and empty
observation. Construct the reachable prefix with one linked node and Tail still
at the dummy, then call the actual frozen CMX consumer: it must help Tail and
return the item. Keep this prefix experiment distinct from an instrumented
concurrent execution. Show why Tail movement cannot be the mandatory insertion
point. Check real FIFO empty/full success cases separately from pool admission.
The original changed bounded-failure task must distinguish the following
contracts: logical-capacity FULL in the history checker, and CMX_FULL meaning
the bounded free-node scan obtained no node. CMX can reject an empty logical
queue when all nodes are retired. Under concurrent collection its scan is not
an atomic global pool-full snapshot; admission may fail even if some free slot
exists at each instant. A permissive try-admission failure can linearize during
that call without changing FIFO contents. A specification demanding exact
logical/full-pool failure needs stronger admission coordination and a new proof;
the existing CMX result cannot establish it by renaming the enum.

**D02a — progress and changed retry semantics.** State the participant/step
quantifiers for blocking, obstruction-free, lock-free and wait-free behavior.
Analyze four declared algorithms: a mutex owner that can stop; an intent-and-
withdraw attempt that succeeds when isolated but repeats under collisions; a
CAS retry operation in which a competitor wins each observed loss; and an
operation with three bounded own steps. Supply repeating-state witnesses and
the conditional proof for each supported level. `ah_retry` reports VALID only
for an actual changed result, NONLINEAR for another allowed attempt and
INCOMPLETE when a failed attempt budget is exhausted; these names belong only
to this diagnostic. Reject zero budgets and attempts above budget. A capped
return terminates the try operation, not an obligation to successfully enqueue.
No wall-clock bound follows from a bound on a scheduled thread's own steps.

**D02b — whole-path dependency audit.** Inventory retained CMX's32-node bounded
allocation scan, explicit collector scanning32 nodes×8 hazard slots, head/tail/
state atomic widths, retry loops and absence of allocation/callbacks/OS calls.
Count the scan bounds separately from unbounded contention retries. Record
atomic lock-free queries and actual selected-image imports for each compiler/
target. Weak CAS spurious failure and compiler helper progress are additional
premises; neither the paper nor successful execution proves them universally.
The changed malloc/free adapter must visibly import those functions and fail
strict no-runtime closure. Allocation failure, locks, allocator metadata and
reclamation lifetime now become external dependencies; malloc is not guaranteed
lock-free by ISO C. If a callback is later added, include its effects and bound.

**D02c — system progress versus service.** Produce1024 CAS-loss rounds with a
competitor's1024 completed changes and zero completed operations for the losing
participant. The state pattern can repeat despite both threads taking steps;
CPU fairness alone supplies no successful CAS fairness. In the actual frozen
queue, perform31 push/pop pairs without collecting, show logical empty with31
retired nodes and failed admission, then collect and recover. Count resource
pressure separately from algorithmic retry progress. The original changed
per-request requirement must name a helping/service algorithm and its own-step
bound, bounded primitives and effects, plus explicit scheduling/admission
assumptions for a time bound. Model the premises and reject a service claim
derived only from finite successful stress or a retry cap.

## Deliverables, controls and assessment

Supply D01a–D02c source, witnesses/counterexamples, ownership and resource
invariants, first broken relation, raw commands, target object/link evidence,
independent oracle results and the exact original delayed defense. Never count
one checker result as passing all six rubrics. The public scaffolds must compile
and fail their named AH_ASSERT controls; private implementation/oracles/analysis
remain under assessor. Source-bound author campaign:
`tools/run_advanced_history.py`. Its finite corpus and target link checks do
not prove the infinite-history/liveness obligations or establish learner mastery.

Hint1: write the sequential object specification before ordering concurrent
events. Hint2: pending calls may already have affected completed calls; preserve
real-time edges when selecting an extension. Hint3: locate the pointer change
that changes reachable FIFO contents, then inventory the resources needed to
reach it. For progress, distinguish the repeatedly losing request from all
requests and state which implementation/scheduler premise the argument uses.

Retain the original four levels:0 incorrect/unsafe or unsupported claim;1 a
matching ordinary result without causal argument;2 the corresponding concrete
packet, boundary, independent expected result and intended negative control;
3 the unchanged original delayed task independently derived after its interval.
SC versus FIFO, changed SET, pending selection, Tail helping, resource admission
and end-to-end progress are separately assessed. Authoring these solutions does
not constitute the learner's delayed attempt or external review.

Security scope is the original local synthetic teaching program. Do not execute
native weakened-order payload races as numeric demonstrations. The harmless
failure is a rejected history, boundary assertion or failed strict link, never
a claim of code execution or privilege. Physical, kernel, independent learner,
final-standard source and broader reclamation obligations remain open.
