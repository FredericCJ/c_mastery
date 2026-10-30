# Checked construction, flexible storage and relocation — PR-T03-04

This required successor implements the nine unchanged T03.11–T03.13 criteria
and their original seven-day delayed tasks in
[PR-T03-04](../../../curriculum/projects/PR-T03-04.md). It uses the canonical
legacy assessment shared contract and original sizing fixture. Historical eight
hours do not limit this extension. Learners implement `learner.c`; private
implementations, independent oracles and solutions remain separate.

## Prerequisites and environment

Use the previous object-lifetime, actual allocated storage, representation and
checked extent artifacts. Baseline profile A permits the hosted allocation
boundary. The core instead takes explicit allocate/resize/release callbacks so
failures and movement are reproducible. Callbacks supply actual live allocated
storage, adequate size/alignment, ordinary nonzero realloc content/failure
semantics and matching deallocation. They are single-owner and nonreentrant.
No arbitrary-pointer validator or hostile allocator containment is promised.

C17 is the active mode. Compare actual GCC/Clang C99/C11/C17/C23 VLA frontend
modes without treating successful compilation as edition conformance. Source
authority is inspected N1570, issue0400 marked fixed in C17, issue0451's closed
committee response and N3096 working draft. Published final C17/C23 remain
unavailable. Host sanitizers, Memcheck, Python models and private i386 runtime
are verification tools. No firmware execution or physical timing is claimed.

## Problem and necessary distinctions

Implement `buffer.h` from permitted primitives. All control objects are live,
correctly typed, initially zero-initialized when their contract requires empty
state, and distinct from inputs/output pointers. Output parameters already
owning resources may not be overwritten. No function is concurrent or reentrant.

`struct ob_construction` separates an initialized state mask from a record containing
int, pointer and char fields. Set the mask before any failure edge. Each set bit
authorizes reading exactly its corresponding member. Fail after the integer or
pointer store on request. A successful pointer-store edge transfers ownership
of that argument to the construction; before it, the caller retains ownership.
Cleanup reads/frees the pointer only when that edge completed, and consumes the
mask. Member equality operates only on fully initialized records; padding is
neither a value field nor an equality key. Encode a fully initialized record as
two big-endian bytes for number0..65535, one unsigned tag byte and one Boolean
pointer-presence byte. Do not serialize addresses or representation padding.
Write exactly four output bytes; preserve all following bytes and output on
admission failure. Null is established by a typed pointer assignment, not by
assuming all-zero pointer bits.

`ob_size(prefix,width,count,limit)` rejects width0, prefix beyond limit and any
mathematical total above limit before arithmetic; output is unchanged on failure.
Use actual sizeof(ObFlex) plus capacity bytes for allocation. Zero payload still
allocates the nonzero header and grants no payload access. The selected ObPadded
layout has a double, char and flexible bytes; measure both sizeof and member
offset. Tail padding is not additional logical capacity. Initialize every
capacity byte the contract permits later reading; copying the fixed struct alone
does not copy an arbitrary flexible payload.

`ob_workspace` admits signed bounds1..16 and sufficient caller storage before
any VLA declaration. Evaluate the local bound once, initialize all elements to
1..n and return their sum. Its array lifetime ends at block exit; no view escapes.
The changed function parameter uses `int values[static n]`: it adjusts to a
pointer and requires n accessible elements at each call, not a runtime bound
check. A build with the project policy `OB_NO_VLA` removes all VLA declarations
and uses caller storage. Do not redefine standard feature macros or pretend
this policy demonstrates a different compiler's absence of VLA support.

Compare ObFlex's one allocation and contiguous bytes with ObSplit's separately
allocated header and payload. Both begin with zero logical length and initialized
capacity bytes; zero-capacity split storage allocates only its header. On a
second-allocation failure, release the new header without publishing output.
The changed immutable representation is ObShared: copy the source once, retain
up to three references, reject further retention unchanged, and release payload
only after the final owning reference. A reference must remain while a const
borrow is in use; const does not enforce the absence of another writable alias.
No threads or atomic refcount safety are implied.

`struct ob_table` owns one flexible object and an externally assigned, nonzero unique
identity. A new table starts generation1; each successful resize or erase consumes
one generation, with maximum3. Refuse further invalidating operations unchanged
instead of wrapping. Append requires available capacity and never relocates.
Resize rejects capacity0 before calling any allocator. Reset is the separate
release policy; it retires this table identity permanently. A new logical owner
requires a new identity unique over the full lifetime of external tokens.

Nonzero resize uses a temporary result. Failure leaves old pointer, contents,
capacity and generation intact. Success publishes only the new pointer, retains
the preserved prefix, truncates logical length on shrink, initializes new capacity
bytes and invalidates all prior raw allocation views and generation tokens.
Never evaluate an old pointer after successful realloc, including equality.
An integer address snapshot taken beforehand may record numerical reuse but
cannot authorize access. The harness also forces move by allocation/copy/free.

Erase shifts following bytes without allocation and invalidates all logical
handles by generation advance. A pointer to a still-live slot may now name a
different logical element; distinguish address validity from identity. Handles
contain identity, generation and index, and resolve through an explicitly supplied
current table. `ob_table_move` transfers ownership to a distinct empty table,
retires the old live control and preserves identity/generation. External routing
must now select the new control. No pointer to the old control is hidden in the
handle, and copying the control without consuming its old ownership is forbidden.

## Required reasoning and deliverables

- D1: initialized-member/read-path ledger, early-failure cleanup trace, fieldwise
  equality and four-byte encoding proof. Classify padding separately from missing
  scalar/pointer initialization; discuss issue0451's unresolved representation
  subtleties without inventing a new base-standard rule.
- D2: mathematical size proof, original16/4/count0,3,100/limit255 results,
  actual layout observations, zero payload and overflow/limit failure evidence.
- D3: VLA bound evaluation/lifetime trace, adjusted parameter preconditions,
  caller-storage implementation and edition/source-support comparison.
- D4: two representation implementations, second-allocation rollback, immutable
  sharing/refcount exhaustion and a resource/locality/copy/lifetime comparison.
- D5: owner and view transition diagrams for append, failed/successful resize,
  erase, reset and table move; token exhaustion proof and unfamiliar changed trace.
- D6: original diagnostic warning/optimization/branch comparison, raw tool logs,
  source/tool hashes, positive/negative tests, independent model packets and
  evidence limits. Diagnostic outputs are observations, not semantic answers.

Core traversal/copy is O(n); size checks, generation checks and view lookup are
O(1). Erase and resize initialization/copy can be O(n); callback allocation cost
is outside that bound. Account for header padding, two-allocation overhead,
reference metadata, finite token capacity and retained old owner on failure.
No finite test establishes the arbitrary-size overflow argument.

## Verification and acceptance

`tests.c` reads its selector with the course parser, so compile it together with `labs/support/cm_parse.c` and add `-I labs/support` to the include path.

`tests.c` selectors0..5 cover construction, sizes, workspace, representations,
relocation and handles. Run six GCC/Clang debug/O3/ASanUBSan profiles plus actual
i386. For each, compare320 selected arithmetic boundaries against unbounded
integer arithmetic, then625 four-event mutation histories against an independent
logical model. Retain raw inputs/results. The matrix includes SIZE_MAX values
without requesting those allocations. Six compiled controls falsely clean an
unestablished pointer state, omit payload size, lose a shared reference, advance
generation on failed resize, accept stale generation or omit erase invalidation.
Reject each at its intended assertion, with the harness safely releasing owned
resources. Incomplete learner stubs must compile and fail all six selectors.

Compile the original `warning.c` only at O0/O3 and retain GCC/Clang diagnostics.
Run `diagnostic.c` only in the separate local diagnostic experiment: initialized
branch1 is a defined positive; branch0 deliberately uses uninitialized heap
state. Debug Memcheck must expose its origin, while optimized Memcheck and
ASan/UBSan outcomes are retained even when silent. No branch0 value or exit code
is a golden C result. Compare actual generated diagnostic sensitivity with the
inspected tools' promises. Positive initialization/relocation/handle tests must
also pass Memcheck, and changing coverage to branch1 must remove the defect.

VLA-disabled policy must compile with `-Wvla -Werror`; the VLA path must trigger
that diagnostic when the same policy flag is absent. Valid VLA mode comparisons
use C99/C11/C17/C23 on both compilers, with exact feature/version outputs retained.
Keep zero-size realloc unexecuted: issue0400 distinguishes zero failure, whereas
N3096 makes size0 with a nonnull prior allocation undefined. The API's separate
reset avoids dependence on either behavior.

## Rubric, hints and changed conditions

Retain the canonical objective-specific criteria and exact delayed IDs. Level0
violates lifetime/bounds or claims invalid output proves semantics; level1 gets
ordinary output but lacks ownership/initialization reasoning; level2 implements
the named contracts and relevant failure evidence; level3 independently adapts
the original changed condition with a counterexample to a plausible wrong claim.
D1/D6 assess.11a/b/c, D2/D3/D4 assess.12a/b/c, and D5 assesses.13a/b/c.

Hint1: track established members outside the record before reading any member.
Hint2: separate logical length, allocated capacity, table identity and generation.
Hint3: derive admission using `(limit-prefix)/width`; keep the old pointer only on
failure, and distinguish a VLA parameter from its caller's array. The separate
assessor analysis contains full solutions and per-leaf critique.

All nine original delayed packets remain mandatory after at least seven days:
selective serialization, failure after one member, optimization/branch coverage,
tail padding, parameter adjustment, shared immutable storage, zero-size edition
comparison, in-place erase identity and moving the handle table. Author execution
does not satisfy an independent learner's delayed attempt.

## Security and limitations

The only deliberately undefined executable target is original `diagnostic.c` in
a local short-lived process, containing a synthetic single-int allocation. The
learner chooses initialized versus missing-initialization branch; success is a
tool diagnostic or its documented absence, never disclosure, arbitrary execution
or privilege escalation. Its repair initializes before every read, retained as
branch1. No real input, credential, external service or host-security setting is
involved. Run branch0 only as directed by the isolated diagnostic harness.

The remaining mutants violate logical counts/state without deliberately accessing
expired storage. Unique identity allocation and borrow withdrawal remain caller
discipline, not ISO-enforced safety. Actual author receipts are separate under
`research/v5/expansion/legacy_object_buffer`; published ISO source gaps, external
review, independent learner mastery and physical observations remain open.
