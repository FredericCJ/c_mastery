# T68.03 required effectful policy composition

Retain X-T68-002, the containers.array extent/ownership contract and actual
mp_reduce/mp_reduce_reject/mp_reduce_saturating. The original pure reducer has
no allocation or callbacks. This changed condition explicitly permits allocation
inside a supplied element-acquisition callback; the reduction core still neither
allocates nor uses a library fold. Python supplies an independent mathematical
and effect-sequence oracle. Use hosted ISO C17 and the declared GCC/Clang debug,
release and ASan+UBSan profiles. All operations are externally serialized.

## Exact mechanism and callback boundaries

Implement mpp_dynamic(count,limit,saturate,ops,out), mpp_reject and mpp_saturate.
The latter two generate the same loop with policy fixed0/1 rather than pass an
unchecked runtime flag. Count0..32 and uint32_t values form the finite teaching
profile. Required output and all three callbacks must be nonnull even when count
is zero. Dynamic policy must be0 or1. Invalid admission returns MPP_INVALID with
no callback and unchanged output. Empty valid input returns0 with no effects.
Outputs are live and disjoint from every callback context/input/temporary; callbacks
and context remain live until return. No pointer-validity or concurrent-lifetime
guarantee is inferred from these checks.

For each index i in strictly ascending order:

1. Zero-initialize MppItem{owner,raw} and call acquire(context,i,&item) once.
   Zero means success. A nonzero failure may leave an empty or partially acquired
   valid owner. On either acquisition failure, call release once and return
   MPP_ACQUIRE without projection or later acquisitions.
2. After successful acquisition call project(context,&item,&term) once. Zero
   means a complete uint32_t term. Nonzero means failure; the term is ignored.
   Regardless of projection success, release the item once before any return.
3. Projection failure returns MPP_PROJECT. Otherwise apply the numeric policy:
   rejecting policy returns MPP_RANGE if mathematical sum+term exceeds limit;
   saturating policy clamps to limit. Release has already occurred.

All failures preserve the caller's output, but already executed callback effects
and consumed input are retained. Publication happens only after the whole loop.
The invariant sum<=limit makes limit-sum safe before comparison. A single term
greater than limit follows the same rule, including limit0. Saturation never
skips later acquire/project/release effects: a later acquisition or projection
can still fail the operation and leave output unchanged. Rejection intentionally
stops at the first excessive prefix. These policies are different contracts.

Release is total for empty/partial/full item, cannot fail, consumes the owner and
leaves an empty item. It may record cleanup effects. Acquisition must create a
unique owner on success; projection borrows it and may not release it. Callbacks
must return normally, must not reenter or mutate the core's output/control/item
except their designated output, and must not retain borrowed temporary pointers.
No callback may invalidate the context or another live owner. There is no unwind
support for longjmp/foreign exceptions. The harness uses actual malloc/free and
a separate emergency registry solely after a failed mutant assertion; production
cleanup counters are not repaired by that registry.

Concrete trace: values7,5,9, limit10, reject gives effects A0,P0,R0,A1,P1,R1 then
RANGE and sentinel99. Saturate gives all nine effects and output10. If projection2
fails after saturation, all three releases still occur and output remains99.
An acquisition failure after allocating item1 gives A0,P0,R0,A1,R1 and ACQUIRE.
The supplied second projection computes raw*3 modulo2^32 before the checked sum;
that projection arithmetic is deliberately a different rule from sum overflow.

## Required artifacts and independent falsification

D1 contrasts result, effect and failure equivalence, and supplies the runtime
versus generated policy interface. D2 proves the prefix/ownership invariant and
every release/publication point. D3 records predictions, the actual retained
reducer and eight compiled semantic controls, all acquisition/projection cuts,
independent ordered traces, code/stack/dependency observations and paired samples.
D4 implements the fallible projection/acquisition changed condition; changing a
callback effect or late failure invalidates an optimization even if final sums
usually coincide. Keep actual policy/client sources rather than a diagram alone.

Four anchors separate callback order, failure cleanup, saturation effects and
the actual retained pure reducer. Controls omit partial cleanup, publish a partial
sum after projection failure, stop on saturation, reverse acquisition order,
skip saturated projections, duplicate projection, and restore original accidental
saturation/wrong-empty-identity faults. They must compile and fail an ordinary
MPO_CHECK assertion in their selected anchor. Signals, compilation failures and
sanitizer findings do not count as semantic rejection. Learner stubs compile and
must fail all four anchors before implementation.

The independent oracle uses an unbounded mathematical sum and an explicit ordered
event list/hash. It checks both generated and dynamic variants, before/after
allocation failures, projection failure at each tested index, zero/maximum values,
limits0/5/UINT32_MAX, two projection rules and seeded32-element cases. It also
checks the actual retained pure reducer on the complete projected value sequence,
separately from the effectful operation's possibly early termination. Outputs,
callback counts, exact event hash and live allocation count are all obligations.

R1 assesses policy equivalence; R2 bounds/ownership/side effects; R3 actual
original+successor falsification and equal-contract measurement; R4 complete
effectful transfer. Level1 is nominal outputs. Level2 requires the full arguments
and every failure effect. Level3 requires an unaided delayed changed case after
seven days, with assistance recorded. For X-T68-003-DELAYED make saturation occur
at index0 and the final acquisition allocate then fail: all earlier projections
still occur, final partial owner is released, no result publishes. Then replace
the projection with an effectful modulo transform and require the revised oracle.
Author implementations cannot supply that learner assessment. Hints: place release
before each failure return; distinguish result rollback from effect rollback;
then test limit0 before attempting short-circuit optimizations. The private
ANALYSIS.md is excluded from learner publication.

## Thirteen costs, language comparison and limits

Representation: one borrowed owner/value item, three function pointers/context,
scalar accumulator and explicit status. Reachable storage: at most one acquired
owner at a time under the callback contract. Lifetime: acquisition to release
within the same iteration; no temporary pointer survives. Mutation/aliasing:
staged output, disjoint caller state, external callback effects remain observable.
Allocation: none in core, one real callback allocation per success/postallocation
failure in this harness. Movement/copying: fixed metadata and projected uint32_t,
no retained element buffer. Destruction: one nonfailing release per attempted
acquisition, whose backend work/blocking is separately bounded by the caller.
Dispatch: three indirect callbacks per admitted element, fixed versus runtime
numeric policy; specialize policy without pretending callback dispatch vanished.
Concurrency: external serialization, no cancellation/reclamation protocol.
Failure: output unchanged, effect prefix/partial cleanup explicit. Compilation:
actual generated definitions, C/Rust const-bool policy comparison and clean versus
unchanged builds. Binary/stack: recorded O0/O3 function sizes, disassembly, helper
and stack observations. Boundary: ISO C typechecking, callback laws by contract,
runtime admission, mathematical proof and finite tests are separate enforcers.

The inspected Rust const-parameter rules admit bool; the actual comparison uses
two const-bool policies with explicit Result behavior. It compares numeric policy
specialization, not this C callback ABI, allocation failure or automatic resource
semantics. No abstract trait proof follows from token substitution or const bool.

Seven10000-round samples per policy compare dynamic and generated forms with the
same four values, same allocation/project/release callbacks and exact40000 calls
of each. Reject uses limit100/checksum100000; saturate uses limit5/checksum50000.
Compare within a policy only; the two policies perform different numeric work.
Record raw CPU time/observer and allocator overhead, optimization and environment;
no speed ranking or universal zero-cost claim is assumed. All security examples
are original local programs and synthetic resources. Lost ownership or wrong
effect order is a harmless property violation, not demonstrated code execution.
Published C17/C23 access, independent learner results, non-author review and
physical behavior remain separate gaps; inspected drafts are not relabeled.
