# X-T66-011: exact publication and fallible-close assessment support

This required supplement retains the actual mc_publish_pair and
mc_publish_recoverable implementations, public headers and compiling learner
stubs. It does not create a competing allocator, parser or recovery machine.
Complete the original X-T65-002, X-T65-004, X-T66-002 and X-T66-009 prerequisites;
the actual source imports are errors, ownership, coroutines and messaging as
declared by the existing composition contract. The latter two remain translation
and link dependencies of composition.c; this supplement makes no new coroutine
or channel claim. Use completed learner workspaces explicitly; no assessor
source is copied into their exports. C17 hosted Linux is the baseline.

Read ../CONTRACTS.md transactional publication and five-resource recovery,
../include/composition.h and recovery.h. They supply the complete representation,
operations, lifetime, callback, extent and generation preconditions. All close
callbacks are synchronous, nonreentrant and return only declared outcomes.
This supplement adds tests and independent expected outcomes for original
R1-R4 and X-T66-011-DELAYED, not historical acceptance evidence.

## Concrete tasks and supplied artifacts

R1/D1: draw the product state (parse, staged memory, opened resources, current,
pending). For an old owner90,91,92 explain why a parser's unchanged output does
not authorize early destruction of the prior owner. Predict each publication
and cleanup transition before executing tests.c. Input3:x is invalid;
65536:0 and65535:1 are range failures; empty, missing operand and trailing bytes
are invalid. All require zero additional allocations and unchanged old pointers
and values. INT32_MAX+1 length refuses before dereferencing the input.

R2/D2: implement the existing adapter using actual er_parse_pair, ow_construct,
ow_publish and V4Allocator. Publish three staged cells only after complete
construction. For3:7 the new cells are10,2,3. Failure on new attempts1,2,3
preserves old identity and releases exactly0,1,2 staged blocks. The transient
peak is3,4,5 respectively; success peaks at6, releases3 old blocks and retains3.
Destroy the final owner: total releases are3,4,5 after failed attempts1,2,3,
and6 after success, with no live blocks or invalid release. These are real
observations of the unchanged current T66 counting allocator, not an inferred
cost from source syntax.

R3/D3: run publication_admission and publication_prefix with the above exact
predictions. Compare compiled MC_MUTANT1 early destruction and MC_MUTANT2 lost
old owner at their named assertion. Both must build, then fail semantically;
compiler errors or sanitizer termination do not count. Preserve exact source,
compiler, include/link closure, raw logs and independent predictions.

R4/D4/delayed: the implemented changed contract has five resources: three typed
cells and two external identities. Execute all81 four-result words over
RELEASED, RETAINED and RELEASED_ERROR across two cleanup attempts. The independent
Python set model predicts pending mask, diagnostic count, completion status,
live cells, live resources and exact close count after each attempt. Callbacks
use original local /dev/null descriptors: RETAINED injects before close;
RELEASED_ERROR injects a diagnostic after actual successful close. This is not
a forced kernel close failure or a durability experiment. A released identity
must never be retried, even if its numeric descriptor is reused. Resources
close in reverse order; cells remain until both resource obligations end.

Also inject failure at either external open after staging. First-open failure
can destroy all staged memory immediately; second-open failure with retained
close keeps three staged cells and one resource, while preserving the old
current owner. Recovery pending blocks further publication and stale tokens
leave the error output unchanged. Retry may fail indefinitely. At generation
MAX-2 admit the final publication to MAX-1, reject another without callbacks,
then admit final retirement to MAX and drain it. Later retirement refuses with
unchanged output. Counter exhaustion must not strand the final existing owner.

The original public close fixture has twelve allocation/open combinations;
retain it as a separate oracle. This supplement's81close-order words extend
that evidence without treating one model as exhaustive of the other. The
provided tests, original stubs, CMake and tools/verify_ownership_publication.py
are learner-facing; the worked reasoning is assessor-only. Seal predictions
before reading it. Changed independent assessment chooses a previously unseen
retained/released-error order and requires ownership and release accounting,
not only a final successful status.

## Reasoning, rubric and hints

Precommit failures retain the old current owner; postcommit cleanup failure
retains a distinct pending owner and does not roll back external effects or
the new publication. Memory rollback is not descriptor rollback. A close
diagnostic says nothing about retained ownership unless the callback contract
classifies it. No retained native pointer may outlive its owner. All calls
and callback effects obey the existing exclusive-owner serialization rule.

Each original criterion is assessed separately: level0 for violated ownership,
incorrect effect classification or an unsupported execution claim; level1 for
nominal completion without accounting; level2 for all specified predictions,
implementation and named failure observations; level3 for independent transfer
to a changed close policy and a concrete counterexample to the old proof.
Passing public tests alone cannot earn independent mastery. Record assistance
and the original minimum seven-day assessment delay.

Hints: (1) stage before committing; (2) label each resource with its current
owner after every callback; (3) a diagnostic and a retry obligation are two
different facts. The solution and explanation of every outcome are separate.

## Cost and enforcement

1. Representation: actual OwOwner/McFive/McRecovery; report real sizeof/padding.
2. Allocation: three cells per version, at most six simultaneously; actual
   counting allocator distinguishes attempts, releases and peak live blocks.
3. Copy/move: control copies represent explicit ownership transfer only under
   the contract; ISO C does not enforce moves.
4. Locality: typed fixed backing; no cache-line or physical memory-traffic claim.
5. Dispatch: allocator and open/close callbacks are explicit dynamic calls.
6. Work: parse O(n), at most three allocations and two opens, at most two closes
   per recovery attempt; number of retained retries is unbounded.
7. Failure: syntax/storage/resource/recovery diagnostics remain separate;
   postcommit cleanup failure cannot be relabeled uncommitted.
8. Concurrency: exclusive caller serialization, no atomic publication claim.
9. Code/stack: inspect emitted objects/imports, stack-use files and executable
   size for both compilers and modes, including any implicit helper imports.
10. Latency: seven10000-update CPU-time samples with checksum700000; each
    sample actually allocates/releases30000blocks. This is not close latency
    or a real-time bound.
11. Build: compiler-derived include/link dependencies bind each executable;
    retain commands and source identity, not broad unrelated family pins.
12. Enforcement: runtime statuses/checks and programmer callback/lifetime
    discipline; diagnostics/tests/models do not prove ownership universally.
13. Composition: actual parser, allocator and owner publication plus a separate
    fallible external-close protocol. Existing inspected Linux close passages
    are platform-specific; published C17 access remains a gap.

## Security, validation and limitations

Only original local teaching programs and synthetic data are used. The
intentional defects lose or retry owned resources; harmless success is a named
MC_CHECK assertion in this process. No external target, credential or deployed
service is involved. Restore ordering and replay failed/neighboring histories.
No arbitrary code execution, kernel error injection, filesystem durability,
physical validation or independent learner/human review is claimed. The exact
current campaign records author executions; the older family receipt remains
historical, including its then-current source pins.
