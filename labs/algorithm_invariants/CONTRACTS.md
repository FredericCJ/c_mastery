# T36.01 invariant and operation-ledger successor

This additive contract implements the three original `AX-PR-T36-01-T36.01.a/b/c`
criteria and their exact `-DELAYED` variants; each delayed learner gate still requires
at least seven days. Parent teaching, prerequisites, objective rubrics and the legacy
shared assessment contract remain authoritative. Families A/B/C/D/F/H/I/J apply.
This does not close T36.02/.03–.10 or any T37 obligation.

## Environment, prerequisites and supplied artifacts

Use active C17, live typed objects, exact uint32_t and hosted Linux GCC/Clang under
O0/O3 and ASan/UBSan profiles. `ai_step` and `ai_permutation` are Profile C components:
no allocator/library algorithm, I/O, library container or runtime dependency. Separate
`ai_growth` and the experiment harness are Profile A adapters. Growth imports the
learner's completed `struct cd_vector` through `labs/containers_depth/containers.h`; its
callbacks may use genuine malloc/free storage. Queue comparison imports actual
`struct cd_ring` and `struct cd_queue`, with their frozen `CONTRACTS.md`. Do not rebuild these
containers. Diagnostic counters, Python oracles and sanitizers are host verification
dependencies. The actual provider identity and generated instrumentation are retained
with source hashes; original provider files and acceptance records remain unchanged.

Supplied files are `invariants.h`, `instrument.h`, `learner.c`, `tests.c`, the provider
header/contracts and `fixtures/algorithms/PR-T36-01.json`. Know unsigned arithmetic,
loop preservation/progress, multiset versus set, object lifetime and the prior queue/
vector failure contracts. Produce your own prerequisite learner implementations;
assessor implementations and source-transformation tools are separately packaged.

## Problem and mechanism

Implement `ai_step` over `struct ai_state{a,b}` with values0..4095 and nonzero pair. Mode0
performs one Euclidean remainder transition `(a,b)->(b,a%b)`, only when b>0.
When b=0 return AI_DONE without touching output. Mode1 repeatedly subtracts the
smaller positive operand from the larger; equal operands or either zero return
AI_DONE unchanged. Reject null output, unknown mode, range overflow and (0,0)
without output mutation. Inputs and outputs are disjoint live objects. Distinguish
the first operation no longer admitted (modulo by zero) from the valid terminal
observation (gcd is the surviving operand). Mode1 has a different decreasing measure.

Implement `ai_permutation` for at most16 actual `{uint32_t key,tag}` records. Null
spans are accepted only at zero length. Publish a boolean for full-record multiset
equality; return AI_INVALID without changing output on invalid length/control. Use
only primitive comparison and caller-independent bounded local scratch. Equal keys
do not make different tags equal. Membership in a set does not preserve multiplicity.
This operation checks permutation, not stability or sortedness; those are separate
properties. Analyze `[2,1] -> [1,1]` and equal-key records whose tag multiplicities
change while the set of full records remains the same.

Run actual CDVector appends for n0..32 under two policies: the original doubling
policy, and additive growth implemented by `cd_vector_reserve(length+1)` before
each append. Do not replace the frozen vector implementation. Report actual copy
counter differences plus the new-element write, worst single-append payload work,
final capacity and acquired/released backing blocks. On an adapter/provider failure,
destroy the temporary vector and leave public result unchanged. The harness's malloc
failure is separate from the already validated provider failure-injection portfolio.

Replay put11,put22,take,put33,put44,failed put55,take through capacity3 CDRing and
the first three admitted CDQueue nodes (refill disabled). Compare actual logical
state, ring head/count, node head/tail/live links, removed values and full/empty
failure snapshots. Both owners stay at stable addresses; copying an AIQueues owner
does not retarget its ring backing pointer. The adapter is a diagnostic composition,
not a failure-atomic abstraction over inconsistent producer implementations: a
producer disagreement is CD_FAILED and its state must be inspected, not retried.

## Exact operation model

Count every evaluated `if`, `for`, `while` and conditional-expression decision in
the selected actual queue functions, including their selected helpers. One entire
controlling expression is one decision: individual short-circuit operands, calls,
integer arithmetic and assignments are not silently counted as extra decisions.
Count one logical payload transfer for each admitted put/take, every typed-node
occupancy probe including the final free-slot check, and every followed queue-next
link on removal. Observer traversal to print snapshots is excluded and named
separately. These are source-level units, not instruction counts, cache traffic,
time or all possible C operations. Return boolean expressions that do not control
flow are outside the decision unit; conditional expressions inside them still count.

Instrument copies generated from the exact frozen source by inserting comma-expression
counter increments around controlling expressions and named transfer/probe/link
expressions. Evaluate original expressions exactly once and preserve short-circuit
behavior. Run both unmodified and instrumented producer copies with the same state
oracle. A transformation mismatch fails rather than guessing after provider changes.
Explain every row, especially the full insertion: it performs admission decisions
but no payload transfer, node scan or link follow. A single aggregate operation count
that omits this failure is insufficient.

## Reasoning, deliverables and verification

D1: derive common-divisor preservation from `a=q*b+r`, show b decreases in modulo
mode, and a+b decreases in positive subtraction mode. Establish bounded unsigned C
operations separately from the mathematical argument. Provide 1071/462 traces and
zero/equal/extreme cases; never execute remainder by zero as a demonstration.

D2: actual provider identities, states.csv and cost table with all declared units,
plus doubling/additive derivations. Eight doubling appends make7 old-value copies
and8 new writes (15); additive growth makes28 copies (36 writes). Show why the
worst operations differ from amortized sequence bounds and name allocator/control
work excluded from the payload metric. The tests also admit every n0..32.

D3: first failing transition and repaired invariant for the lost-value trace;
independent full-record multiplicity counterexample, unchanged invalid outputs and
mutant-specific diagnostics. Provide sortedness, multiset and stable-order predicates
separately; none can stand in for the others.

Run all four public cases with the unmodified producer and a separate instrumented
copy. Independently compare Euclid against enumerated common divisors for all4095
nonzero pairs in0..63, both modes (8190 cases); also test4095/1 subtraction. Compare
the supplied fixture with an independent deque/math/Counter oracle. Inspect object
undefined symbols for the Profile C core; hosted allocation belongs only to the
adapter object. Compile learner stubs successfully and require semantic rejection.
Retain distinct mutants that reject valid zero terminal states, mistake set membership
for multiplicity, and omit old-value copying from payload work. Each must fail its
intended assertion while the repaired implementation passes every required success.

## Rubric, hints and changed conditions

Level0: wrong/missing result or unsafe admitted operation. Level1: supplied numbers
reproduced without the relevant preservation/cost/multiplicity argument. Level2 must
meet every exact original task and these objective-specific discriminators:

| Objective | Independent evidence required | Level3 delayed transfer |
| --- | --- | --- |
| T36.01.a | Correct relation, terminal admission and separate termination proof with zero/positive boundary | `AX-PR-T36-01-T36.01.a-DELAYED`: replace remainder with subtraction, prove changed measure and derive11 versus3 operations |
| T36.01.b | All declared queue primitives including full failure and actual15-write doubling ledger | `AX-PR-T36-01-T36.01.b-DELAYED`: additive reserve policy, derive quadratic total and worst append |
| T36.01.c | Locate first lost multiplicity; strengthen invariant to full-record multiset without confusing sortedness | `AX-PR-T36-01-T36.01.c-DELAYED`: fresh tag corruption preserving keys and set but not multiplicity |

Hints in order: identify which state may legally be the next operand; write the
invariant and strictly decreasing nonnegative measure separately; charge a failed
operation's checks before counting movement; mark each consumed occurrence in the
permutation comparison. Private analysis contains exact solutions, not learner imports.

## Security, validation and limits

Only original synthetic states/records are used. Logical corruptions are defined
values and no stale pointer, out-of-bounds access or zero division is executed.
No external targets, real data, privileges, host settings or hardware actions occur.
Validation is a separately bound author receipt; it cannot establish independent
learner work, external review, physical costs or absolute mastery. ODS is a C++
mechanism comparison, not a C object/lifetime guarantee. Exact published C17/C23
and unsampled source passages remain explicit reference gaps. Historical hours
remain descriptive; this addition has no learner-time ceiling.
