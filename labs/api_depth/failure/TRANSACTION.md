# T21.05: commit, publication and ambiguous effects

Implement `transaction.c` against `transaction.h`; `transaction_tests.c` is public.
The three original T21.05 criteria and their delayed packets keep their IDs.
Prerequisites: T18 output preservation, T20 effect boundaries, TAXONOMY.md and
CONSTRUCTION.md. The bounded core uses only caller storage and C17 primitives.
The changed concurrent port in `publication.h/c` is hosted Linux/POSIX and requires
the mutex, publication and lifetime parts of T51 before independent assessment.
This dependency does not make the ordinary transaction depend on T51.

`struct ae_config` holds at most three values0..9 and a version0..3. Replace a valid
configuration using input length0..3 and a destination capacity0..3. Input may
alias the old configuration. Copy admitted input into staging before mutation.
Fail1 occurs at the validation boundary, fail2 before staging, fail3..n+2 before
individual copies, and failn+3 before commit. Failure0 means uninterrupted.
Malformed input always preserves the old object. Version3 refuses admission;
there is no wrap. Each actual publication advances the version exactly once.

Derive three contracts for old[1,2], proposed[3,4,5], capacity2. Strong failure
preserves[1,2]; basic failure publishes the valid empty configuration; explicitly
partial failure publishes[3,4] and reports accepted2. The status alone cannot tell
a caller which contract it received. Under this selected basic contract, failure
after admission empties the object, even if staging failed before the first copy.
Fail1 preserves all three forms. On failure the partial form publishes only the
successfully staged prefix. Zero-length replacement is a successful empty value.

Observers run at stage boundaries and after publication. In the serialized core,
they may see the complete old or complete new configuration, never a mixed old
version with new contents. An observer must not mutate or recursively update the
object. A struct assignment is a commit point **only under this serialized access
contract**. It is not atomic publication to arbitrary concurrent readers.
An external notification can fail after successful publication: return AMBIGUOUS
with committed=1 and retain the new state. Do not reinterpret that as strong
rollback of the external effect or as authorization for unqualified replay.

The changed hosted port uses immutable heap nodes and a stable owner mutex. A
reader acquires its node reference while holding the same mutex used to replace
the root. It reads immutable contents after unlocking, then releases its lease.
A retired node is freed only on its final reference. At most three admitted nodes
may be alive; a new replacement refuses if all three are held. The root owner
must outlive every lease. Closing drops the root but existing leases still work;
destroy refuses until they are released. Calls starting after destroy are outside
the contract. The writer is single; counters stop at65535 rather than overflow.
An allocation failure leaves the old root. Count the transient attempted allocation
separately from admitted nodes when measuring peak memory.

`struct ae_journal` is an explicit persistent-*state model*, not a filesystem protocol.
IDs1..7 and deltas1..7 use two completion records, value bound63 and effect bound31.
A retained identical request deduplicates; changed content conflicts. FIFO expiry
permits an old ID to cause another effect. An intent is recorded before the effect.
Crash1 leaves intent before effect; crash2 leaves intent after effect; crash3 leaves
completion before acknowledgment. Both unresolved-intent cases refuse replay as
AMBIGUOUS. Without an atomic effect-and-completion store or an external query by
idempotency key, intent alone cannot determine whether replay is safe. The exposed
synthetic effect counter is an oracle, not authority available to a real recovery
client. Never claim these struct writes establish persistence across an OS crash.

Deliver contracts, commit-point diagram, independent stage/failure oracle, exact
thread ownership diagram, bounded cost table and one counterexample to equivalence
with a language-native exception or ownership guarantee. The API enforces selected
runtime bounds; ISO C does not prevent forged leases, duplicated release or races.
Run the public grids under both compilers, optimization/NDEBUG and ASan/UBSan; run
the concurrent port under TSan. Explain why sanitizer silence is bounded evidence.
Hints: distinguish accepted from committed; pin before releasing the publication
lock; compare intent-before-effect with intent-after-effect recovery views.
Level1 reproduces an example; level2 explains and verifies every failure state;
level3 independently derives concurrent reclamation and crash ambiguity. Keep the
original minimum-seven-day assessor packet separate from initial practice.
