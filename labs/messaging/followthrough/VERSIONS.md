# Owned versions and predicate validation — X-T73-008

This required successor implements X-T73-008-R4 and supports the visibility,
pin, reclamation and failure arguments in R1–R3. It supplies implementation and
changed-condition evidence; an unseen delayed assessment remains a separate
learner event. Implement `src/versions_follow.c` using `versions.h`, the completed
`struct msg_versions`/`struct msg_store` cores, and the retained-owner primitives in `LIFETIME.md`.
Do not substitute a database, transaction library or reference-counted container.
Use ISO C17 plus the explicit POSIX mutex interface on hosted Linux. Compiler
sanitizers and Python models are host verification tools, not mechanism imports.

## Ownership, operations and failure contract

`struct mfv_store` maps each live core version slot to one independently owned immutable
payload. `mfv_init` consumes the initial unique `struct mfo_value` only on success. The
store starts at timestamp0; its control storage must not already be initialized.
A failed constructor leaves the initial owner intact and no initialized store.
The same rule applies if allocating the shared-owner control fails. Store/output
objects are disjoint, live, correctly typed storage; an output owner slot owns
nothing before a call, although its bits may be unspecified until success.

`mfv_snapshot` pins the current version using the actual core's slot/generation
identity. `mfv_read` validates that identity, chooses the version visible at the
pin's timestamp, retains its owner while acquisition is protected, and only then
publishes a new shared-owner token. A failed read leaves output unchanged. The
returned token can outlive its pin and store; drop it exactly once. Its allocator
context and destructor dependencies must remain live until the last token drops.
A retained descriptor does not authorize mutation of the underlying payload.

`mfv_publish` invokes the caller's clone with a borrowed source object and an
initially empty temporary `struct mfo_value`. The source and callback context remain live
and unchanged until return. On `MSG_OK`, clone supplies a fully constructed unique
owner and a valid destructor; on any failure status it destroys its own partial
construction and leaves the temporary empty. The wrapper propagates that failure.
It constructs a shared-owner control before publication, destroys a successful
clone if this allocation fails, and publishes under the store mutex only after
all construction succeeds. A clock-exhausted publication releases the candidate
and preserves existing visibility. Observable allocation/failure counters may
change; caller output and old payloads do not. Clone and ownership callbacks
return normally and never reenter or destroy the participating store/control.
Allocator callbacks follow `LIFETIME.md`, including concurrent-call requirements.

The wrapper must detach any payload occupying the newly reused version slot,
even when the core has already made that slot live again. It also releases other
reclaimed slots. Payload destruction occurs outside the store mutex. Four active
pins plus current require at most five distinct retained versions; eight core
slots admit another version under the valid representation. A forgotten pin can
retain its version indefinitely. Returned leases and concurrent in-progress
clones are additional ownership, so the five-version bound is not a bound on all
allocated payloads. `mfv_destroy` returns `MSG_BUSY` without destruction while any
pin is active. Otherwise an external admission gate must exclude all concurrent
store calls, including callbacks still constructing candidates. It releases the
store's owners and mutexes; external leases remain valid. No call, including a
second destroy, may use the destroyed store before reinitialization. Never access
or mutate the embedded core independently while the wrapper is live.

## Concrete implementation and experiment

Supply a two-allocation clone: one `Payload` record and a separately allocated
byte buffer, plus the shared control's third allocation. Inject failure at each
allocation in turn. With pinA reading byte4, every failed candidate byte9 leaves
A reading4 and the clock unchanged; all temporary allocations are reclaimed.
Successful publication leaves A reading4 until release. Support an empty payload
without a byte allocation, initial-control allocation failure, four different
pinned versions and fifth-pin refusal, stale generation after reuse, retired pin
slots, clock exhaustion and repeated publication into reclaimed slots. Retain a
read token, release its pin, destroy the store, then verify its original bytes
before dropping the final token. Count successful allocations and deallocations;
never use a stale pointer as the expected-result oracle.

## Independent keys and delayed predicate variant

`struct mfk_store` contains two logical unsigned keys and the actual `struct msg_store` core.
`mfk_begin` takes a consistent two-key value snapshot under an outer mutex and
returns a transaction owned by the invoking thread. The output must not contain
an active transaction. `mfk_read` returns a staged write or captured snapshot and
records its key. `mfk_nonzero` observes both keys, including zero-valued keys, and
returns their nonzero count. `mfk_write` stages one key. Keys outside0–1 and an
inactive transaction are invalid. Reads and failed operations leave caller outputs
unchanged. Transactions and controls cannot be copied after initialization or
used concurrently by multiple threads; different transactions may run concurrently.
The store and snapshot remain live until each transaction is completed or aborted
through the actual core's abort operation. Destroy requires no active transaction
and no admitted concurrent call.

Commit always validates write-key versions against the captured timestamp
(first committer wins for overlapping writes). With `validate_predicates=false`,
unwritten read dependencies are not checked: this finite model admits snapshot
isolation write skew. With `true`, it also validates every observed key. The entire
predicate universe is the fixed two keys; an open-ended inserted key or range
requires a new representation/validation argument. Commit success publishes all
writes; any conflict or clock exhaustion publishes none, restores acquired core
locks and makes the transaction inactive. The core may leave timestamp gaps on
failed commits. Commit callbacks/external effects are not part of this adapter.

Start from[1,1]. Two actual threads capture the same snapshot and each sees count2;
one stages key0=0 and the other key1=0. With write-only validation both commit and
the result is[0,0], violating the intended at-least-one invariant. With predicate
validation exactly one commits and the other returns `MSG_CONFLICT`; the result
has count1. Also test both writing the same key (second conflicts in both modes),
a zero-valued predicate input changed after observation, read-your-writes,
read-only transactions, an unrelated unobserved key change and clock exhaustion.
This is a finite first-principles comparison, not a PostgreSQL implementation or
an assertion that every snapshot system uses these exact rules. The inspected
PostgreSQL18 sections distinguish snapshot anomalies and predicate dependencies;
ISO C only supplies the low-level language semantics, not transaction isolation.

## Verification and assessment

Run `messaging.versions_owned` and `messaging.versions_predicate`. The separate
Python oracle uses immutable value histories and set-based conflict decisions,
not the C slot-selection or lock algorithm:3,910 version traces and1,296 paired
transaction cases. The public tests also coordinate two real pthread bodies;
finite histories and TSan do not establish all schedules or fairness. Four
compiled semantic controls omit reused-slot release, omit reader retention,
ignore predicate reads, or ignore write-key validation. Require the intended
`SC_CHECK` assertion, not a compilation failure or a memory-error symptom.

Submit D1 ownership/visibility and conflict proofs, D2 implementation, D3 exact
allocation/operation traces and independent model results, and D4 a delayed
changed-condition attempt plus all thirteen modern cost/enforcement dimensions.
Explain O(payload-size) cloning, constant bounded scans, retained bytes per pin
and lease, mutex acquisition and callback costs, lack of fairness bounds, and how
changing the key universe or callback model invalidates arguments. Compare an
immutable shared-owner language mechanism: ISO C does not enforce affine tokens,
deep immutability, callback purity or admission shutdown. Tests and runtime checks
cover particular violations; other obligations remain programmer discipline.

Level1 is nominal visibility alone. Level2 requires every failure, lifetime,
reused-slot and conflict invariant with matching evidence. Level3 additionally
reconstructs the changed predicate universe or independently chosen ownership
variant without the worked implementation, and states the missing enforcement.
Hints: (1) a slot's live bit does not identify its former payload; (2) acquire a
reference before dropping the lock that protects acquisition; (3) validating only
written keys cannot detect every decision based on other keys. Assessor analysis
and implementations remain separately packaged.

Use original local targets and synthetic bytes only. Retention failure can cause
a resource leak or invalid lifetime; neither demonstrates arbitrary code
execution. Authored support is executable; actual execution status is exclusively
in a source-bound campaign. No independent-human, learner-mastery, hardware or
database execution claim is made.
