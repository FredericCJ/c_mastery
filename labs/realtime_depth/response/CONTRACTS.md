# Checked fixed-priority response analysis

## 1. Stable identity and families

Required successor to `PR-T52-03`, assessing `AX-PR-T52-03-T52.03.a`, `.b`, `.c`
and their existing `-DELAYED` variants. Families: semantic reasoning, derivation,
construction, arithmetic boundaries, diagnosis, falsification, resource analysis
and changed assumptions. Existing project identities, profile E, prerequisite
chain, original fixture and historical hours remain unchanged.

## 2. Objectives and inspected references

T52.03.a derives the recurrence and its assumptions; .b implements checked
integer iteration and failure publication; .c independently checks each term
and separates modeled costs from measurements. Exact source records are
`R-LRT-PERIODIC` and `R-LRT-COURSE` in
`research/v5/expansion/legacy_realtime/resources.json`. The inspected book
pages122–123 give a periodic no-jitter/no-blocking recurrence. They do not prove
the more general course arrival-bound model below or establish that the cited
Audsley papers were read. The selected PDF and its exact pages/hash are recorded.

## 3. Prerequisites and artifacts

Import actual completed `PR-T52-02` EDF/job reasoning and `PR-T51-04` mutex/
blocking evidence. Supply the completed original `cmx.c` and public `cmx.h` to
the runner explicitly. The `basic` case executes the actual `cmx_response` on
the common bounded input and compares it with the new implementation. Preserve
its existing smaller parameter/iteration contract. Bring a written ownership
and arrival-count argument; a library numerical solver is not permitted.

## 4. Environment and permitted dependencies

ISO C17 with exact `uint64_t`, explicit caller storage and no allocation. Host
stdio/Python/big integers/sanitizers are verification tools. Strict Cortex-M4
images use original CMX and explicit original byte-support helpers, with no
libc, libgcc, compiler-rt, libatomic or hidden division/multiplication helper.
Audit link maps, undefined symbols and generated instructions. Software
restoring division is part of the exercise; a compiler helper is not an
unexamined permitted implementation. Emulator arithmetic is not a measured
physical WCET or schedulability guarantee.

## 5. Inputs, outputs and implementation task

Implement `response.c` using only the public `response.h` representation.
`RTATask` has cost, period, relative deadline, higher-priority release-jitter
bound, target blocking bound and per-job overhead as separate uint64 fields.
Input tasks are already ordered from highest to lowest fixed priority, with
at most16 entries. The requested target index selects only earlier interferers.
Iteration capacity is1–128. The result contains the last estimate, exact
iteration count and every estimate including the initial value.

The concrete original case is target C2,B1,D10, higher C1,T4,J0: `3,4,4`.
Changing J to1 gives `3,4,5,5`; target D4 first exceeds at5. Test all five public
cases (`basic`, `jitter`, `arithmetic`, `limit`, `interference`) and the oracle
stream. The stream has3,560 exact ceiling cases and3,000 recurrence cases,
including full-width operands and invalid/overflow states. Do not replace the
oracle with calls to the C implementation.

## 6. Why this exercise is necessary

The retained original function uses bounded parameters up to one million and
a fixed128-step cap. Its outcome does not distinguish all resource/invalid
conditions and is not a full-width arithmetic implementation. Naively forming
`R + J + T - 1` may overflow even when the rounded quotient fits. An iteration
cap is not convergence, and a conservative exceeded bound is not itself an
observed missed deadline. A measured mean or maximum is not automatically a
worst-case parameter justified for every admissible execution.

## 7. Derivation, assumptions and contracts

Use this explicit original course model, and prove its use before applying it:
one preemptive processor; fixed priorities; no target self-suspension; one
target blocking allowance B; higher-priority eligible workload in each busy
interval of length x bounded by `ceil((x + Jh)/Th)` jobs, each of cost
`Ch + Oh`; no other uncharged carry-in, interrupt, migration or processor
unavailability. Target response is measured from its eligibility, not its
nominal release. Target jitter is therefore not silently added to this result.
The arrival bound is a premise to establish from the selected release model,
not a theorem about every jittered or self-suspending task system.

Set `b = C + O + B` and iterate
`R[0] = b`, `R[k+1] = b + sum_h ceil((R[k]+Jh)/Th)*(Ch+Oh)`.
Starting at b gives a nondecreasing sequence because the interference function
is monotone. A fixed point within D is a response bound under those premises.
The first estimate above D rejects this sufficient argument; it need not be
the exact worst response. Reaching the iteration cap without either event is
unresolved, not feasible. A target with b=0 completes at0 without interference
iteration. A positive blocking/overhead term changes that special case.

All periods and deadlines must be positive; zero execution costs are allowed.
Input and result storage must be live, aligned and disjoint. `RTA_INVALID`
(invalid count/index/cap/pointer/period/deadline) and `RTA_OVERFLOW` preserve
the entire output. `RTA_CONVERGED`, `RTA_EXCEEDED` and `RTA_LIMIT` publish a
complete new result. Exceeded may occur at the initial estimate with zero
iterations. Refuse a mathematically unrepresentable job count, charged cost,
product or accumulated bound; do not saturate to an apparent deadline success.
Lower-priority blocking fields are not automatically summed; the caller must
derive the target's single B under its actual resource protocol.

Derive `ceil((a+b)/d)` without requiring `a+b` to fit: divide a and b separately,
combine quotients with checked addition, combine remainders by subtraction
against d, then round once. The full-width test includes `a=b=d=UINT64_MAX`,
whose answer2 is representable. Implement unsigned restoring division with a
bounded64-step argument and checked multiplication/addition. Every shift count
is0–63; unsigned wrap used inside restoring subtraction must have a stated
invariant, not be mistaken for undefined signed overflow.

Worst algorithmic work is O(L*N*64), N≤16, L≤128; result storage is129 uint64
estimates plus fields. Compiler stack allocation and helper calls require an
actual target audit. This count is not a timing bound or hardware measurement.

## 8. Deliverables

Submit complete learner arithmetic/iteration code, a premise checklist applied
to each task set, recurrence tables, actual original-CMX comparison, exact
host/target commands, source hashes, raw oracle rows, generated-code/link
audits and the independent falsification controls. Include a cost-evidence
table with columns for assumed execution, blocking, release jitter, per-job
overhead and interrupt contribution. Label each as measured/derived/assumed,
identify the measurement clock and resolution where actual measurements exist,
and retain absent physical data as missing. Synthetic table numbers are not
measurements and must never be reported as such.

## 9. Verification and explicit acceptance

Require the exact published result/status/trace and unchanged refusal output.
Test native64 operands above32-bit range, carry in a+b, rounding-only overflow,
true C+B/product overflow, zero period, exact deadline, zero work and finite
iteration exhaustion. Independent Python integers use direct mathematical
division and recurrence, not the restoring C algorithm. Reject floor ceiling,
omitted blocking, omitted jitter, omitted higher-job overhead and cap-as-success
controls in their discriminating cases, plus a control that overwrites a trace
field before refusal. Compare every defined result member, not just the final
estimate. Compilable unsolved code and an
unsolved original CMX import must each fail a semantic assertion; compiler
errors, crashes and generic failure do not satisfy that gate. A passing finite
stream is evidence for those values only.

## 10. Per-objective rubric

0: incorrect premise/result or fabricated measurement. 1: reproduces a normal
number but cannot distinguish limit, bound, actual miss or overflow. For .a,
level2 derives each term and both jittered/unjittered traces under stated
premises; level3 independently handles D4 and explains first exceed5. For .b,
level2 meets the full transactional/status contract and rejects arithmetic
controls; level3 rederives bounds and audits full-width target helper closure.
For .c, level2 compares independent expectations and labels all cost evidence;
level3 adds the explicit interrupt contribution without also charging it as
blocking or per-job overhead. All level3 work uses the original minimum delay
and an actual unaided learner attempt, never an author campaign as credit.

## 11. Hints and separate assessor analysis

First write the busy-interval workload inequality. Then inspect the smallest
counterexample to floor rounding. Next split a+b into quotient/remainder terms
before rounding; do not start by widening an already overflowing expression.
Finally decide which outcomes publish a bound versus preserve output. Assessor
implementation and technical analysis remain separate and are not exported
with learner material.

## 12. Required changed conditions

Retain the three exact delayed tasks. D4 with J1 exceeds at5. Increasing ranges
requires full-width checked arithmetic and actual helper audit, already supplied
as a reference opportunity. For the interrupt variant add a higher-priority
C1,T3 workload alongside C1,T4 and target C2,B1: `3,5,7,8,8`. Its source of
interference is separate from B and O. A per-preemption overhead is not the
same as the specified per-job overhead: derive a new arrival/work bound or use
an explicit schedule model before applying it. Measurements of physical IRQ
arrival/bursts remain distinct obligations for physical claims.

## 13. Laboratory scope

Original local integer models and a dedicated QEMU guest with synthetic costs.
No host scheduling changes, external targets, board flashing or security
settings. No real-time worker can monopolize the host in this arithmetic
exercise. Deliberate mutants are confined to its own compilation/tests.

## 14. Status and evidence limits

Authored reference and learner scaffold; current producer receipts separately
record executed host cases, oracle rows, freestanding image audits and emulator
observations. No board WCET, complete jitter theorem, independent human review
or learner mastery is claimed. The other T52 scheduler/server/comparison
mechanisms remain required separate work. Original accepted records are not
rewritten.
