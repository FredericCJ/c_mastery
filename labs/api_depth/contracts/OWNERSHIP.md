# Required T18 ownership, identity and lifecycle work

Bindings are AX-PR-T18-01-T18.01.b/.02.c; all three T18.03 criteria;
T18.04.b/c; and all three T18.05 criteria, with their original DELAYED variants.
The callback timeline .04a is completed by the callback package. Families
A/B/C/D/F/G/I/J; C17 profileC bounded component, hosted test recorder. Required
artifacts are the learner's byte-owner, view and contract implementations plus
T12 failure semantics. Implement ownership.c against ownership.h; import the
actual f_view/f_view_read/f_append prerequisite instead of copying its code.
Four resource kinds below are synthetic memory/descriptor/lock/session tokens,
not a claim that operating-system resources have identical failure behavior.

AcLedger stores four ownership bits, operation keys, acquisition/release counts
and a64-event audit. All control storage stays live, access is serialized and
counters have caller-guaranteed headroom. Successful acquire records one event
and creates one release obligation. Failure before acquire creates none. A lost
acknowledgment returns BACKEND after acquisition; query(resource,key) determines
whether this still-live operation owns the token. Repeating the same acquire
while held is idempotent; a different key is FULL. The protocol does not promise
exactly-once behavior after control loss, key reuse or release/reacquire. Explain
the extra persistent identity/reconciliation mechanism those changes would need.

Close has an explicit per-resource consume-on-error policy. A retaining failure
leaves the token held; a consuming failure reports BACKEND but clears ownership
and records its release. Success consumes. A duplicate close is INVALID.
ac_close_all processes resources3..0, preserves the first failure and returns
the exact retained mask; retry only retained obligations. Audit space is reserved
before the aggregate cleanup, so logging exhaustion does not silently lose
ownership records. Compare this state table with the inspected SQLite finalize
contract: an error return does not authorize reuse of the finalized statement.
Do not infer a generic close-retry rule from an error code alone.

Draw an owned buffer, two non-owning views, a logical move and a weak token.
Raw C assignment copies bits without clearing the earlier owner's release
obligation; the logical move must do that explicitly. The small AcShared control
uses strong counts1..3, rejects overflow, keeps weak control storage live and
releases the payload once at zero. A parent link owns one child reference. Prove
release for an acyclic tree, then build a two-object cycle, drop outside references
and observe that both remain live. Break one owned edge explicitly and release
the corresponding reference to reclaim the cycle. This is not a tracing collector
or language-enforced ownership type. Control lifetime, graph integrity and
concurrent access remain programmer obligations. Cost is O(number of released
edges) with recursion proportional to released graph depth; the laboratory graph
has two nodes, not an unbounded-stack guarantee.

The two-slot AcTable issues generations1,2,3 and reserves0. Allocation increments
the chosen inactive slot's generation; exhausted slots retire forever. Free
does not reset identity. A token must match a live slot and generation before
value access. Enumerate all six sequential allocate/free/reuse opportunities,
checking every old token after each allocation and unchanged outputs on rejection.
The registry validates an index/generation in its own live control storage. It
cannot make a dangling table pointer valid, prevent a race with reclamation,
or enforce a borrow after returning a direct pointer. Keep an owner lease or
lock through final use; a check followed by unsynchronized free is not protected.

For the changed lease, use an8-bit clock with at most127 ticks between relevant
observations, a maximum valid token age3 and reuse only after age>3 while the old
owner is inactive. Validate the issued timestamp and age before reading. The
fixture enumerates32,768 issue/age pairs, including counter wrap. The application
must never use retained tokens beyond their promised lifetime; after a full256
ticks an ancient timestamp can repeat. A reclamation scheme permitting such
retention needs additional identity or a quiescence protocol, not this check.
Explain why finite generation retirement and bounded-time reuse have different
premises and resource costs.

The ordinary imported FView policy retains views after a failed append and an
empty append, and invalidates them after successful structural append. The
changed AcEpochOwner separates structural generation from value revision.
Nonmoving value update increments the value revision; a live structural view
observes the new value while an earlier value snapshot becomes stale. Reset
increments structural generation and clears length. A logically destroyed owner
rejects snapshots while its control object remains alive. Exhaustion rejects
before mutation. This does not authorize accessing actual dead storage. Compare
with SQLite's inspected conversion/step/reset/finalize invalidation rules rather
than assuming every owner retains every borrow until close.

The lifecycle has NEW,PARTIAL,ACTIVE,FAILED,STOPPED,DESTROYED states in initialized
caller storage. NEW has mask0; PARTIAL has resource0 only; ACTIVE has mask3,
or mask1 when the second resource is explicitly optional. Other states have
mask0. Constructor failure points are before resource0, after resource0, before
resource1 and after resource1. Failed construction rolls back exactly its
acquired prefix and publishes FAILED/mask0; this is its stated failure state,
not a promise that every object byte is unchanged. An active tag with mask0 is
rejected. Stop accepts PARTIAL/ACTIVE/FAILED, releases the prefix and advances
epoch; STOPPED stop is idempotent. Destroy accepts only STOPPED. Init accepts
only NEW; restart accepts only STOPPED and preserves its already advanced epoch
while obtaining fresh resources. A terminal epoch31 cannot restart/acquire
another session that could not later stop.

Generate the complete six-state/four-operation table for init,stop,destroy,restart.
Rejected transitions preserve state/mask and event counts. Calls after actual
storage lifetime ends are reasoning cases, never executable expected failures.
Moving the state tag into a stable handle table makes control independently
available, but still needs a lease/quiescence rule before reclaiming the object
it names. Old registrations/views are canceled or invalidated before restart;
reusing a numerical slot is not permission to revive them.

Deliver state/resource/ownership diagrams, working source, exact ten constructor
paths and24 operation cells, mixed-close traces, stale-token histories, split-epoch
tests and the cycle counterexample. Six native compiler profiles must pass;
compiled missed-release, consuming-retained-error and generation-check mutants
must fail the intended assertions. Storage is bounded; the audit can fill and
reports FULL. No measurements of real devices or physical interrupts are inferred.

Rubric:0 double release/stale access or unsupported lifetime premise;1 nominal
counts only;2 derives each ordinary invariant and validates distinct failures;
3 independently handles the original lost-ack, fallible-close, lease, optional
resource and external-control changes after7days. Hints: write the owner after
each error; separate operation result from resource state; locate the last borrow;
try a cycle before assuming reference counts prove collection. Actual learner,
external review and physical evidence remain separate from reference execution.
