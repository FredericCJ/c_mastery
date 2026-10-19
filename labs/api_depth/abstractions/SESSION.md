# T19 objects and exact operation tables

This packet supplies T19.01.a–c and T19.02.a–c, ordinary and original changed
tasks, for PR-T19-01. Retain each original AX identifier and minimum-seven-day
variant. Families A/B/C/D/F/G/H/I apply. Prerequisites: the T18 ownership/API
contract dossier, compatible function types, object representation and allocation
alignment. Reuse the actual `AcLedger` resource provider; its implementation is
an explicit imported reference during author validation, never learner evidence.

Use C17 hosted Linux, strict GCC/Clang warnings, O0/O2 and ASan/UBSan. Public
`session.c` is an intentionally incomplete learner scaffold. Reference bodies
and worked analysis live only in `assessor/api_depth/abstractions`. This scope
does not authorize deployed targets, hardware actions or changes to host policy.

## A state object is more than fields

Implement a session owning resource IDs0 and1 in a live `AcLedger`. The counter
is0..15; NEW, ACTIVE, SUSPENDED and STOPPED are explicit states. NEW and STOPPED
have no resources; construction may internally set ready=1 after resource0 is
acquired, but the state remains NEW and every public operation rejects until the
second resource is acquired. ACTIVE/SUSPENDED require both resources and ready=1.
The session has exclusive access to those ledger resources and all calls are
serialized. A live pointer is a caller precondition; validation cannot establish
arbitrary pointer validity. Do not copy active ownership objects.

| Event | NEW | ACTIVE | SUSPENDED | STOPPED |
|---|---|---|---|---|
| start | Acquire0,1; active count0 | Reject | Reject | Reject |
| increment | Reject | Increment below15, else full | Reject | Reject |
| stop/cancel | Reject | Release1,0; stopped | Release1,0; stopped | Idempotent success |
| suspend | Reject | Retain resources; suspended | Reject | Reject |
| resume | Reject | Reject | Active | Reject |

Rejected events leave session and ledger unchanged. Admission rejects malformed
state/counter/resource relations, not merely out-of-range enums. Model count0..15
for every state/event cell, including representable NEW counts that start resets
to zero. The state table is an independent specification rather than a second
implementation of the branch structure. Enumerate384 cells and retain outcomes.

Before construction reserve space for four ledger events, enough for either
two acquisitions or rollback of acquired resources. Inject failure acquiring
resource1: the first acquisition is released, no active object is published and
the state becomes STOPPED. A construction observer runs after all fields needed
by it are initialized and resource0 exists, but before resource1. It may inspect
the partial state and attempt public operations; all reject. It must not mutate
fields, retain the partial object, release its resources or escape nonlocally.
Adding a callback therefore changes initialization order without relaxing the
active invariant. Prove resource/event-space bounds before introducing that edge.

For ordinary destruction reserve both close events before side effects. Cleanup
uses the selected infallible-consume close mode; a logging-capacity failure must
not silently leak or partially release the object. The opaque destructor returns
failure while retaining its allocation if cleanup cannot be completed. A caller
must resolve the blocked resource/logging condition and retry, not free retained
storage. Null opaque destroy is successful. Successful destruction consumes the
pointer; a second call through that pointer is invalid, unlike repeated stop on
still-live caller storage.

Compare a caller-owned `AsSession` embedded as a genuine typed aggregate with
an allocated incomplete `AsOpaque`. Both use the same state core. A legal client
assignment `plain.count=16` breaks the former invariant and must be rejected by
the next operation. Ordinary field access through the incomplete public type
must be diagnosed. This is representation hiding, not affine ownership, borrow
checking or data-race prevention. Factory failure preserves its output pointer.
Measure actual size/alignment and allocator calls; do not turn a byte array into
a hidden struct merely to claim heap-free embedding.

## Exact tables, context and represented extent

Use `AsRun(void *,AsEvent)` exactly. One backend borrows `AsSession *`; another
borrows a live `AsExtra` containing that session pointer and a bounded invocation
counter. The adapter converts the object pointer to its real type; it does not
cast an incompatible function pointer. Diagnose a `long(int *,int)` assignment
under strict warnings and never invoke it. The extra context must outlive every
call and may not be shared concurrently without its own synchronization.

Three instances share one const descriptor while retaining distinct counters.
Mutating instance1 must not affect instances0/2. Compare this representation with
three copied per-instance tables and with the extra-context strategy. Count table
bytes once for the shared case, instance/context bytes per object, and table bytes
per object in the copied case. `const` is an interface promise about mutation
through that object, not a universal protection against invalid casts or races.

Descriptors are native in-process representations of valid `AsOpsV1/AsOpsV2`
objects. They are **not a wire format**: random bytes cannot supply valid function
pointers. The extent must refer to live readable storage. Copy the header only
after `available>=sizeof(AsHeader)`, validate represented bytes against available,
then copy only the known prefix. Version1 requires run; version2 also represents
a flush slot that may be null. Missing flush returns unsupported; a missing
required run is invalid. Reject unsupported versions and every unknown flag.
Version2 descriptors may grow beyond the known prefix if version/flags preserve
that prefix contract; ignore unknown trailing bytes. A short v2 object must reject
before reading its absent optional slot. These admission checks do not validate
that a function pointer's target has the claimed semantics.

Deliver implementations, lifecycle/resource proof, exact-signature negative
fixture, all state/constructor/descriptor traces, exposed/opaque/embedding
comparison, and measured shared/copied table ledger. Run both compilers in three
profiles; wrong function type and opaque field access are translation controls,
while overflow/early-active mutants must compile and fail semantic checks.

For each leaf: score0 incorrect;1 nominal demonstration without invariant or
ownership proof;2 all ordinary obligations and discriminating evidence;3 derives
the changed contract independently and explains enforcement/cost limits. After
seven days, withhold assessor solutions and add suspension, construction callback,
aggregate embedding, extra-context backend, per-instance strategy and larger
descriptor/unknown flag cases. Preserve actual learner evidence separately.

Hints: first write ownership masks beside states; reserve rollback capacity before
effects; select functions with exact signatures; prove a prefix is present before
reading it. Source register: `research/v5/expansion/legacy_abstractions/resources.json`,
especially actual original T19 tasks/providers and N1570 function/type clauses.
N1570 is a C11 draft, not the published C17 standard. Hanson3.1/3.2 supplies a
historical interface/implementation comparison, not this session's semantics.
Current source-bound receipts distinguish authored work, executed author work,
independent review/learner assessment and physical observation.
