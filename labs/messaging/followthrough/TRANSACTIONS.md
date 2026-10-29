# Transaction effects, phase faults and privatization — X-T73-007

This required successor concretely supports X-T73-007-R3, R4 and the changed
allocated-node assumptions in X-T73-007-DELAYED. Implement
`src/transactions_follow.c` through `transactions.h`, using completed
`struct msg_store`/`struct msg_txn` begin/read/write primitives, `struct mfl_shared`, and unique `struct mfo_value`
ownership. Use C17 atomics and the declared POSIX mutex, not a transaction or
memory-management library. The fixed-cell core remains a separate required
implementation. The phase engine is an explicit instrumented commit implementation;
it is not a claim that the unchanged core already has a hook API.

## Instrumented commit and failure boundary

`mft_commit_phase` accepts a live active `struct msg_txn` and optional `mft_hook_fn`. It acquires
write-cell locks in key order with nonblocking compare/exchange, reserves a
nonwrapping global timestamp, validates recorded reads, publishes buffered atomic
payloads, then releases every owned lock with the new version. All these accesses
use the stated sequentially consistent C atomic baseline. Failed acquisition or
validation returns `MSG_CONFLICT`; timestamp exhaustion returns `MSG_OVERFLOW`.
Every acquired lock must recover its previous stamp on failure, every buffered
payload remains unpublished, and the transaction becomes inactive. Reserved
timestamps need not be rolled back. Read-only transactions reserve no timestamp.
The supplied engine retains the original core's read-set semantics; the owned
adapter explicitly adds write keys to validation as described below.

Invoke hooks after each acquired lock (`MFT_LOCKED`, key0–3), after clock
reservation (`MFT_CLOCK`, key4), after read validation (`MFT_VALIDATED`, key4),
and immediately before the first payload store (`MFT_PREPUBLISH`, key4). A false
return at these points injects `MSG_FAILED` and performs the same rollback.
Then report each payload store (`MFT_PAYLOAD`, its key), each lock release
(`MFT_UNLOCKED`, its key), and completion (`MFT_FINISHED`, key4). These later
calls observe a commit that must finish; their false returns do not abort or undo
publication. This boundary is essential: a generic rollback claim after arbitrary
payload exposure would be false. Hooks return normally, do not modify the store,
transaction or ownership, and do not reenter the owned adapter. An observer may
inspect atomic state or perform a separate raw read for the supplied phase test;
it must not rely on an uncommitted result. An actual asynchronous thread
termination, process failure or hardware failure inside commit is outside this
normal-return fault model.

For four staged writes, inject refusal after each of four locks and after each
of clock reservation, validation and prepublication: seven distinct rollback
cuts. Verify all payloads and owned stamps unchanged, and a timestamp gap only
if reservation occurred. Supply a positive control in which each later hook
returns false in turn; all writes still commit. At every payload hook, a new raw
transaction reading the still-locked cell must return conflict with its output
unchanged. Force a separate cell already locked to test partial lock unwind and
preserve that other owner's lock. Also test stale read-set validation, overflow,
read-only commit and two native threads performing2,000 total two-cell increments.

## Allocated-node ownership adapter

`struct mft_store` has four current shared payload owners, actual atomic core cells
containing presence flags, an outer mutex, a64-entry in-memory synthetic event
log, and an active-transaction count. `mft_init` initializes empty cells/log with
no heap allocation. Supply a valid `struct mfl_allocator` and lifetime-compatible
callbacks as in `LIFETIME.md`. Typed control storage must not already be live;
store and output objects are disjoint. Never access the embedded core or current
node pointers through another API while this adapter is live.

`mft_begin` publishes a private active transaction and increments admission count;
its output must not already own an active transaction. `mft_read` validates the
actual cell under the outer mutex and, before releasing that protection, retains
the current node if present. It returns a borrowed immutable `struct mfo_value` descriptor,
or NULL for an absent node. A read-your-writes result refers to the staged owner.
The borrow ends when its staged owner is replaced/removed, or the transaction
finishes or aborts. A caller needing a longer lifetime must use a separately
specified owning operation; copying this descriptor or pointer does not retain
it. Admission errors such as a null output or out-of-range key leave the active
transaction and output unchanged. Once a read is admitted, conflict or retention
failure leaves output unchanged, aborts the adapter transaction and releases all
its retained/staged owners. Repeated successful reads must not leak
additional references. Existing retained reads may survive another transaction's
removal until their own completion, but a subsequent transactional read can still
conflict. Stable memory is not a guarantee that an old transaction can commit.

`mft_write` constructs a shared control from a supplied unique owner; failure
leaves that input and current transaction writes unchanged. Success consumes the
input and replaces any earlier staged owner for that key, releasing the replaced
one. `mft_remove` stages absence and releases an earlier staged owner. Neither
operation mutates the current published node. Keys outside0–3 and inactive
transactions are invalid. `mft_log` stages up to four unsigned synthetic events;
a fifth returns `MSG_FULL` without changing prior events. `mft_abort` releases
all retained reads and staged writes, closes the transaction and releases its
admission count; aborting an already closed transaction is a no-op.

Commit holds the outer mutex across preflight, core commit, node replacement and
log append. It first rejects a log that will not fit, without any payload/log
publication. It adds all write keys to validation, calls the instrumented core
commit, and on success transfers staged owners to the current-node slots and
appends staged events in order. No allocation or fallible callback occurs between
successful core commit and these ordinary stores. It detaches old owners and
releases them, as well as retained reads and abandoned writes, outside the mutex.
Every result closes the transaction and decrements admission count. The log is
an in-memory teaching target, not an operating-system write, durable journal,
network message or general rollback mechanism.

The store and allocator contexts remain live for every transaction and retained
owner. One thread owns each transaction, although different transactions may run
concurrently. Do not copy active controls. `mft_destroy` returns `MSG_BUSY` while
transactions are active. Successful destruction requires an external admission
gate preventing all new/concurrent operations; it releases current owners and
the mutex. Reuse requires reinitialization. Payload destructors return normally,
do not mutate participating controls and meet the allocator lifetime contract.

## Retryable bodies and concrete privatization experiment

`mft_run` calls a body with a fresh transaction up to a positive attempt limit.
The body returns normally, respects every operation's failure, and may stage
writes/events or return an error; it must not commit, abort, copy or retain the
transaction itself. A conflict retries after cleanup; another error stops.
The output attempt count records bodies actually invoked, including a failed
attempt. Invalid arguments leave it unchanged. The body may coordinate another
valid transaction to expose a conflict, but cannot replay non-idempotent external
effects as though they rolled back. Successful completion consumes staged owners;
failed attempts release them. No fairness or eventual-success bound is promised.

Start with node0 carrying1. The first body reads1, stages2 and event101; a
competing transaction installs11 before its commit, forcing conflict. The second
body reads11, stages12 and event111 and commits. An intentionally immediate local
counter records two body calls; the synthetic committed log contains only111.
Demonstrate body failure, exhausted retry budget, per-transaction log capacity,
store log capacity and allocation refusal without losing the caller's owner.

For privatization, a reader retains node42 and pauses while a real second thread
removes it and commits. The node is no longer reachable from the store, but its
destructor count stays zero and the reader may still inspect42. Only reader abort
releases the last owner. The bad retention control must fail the destruction-count
assertion before any stale pointer is evaluated. Also replace a staged owner,
remove a staged owner, fail allocation, read absent nodes, and conflict after a
newer publication. All allocations and deallocations must balance at shutdown.
This protocol is a mutex-protected reference acquisition scheme; it does not
provide lock-free reclamation, cyclic collection, arbitrary raw-pointer escape
or a bound on retention by a stalled transaction.

## Verification, transfer and enforcement

Run `messaging.transactions_effects`, `.transactions_privatization` and
`.transactions_phases`. The independent Python history model enumerates all70
order-preserving interleavings of two begin/read/write/commit sequences across
three initial values and sixteen read/write-key combinations:3,360 histories.
It tracks abstract committed values and versions rather than emulating C lock
instructions. Keep actual threaded observations distinct from this sequential
interleaving model and from weak-memory model checking. No claim that this model
explores every atomic instruction schedule is allowed.

Six compiled controls publish a log too early, omit the committed log, omit node
retention, omit aborted-write cleanup, omit acquired-lock rollback, or ignore
prepublication hook rejection. Require their named semantic assertions. Separate
GCC/Clang profiles, ASan+UBSan and TSan expose particular execution defects; they
do not certify arbitrary callbacks or histories. Retain exact commands, source
pins, failed controls and a minimized counterexample for every disagreement.

Submit D1 invariant and visibility arguments; D2 core/adapter and callback
implementation; D3 phase traces, owner counts, independent oracle and native
results; D4 a changed allocation/privatization or effect model with all thirteen
cost/enforcement dimensions. Explain bounded O(4) metadata scans, payload costs,
reference traffic, mutex contention, retained bytes per active transaction,
retry-amplified work and exhaustion. Compare language-level transaction or effect
constructs cautiously: C's type system does not enforce retry purity, unique
ownership, deep immutability or quiescent destruction. The inspected TL2 paper
motivates versioned locking and discusses quiescence; it is not a C17 authorization
to access already reclaimed objects or a proof for this adapter.

Nominal successful transactions earn at most level1. Level2 requires each failure
boundary, ownership and effect invariant with explicit limits. Level3 reconstructs
an unseen node topology/effect mechanism and supplies the counterexample showing
why the old fixed-cell argument no longer suffices. Hints: (1) distinguish a
speculative event from a committed event; (2) acquire retention while acquisition
is protected; (3) identify the last point at which failure can still mean no
publication. Technical solutions and implementations are separately packaged.
Use only original local targets and synthetic data. A resource/lifetime violation
is not evidence of code execution. Actual validation is reported by a matching
source-bound campaign; independent learner and human review remain separate.
