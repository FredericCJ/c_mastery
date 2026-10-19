# Consume publication: value-derived and branch-selected indices

Required companion for `AX-PR-T25-03-T25.08.c-DELAYED`, supporting the ordinary
`AX-PR-T25-03-T25.08.c` and `LRUN-16` analysis. Families A, B, D, F and I.
Prerequisites: PR-T25-03 ordinary atomic publication, T03 live object/array bounds,
and the learner's dependency diagram. The active execution language is C17.
The source argument below uses the explicitly identified C11 committee draft
N1570; exact published C17/C23 comparison remains unavailable, not inferred.

## Representation and execution contract

Implement the five access functions and `ci_dependency_graph` in `learner.c`.
All controls, nodes and four-element arrays are live, initialized and immutable
for each call. The atomic pointer is null or names a `CINode` whose `index` is
0..3. No control aliases array storage. There is no allocation, ownership
transfer, callback or retry. Node and array lifetime are caller obligations.
The functions do not publish a pointer or retain a borrow. Do not use a library
to compute the dependency relation.

`ci_pointer` returns node.value, or zero on null. `ci_value_index` first loads the
pointer with consume, then copies node.index and returns values[index], or zero
on null. `ci_branch_index` loads with consume, sets index=1 if nonnull and index=0
otherwise, then returns values[index]. The selected index is independent of
node.index and node.value. `ci_killed_index` is the value-index version with
`kill_dependency` applied to the index. `ci_acquire_branch` changes only the
branch version's memory order to acquire. Each operation performs one atomic
load. All functional tests use immutable ordinary data and are defined even for
the two access forms lacking a dependency argument.

The graph API takes 0..728, encoding the six forward edges 01,02,03,12,13,23 in
base three. A digit is absent=0, value-use=1 or control-only=2. Return a four-bit
mask of vertices reachable from vertex0 through value-use edges, including
vertex0. Invalid encodings are outside this small API's precondition. Its
constant-size representation and six-edge traversal must be explained. This
is a labeled graph exercise, not an automatic parser or C memory-model checker.

## Required reasoning and deliverables

Before executing, draw the operand/scalar-copy chain for each function. Label
the release store, the consume/acquire load, its reads-from source and the
ordinary access separately. Explain why a null guard need not destroy a later
value-derived chain, whereas selecting a constant index solely by a branch does
not supply that chain. `kill_dependency` preserves the numeric result while
ending this dependency argument. Acquire instead supports subsequent accesses
when it reads the matching release sequence. State the extra lifetime and
no-conflicting-write premises; no memory order makes a dangling node valid.

Submit: (D1) five dependency diagrams and exact-edition comparison with explicit
access gaps; (D2) the implementation and 729-row graph output; (D3) native tests,
two independently rejected functional defects, graph defect, and GCC/Clang
native/Arm disassembly; (D4) a delayed analysis changing the writer to initialize
values[1] then publish the node. For the branch-only reader, identify the missing
ordering path and resulting possible data race. Keep that counterexample as
analysis; do not execute racing ordinary accesses as an outcome oracle. Repair
it with the acquire version and separately with a genuine value-derived index,
stating each repair's assumptions.

The supplied local driver is `python3 run.py`; `--source` selects an explicitly
provided implementation. It has no assessor fallback. Empty stubs must compile
and fail the assertions. Functional agreement alone cannot establish the
dependency proof: killed and value-derived forms intentionally return equal
numbers. Explain a compiler's stronger lowering without promoting it into an
ISO guarantee. Compare costs only under equivalent synchronization contracts.

## Verification, rubric and hints

All node indices0..3, null/non-null controls, and distinct values11,23,37,53 are
checked. The independent assessor enumerates paths for all729 labeled graphs.
GCC/Clang debug, optimized and AddressSanitizer/UndefinedBehaviorSanitizer builds
exercise the defined cases. Arm M4 object/link inspection audits unresolved
helpers and lowering; it does not execute a target or observe a board.

Rubric0: wrong index, missing lifetime premise, or claiming a passing run proves
the branch dependency. Rubric1: functional outputs correct but graph/edition
reasoning incomplete. Rubric2: all cases and graph relation correct, exact
reads-from/dependency argument, controlled defects and evidence limitations.
Rubric3: independently derives the changed writer case after the parent delay,
repairs it and rejects the incorrect claim that equal assembly implies equal
portable contracts. No independent learner attempt is recorded by author tests.

Hints: (1) trace values, not only control flow; (2) inspect the operand supplying
the array address; (3) distinguish whether an index comes from node.index or a
literal selected by a conditional. A separate assessor analysis supplies the
answer. All data are original synthetic local values; there is no exploit,
external service or security-setting change.

## Inspected sources and validation boundary

N1570 (April12,2011 committee draft), 5.1.2.4p14-18 and7.17.3.1, was read directly
for value-use/scalar-copy/transitive dependency, its exceptions, and the killed
chain. GCC's current `__atomic` builtins documentation explicitly describes
consume as implemented using acquire; its memory-order overview is illustrative,
not an ISO specification. Exact links and source-access evidence are in the
companion research register. These bounded source passages and compiler tests
do not close the exact published C17/C23 access gaps or establish all C executions.
