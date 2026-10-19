# T36.01 assessor solution and instrumentation analysis

## T36.01.a

For b>0, a=q*b+r,0<=r<b makes each divisor of a,b a divisor of b,r and conversely.
The modulo trace1071,462 ->462,147 ->147,21 ->21,0 has3 transitions. The next
remainder operation would use divisor0 and is not admitted. No such operation is
executed. Terminal observation and illegal transition must not be conflated.
Subtraction ends at21,21 after11 transitions. Positive a+b decreases, while gcd is
preserved; at equality the common value is the result. Zero inputs are handled
before subtraction, because subtracting zero would not progress. For4095,1 it takes
4094 steps, demonstrating a different operation-count model from remainder. Both
operands<=4095 keep sums<=8190 and subtraction admitted only in the nonnegative
direction. Mathematical termination alone does not authorize finite-width overflow.

## T36.01.b

Actual CDVector receives the same increasing values. Doubling performs copies at
lengths1,2,4:7 old writes+8 new writes=15, worst4+1=5. Additive pre-reserve makes
0+1+...+7=28 copies, total36, worst7+1=8. Generally additive total n(n+1)/2 and
worst n; doubling copies a geometric series below2n and payload work below3n.
These omit allocation, metadata, branch and cache costs by explicit definition.
Both modes release every acquired live block before reporting.

Source-preserving instrumentation surrounds controlling expressions, not whole
operations. The following exact seven rows count decision evaluations/payload
transfers/typed-node occupancy probes/next-link follows:

| Event | Ring | Node queue |
| --- | --- | --- |
| put11 |20/1/0/0|4/1/1/0|
| put22 |20/1/0/0|5/1/2/0|
| take11 |7/1/0/0|3/1/0/1|
| put33 |20/1/0/0|4/1/1/0|
| put44 |20/1/0/0|6/1/3/0|
| failed put55 |5/0/0/0|3/0/0/0|
| take22 |7/1/0/0|3/1/0/1|

Derivation: ring admitted one-item push evaluates4 decisions before reserve;
reserve validates its owner (including the head-bound ternary), busy and capacity
in4, then2 tail ternaries; spans contributes3; returned-status check1; two copy
loops contribute3 including terminating checks; commit contributes3: total20.
Full push exits after5 (validation/head-bound, busy, capacity, partial-policy).
Ring take has validation/head-bound2, busy1, amount1, loop2, head-wrap1=7.
Node push has admission1, full1, occupied-slot loop k+1, tail1 =>k+4. The successful
indices0,1,0,2 require1,2,1,3 probes. Full push has admission/full/refill-policy3.
Node take has admission, empty and final-empty3 plus one followed link. Totals are
ring99 and node28 decisions,6 transfers each,7 node probes and2 follows. Local
temporary loads, metadata stores, uninstrumented initialization, snapshot traversal
and short-circuit operands are outside the named metric and must not be presented
as measured zero cost. Runtime comparison uses actual frozen producers; no queue
policy is reimplemented to manufacture these figures.

## T36.01.c

The first mutation overwrites2 by1: sortedness may improve while multiplicity loses
one2 and gains one1. Restore a multiset-of-full-records relation plus the actual
pending-record invariant where insertion temporarily removes a value. A distinct
changed case uses keys all1: before tags10,10,20; after10,20,20. Both full-record
sets are identical but multiplicities differ. Marking a matched occurrence prevents
its reuse; the deliberately unmarked set-only mutant accepts this corruption.
Multiset preservation still does not prove stable ordering, so stability remains a
separate obligation in the sorting project. Matching uses O(n^2) comparisons and
16 bounded flags, with no arbitrary struct reinterpretation or library sort.

## Boundaries and assessment

Author execution covers these exact ordinary/changed packages, finite integer pairs,
actual imported containers and source-defined counter paths. It does not execute
an independent learner's delayed work or establish all algorithm correctness. All
original criteria/delay IDs remain unchanged. T36 paired ABI kernels, hash lookup,
graph wrappers, typed/borrowed interfaces and T37 obligations are separate required
packages. No process, timing, allocator or physical behavior follows from these
counter totals. Reject a report that calls99 decisions99 instructions or predicts
hardware time from payload writes.
