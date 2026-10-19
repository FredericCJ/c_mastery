# Protocol continuation assessor analysis

## X-T71-001

The owning recovery capability carries both a controller epoch and a DMA ticket.
A reset can make a fresh register look idle while an old bus transfer remains
possible. Releasing on the new status alone is the first mutant; ignoring a
ticket admits an old alias into the next allocation and is the second. Safe Rust
rejects the actual moved-owner use, whereas correct-type C wrapper copying
compiles. This distinction does not permit dereferencing a freed PFDriver. At
epoch exhaustion, drain and free, then retire; failing forever before cleanup is
not a satisfactory ownership policy.

Delayed assessment **X-T71-001-DELAYED**: a reset acknowledgment and a buffered
completion arrive in reverse order while an old capability copy survives. Ask
the learner to identify exactly which observation establishes old bus quiescence
and what must remain alive. Award the original .d criterion only for a new
argument/implementation without copying this solution.

## X-T71-002

Mode and callback form a sum; base fields form a product whose semantic invariant
crosses fields. Replacement must construct and validate a proposal before
mutating the builder. The alignment mutant violates the DMA variant; the early
publication mutant exposes a failed replacement that has changed the old value.
Delayed **X-T71-002-DELAYED** changes a completed field and then switches transfer
mode: preserve the explicit rule that original mode is one-shot or design a new
whole-configuration replacement contract. Merely clearing a presence bit is not
validation.

## X-T71-003

The server effect and ACK are separate events. After a modeled crash, preserving
the deduplication tuple prevents a repeated effect; suppressing retries without
that tuple instead loses progress. The serialized tuple assumption must be
replaced with a genuine durable transaction before any real crash claim. The
cancel mutant admits progress after a terminal cancellation. Delayed
**X-T71-003-DELAYED** removes durable dedup state: require the learner to exhibit
indistinguishable apply/no-apply histories and select an honest at-most-once,
at-least-once or transactional contract, not claim universal exactly-once.

## X-T71-004

Guard false propagates to the parent, unlike an explicit handled no-op. A local
ONLINE transition preserves ONLINE while exiting/reentering its selected leaf;
an external region transition does not. The entry callback can enqueue START but
cannot recursively dispatch it during CONNECT. Compare exact trace/actions, not
only final state. Callback effects after admission have a basic guarantee.
Delayed **X-T71-004-DELAYED** adds a parent entry action with a resource: identify
which local path does not reacquire/release that resource and rederive callback
failure cleanup rather than suppressing an error return.

## X-T71-005

Locking only the ring is insufficient: two consumers could concurrently mutate
the same HSM. The lock covers dispatch plus removal. An unhandled head stays
present until counted administrative discard. Actual pthread executions and
ThreadSanitizer observations bound tested schedules; the Cortex-M4 PRIMASK port
has no NMI, reentrant or SMP coverage. Delayed **X-T71-005-DELAYED** changes one
producer to a higher-priority IRQ and asks for saved-mask, producer lifetime and
bounded-latency arguments; a pthread call in that handler is not an adapter.

## X-T71-006

The header and body children each retain state in the parent frame; they cannot
suspend an ordinary child's automatic stack. Publication transfers the sole
allocation; cancellation of a completed frame must not free the caller's result.
The deadline mutant accepts the final checksum at the excluded boundary. The
cleanup mutant loses an allocation after cancellation. Delayed
**X-T71-006-DELAYED** orders timeout and the third byte differently. Require an
explicit event-order/linearization rule and new tests, not a claim that both
results are interchangeable.

## X-T71-007

Shared immutable topology does not justify shared mutable activation state.
The expanded occurrences for two edges to one leaf have distinct identity.
Guard false aborts the second running occurrence, not the first completed one.
The frame bound rejects exponentially many paths before callbacks. Completed
external effects remain even after guard failure. Delayed
**X-T71-007-DELAYED** replaces expansion by a bounded activation stack and asks
for fresh capacity/abort identity arguments. A shallow copy of node cursors is
not a new ownership representation.

## X-T71-008

The copied queue owns a slot reference, the brand owns message lifetime, and the
driver owns the buffer. Consuming a QUIET message that returns PENDING destroys
the first two ownership obligations but leaves the third. Ignoring idle releases
early; forgetting brand release makes a four-message pool eventually unusable.
The test executes actual producers, but scheduler states and peripheral numbers
remain models. Delayed **X-T71-008-DELAYED** makes the buffer cacheable/noncoherent:
require cache-line-exclusive allocation, clean completion before DMA publication
and actual old-epoch DMA quiescence before reuse. Refer to the separately checked
B06 selected-target adapter, and require independent physical evidence before
asserting real cache/DMA behavior.

For every section, original `.a/.b/.c/.d` criteria assess the explanation, code,
falsification and transfer separately. An author-supplied solution and finite
passing controls cannot satisfy independent learner performance or external
review. Exact source/command evidence is in the scoped receipt; do not infer
unexecuted board, persistence or ABI guarantees from the artifact names.
# Additional retained-construction obligations

The staged builder deliberately fixes construction order while retaining the
original final accepted configuration set. A successful diagnostic fixture
only establishes incompatible argument typing; public structs remain forgeable
and copyable. The macro sum has an outer completed guard and restores its prior
pc on overflow; automatic previous-pc exists only within one invocation, while
count and sum survive in the explicit frame. The two-endpoint session driver
uses actual dual transitions rather than comparing only a hand-written trace.

The final adapter's deadline is a trusted monotonic tick value. Full queue
preserves the armed deadline for retry. An enqueued timeout owns a message slot,
not the DMA buffer, and old-epoch quiescence remains necessary after its RESET.
Timer configuration or timeout alone is never evidence the device stopped.

The actual copied queue lacks a peek/rollback transaction. The active adapter
therefore transfers the physical head into a retained pending owner. Its logical
capacity includes that owner, and neither another consumer nor a failed dispatch
may bypass it. Treating physical dequeue as successful handling is the first
mutation; forgetting the held item in admission permits a fifth logical item.

