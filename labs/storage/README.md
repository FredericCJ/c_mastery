# A bounded persistent B+tree

Start in `src/store.c`. Its placeholders compile and fail the acceptance tests.
Keep your working copy outside the curriculum repository. The public API is
`include/store.h`; the byte format and persistence model are in `FORMAT.md`.
Use `HINTS.md` one stage at a time after writing a prediction and failing test.

Progressive gates are listed per project in `family.json`. Start with
`storage.codec` and `storage.initial_image`, then `cache`, `lookup`, `insertion`,
`deletion`, `transaction`, `publish` and `checkpoint_order`. The three public
page codec/cache functions make early component testing possible. Early tests
inject explicitly valid cache/tree/scratch states, avoiding a dependency on an
unfinished opener, later mutation API or recovery engine. They are component
evidence, not proof of a durable database. PR-T47-09/10 runs the deferred complete
image/tree/crash/recovery tests. Assessor-only mutants are separate from ordinary
learner prerequisites. Run all suites only after the corresponding mechanisms
exist; a later unfinished test must not prevent completion of an earlier gate.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Implement byte encoding, cache, tree and persistence yourself. The supplied
POSIX file adapter is transport scaffolding: inspect its partial-transfer loop
and `fsync` error path. It does not implement the educational mechanisms.
The model harness is an independent fault source and result oracle, not a tree
implementation. Its sorted key/value reference model compares actual C calls.

An assessor can explicitly select the separately distributed reference directory:

```sh
cmake -S . -B assessed -DCMASTERY_ASSESSOR_ROOT=/absolute/path/to/assessor/storage
cmake --build assessed
ctest --test-dir assessed --output-on-failure
```

No relative search for an assessor directory exists. Learner exports must exclude
`assessor/`; viewing the complete implementation is an assessor action after the
relevant learner gate. Three compiled wrong implementations exercise semantic
mutation tests: omitted commit flush, premature log reclamation, and wrong
separator equality direction. CTest's `WILL_FAIL` means those cases must return a
nonzero test failure. A setup failure is not evidence that the mutation was killed;
inspect the assertion and compare the unmodified case.

The reference uses 4 KiB pages, 32 data pages, two checkpoint generations and one
bounded 32-page redo batch. Keys are 1–16 bytes, values 0–64 bytes. A node holds at
most seven keys; the deliberately small fanout makes multi-level splits and
underflows visible in a short test. Leaves hold all values and a next-leaf link.
Internal keys duplicate the minimum key in their right child. Sorting is unsigned
lexicographic byte order, with a shorter equal prefix first. These are original
educational choices, not a SQLite or other database file format.

One caller serializes every operation; there are no concurrent readers, process
locks, snapshots, SQL, compression or encryption. `store_begin` pins all live
nodes into a bounded typed transaction workspace. No uncommitted home page is
written. Allocation exhaustion aborts the entire in-progress transaction; input
validation failures leave prior staged changes available. An explicit abort
restores the previous root/allocation state. Once commit publication is attempted,
an I/O error means `STORE_INDETERMINATE`: discard the handle, reopen, recover and
inspect state before deciding what the application should do. This API has no
application transaction ID or exactly-once retry facility.

Lookup and structural descent follow tree height. The initial implementation
refreshes separator minima over allocated nodes after each update: its update
bound includes O(P) work, where P is allocated pages. Optimize that refresh only
after preserving the invariant and rerunning split/merge/differential cases.
Full-image transactions deliberately write all 32 after-images. Measure their
amplification; a later dirty-page WAL is a separate format/recovery design.

The C harness executes 800 deterministic mixed operations against an independent
80-key oracle, a 92-key multi-level structural sequence, and every write/flush
boundary of representative commit, checkpoint and recovery executions. At each
boundary it tries five persistence patterns: discard pending writes, persist an
even-page subset in reverse order, half-page prefix tear, half-page suffix tear,
and complete persistence despite an error return. These are representative
admitted states, not every possible tear offset or subset. A lost response may
hide a durable commit. Interrupted recovery is restarted and checked again.

Checksums detect the exercised damage. The durability argument excludes checksum
collisions, post-flush media corruption, writes damaging other pages, and a device
lying about a successful flush. A modeled crash is not physical power-cut evidence.
The Linux test establishes successful system-call/reopen behavior for an existing
preallocated image; it does not establish physical drive durability or newly
created pathname/directory persistence. `storage.file` reports unavailable on a
non-POSIX host; all portable memory-model tests remain applicable.

For cross-family integration, reuse the portable callbacks. A VM host service
encodes one fixed integer under the key `vm-result`; it begins, writes and commits
with explicit outcome handling. An educational-kernel process adapts its bounded
block syscalls to `store_io`. See `INTEGRATION.md`. Passing this family's tests is
component evidence; actual VM/kernel integration needs its own executed transcript.
