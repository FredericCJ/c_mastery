# Assessor: T67.05 relocation and copied access

The original owner pointer distinguishes simultaneous live pools but not reuse
of the same address after its lifetime. It must remain fixed/live with no old
handles at reset. The successor moves that identity premise to an externally
nonreused uint64 ID. This removes the address requirement for the logical pool,
but not the lifetime requirement for the particular shell passed to a function.

Every successful get reads and copies while the same mutex protects release,
reuse and block replacement. A reader cannot observe validated old generation
and a later incarnation's non-atomic payload: those two accesses occur in one
critical section. The teaching atomic handshake deliberately places them on
opposite sides of reuse and obtains999; even sequentially consistent atomics
do not make separated validation and access an indivisible reservation.

Compaction acquires new storage before releasing the old storage. A real malloc
provider cannot alias the still-live old block, so this exercises payload move,
not just identity-preserving assignment at the same allocation. No client raw
address survives. Shell move is quiescent, initializes a separate mutex and
transfers only ordinary identity/block/callback fields. The old shell still
needs mutex destruction; freeing its block again would duplicate ownership.
Do not suggest pthread_mutex_trylock or a successful lock proves no future
waiter can arrive. Admission/lifetime must be closed by the caller's join rule.

At MAX-1, release gives FREE/MAX; one final acquisition/release retires it. A
fully retired pool returns FULL permanently. At ordinary generation1, release
must yield2, not3: skipping generations violates the stated transition contract
even though it can remain stale-handle safe. Require learners to distinguish
contract correctness from a narrower safety invariant. Identities also exhaust
without wrapping, and restarting an issuer can collide despite mutex safety.

The independent Python model uses occupancy/generation records for both the
retained pool and successor. Its finite traces do not enumerate pthread orders.
Native readers accept only old123, STALE or CLOSED, preserve refusal outputs,
and join before destroy; successful sanitizer runs remain sampled executions.
Ten controls fail ordinary assertions. The allocation registry cleans leaked
teaching controls only after semantic assertion exit, never altering expected
live/free counters or laundering a resource leak as successful cleanup.

The initial eight-profile attempt exposed a harness bug: a mutant's failed
first-reader check returned before joining the remaining readers. Its raw GCC
control crashed and thread instrumentation reported unfinished threads. That
attempt is retained as failed evidence. The corrected harness joins every
created reader before inspecting semantic failures, and the final campaign
must observe ordinary assertions for every control. The relocation case also
joins old-shell readers before move and restarts3000 copied reads on the new
shell using the same handles, with MAX-1 generation and allocation refusal.

The learner's changed-condition repair must move after source quiescence and
restart readers on the destination. Old handles route successfully because ID
and slot generation survive; using the old shell must return CLOSED. A caller
that reuses numericID1 while oldID1 handles survive violates the issuer contract,
even if all slots and mutexes are freshly initialized. A durable nonreuse issuer
or a separate lifetime namespace is needed; this module intentionally supplies
neither and cannot claim unforgeability or cryptographic authority.

For the C probe, a distinct ADNodeIndex cannot be implicitly passed as ADBrand.
A reconstructed ADBrand with the same public fields is legal and not a new
owner. Rust PhantomData type parameters can reject the named B marker where A
is required, but a second value with markerA is still constructible. Zero size
of PhantomData does not imply an entire handle is zero sized or a runtime
generation check is erased. Compiler versions and options bound the observed
diagnostics. No Rust generative-lifetime API is demonstrated by that simple
category comparison.

Use POOL.md's per-objective rubric and thirteen cost dimensions. Record original
ADPool's weaker concurrency contract beside timing comparisons. Mutex internal
resource costs, all interleavings, constructor mutex-resource exhaustion,
physical target effects, independent learner performance and non-author review
remain outside these author executions. POSIX current-origin access gaps remain
explicit; the identified2017 reprint is not evidence of2024 edition inspection.
