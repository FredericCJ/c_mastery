# Assessor-only T46.07/08 technical analysis

Do not export this file, collection.c or oracle files with learner packages.
Grade the original criterion and delayed variant separately. Require the source
argument and typed state trace even when all executable checks pass. These are
bounded author examples, not a certificate of independent mastery.

<a id="07a"></a>
## T46.07.a — three levels of identity

Ordinary reuse changes generation while retaining the slot. At the final
generation, reclaiming a slot does not make increment safe: uint32 wrapping is
defined C arithmetic but destroys the intended identity guarantee. Retirement
trades eventual capacity for non-aliasing. The original pool similarly retires
ticket slots on release at the last generation. Distinguish heap and lease
generations; they identify different resources.

Reset deliberately erases the original VM generations. The two successful
allocations around reset both yield slot0/generation1; this is a concrete
counterexample to raw-handle cross-reset safety. VgRef's owner+epoch rejects it.
The controller must be immovable and live; an address/epoch combination does not
survive destruction and placement of another native object at that address.
Epoch saturation rejects reset. Native callers may forge fields: no C language
protection is gained. Variant1 omits epoch validation and must compile yet fail
the first old-reference access after reset. A solution that compares only slot
or that changes a scalar output on failure earns at most level1.

Changed task: administer the unchanged `...07.a-DELAYED` after at least seven
days. Give two reset epochs with identical raw handles and require the learner
to derive an instance identity without this implementation. Accept an external
nonwrapping identity registry with equivalent lifetime assumptions; reject a
random token presented as collision-free proof. No author run counts as that
assessment.

<a id="07b"></a>
## T46.07.b — occupancy, policy and graph transfer

Unrooted arena cells remain occupied. For16 scalar pairs before collection,
occupied=16, reachable=0, reclaimable=16; the last quantity describes what a
collector could reclaim, not what the arena policy permits. The actual stopped
raw VM test changes arena to collection policy and the next scalar allocation
leaves one occupied cell. In the two-child allocating-parent test, two operands
become temporary roots and the result has three occupied cells instead. Do not
confuse those experiments.

The transfer fixture has y→x twice and x.right→y. Destination slot0 is occupied
and slot1 retired, so y maps to2 and x to3. The map is installed after placeholders
exist, preserving both aliases and the cycle. All seven cuts (2 allocations,
4 rewrites, publication) preserve the destination's semantic fields and output91.
Variant6 publishes the staged VM before cut7 and fails that comparison. Source
reset invalidates its old references while the copied graph stays valid.

For scalar input R=0, no heap allocation is needed and cut1 is still the
publication cut. For insufficient space, no staged partial graph escapes. A
valid alternative uses a bounded undo journal with the same failure contract;
it must restore generations and allocation counters too. Directly translating
source slots, copying only one edge, or recursively copying without a visited
map fails this objective. The full VM temporary has a real stack cost and the
target audit must include it. Seven-day variant uses a different cyclic survivor
graph and a partially occupied destination; do not replace it with another
constant-sized scalar example.

<a id="07c"></a>
## T46.07.c — host possession does not publish a root

The original collector scans explicit roots, operand stack and initialized frame
slots, not native locals. At full heap the helper's scalar allocation reclaims
the unrooted host result and its old generation rejects access. The repaired
trace pushes that result before entering the helper and pops only after parent
publication. Root ownership must name who balances the push on each return.
Changing a stopped arena to GC preserves identities but introduces future
reclamation; reset destroys all identities. Neither transition implicitly
registers old native handles.

Actual ORVM registrations point at controller-owned ORVDerived subobjects whose
lifetimes exceed leases. algo_pool tickets are separate from array occupancy.
Two clients can own distinct leases for the same value. Releasing either leaves
the other root; cross-client, duplicate and stale-epoch releases refuse. The
last-generation ticket is retired. Reset refuses live leases rather than
silently invalidating an unrelated client's ownership record. A refcount on the
pair alone is not equivalent because cycles and scope ownership differ.

Delayed package: hand out two embedding clients with separately failing cleanup
paths. Require lease acquisition/publication/release tables and show that one
client cannot consume the other's lease. The reference has serialized calls;
claiming thread safety without synchronization is incorrect. Object creation
and destruction of the native controller remain caller obligations.

<a id="08a"></a>
## T46.08.a — exact roots, finite graphs and weak processing

Original marking sets marked before enqueueing, so aliases/cycles enqueue each
of16 objects at most once. The worklist capacity is16 even though edges number32
and roots can repeat. The independent oracle enumerates all simple vertex paths
of length1..3 on every three-node/two-field graph:4^6 graphs times8 root subsets
=32768 cases. Every reachable vertex has a simple path; a cycle is unnecessary
to witness reachability. This is independent of the implementation's marker or
ORGraph gray search. The runner maps bits to three actual VM root classes and
checks weak clearing as well as occupancy. It does not cover every16-node graph.

Uninitialized frame fields are not roots even if their bytes contain a valid
handle. The baseline cycle fixture proves that distinction. A weak slot does
not contribute to strong closure; clear it after closure and before reuse.
Weak-to-strong upgrade is serialized and obtains the root lease before exposing
the value. There is no finalizer resurrection or ephemeron promise here. The
actual T74 weak processor receives black objects as strong roots but no strong
edges: recomputing reachability could accidentally repair a missing barrier and
make its regression vacuous. Variant4 turns a weak reference into a strong one
and is rejected by the exact dead mask.

After seven days give a weak-only cycle and one independently strong root.
Require a separate weak pass and refusal of an upgrade after reclamation.
Conservative scanning is a different permitted design only with an explicit
changed contract and retention analysis; it does not satisfy this exact-root
assessment by claiming all integer-looking values are pointers.

<a id="08b"></a>
## T46.08.b — allocation boundaries and temporary ownership

On the actual original CVM, full-heap pair allocation pushes both inputs before
collection. With seven existing roots the first temporary push succeeds and
the second refuses at capacity8. It must pop the first, restore root_count7 and
leave the parent output91 and heap unchanged. This is checked separately from
the Vg adapter's two extra temporary slots. Normal original pair construction
also proves both operands survive; the successor uses the actual incremental
components and its own precise temporary root source.

Variant5 omits that temporary source; collection reclaims the child and the
subsequent actual cvm_pair rejects the stale operand. This is a semantic failure,
not a mutant that fails compilation. The changed helper is a real allocation
between evaluation of left and right: root left before calling the helper, then
construct the parent with both values protected. Rooting only inside the final
pair function is too late. The deliberately flawed local trace observes guest
STALE and has no claim of native memory corruption or exploitation.

Delayed assessment gives a helper that may refuse as well as allocate and asks
for every cleanup path. Accept scoped cleanup structs or a disciplined cleanup
label; require exact balance and no escaping output on refusal. A global
permanent root list that merely avoids collection fails the reclamation contract.

<a id="08c"></a>
## T46.08.c — incomplete marking and mutation after READY

The fixed baseline fixture has3 roots and4 live cells. Its exact budget is
3+2×4+16=27. Each initial budget0..26 fails without freeing an object; retry27
starts with cleared marks and succeeds with residual0. Failed collection may
consume budget and alter marks; calling it fully transactional would be false.
Actual old values reject after reclamation and reuse. Guest fuel is irrelevant
to the native loop and does not establish a stop-the-world latency deadline.

For the changed mechanism, shade the target of a black→white store. A root added
after DONE likewise introduces work. The controller preserves the underlying
barrier's active bit until commit, then reopens MARK for either transition.
Variants2/3 remove the store/root barrier and fail distinct live/dead checks.
Allocation is forbidden during this cycle, bounding objects to16. Removing roots
can retain floating garbage until the next cycle, which is safe but must be
reported. The adapter's inspection/scan/commit counters are not the original
root/edge/sweep units; cost comparisons need a new model.

For independent delayed work, give the READY→store→commit schedule without
revealing the repair. Require a color trace, a rejected premature commit, new
scans and preserved child. Snapshot-at-the-beginning barriers are an alternative
only with a complete changed invariant and deletion logging; naming the algorithm
does not implement it. This adapter is incremental but serialized, not concurrent
or real-time. Strong stale references in used cells are invalid input, even if
currently unreachable; the stricter validation is explicit.

## Common discrimination and evidence

Rubric2 requires all named ordinary relations and causal reasoning; rubric3
requires the original changed task plus a novel counterexample and revised cost
and ownership argument. Grade each of the six objectives independently. The
public scaffold must compile and fail behavior, and all six selected mutants
must compile before their failures count. Interpret source stability, raw logs,
object imports, target closure and independent oracle together. Sanitizer success
does not prove absence of UB; finite enumeration does not cover arbitrary heaps.
M4/PPC64 emulation is not a board measurement; parallel authorship is not review.

R-VG-ORVM-ENUM-2026-01 records a target compilation defect in a retained provider:
GCC Arm chooses an unsigned CvmTag and diagnoses `tag<0` as always false. The
successor port changes the exact guard to `(unsigned)tag>3`, preserving valid
and invalid tag classification on the recorded profiles. It neither changes
the provider nor suppresses warnings. The builder pins original/generated files
and refuses an unexpected source. Native original-provider and target successor
behavior are recorded separately. No binary layout equivalence across arbitrary
compiler enum options is inferred.
