# T19 pure transitions, effects and replaceable strategies

This required PR-T19-03 packet implements T19.06.a/b/c and their original changed
tasks. Use the original AX identifiers and seven-day assessment delay; families
A/B/C/F/G/H/I/J. Prerequisites are T18 ownership/failure contracts and T19 exact
function-table/session reasoning. The tests use C17 hosted GCC/Clang O0/O2 and
ASan/UBSan. No operating-system effect, deployed service or physical target is
implied by the synthetic counter backend.

## Return an effect request before executing it

`AmMachine` contains phase IDLE/READY/WAIT/FAILED and sequence0..7. IDLE requires
sequence0; WAIT/FAILED require nonzero identity. READY may retain the identity
of the previously acknowledged request. The reducer takes a machine by value
and returns next state plus either no effect or WRITE(key). It performs no
allocation, I/O, callback or global mutation. Rejection preserves the output.

| Phase | Start | Send | Ack | Timeout | Effect failure | Cancel |
|---|---|---|---|---|---|---|
| IDLE | READY | Reject | Reject | Reject | Reject | Reject |
| READY | Reject | WAIT, new key | Reject | Reject | Reject | Reject |
| WAIT | Reject | Reject | READY | WAIT, same key | FAILED | FAILED |
| FAILED | Reject | Reject | READY | WAIT, same key | Reject | Reject |

Send increments sequence and emits WRITE; sequence7 returns full before increment.
Timeout emits the same key, including after a reported failure. Cancel does not
undo an issued effect: FAILED retains its identity for reconciliation. Ack changes
only phase. The state itself is a one-entry outbox for this fixed payload: phase
WAIT/FAILED plus key reconstructs the outstanding increment request. Variable
payloads would require separately retained owned data; resetting state would lose
that obligation and is not allowed during accepted work.

The adapter executes a WRITE by incrementing a synthetic counter. It may fail
before the effect or lose the acknowledgment after committing. Both are reported
as failure to the reducer; they cannot be distinguished from that status alone.
The selected backend retains the last committed key and its counter value. A
duplicate key returns success without incrementing again. Only the next key may
commit. Test nominal completion, failure-before-effect/retry and lost-ack/retry:
all result in one committed increment; the latter two have two attempts. Preserve
the identity throughout. A new identity on retry or unconditional repeat increment
must fail. Limit attempts to31 and keys to7, with explicit rejection at exhaustion.
This bounded in-memory idempotency record supplies neither crash durability nor
unbounded replay protection. No universal exactly-once delivery claim is permitted.

## Three representations of the same relation

Implement switch-based, indexed-table and exact-function dispatch. They may share
the state/effect construction routine but must implement different dispatch
mechanisms. Validate state/event/identity before any array access. Compare all
256 combinations of four state tags, eight sequence values and events-1..6 with
the independent published relation, including malformed state combinations and
output preservation. Check full next-state/effect values, not merely status.
Retain O0/O2 object code, relocations, section sizes and symbols under both
compilers. Identify whether each compiled form contains branches, indexed data,
direct calls or indirect calls; source syntax does not determine final machine
dispatch. Shared effect construction is verified separately by the effect histories.

The changed state-set variant uses external IDs11,29,71,101. Map through a bounded
four-entry lookup before using the dense table. Reject70 without indexing. Compare
four comparisons in the worst lookup case with the dense range check, including
identity validity. Sparse numeric values are not array indices. More states could
justify another representation, but this experiment makes no universal speed claim.

## Runtime strategy needs a lifetime protocol

Decide between direct/static selection and a table using explicit variation time,
instance count, table/context bytes, indirect calls, ownership and test obligations.
A single fixed device backend is a counterexample to mandatory runtime dispatch;
per-instance live backend replacement is a counterexample to a compile-time-only
choice. Use the actual size/code observations to support the decision.

For replacement, `AmHot` owns one current strategy and at most one retired strategy.
A lease borrows exact function/context/epoch from the current version and increments
its in-flight count, bounded at3. Replacing publishes a higher nonzero epoch; with
old leases, transfer the old strategy into the retired slot. Otherwise release it
immediately. A second replacement while that slot is occupied rejects unchanged
and transfers no incoming ownership. Old leases continue calling their original
context. Last retired release consumes that context exactly once. Close refuses
while leases exist, then consumes current context; repeated close is inert.
Closed storage cannot be reopened by replacement.

All operations are serialized; lease objects are linear by contract and may not
be copied, forged, used after release or passed to another owner. Strategy contexts
are distinct owned resources with infallible, nonreentrant release hooks. The C
struct representation does not statically enforce these conditions. Test three
backend contexts, old and new operations in flight, rejected second replacement,
old-version release, duplicate lease release and the three-lease bound.

This exercise's backends have separate effect namespaces: identity is
(strategy epoch,request key), and an old operation stays bound to its leased
namespace. Replacing a backend does not migrate an in-flight reducer's state or
dedupe history into the new context. A service promising one continuous external
state would additionally need state transfer, shared durable identity or a drain
barrier. Include that counterexample in the decision record. Ownership-safe
replacement alone cannot establish semantic continuity for arbitrary backends.

Deliver the reducer variants, relation oracle, effect adapter and failure histories,
outbox/idempotency proof, code-cost annotations, hot-strategy implementation and
decision record. Hints: write requested versus committed as separate columns;
keep the retry identity; retain old context until its last lease. Per leaf,0 means
incorrect,1 nominal replay only,2 ordinary contract and discriminating evidence,
3 independently derives the changed identity/sparse/hot-replacement requirements.
Administer original changed tasks after at least seven days with private solutions
withheld. Current source-bound receipts are author evidence only.

The T19 source register records original task and C source inspection. Pure reducer
semantics and the bounded backend are original exercise contracts. Language typing,
runtime admission, external synchronization and programmer ownership discipline
must be distinguished. Finite native/sanitizer runs are not universal proofs or
independent learner performance.
