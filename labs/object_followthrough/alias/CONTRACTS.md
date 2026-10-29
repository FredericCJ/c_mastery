# Allocated objects, aliasing and provenance — PR-T03-03 successor

This required supplement preserves the twelve original T03.07–T03.10 criteria
and their seven-day delayed tasks. It incorporates the exact
[foundation contract](../../foundation_depth/CONTRACTS.md),
[public producer header](../../foundation_depth/include/depth.h) and original
[project](../../../curriculum/projects/PR-T03-03.md). The historical ten hours
are descriptive; this work adds workload. The unfinished `learner.c` is the
implementation surface. The private assessor implementation is not exported.

## Prerequisites

Use the T03.01–T03.06 lifetime, representation, row and cursor artifacts and
their authority classifications. Required actual producers are FDSlot,
fd_move and FDPool from `foundation_depth`. If importing learner versions,
record exact source identity and rerun their existing contracts. Knowledge of
callbacks does not authorize using a library allocator to implement the core:
allocation is only the explicit injected boundary.

## Environment

The core is profile C, ISO C17 mode, with no direct library calls, OS services
or hidden runtime. Public standard headers supply types/macros. Actual source
rules are inspected N1570 C11 draft clauses, not a claim to have read published
C17 or C23. Host tests use malloc/free, aligned_alloc, clocks, diagnostics and
sanitizers separately. The extension experiment explicitly selects GCC/Clang
`may_alias` and binary32 assumptions; it is not a strict ISO exercise.

Native LP64 and actual i386 runs report layout and tool versions. Four M4
object/final-link audits use GCC/Clang at O0/O3, `-ffreestanding -fno-builtin`
and `-nostdlib`; retain assembly, maps and undefined-symbol output. Those
images are dependency probes, not bootable or executed firmware. Support for
64-byte extended alignment is an explicit selected implementation assumption.

## Problem

Implement `alias.h` without replacing imported mechanisms with lookalikes.
All supplied controls are live, correctly typed objects; callbacks are truthful
and nonreentrant. Required output/control objects are distinct from input and
owned storage unless the specific API below permits sharing. No function
admits an arbitrary address by inspecting it. Failures leave prior output and
ownership unchanged, except explicitly published cleanup state.

1. `os_acquire` admits truthful allocator capacity/alignment metadata before
   calling FDSlot acquisition. Reproduce empty→acquired→initialized→published
   and reset. An initializer may allocate a child, but owns rollback of that
   child on failure; successful initialization transfers its cleanup to the
   registered destructor. Withdraw all published borrows before reset.
2. `struct os_wide` has requested alignment64. Admission requires sufficient capacity
   and an alignment compatible with `_Alignof(struct os_wide)`. Actual allocation has
   no declared type. Initialize fields by typed writes. Its slot has EMPTY,
   LIVE, BORROWED and CLEANUP states. Borrow is exclusive. Return consumes the
   exact outstanding pointer. Reset while borrowed fails. A fallible destructor
   failing in CLEANUP retains the allocation and all resources needed to retry;
   no new borrow is allowed. A successful retry frees the child and allocation
   once. Callback failure must preserve retryability; this is a stronger
   callback contract, not an automatic C guarantee.
3. `os_clone` copies the complete representation of a live FDValue into actual
   disjoint allocated storage of sufficient size/alignment. Character copying
   must not read arbitrary serialized bytes as a value. Separately inspect a
   declared character array without casting it to FDValue. `os_wire` accepts
   exactly eight bytes: nonnegative signed32 value as big-endian u32 (reject
   values above INT32_MAX), then big-endian u32 serial. Assign actual fields;
   rejection preserves output. `os_host_representation` copies a valid live
   uint32_t representation and returns the same value.
4. Implement identical XOR mappings in ordinary, restricted and nested-block
   loops. Inputs and output of each transform are disjoint for positive length.
   Two inputs of `os_shared` may be identical because they are read-only; the
   modified output is disjoint. `os_partition` validates integer offsets within
   one actual character array before forming pointers. Adjacent nonempty ranges
   are disjoint; partial overlap fails unchanged. Identical positive ranges fail
   unless `inplace` selects a separate ordinary loop. Empty ranges, including
   null base/zero length, succeed without pointer arithmetic. `os_move` composes
   actual fd_move for overlap with snapshot semantics.
5. Use actual FDPool retirement after generation3. Show a deliberately wrapping
   control returning a new logical value through a stale generation1 token, then
   refusal by the corrected pool. Its owner remains live: this is not a freed
   pointer test. `os_member` receives the live enclosing owner and a live member
   pointer; verify the explicit member's offset and identity before accessing
   through the owner. The optional uintptr tagging experiment retains that live
   owner, checks actual low bits and restores the exact original integer before
   the documented roundtrip. Never convert a tagged integer to a pointer or
   claim arbitrary forged integers acquire authority.

## Necessity

An aligned byte buffer does not thereby become an arbitrary declared object.
Compiler agreement does not legalize incompatible lvalues. `restrict` concerns
execution blocks, based-on access paths and modifications, not merely unequal
pointer values. Destruction can fail after borrow withdrawal; freeing then is
not rollback. Pointer representations, live allocation identity and logical
token generations answer different questions. These mechanisms require separate
proofs even when their ordinary tests print the same values.

## Reasoning

Submit a transition table with callback ownership at every edge and a proof
that no free occurs before the last borrow or after a failed destructor.
Explain truthful size/alignment and declared-versus-allocated storage premises;
the implementation cannot verify a malicious allocator's actual storage.
Classify each lvalue in `analysis_only.md`, including qualified, corresponding
signed/unsigned, character and containing-aggregate cases. State why uint32_t
has no padding/trap representations when provided; do not generalize that to
float unions. Padding bytes are not a semantic equality test.

For each restricted call identify its block, based-on pointer associations,
modified objects and every access path. Show the outer-to-inner restricted
assignment separately from invalid same-block assignments. Prove range checks
without overflowing offset+length, and state O(n) work/O(1) storage. Allocation,
cleanup and pool lookup have fixed bounded core cost but callback cost is
external. Report type size/alignment, copy traffic, stack/object size, pool
capacity loss from retirement and pointer-versus-handle cost.

Keep a three-column claim ledger: inspected base C rule; named proposal revision;
selected compiler implementation/extension. Compare N2219v3 wildcard provenance
with N2263v4 empty/single identities and reconstruction question Q3. Note the
v4 Q2/partial-core wording tension instead of pretending the document is a
complete normative semantics. Actual compiler flags are not the proposal's
hypothetical switches.

## Deliverables

- D1: implementations, state/ownership tables and fault traces for ordinary and
  child-allocating initialization, extended alignment and fallible cleanup.
- D2: effective-type/lvalue table, live representation and wire decoder outputs,
  union/extension analysis, and explicit nonexecuted invalid fragments.
- D3: range partition proof, ordinary/restrict/nested/read-only sharing tests,
  independent finite range oracle and overlapping move composition.
- D4: four compiler/alias-mode timing packets, raw samples, assembly/vectorization
  diagnostics, code sizes and equivalent correctness/precondition statement.
- D5: pointer/provenance authority ledger, allocation-reuse analysis, harmless
  generation-wrap counterexample, member/tag audit and narrower ABI cost report.
- D6: exact tool/source hashes, host-versus-target dependency ledger, final link
  maps and generated-code checks, mutation/stub rejection evidence and limits.

## Verification

`tests.c` selectors0..4 exercise lifecycle, wide cleanup, representations,
restricted ranges and identity. Independent assessor set-intersection compares
all6048 small range inputs, including invalid offsets and empty endpoints;
this is finite falsification, not proof for all size_t values. Required controls
remove initializer cleanup, ignore destructor failure, misdecode the first wire
word, falsely accept partial overlap, copy overlap forward and wrap generation.
Every control must fail its intended assertion with cleanup still well-defined.

Compare GCC and Clang at O0, O3 strict aliasing, O3 disabled strict aliasing and
ASan/UBSan. The invalid casts/restrict calls are never executed in any mode.
Run the extension separately. Timing uses disjoint arrays with sizes64/4096,
zero/pseudorandom data, offsets0/1, fifteen samples per cell and alternating
variant order:240 samples per compiler/alias-mode experiment. Verify all output
bytes and retained calls before discussing timings. Host noise, cache state and
code generation constrain conclusions; identical or slower results are valid.

For strict closure, root every taught core/imported mechanism, inspect objects
and final maps, require no unresolved helper and reject an injected malloc
reference. A target link success supplies no measurement of allocation or board
behavior. Export with `cmastery init PR-T03-03`; compile learner+public foundation
stubs and require all five semantic selectors to reject until implemented.

## Rubric

The unchanged canonical card contains objective-specific levels and delayed IDs.
For every objective, level0 misses a contract or executes forbidden invalid
access; level1 reproduces output but omits its premises; level2 supplies the
correct implementation/analysis and relevant falsification; level3 independently
adapts the named changed condition, explains a plausible wrong argument and
states evidence limits. A benchmark alone cannot pass effective-type reasoning,
and a sanitizer pass cannot pass lifetime/provenance proof. D1 assesses.07a/c;
D2 assesses.07b/.08a/b/c; D3/D4 assess.09a/b/c; D5 assesses.10a/b/c.

## Hints

1. Draw ownership before choosing a pointer representation. Keep byte and value
   outputs separate, and list which inputs are actually modified.
2. Use subtraction-based bounds and integer range sets before forming pointers.
   An initializer's partially acquired child is not yet owned by a destructor.
3. Locate N1570 6.5p6–7 and6.7.3.1 examples3/4; then compare actual callback and
   live-owner premises with the proposal text. Do not infer semantics from one
   emitted instruction sequence. Private analysis supplies full assessor reasoning.

## Variants

Preserve each original `AX-PR-T03-03-T03.xx.y-DELAYED` task and minimum seven days.
The concrete changed packets are child-allocation rollback (.07a), wire-field
construction (.07b), stricter type plus fallible cleanup (.07c), aggregate alias
(.08a), wire versus host integer (.08b), second compiler extensions (.08c), nested
restrict (.09a), exact in-place admission (.09b), alignment/data changes (.09c),
generation wrap/exhaustion (.10a), actual proposal revision (.10b), and owner-relative
index/generation alternative (.10c). Use unfamiliar traces/inputs within those
mechanisms; passing the author examples is not the learner's delayed assessment.

## Security

The sole executable security-property counterexample is an original local pool
whose deliberately wrapped generation lets an old token retrieve synthetic
value4. Attacker capability is retaining a previously issued token and requesting
bounded allocate/release transitions in this teaching process. Success means only
that stale-token authorization fails; there is no arbitrary code execution claim.
The repaired pool retires exhausted slots and regression checks reject the token.
Capacity exhaustion and a requirement for a stable live owner remain. No external
target, real data or host-security change is involved.

## Validation

Current author execution is recorded separately in
`research/v5/expansion/legacy_object_alias/validation.json` and the exact
completion/guard/export records. The core and probes are authored; a source-bound
passed receipt is required before crediting execution. Published ISO texts remain
unavailable. External review, independent delayed learner assessment and physical
validation remain open. Finite successful checks do not prove absolute mastery.
