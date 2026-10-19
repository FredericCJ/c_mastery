# Acquired storage, initialized values and dependency cleanup

Required X-T66-001/R1-R4 and X-T66-001-DELAYED. Prerequisites are the original
three-resource OwOwner exercise, V4Allocator's typed int32_t storage contract,
checked byte extents and the exact inspected failure-injection artifact. Import
actual ow_construct/ow_destroy and ow_allocator in the reference composition;
learner work supplies the matching declared ownership provider, not assessor code.
The required graph successor is hosted C17, GCC/Clang, no extensions. Python's
set/list oracle and the separately executed Rust comparison are host tools.

## Representation and construction

Keep the original three-array API unchanged and extend its tests with exact
reverse release traces. ow_construct acquires three typed regions in order and
publishes only after all succeed. Every length is positive and at most
SIZE_MAX/sizeof(int32_t), checked before any callback. A late invalid length
must not cause earlier allocations. Destruction is idempotent for the empty
valid owner. A prefix count models this particular sequence, not every graph.

The successor names A=0, B=1, C=2, D=3. A must be initialized before either B or C;
B must be initialized before D. C is not a prerequisite of D. Valid construction
orders are A,B,C,D; A,B,D,C; A,C,B,D. OcfPlan supplies all four IDs exactly once
and one positive representable typed count per ID. Reject every invalid order,
duplicate, count, missing callback or nonempty destination before any effect.
The actual bounded OwAllocator returns at most16cells per allocation from eight
real int32_t[16] objects. A representable count17 is a backend NOMEM result,
not graph or byte-count INVALID. No arbitrary typed cast from a byte pool occurs.

OcfOwner records a pointer/length per ID, acquisition order/count, acquired and
initialized bit sets, the exact allocator and init/destroy callbacks. It is a
live, fixed-address, unique, noncopied control. Before construction it is zero
initialized, or the valid empty result of its own destroy. A corrupted control
is outside the contract; numeric checks cannot establish pointer provenance.
Inputs, output, controls, callback contexts and allocated regions are disjoint.
The plan/context and callbacks live for the operation and through destruction
where retained. Calls are externally serialized; no concurrent access is supplied.

Stage a private owner. After each successful take, record acquired storage before
calling initialize(context,id,storage,count,view). The view contains only prior
initialized nodes with const pointers and lengths; absent nodes are NULL/0.
The initializer may write all of its own storage and read initialized dependency
values. It returns0 only after establishing that node's complete invariant.
Any nonzero result means that node never became constructed. It must already
have undone its own extra transient resources; this API owns only the supplied
typed storage, not undocumented suballocations. The explicit changed case writes
C's first cell then fails. Record C as acquired but not initialized.

After all four initialize successfully, assign the complete staged owner to the
destination. An earlier take failure returns NOMEM; an initializer failure returns
INIT_FAILED. Both release the entire acquired set and preserve the destination.
Initializer side effects on external logs are not rolled back. No automatic
destructor, exception unwind or arbitrary dependency discovery is implied by C.

## Cleanup and callback boundary

Walk actual acquisition order backward. Remove the current node from the
initialized set before its destructor callback, so the callback view names only
remaining initialized dependencies. Invoke destroy exactly once only for a node
whose initialization completed, then release its exact storage once. Uninitialized
acquired storage skips destruction and is still released. Clear consumed pointer,
length and acquired state. The callback cannot retain its borrowed storage or
view beyond return. During destroy it may read its own storage and the still-live
dependency values but cannot acquire new ownership or mutate dependencies.

All allocator/init/destroy callbacks are synchronous, nonreentrant, cannot mutate
the owner/plan, cannot exit nonlocally, and cannot fail release/destruction. The
published output remains untouched throughout construction callbacks. A destructor
may observe the provided view but may not use it to reenter construction/cleanup.
These are programmer-enforced preconditions, not a capability security boundary.
Fallible destruction requires the later recovery-owner exercise and is not
quietly implemented by discarding a callback error here.

For A,B,D,C with C initialization failure, the exact event sequence is
takeA,initA,takeB,initB,takeD,initD,takeC,initC,releaseC,
destroyD,releaseD,destroyB,releaseB,destroyA,releaseA.
There is no destroyC. A FIFO rollback would destroy A while B/D still depend on
it. Totals alone cannot detect this. Repeated destroy on the resulting empty
owner does no further callback; no counter exhaustion prevents mandatory cleanup.

## Comparison, diagnosis and verification

tests/drop.rs is an actual Rust local-ownership comparison. Each Storage owns a
release event; Value owns Storage and has an additional destructor. Failed C
initialization drops Storage without constructing Value; returning early drops
the already constructed local Values in reverse declaration order. Run both
success and partial cases and compare their exact traces with C. The compiler
and Drop protocol enforce ordinary local ownership cleanup; they do not prove an
arbitrary user dependency graph, undo external effects or supply fallible-drop
recovery. The supplied comparison uses one fixed valid order, not a claim that
all C callback contracts have a language-level equivalent.

D1 derives initialized subset acquired, dependency closure and the reverse
topological proof. D2 implements frontier.h and the original three-resource
provider composition. D3 supplies exact per-node events, poison/live counters,
unchanged publication and the independent902-case set model: all24permutations,
all four take-failure positions, all four initializer failures and both size
admission/backend-capacity boundaries. D4 seals the changed graph/order and C
partial-initialization analysis before an unaided seven-day delayed variant.

Five anchors distinguish retained prefix construction, graph construction,
partially initialized rollback, admission and callback-view lifetime. Nine
compiled controls destroy an uninitialized value, use FIFO cleanup, publish a
partial owner, omit B-before-D, leak failed-initializer storage, omit C's initialized
bit, compile actual OW_MUTANT1/11, or clear initialized state too late. Require
exit1 plus the intended OW_CHECK assertion. A sanitizer abort is not a semantic
control success. Learner stubs compile and must fail the five positive anchors.

R1 assesses the two-state invariant and enforcement comparison; R2 exact provider
composition/transactional publication; R3 all acquisition failures, prevalidation,
reverse traces and repeated cleanup; R4 graph dependency and partial-init transfer.
Nominal reproduction earns1, complete justified bounded evidence2, independent
changed-condition reasoning3. A double release, absent cleanup, published partial
owner or destruction before initialization fails regardless of total score.
Hints progress from distinguishing storage/value, to drawing the dependency DAG,
to writing exact failure traces, to publishing the staged owner only at the end.
The separate assessor analysis is not learner material.

## Cost review (thirteen dimensions)

1. Representation: four pointer/count pairs, order and two masks plus callbacks;
   record actual sizeof, not a serialized layout assumption.
2. Work: four prevalidation visits, up to four acquisitions/initializers and
   four reverse cleanup visits; general N-node validation needs its own graph cost.
3. Space: fixed staged control and four-node callback view; no recursive cleanup.
4. Allocation: actual OwAllocator calls; four successes and four releases, failure
   at take k releases exactly k-1, initializer failure also releases its storage.
5. Traffic: initializer writes owned cells, callback views copy only metadata;
   poison writes are instrumented-backend overhead.
6. Dispatch: real typed allocator and initializer/destructor function pointers.
7. Code size: preserve optimized object/disassembly and complete compiler flags.
8. Stack: preserve emitted .su observations; these are native implementation facts.
9. Timing: seven1000-construction samples with checksum105000 and28000takes/releases;
   compare equivalent successful contracts, never successful versus failed work.
10. Failure: distinguish invalid plan, backend exhaustion and incomplete initialization;
    logs remain effects even though published output is unchanged.
11. Scheduling: external serialization and nonblocking nonreentrant callbacks;
    no multithreaded, cancellation or asynchronous-destruction guarantee.
12. Builds: clean/incremental observations, actual provider imports and Rust
    compiler version/invocation; ordinary header inclusion does not prove ABI reuse.
13. Portability: C17 and exact typed allocator counts; changed pointer/data model
    needs fresh size/stack observations, not renamed compiler evidence.

Only original local allocation/logging fixtures are used. No host setting change,
external target or physical result. Passing finite author campaigns does not
establish independent learner mastery, arbitrary callback safety or human review.
