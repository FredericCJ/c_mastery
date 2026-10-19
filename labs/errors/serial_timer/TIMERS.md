# Arithmetic policy, partial serial order and timer residence

Required X-T65-007/R1-R4 and X-T65-007-DELAYED. Complete X-T65-006 and the
accepted `bytes.checked` reasoning/artifact first. Use the actual learner
er_add16/er_serial16 implementations; no installed timer/container mechanism.
The original C17 core remains unchanged. Hosted GCC/Clang debug, optimized and
ASan+UBSan harnesses are separate from a C23 stdckdint comparison. No target,
interrupt, wall-clock accuracy or strict freestanding execution is claimed.

## Arithmetic and comparison

The retained er_add16 contract admits uint32_t inputs0..65535 and three policies:
checked rejects a mathematical sum above65535 without changing output; wrapping
publishes sum modulo65536; saturation publishes min(sum,65535). Invalid inputs,
policy or output pointer are rejected. Admitted sum is at most131070, representable
in uint32_t, without signed overflow. This generic wrapping policy is not the
RFC1982 rule for which increments may safely be compared with an old serial.

eat_serial admits width2..16, values below2^width, and a live int output. It
delegates width16 to actual er_serial16. Other widths use unsigned bounded
intermediates. Equal gives0; forward modular distance1..half-1 gives1; backward
distance gives-1; exact half returns RANGE with output unchanged. RFC1982 leaves
that relation unresolved; this C API explicitly returns an error. This is not
ISO C undefined behavior. For width4,0>11,11>6,6>0 forms a cycle: the relation
cannot serve as an unrestricted sorting comparator. Exhaust all256pairs and
4096triples at width4; verify antisymmetry only for comparable pairs and retain
the336oriented cycles, not a false transitivity theorem.

## Bounded-residence queue

EatWindow has width, low counter, count0..4 and four insertion-ordered entries
{id,deadline,age}. IDs are uint32_t, including0, unique only while queued. No
external handle or asynchronous callback identity is promised. Fresh init admits
width2..16 and now<2^width. Arm requires delay<half, duplicate absence and space;
deadline=(now+delay) modulo2^width, age0. Duplicate refusal precedes full refusal.
Delay bound precedes both. Invalid arguments return INVALID; capacity/delay limits
return RANGE; every refusal preserves the entire valid control/output object.

advance receives an externally certified elapsed interval, not two sampled low
counter values. Require elapsed<half and age+elapsed<half for every queued entry,
including entries already due. Preflight all entries before changing any field.
Then update the low counter and ages. Since both age and delay are strictly below
half, their mathematical difference lies strictly between -half and half: comparing
now against each deadline is unambiguous. The largest addition before checking
is2^width-2, representable in uint32_t. Sampling only the low counter cannot certify
that no complete revolution passed; identifying that missing assumption is graded.

pop selects a due entry using the actual serial comparator; among due entries,
greatest lateness comes first, with insertion order breaking equal deadlines.
It removes that entry, compacts remaining entries and writes its ID. No due entry
returns AGAIN with output and queue unchanged. Capacity4 makes scan/compaction
bounded; no library sort is allowed. Callback execution is outside this API.

Example width16: start65534, arm(11,2), arm(22,0), arm(33,2); pop22, advance3
wraps now to1, then pop11 followed by33. Example width4: start14, arm(1,3),
advance7 is admitted; another1 is refused unchanged because residence reaches8.
Draining the due item allows subsequent progress. The refusal does not pretend
the real clock stopped: the caller must service before the stated bound, or stop
using this representation and recover with an external absolute-time authority.

## Changed lifetime and width

At low now0 and low deadline1, an event due at absolute1 with actual now16 is old,
while an event due at17 is future. Both have identical four-bit inputs. No function
of those two low values can distinguish them. A wider wrapping integer postpones
the same ambiguity; a half-window check on individual increments alone is insufficient.

EatEpoch changes the contract: now and deadlines are uint64_t absolute nonwrapping
timestamps; display_bits2..16 affects presentation only. The caller supplies the
actual elapsed interval from an external authority, with no reset or unreported
wrap. Arm checks delay<=UINT64_MAX-now before addition. Advance checks the same
bound for elapsed. Pop chooses the least absolute due deadline and stable insertion
tie, independent of low display bits. A resident deadline may survive arbitrarily
many display wraps within the finite64-bit lifetime. At UINT64_MAX, positive
advance/positive-delay arm refuse; existing owners can drain and zero-delay arm/pop
remain legal. This is a finite nonwrapping lifetime contract, not an unbounded clock.

The changed-width proof covers width2,3,4 and16 tests plus exact2..16 comparison
boundaries. State which constants are representation limits and which are physical
rate/lifetime assumptions. Reset/reboot/recovery requires a separate persisted epoch
or a new exclusive service lifetime; no such hardware authority is fabricated here.

## C editions and enforcement

Inspect N3096 draft7.20/7.20.1: checked arithmetic converts the mathematical result
and reports representability, including writing a wrapped destination on failure.
Thus raw ckd_add is not the unchanged-output er_add16 contract. The C23 companion
executes uint16_t overflow to0, then a local temporary that preserves caller91 on
failure, and exactly-once operand side effects. The manual core stays C17. Installed
header/compiler behavior, draft wording and inaccessible published C17/C23 editions
are separate evidence. Neither checked arithmetic nor a C type proves timer residence.

## Deliverables and assessment

D1 derives arithmetic range, the pair relation,336cycles and both queue invariants.
D2 implements the exact header with fresh init and valid live disjoint serialized
controls; no byte corruption, forged control, concurrent access or reentry is admitted.
D3 supplies independent integer-timeline cases, canaries, cumulative-age refusal,
stable ties, capacity/duplicate boundaries, UINT64_MAX drain, compiler logs and all
eight compiled controls. D4 seals the low-pair indistinguishability argument and
new absolute-time contract before implementing the changed-width delayed case.

R1 requires arithmetic/model distinction; R2 transactional implementation; R3
independent pairs/triples and actual C23 comparison; R4 the real queue plus revised
lifetime and width proof. Nominal reproduction scores1; justified bounded evidence2;
unassisted transfer with a counterexample3. Lost refusal state, unsafe signed
conversion, arbitrary half-range order or a claimed infinite64-bit lifetime fails.
Hints: use mathematical age-delay, distinguish observation from authority, stage
before publishing, and select a common live window rather than globally sorting.
Assessor analysis is separate. Seven-day delayed learner assessment stays unperformed.

The Python oracle orders unbounded absolute due times, not serial differences. It
compares every driver result for324policy,417serial,70263window and1875epoch cases
per profile. Six anchors reject compiling stubs. Controls change state before
refusal, omit accumulated age, compare low values numerically, reverse equal ties,
wrap an exhausted absolute clock, or revert to low comparison; actual ER_MUTANT13/14
cover old write-before-reject and half-range acceptance. Require ordinary ER_CHECK
failures, not sanitizer deaths. Finite cases do not prove all implementations.

## Cost review (thirteen dimensions)

1. Representation: record both struct sizes/alignment and field widths; host padding
   is not a wire format.
2. Work: scan at most4entries for duplicate/advance/pop and move at most3on pop.
3. Space: fixed4entry caller storage; no growing queue or recursive algorithm.
4. Allocation: zero mechanism allocations; harness/libc costs are separate.
5. Traffic: preflight reads all live ages; pop compacts typed entries and publishes
   the result once.
6. Dispatch: no mechanism callbacks; actual width16 serial call remains an import.
7. Code size: preserve object/executable observations and compiler/flags.
8. Stack: retain emitted stack-use files; structure init can induce helpers.
9. Timing: seven10000-run absolute-queue samples, checksum770000, identical workload;
   no comparison with a different correctness contract or physical deadline claim.
10. Failure: unchanged state on bound/capacity rejection, due-item drain at exhaustion.
11. Scheduling: externally serialized, externally certified elapsed intervals; the
    queue does not supply interrupt delivery or timer fairness.
12. Builds: clean/incremental observations and actual dependency/object closure.
13. Portability: repeat source/width/rate/libc reasoning on changed assumptions;
    C23 header experiments do not change the C17 baseline.

Only original local synthetic timer states are used. No external system, credential,
hardware configuration or deployed device is involved; no exploit claim follows
from a stale timer. Author execution, external review, physical measurement and
independent learner performance remain separate dimensions.
