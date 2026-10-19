# T19 generic/variant assessor analysis

T19.04.a: The controlling expression does not run; the selected function call's
argument runs once. An argument expression in both syntactic positions therefore
does not imply two evaluations. Unselected associations remain subject to semantic
constraints, so select function identifiers rather than embedding invalid accesses.
The exact-int facade deliberately diagnoses double; direct C int parameters permit
conversion and offer a weaker check. Array-to-pointer conversion loses extent;
`&array` preserves it. Top-level pointer const is removed but pointed-to const
remains part of the pointee type. Record all three edition observations separately
and cite DR481's correction without relabeling the C11 draft as published C17.

T19.04.b: At first clone success, source owners2, old destination owner0(integer)
and staged owners1 exist. Second allocation failure creates no new owner; rollback
drops exactly that staged1. Destination count1 and int7 remain. A successful retry
adds two distinct destination allocations. Representation relocation transfers the
staged obligations; no destructor runs on those moved staging slots. Self-pointer
elements would violate this relocation contract and require a move/relocate hook.
The typed arrays avoid a false claim that aligned unsigned-char storage acquires
another declared type. `storage_alignment` is a caller-provided fact whose numerical
compatibility can be checked; it is not verification of arbitrary pointer alignment.

The move-only task fails before writing either operand. Success copies the selected
representation then makes source inert; destination must initially have no owned
resource. A valid alternative swaps an existing destination owner into the source,
but that is a different public contract and must be explicitly versioned. Double-
drop suppression based on a global pointer table hides a broken transfer protocol.

T19.04.c: Examine exact text/data sections and component call sites for32 compiler/
optimization/LTO/form combinations. The linked executable also includes hosted
printf and startup code; whole-image indirect counts alone are not dispatch counts.
The generated form may inline readily; erased hooks may devirtualize if the
compiler sees constant descriptors. Neither outcome is a C semantic guarantee.
Heterogeneous form3 has richer state and a different type contract; explain that
its larger size cannot be attributed solely to erasure. Staging storage and owned
allocation costs are separate from descriptor bytes included within each instance.

T19.05.a: A tag authorizes one union-member interpretation under an invariant.
Embedded zeros are ordinary TEXT bytes because extent is explicit. Empty TEXT is
success and differs from ERROR. OWNED cloning allocates/copies, borrowed cloning
extends no lifetime, and a move changes the release-obligation owner. Failure
preserves the destination; successful overwrite of an already-owning destination
is forbidden by the constructor contract. Runtime validation cannot prove the
borrowed pointer is live or the allocator context will outlive later drops.

T19.05.b: `-Wswitch-enum` catches a deliberately omitted AV_OWNED in tested compilers;
invalid99 is rejected before payload reads. A single tag/name list ensures declared
metadata shares the enumeration but cannot verify branch arithmetic or ownership.
The compiled range-check mutant is a concrete semantic counterexample despite
all tag names and cases existing. Do not present generated enumeration consistency
as exhaustive proof of variant safety.

T19.05.c: Prefix-limit check uses quotient/remainder before multiplication. A
negative limit32768 differs from the positive32767 limit. Conversion of negative
minimum avoids casting32768 to an insufficient signed int or negating INT_MIN.
The static range premise excludes unsupported int representations. Invalid-byte
and overflow offsets are the first failure in left-to-right parsing. Python's
unbounded decimal prefix interpretation provides an independent oracle, not a
translation of the fixed-width quotient algorithm. All result variants are valid.
Accumulation preserves each independent field's offset; within one field, a first
failure terminates that parse because later overflow positions lack an established
valid prefix. A different requirement for multiple lexical errors within a field
needs a separate result shape and should not be silently inferred.

Use exact ordinary/changed0–3 rubrics, not one aggregate pass score. Ask for a
self-referential element counterexample, a failed copy that acquired resources
before returning failure, and an expression whose generic arm is unselected but
invalid. These reveal missing contracts that nominal integer tests do not. Delayed
learner and external review evidence remain distinct from author experiments.
