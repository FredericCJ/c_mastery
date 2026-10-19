# CBS technical analysis and assessor discriminators

The two soft modes import the actual retained `cmx_cbs_init/release/charge`.
An explicit queued bit prevents the common mistake of resetting the retained
active flag on idle: that flag also indicates that a reservation exists.
The book mode changes only the equality activation boundary. At Q4/P8,
remaining2 and distance4, both cross-products are16; book reset gives4/12,
strict retained behavior gives2/8. This difference is exposed, not hidden by
renaming both functions CBS.

The hard documented comparison is separate original code. D controls a fresh
deadline, P controls density and subsequent replenishment. It keeps zero
remaining budget while throttled. Late refill more than a whole period behind
the observation is explicitly outside this small model; FULL preserves all
state. The 600-workload oracle includes five such failures instead of silently
dropping overload inputs or inventing current Linux catch-up semantics.

Each successful charge uses at most the available budget. Soft depletion
postpones the deadline before another unit may run. A single client can finish
eight units at time8 with Q3/P10; a throttled client finishes22. With the Q2/P5
competitor, the first soft budget depletes after time slots2,3,4; at5 its new
deadline20 loses to the competitor's10. Client0 finishes14; client1's six
jobs each finish release+2. Application deadlines are recorded independently.

All parameter operands are below2^31. Each density product is below2^62 and
the sum of two remains below2^63, so uint64_t suffices without division.
Modular observations must advance by less than2^31. Postponement refuses an
ambiguous forward ordering span before publication. A staged workload trace
is copied only after the whole finite model completes; rejected late-model
experiments preserve sentinel-filled output and are independently predicted.

The mutex experiment invokes actual CMX task creation, sleep, EDF selection,
mutex lock/block/unlock/handoff and exit. The owner consumes its own reservation
while the waiter is genuinely WAITING in the CMX model. The waiter receives
ownership only through the real unlock handoff. With a two-unit critical
section it finishes3; with seven units it finishes8 and misses application
deadline4. It retains both budget units while blocked. Priority donation is
not bandwidth donation; charging the blocked waiter is the dedicated fifth
mutant. This is a counterexample to applying an independent-task isolation
claim after introducing a shared lock, not an implementation of BWI or a
claim that real task execution has been measured.

The main isolation theorem statement was read in the original1998 paper, whose
proof points to RETIS TR98-01. That report's full text was not obtained. Require
students to state that evidentiary boundary; a test campaign does not replace
the proof. The local transition arguments and finite oracle are original
course analysis. Keep all three criterion rubrics explicit and do not award
the changed-D or shared-lock assessment solely for passing the ordinary case.
