# Corrective-edition review and normalized resize ownership

## Exact tasks and source authority

This companion implements the six existing ordinary and `-DELAYED` tasks
`AX-PR-T05-03-T05.06.a`, `.b` and `.c`. Keep the parent project, source-analysis
rubrics and historical acceptance metadata. Prerequisites: PR-T05-01 authority
ledger, PR-T05-02 separate capability layers, T03 allocation lifetime and T04
checked extents. Bring the learner's own source ledger and resize notebook;
no assessor source is imported. Families A, B, C, D, F, G, I and J address
correction interpretation, ownership construction, refusal, allocation failure,
falsification and changed allocator/mode assumptions.

Read the actual N1570 memory-management and realloc passages, DR400 Summary,
Suggested Change, final Proposed Technical Corrigendum and current issue
status. The early suggestion about errno is not the final record. The final
record distinguishes a nonzero failed allocation from the zero-size case,
where deallocation may be implementation-defined. Its current WG14 issue page
says Fixed in C17; this is change-record evidence. Exact published C17 text
has not been inspected and remains an access gap. Do not reclassify this
record as a new C23 rule or infer every later issue applies to C17.

For the delayed `.b` task independently inspect DR452, including its original
non-lvalue/effective-type problem, earlier committee direction, final wording
and its own Fixed-in-C17 metadata. Its proposed correction concerns effective
type of temporary objects and permits nonunique addresses. It does not grant
permission to modify a temporary object or retain a pointer beyond its full
expression. `temporary.c` records the selected compiler's within-expression
address comparison without imposing a portable true/false golden.

## Representation and allocator contract

`LrBuffer` is a live, uniquely owned, noncopied control: either `data==NULL`
and `size==0`, or it owns exactly1..64 writable character bytes. Its allocator
and context remain live until all operations finish. Calls are serialized.
Initialize only fresh or already nonowning control. Input/control/backend
storage is disjoint. Callbacks must not reenter, mutate the buffer control,
retain borrowed pointers after return, or destroy their context. A release
callback may inspect the control read-only to verify it is already empty.
The caller must not mutate fields directly except owned payload bytes.

`lr_init` rejects NULL control or missing required callbacks without writes.
`lr_resize` rejects zero and sizes above64 before any allocator call. A valid
nonzero callback returns either NULL while preserving the entire old allocation,
or a new owned allocation preserving the first min(old,new) bytes and consuming
the old allocation. Success invalidates every old borrow even if the address
bits happen to be unchanged. On callback failure, pointer, size and all payload
bytes remain unchanged, and old borrows remain live. On success, the wrapper
initializes newly added bytes to zero and then publishes its new pointer/size.
There is no fallible operation after the provider succeeds.

`lr_release` publishes the empty control before one nonfailing matching release
callback. Repeated release is a no-op; the initialized allocator remains usable
for later nonzero resize. No automatic release, alias checking, callback-law
checking or borrow checking is provided by ISO C. Those are explicit programmer
premises and externally assessed invariants. A failed allocator cannot truthfully
be adapted to this interface if it secretly frees the old nonzero allocation.

| Request/result | Old allocation | New allocation | Published control |
|---|---|---|---|
| nonzero success | consumed; old borrows invalid | owned by buffer, prefix retained, extension zero | new pointer and size |
| nonzero failure | retained byte-for-byte | none | unchanged |
| zero or above64 | retained, callback not invoked | none | unchanged; LR_INVALID |
| explicit release | consumed exactly once if present | none | empty before callback |

Wrapper work is O(max(new-old,0)) zeroing plus provider cost and O(1) additional
control space. The original moving test adapter allocates new storage before
freeing old storage, so peak live payload is at most128 bytes; its copy cost is
O(min(old,new)). Real realloc allocator costs are not inferred from that model.

## Concrete implementation and changed-policy experiment

Implement the public compiling stub in C17. `tests.c` supplies two original
allocator adapters with deliberately different zero-size behavior: one returns
NULL and retains its old allocation; the other returns NULL and releases it.
Direct adapter controls establish those differences using real allocated memory.
They do not call libc realloc with zero and do not claim those are the current
host library's policies. In every wrapper history neither zero path is reached.
Demonstrate zero refusal both empty and live, maximal-size refusal, injected
nonzero allocation failure, growth/shrink prefix retention, initialized growth,
explicit cleanup, repeated cleanup and reuse. The actual libc companion uses
only nonzero realloc, scheduled failure before the libc call, and free.

Deliver a proof that wrapper behavior is independent of the two zero policies:
the admission check precedes callback evaluation, so zero requests have the
same unchanged transition in both models. Test evidence supports this argument
but does not replace it. Do not compare an old pointer after successful realloc;
its lifetime has ended even if a particular allocation reused the same address.

## Correction analysis and release-note transfer

For `.b`, produce a three-column comparison of the N1570 baseline, early
suggestion and final issue record. Preserve exact document identity, passage,
status and inspection limits. For DR452 do the same comparison afresh; copying
DR400's status into another row earns no credit. `claims.json` supplies concrete
claims to challenge and the source-analysis questions; machine parsing of the
file cannot assess whether the learner understood the passages.

For `.c`, falsify the claims that atomics, generic selection and designated
initializers were all introduced by C17. Actual inspected N1570 passages from
2011 already describe them. Its foreword distinguishes third-edition additions
from second-edition changes. This refutes the C17-introduction claims without
inventing inspection of the original published C99 or C11 texts. Keep those
exact-edition chronology/access limits in the ledger.

The delayed `.c` task reads the actual GCC8 release note's C section. It
announces c17/gnu17 options, the201710L macro and the GNU17 default. List what
that statement does not establish: a particular libc realloc policy, all
corrections correctly implemented, atomic helper or threads runtime availability,
another target's behavior, or current execution of GCC8. Current installed
compiler probes are separately labeled. The earlier capability companion's
observed missing helper is a concrete counterexample to equating source-mode
support with runtime closure; do not change its frozen source or receipt.

## Deliverables, verification and rubric

D1 is the exact correction/chronology argument and ownership decision table.
D2 is the C17 wrapper plus the two original adapters and complete callback
preconditions. D3 includes six GCC/Clang debug/release/sanitizer profiles, an
independent finite-history oracle, actual C11/C17 libc companions, raw commands,
hashes, compiler include closure, stack/code-size evidence and failure controls.
D4 is the changed zero-policy proof, independent DR452 analysis and release-note
review after the parent's minimum seven-day delay.

The oracle enumerates39,216 histories of lengths0..5 over grow-to1/2/4,
failed-resize-to3, zero request, explicit drop and payload write, under both
zero policies. It independently computes status, size, payload sum, allocation
attempts, releases and zero calls. The four compiled mutants forward zero,
overwrite ownership on failure, omit deallocation, or leave new bytes dirty.
Each must fail its intended assertion while the harness frees tracked storage;
a sanitizer report or process fault is not the intended rejection.

Pass `.a` only with every ownership row, real allocations, failure preservation
and changed-policy proof. Pass `.b` only with accurate named source distinctions
and its separate DR452 transfer. Pass `.c` only with earlier passage evidence
and a release-note claim boundary. Rubric0 is missing/wrong;1 reproduces a
result but retains the misconception;2 gives the full ordinary derivation and
independent falsification;3 succeeds under the changed assumption without
copying. The supplied source table is not independent learner work.

Hints: (1) Separate size-zero admission from allocation failure. (2) Keep the
old owner until the callback reports nonzero success. (3) Compare final
corrigendum wording with early suggestions before assigning applicability.
The private technical analysis is packaged separately.

## Validation limits and containment

Use only original local code and synthetic payloads. No zero-size libc realloc
execution is required or used as a universal result. The hosted allocator is
an explicit dependency because this exercise studies the resize interface,
not first-principles malloc. C17 remains the implementation baseline; C11 is
an explicitly labeled comparison. This packet makes no freestanding, physical,
independent-human or learner-mastery claim. Author execution and source-inspection
states are separately bound in its readiness envelope.
