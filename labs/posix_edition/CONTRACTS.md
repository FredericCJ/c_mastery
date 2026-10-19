# POSIX edition and implementation boundaries

Required successor exercises `SE-POSIX-CLOSE-01` (families A, D, F, I) and
`SE-POSIX-RDLOCK-01` (A, F, I) resolve source questions `LIO-POSIX2024-CLOSE`
and `G-SOURCE-POSIX2024-RDLOCK`. They extend T23.04.b, T51 synchronization,
and the MC-FALLIBLE-CLOSE composition contract. Read their original teaching
first; bring the learner's descriptor-owner wrapper and reader/writer policy.
The inspected sources and exact locators are in
`research/v5/expansion/source_reconciliation/resources.json`.

Use ISO C17, hosted profile A, GCC or Clang with strict diagnostics. The model
has no system calls and no dependency that implements its decisions. Its enums
are abstract events, not platform errno numbers. Do not call a function on an
uninitialized real lock or intentionally invalid OS handle to test language UB.
The actual Linux I/O and cancellation providers retain their Linux contracts;
this successor does not relabel their results as Issue8 conformance.

## Teaching and construction

Resource ownership and operation success are separate state variables. A numeric
descriptor is a reusable name. A close error need not leave that name owned:
retry after retirement can close a different resource allocated by another
thread. Conversely, a policy that always retires a still-owned descriptor leaks
it. A destructor without an error return must choose where its diagnostic goes;
it cannot make the failure disappear. C enforces neither exclusive ownership
nor the connection between the selected OS contract and an enum value.

For `SE-POSIX-CLOSE-01`, implement `pe_close` in `learner.c` from the header and
supplied table. Inputs select the edition/implementation, API, restart constant
and observed result. Output distinguishes retirement, close-only retry,
unresolved ownership and an invalid handle. Reject tuples outside the declared
contract without modifying output. No allocation, recursion, I/O or retry loop
belongs inside this classifier. Derive constant storage/time bounds.

Issue8 distinguishes EINTR with retained ownership from EINPROGRESS after
retirement. `posix_close(fd,0)` disallows EINTR. A nonzero restart option permits
EINTR with only close operations reliably usable afterward. Other errors retire
the descriptor, except EBADF, which indicates an already invalid name. Invalid
flags may report EINVAL but still retire it. EAGAIN/EWOULDBLOCK are forbidden.
The supplied Linux and Issue7 models have separately bounded outcome sets;
their unlisted events are rejected, not declared impossible on every OS.

Submit a two-owner trace: A closes name7, B acquires reused name7, A retries.
Identify the exact assumption that makes the retry wrong; give the opposite
retained-on-EINTR trace. Derive an adapter to a cleanup interface with independent
`retired` and `error` fields. Explain why unresolved ownership requires a known
platform policy before this adapter can safely return either answer. No generic
`while(close(fd)<0)` implementation satisfies all three contracts.

For `SE-POSIX-RDLOCK-01`, implement the selected Issue8 admission classifier
`pe_read`. Draw an existing-reader recursive acquisition with an equal-priority
writer waiting, and a new reader with that same writer. Distinguish them under
the declared homogeneous FIFO/RR/SPORADIC policy. Without the scheduling option,
writer precedence remains implementation-defined. A writer already held by
another thread blocks readers; self-held write ownership may deadlock.
An uninitialized real lock gives undefined behavior; the model merely classifies
that premise. Mixed scheduling populations are outside this model.

Multiple read acquisitions require matching unlocks. There is an implementation
read-count limit, with EAGAIN permitted at that limit. `tryrdlock` never waits,
and these functions do not return EINTR. The model omits count exhaustion and
signal delivery deliberately: assess them in the reasoning table, not by claiming
the admission result proves an entire lock implementation. Admission precedence
also does not establish bounded waiting or freedom from priority inversion.

The inspected Issue7 priority paragraph lacks the explicit existing-reader
exception present in the inspected Issue8 paragraph. Preserve both editions in
the comparison dossier; do not silently apply one passage to the other's label.
Issue8 also makes its blocking rule independent of the scheduling-option branch.
Neither observation proves what the installed pthread library implements.

## Evidence and progression

Before execution, seal expected decisions and justify each by an exact source
locator. Implement the model; run `python run.py --source learner.c`. Supply your
transition table, implementation, unchanged-output proof, two-owner countertrace,
cleanup adapter argument and a second set of discriminating cases. Public tests
contain70 close combinations,10 read situations and9 API boundary checks.
The separate assessor campaign runs six compiler/optimization/sanitizer profiles
and rejects five compiling semantic mutants. Model execution is not an observed
kernel EINTR, an actual POSIX2024 runtime, or a concurrency proof.

Rubric: level0 has a wrong ownership/admission decision or missing deliverable;
level1 reproduces the table but cannot explain name reuse or policy selection;
level2 derives all decisions, unchanged-output behavior and both opposing retry
counterexamples, and distinguishes ISO C, POSIX and Linux guarantees; level3
solves the withheld changed-interface case after at least seven days without
the worked solution. Close and admission objectives are scored separately.

Hints, in order: (1) Track the resource and diagnostic independently.
(2) Test the no-restart API before deciding what EINTR means.
(3) A waiting writer and a held writer are different facts; a new reader and an
already-owning reader are different actors. Private reasoning and transfer
premises live in `assessor/posix_edition/ANALYSIS.md`.

All inputs are synthetic local model states. No external targets, real
credentials, host setting changes or deployed resource faults are involved.
Validation status is the source-bound report, not this text. Author results do
not establish independent review, learner performance or physical observations.
