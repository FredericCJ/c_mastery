# Nested jump boundaries and actual cleanup ownership

Required X-T65-006/R1-R4 and X-T65-006-DELAYED, early after X-T65-005.
Complete PR-T12-04/T12.15's nonlocal-control reasoning first. The later ownership
stage requires X-T66-002/PR-T66-02 `cleanup.scope` and X-T66-009/PR-T66-09
`memory.adversarial_allocator`; explicitly supply the completed ownership
workspace. Preserve early T65 before T66 and return here for this composition.
Use the actual learner er_run/defer/raise, ow_defer/finish and V4Allocator APIs,
not assessor code or another exception/cleanup library. Hosted ISO C17, GCC and
Clang debug/optimized/ASan+UBSan; hosted setjmp/longjmp are permitted. No strict
freestanding, cross-thread, signal-handler or hardware execution is claimed.

## Original boundary and semantic cases

Study SEMANTICS.md J1-J8, the exact inspected N1570 clauses7.13p1-3,
7.13.1.1p1-5 and7.13.2.1p1-5, and the retained T12.15 CONTROL contract. Derive the
allowed setjmp expression contexts, same-thread live destination and variably
modified scope rule, modified automatic-local exception, volatile repair limits,
and explicit resource cleanup. Invalid examples get explanations and repaired
designs, never undefined executions assigned expected results. The supplied
defined specimen observes valid switch/equality/not placement, volatile7,
external effects and zero becoming one after longjmp.

The actual er_frame owns at most four cleanup registrations. The stack anchor
executes registrations0..5, raised codes0..8 (zero means no raise), and cuts0..6:
cut0 before any registration, later cuts after that many registrations. A fifth
registration returns RANGE without changing the four entries. Normal completion
returns OK17; every admitted raise returns its exact code/offset0. Cleanup is
LIFO exactly once, frames finish inactive and later raise/defer are rejected.
The output trace and registration attempts are distinct. The cleanup callbacks
remain live through finish and do not register, reenter or raise.

## Concrete nested acquisition contract

`ejd_run` accepts a typed allocator, EjdPlan, an external unsigned publication
counter and writable report. Pointer/control/context objects are live, suitably
aligned, initialized and disjoint; no concurrent access or callback reentry is
allowed. The allocator follows the actual T66 take/release contract: allocations
are distinct int32_t regions, release is nonfailing, and callbacks cannot retain
external control pointers or exit nonlocally. All state that changes across a
jump lives outside the function containing the actual setjmp invocation.

Plan cut0..6 requires a non-OK code1..8. Cut7/code0 means normal return. Other
plans, missing pointers/callbacks, or fewer than three increments of publication
headroom return EJD_INVALID before any effect and preserve the report. This
worst-case headroom admission is deliberately conservative even for an early
failure plan. EJD_OK says the experiment ran; inspect report.result for its
typed success/failure. The report is published only after both frames finish.

Outer frame acquires resourceID1 (one typed cell) and registers its cleanup.
It then invokes a separate inner frame while ID1 remains owned. Inner registers
one bridge cleanup that invokes actual ow_finish on a separate OwScope. It
acquires IDs2,3,4, requesting2,3,4 cells respectively. Every successful inner
acquisition increments the publication counter, then registers an OW_ALWAYS
cleanup in the scope. Cuts0/2/4 raise before each acquisition; cuts1/3/5 raise
after acquisition but before its registration, so the unregistered resource
must be released directly before raising. Cut6 raises after the third
registration. Cut7 returns inner OK37. An allocation failure records stage8/9/10
and raises IO. The scope bridge still runs exactly once when no resource was
acquired. No callback itself jumps; the body raises only after it owns a valid
cleanup path.

After the inner boundary returns and its cleanup has completed, outer acquires
ID5 (five cells), registers it after ID1 and returns. Inner failure converts to
outer CONTRACT with offset equal to the recorded inner stage, while the exact
inner result remains in the report. Inner success gives outer OK99. Initial
outer allocation failure gives IO(offset20) without entering inner; final outer
allocation failure gives IO(offset21), preserving any recorded inner result.
Both outer resources drain in reverse order. There is no published surviving
resource owner; the external counter, however, is not rolled back.

Example cut5/THROWN: all five allocations succeed; direct release of inner ID4,
scope release3/2, then outer release5/1 produces trace[4,3,2,5,1]. The remaining
ownership bit masks after each release are[14,6,2,2,0], using bitID. Three external
increments persist although outer returns CONTRACT5. Allocation failure at
attempt4 instead fails the third inner acquisition: inner IO/stage10, outer
CONTRACT10, two increments, four successful acquisitions/releases. Report.held
must be zero, scope empty and both frames inactive. Counter increments are
observable outside this local resource transaction, not durable storage or a
physical device action.

## Deliverables, verification and falsification

D1 includes J1-J8 classifications, stack/ownership diagrams and the proof that
the context is outside the setjmp destination function. D2 implements the exact
adapter and the original boundary; D3 supplies per-cut/per-code traces, an
independent symbolic owned-set oracle, allocation fault schedule, all compiled
controls and exact logs; D4 demonstrates separate nested frames and cause
conversion while preserving outer order. OwnScope finish policy is OW_ALWAYS
for every resource; the bridge uses success1 only because no conditional entries
exist. Do not generalize that choice to arbitrary error/success-only scopes.

Five anchors assess original stack, resource cuts, nested conversion, retained
effects and structural admission. Six new controls omit the scope bridge, reset
published effects, lose conversion, release the outer owner too early, leak an
unregistered acquisition, or erase the inner stage. Two controls compile actual
ER_MUTANT11/12; another compiles actual OW_MUTANT12. Require normal ER_CHECK
failures after successful compilation, not traps or sanitizer aborts. The typed
bounded allocator makes retained-owner failures inspectable without inducing a
double-free or fabricating a heap-leak diagnostic. Learner stubs compile and
must fail their selected anchors.

The independent Python model derives the acquired ID set, reverse lifetime
trace, allocation attempt indices, typed inner/outer results and separate counter
effects. It does not simulate machine saved registers or prove arbitrary jump
code. Compare every emitted field, including masks/canaries. Inspect actual
imports and generated code; the C library supplies the permitted jump mechanism.

R1 scores exact clause/lifetime reasoning; R2 explicit bounded cleanup and result
conversion; R3 before/after registration, capacity, every code, normal paths and
invalid-case analysis; R4 actual T66/resource integration and delayed nested
conversion. Nominal reproduction scores1, complete bounded reasoning and evidence2,
unassisted changed-condition transfer with a counterexample3. An invalid jump,
lost owner or unreported external effect fails regardless of other scores.
Hints: identify the setjmp-containing function, mark ownership before each raise,
distinguish registered from acquired resources, and draw inner cleanup before
outer continuation. Assessor analysis remains separately packaged. Seven-day
delayed assessment is not automatically credited by author execution.

## Cost review (thirteen dimensions)

1. Representation: measure actual frame, scope and report sizes; jmp_buf is an
   implementation-defined representation, not a portable register layout.
2. Work: at most5acquisitions/releases,3inner registrations and2outer resources;
   original stack capacity4 and imported scope capacity8 remain distinct.
3. Space: fixed caller/adapter state, two frames, one scope and five resource
   descriptors; report actual compiler stack use, not an assumed small cost.
4. Allocation: actual T66 requests1+2+3+4+5cells on full success; each failure
   schedule records attempts/live/releases separately.
5. Traffic: typed cell initialization, frame/runtime state, cleanup entries,
   report copies and trace writes; no rollback of arbitrary program memory.
6. Dispatch: actual body/cleanup/allocator callbacks plus permitted jump runtime;
   imported scope does not remove callback costs.
7. Code size: preserve executable/object size and disassembly for each profile.
8. Stack: preserve emitted stack-usage files; longjmp skipping calls does not
   establish a smaller maximum live stack.
9. Timing: seven1000-run samples of cut5/THROWN, checksum91000 and35000actual
   allocation/release calls; same full workload, no exception-overhead theorem.
10. Failure: distinguish pre-acquisition, acquired-unregistered, registered,
    converted inner error and failed outer continuation; every local owner drains.
11. Scheduling: same thread, synchronous callbacks; cross-thread jumps/signals,
    asynchronous cancellation and fallible cleanup require separate contracts.
12. Builds: clean/unchanged builds, actual compiler include closure and imported
    producer object identities are retained.
13. Portability: changed C edition/ABI/libc requires revisiting the exact jump
    clauses, layout and runtime; drafts are not published C17/C23 inspections.

Only original local programs, synthetic counters and typed teaching allocations;
no credentials, external targets, privilege changes or deployed systems. This
scope studies resource/state failures, not exploitability. Current source-bound
author execution, external review, unaided learner success and physical evidence
remain separate. Finite tests do not prove arbitrary C execution or mastery.
