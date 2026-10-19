# Owned-resource snapshot assessment

R1: Logical copies do not need independent backing until one is mutated. The
actual scalar counterexample shows why exposing a mutable alias invalidates
that abstraction even in defined single-threaded C. Rust Cow's borrowed/owned
choice is a comparison, not a claim that this mutex/refcount protocol has the
same type enforcement. Require the learner to identify every raw-pointer and
callback ownership premise.

R2: A block's count equals its one current owner, if any, plus active loan slots
pointing to it. Selection+retain and decrement+reclamation happen under one
store mutex. A private stage is unpublished and owns exactly the initialized
resource prefix, including the failing callback's valid partial output. Reverse
drop of all three positions is safe because unused entries are empty. The old
count/pointer are untouched until complete cloning and element update succeed.
The unique case needs no allocation; the count cannot be1 if an active reader
owns that block. Callbacks under the mutex must not reenter or invert lock order.

R3: Check early raw failure, allocation failures before each element, and errors
after each element acquired storage. Merely testing malloc failure before any
element misses the partial-owner obligation. The observed old and current
values must match after refusal, and real allocator live counts must return to
the pre-call state. Force stale token reuse and UINT_MAX retirement while
quiescent. The scalar oracle uses copied logical lists independently of C
representation and a separate sharing ledger for allocation predictions.

R4: An unprotected pointer load followed by atomic retain can race final release;
making just refs atomic is insufficient. This adapter uses one mutex around
selection and retain, mutation, read and release. Existing old snapshots remain
immutable because any writer with refs>1 detaches. Close removes the current
owner while loans keep blocks and the mutex-bearing store live. Destroy needs
external joined/quiescent callers; the loan count cannot certify this by itself.
Finite pool pressure can refuse writes, so do not require a nondeterministic
success distribution. Under retained initial and current versions, a third
three-resource version exceeds the8block allocator and must roll back safely.

Cost review separates the no-allocation unique path, full three-resource detach,
retained generations, callbacks and synchronization. No lock-free, fairness,
bounded latency, arbitrary corrupt-object or hidden native pointer-safety claim
is justified. The test cleanup ledger prevents leak diagnostics from replacing
an intended mutant assertion; it is never acceptance evidence for successful
resource release. Review exact counters and source-bound logs. No author
execution stands in for an independent delayed learner assessment.
