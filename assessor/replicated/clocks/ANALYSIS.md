# Clock identity and publication assessment

The representation retains identity tags instead of assigning meaning to a
reused array index. Max-merging components is therefore invariant under layout
changes. Adding a zero component is knowledge of a new writer, not an event;
the first new-incarnation event increments it while preserving the old causal
past. Resetting an old slot produces the concrete equality counterexample in
the public identity test. Retention is bounded at six identities and refuses a
seventh; deleting old components without a stability proof is not a repair.

The independent oracle constructs predecessor edges from each process's prior
event and each actual selected send event, then computes ancestor sets. It
compares vector order against those sets rather than deducing causality from
clock values. Scalar/HLC order is checked only in the forward implication;
noncausal pairs with increasing timestamps are retained counterexamples.
Canonical wire bytes are independently assembled with Python integers and
zlib CRC32. The C codec does not serve as its own byte oracle.

For the bounded wall model, every causal predecessor occurs at no later true
time. Its sampled physical value is at most current true time plus2. The HLC
physical component is the maximum of those samples and the current sample;
thus it lies between current true time minus2 and plus2, and is at most4 ahead
of the current wall. This is a proof under explicit assumptions, not a claim
about arbitrary NTP behavior. The runtime ahead check rejects an unsupported
sample; it does not discover whether remote history or clocks are trustworthy.

Preflight scalar/vector resources before deciding whether HLC exhaustion is
temporarily waitable. A representable later physical sample resets the logical
counter, but cannot repair a full vector or exhausted scalar counter. At maximum
physical value no later sample exists. WAIT retains exactly the old state and
requires the caller to schedule a retry; it provides no finite liveness bound
if wall time stops. Wrapping the logical counter would break lexicographic order.

Durable wrappers compare the supplied local snapshot with current stored bytes
before staging a transition. This rejects stale values even if a caller copied
an old well-formed object. A constructor/restore primitive remains privileged:
using rpc_save to replace emitted history is outside the monotonic publication
contract and must be identified as such by the learner. Every normal step and
incarnation replacement commits all three actual page-store records before its
output is written. On uncertain failure, no result escapes, but recovery may
find either old or new authority. If the latter is present, a retry advances
from it, leaving a harmless unused timestamp gap.

The148 cut cases cover event and replacement operations,37 page callbacks and
two persistence outcomes. Their independent committed-new predicate is true
only when the complete commit-page write or final flush persists. Other byte
tears belong to the lower durable-store campaign; this test does not assert
exhaustion of arbitrary sector schedules. The active transaction test ensures
the snapshot helper does not abort somebody else's admitted transaction.

Controls1/2 merge by array position or erase the retired component;3 ignores
the checksum;4 publishes before commit;5 ignores the wall-admission bound;
6 resets an exhausted logical counter. Each should fail a particular semantic
assertion with normal exit status1. Distinguish a valid counterexample from a
sanitizer failure or malformed-pointer call outside the API contract.

For an unseen delayed assessment, keep a message from the old incarnation,
replace the writer durably, then deliver that old message and emit a new event.
Require an independent ancestor graph, retained old component, changed self
component, stale-snapshot refusal and a resource-bound explanation. Global
fresh identity allocation, authenticated membership and reclamation of retired
components require separate protocol assumptions. Passing a fixed set of runs
does not establish those assumptions or independent learner mastery.
