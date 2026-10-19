# Private PR-T03-04 analysis

This is assessor-only. It supplements unchanged original tasks; successful author
examples do not substitute for delayed learner attempts. The baseline permits
hosted allocation but does not make library behavior equivalent to ISO rules.

## T03.11.a

The record's int/pointer/char fields are semantic state; padding is not. Compare
members only after the initialized mask reaches7. Assign NULL as a typed pointer
value, without a zero-bit representation premise. The delayed serializer writes
exactly four deliberate bytes for number0..65535, tag and pointer presence;
remaining0xa5 sentinels survive. It neither copies padding nor emits addresses.
Two equal records need not have identical object representations. Reject a
whole-record memcmp used as semantic equality or a raw struct dump as a portable
protocol. N1570 6.2.6.1p4–8 is the inspected authority for this distinction.

## T03.11.b

The external mask is initialized before stores; bit1 establishes number, bit2
establishes pointer and transfers its ownership, bit4 establishes tag. The early
failure after number leaves pointer unread; cleanup branches on the initialized
mask before accessing it. Failure after pointer transfer releases that child.
Clearing the mask consumes ownership, without subsequently comparing the freed
pointer. The CLEANUP control emits a false release event without actually reading
uninitialized state, so a semantic assertion detects the defect safely.

N1570 distinguishes automatic indeterminate values, character observations and
unspecified padding. Issue0451 is a closed committee response discussing unstable
values and a still-desired better definition, not a blanket new normative license
to inspect arbitrary indeterminate fields. Require the delayed response to prove
which reads are established on every failure edge; a final leak count is insufficient.

## T03.11.c

The original warning-only function conditionally assigns a local int. It is never
called. Actual GCC/Clang O0/O3 output records warning dependence on compiler and
optimization. The separate malloc-int target's branch0 is deliberately undefined
and has no golden value/exit oracle; branch1 initializes17 and is defined. Debug
Memcheck must report the originating uninitialized conditional use. The observer
retains any optimized disappearance and sanitizer silence with their exact flags.
ASan's spatial/lifetime and UBSan's selected checks do not promise general
initialization tracking. In the actual selected Clang O3 experiment, main becomes
a constant volatile sink write and return: allocation, branch and load disappear,
and Memcheck is silent. GCC retains the conditional load and reports it at O0/O3.
Clang's compile-only local warning appears in both modes while GCC's is silent;
both sanitizer runs miss the uninitialized heap use. These are versioned observed
outcomes, not requirements for future compilers. Retained disassembly explains
the optimized false negative. Memcheck tracks definedness at executed machine accesses;
an optimizer can remove an access before its instrumentation sees it. The delayed
task changes both optimization and initialized-branch coverage, not a constant
that leaves the same defect path. Positive core tests also pass Memcheck.

## T03.12.a

Check prefix<=limit and count<=(limit-prefix)/width with width>0 before forming
the product. Actual allocation uses sizeof(ObFlex)+capacity; 16+4×0=16,
16+4×3=28, and16+4×100 exceeds255. The independent Python oracle uses unbounded
arithmetic and320 boundary tuples, including SIZE_MAX. Logical zero payload
grants no element access even if physical tail padding would fit a byte.

The delayed ObPadded layout has double/char/flexible bytes: on the measured
native ABI sizeof16 and offset9, on i386 sizeof12 and offset9. These are observations,
not universal ABI promises. Allocate sizeof plus the admitted logical count,
initialize only those payload members, and do not infer extra capacity from
padding. N1570 6.7.2.1p18–25 also explains why fixed-struct assignment does not
copy arbitrary flexible members. SIZE omits the payload term and fails the
ordinary fixture before any undersized allocation is used.

## T03.12.b

Admit signed n in1..16 and caller capacity before declaration. The local bound
function increments one evaluation counter; sizeof the established VLA gives
the original length and does not reevaluate that declaration's expression.
Initialize every element, compute n(n+1)/2 within int, then end the block without
escaping a pointer. The caller-storage policy supplies the same result with no
VLA declaration and zero bound-helper evaluations.

The changed `int values[static n]` parameter adjusts to pointer type; callers
must already supply n valid elements and a positive bound. It does not allocate
a second array or admit invalid calls dynamically. N1570 6.7.6.2p4–5 and
6.7.6.3p7 are the actual source. N3096 conditional macro text narrows unsupported
VLA coverage to automatic arrays and expressly mandates adjusted parameter
declarations. Tested C99/C11/C17/C23 frontend modes are separate observations;
published final standards were not inspected. `OB_NO_VLA` is a project policy,
not a counterfeit compiler feature macro.

## T03.12.c

ObFlex allocates header+payload once. ObSplit allocates its header, then nonzero
payload separately; failure of the second step releases only the new header
and preserves output. Both publish length0, initialize admitted capacity bytes
and allow the same payload values. The allocation-count and copy/lifetime
comparison is explicit; tests do not infer cache performance from adjacency.

The delayed ObShared copies immutable bytes once and has up to three counted
owners. Retain increments before publishing the new handle; exhaustion leaves
out unchanged. Drop consumes that handle then releases storage at count0.
Separate const borrows require a remaining owning reference. SHARED omits the
increment and fails before any double release. This single-threaded design does
not imply atomicity, safe concurrent publication or enforced immutability.

## T03.13.a

The resize callback receives only a nonzero total size and has ordinary realloc
failure semantics. On NULL the old owner/data/capacity/generation remain valid.
The core publishes the temporary only after success and never evaluates the
old pointer again. The forced-move callback allocates before freeing, preserving
the lesser extent; the actual realloc callback snapshots integer address bits
before the call and records whether the new address matches without reading
the invalidated old pointer. Success invalidates old aliases even in the observed
same-address case. RESIZE falsely advances generation on failure and is rejected.

Delayed zero-size handling is explicit policy: resize0 rejects before invoking
the callback, while reset releases and permanently retires identity. The inspected
C17 issue0400 corrigendum permits implementation-defined zero-size deallocation
and marks that feature obsolescent. N3096 7.24.3.7p3 makes size0 with a nonnull
old allocation undefined, while its null-pointer branch follows malloc. No
zero-size realloc is executed, and no final C17/C23 text inspection is claimed.

## T03.13.b

Append within capacity does not move existing bytes or change generation; prior
element views remain valid, but a snapshot slice length does not grow itself.
Failed resize changes nothing. Successful resize invalidates all old raw views
and handles. Reset ends allocation lifetime. Erase moves following logical
elements within a still-live allocation: the test's pointer to slot1 remains
address-valid but changes from logical value2 to3. This distinction is why erase
also advances the generation even without realloc. ERASE suppresses that advance
and fails the explicit identity assertion. The changed task must explain both
the remaining valid C address and the invalid logical iterator, not call every
mutation a dangling pointer.

## T03.13.c

Handles carry unique logical owner identity, generation and index, with no old
allocation pointer. Resolve checks current live owner, identity, generation and
length before reading. Generation3 is terminal for further invalidating mutations;
the operation refuses unchanged instead of wrapping. Reset retires the complete
identity even if generation headroom remains. The external identity authority
must not reuse an ID while any old token might exist; C cannot enforce this.

The delayed control-table move transfers the actual allocation and metadata
into an empty live destination and consumes the old control's ownership. Existing
tokens resolve only through the updated explicit locator; no pointer to the old
table is retained in them. Copying the table without consumption would double-own
the allocation. A stale control pointer may still be address-valid but cannot be
used as the current registry. GENERATION demonstrates a logical stale-token read,
not a freed-memory exploit. Independent625 histories cover finite mutation
combinations, not all future identity assignments or arbitrary caller misuse.

## Limits

Truthful callback storage/alignment, unique IDs and borrow withdrawal remain
preconditions. No physical target, independent learner, external review or complete
initialization/provenance proof follows from this campaign. The deliberately
undefined diagnostic source is confined to its original short-lived process and
never supplies expected program values to the correctness suite.
