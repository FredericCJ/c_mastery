# PR-T02-02: variadic traversal and expression semantics

This is the executable successor for the nine unchanged T02.06–T02.08 criteria
in the project assessment card. The original project, fixture, objectives,
historical hours and accepted evidence remain intact. Follow the card's ordinary
tasks, then attempt its separately administered changed conditions after at least
seven days. Author reference execution does not establish learner performance.

Use hosted **C17, profile A** on the recorded Linux x86-64 toolchain. Scalar
`snprintf` conversion, a caller allocator, CRT and libc are allowed at this
boundary. Traversal, capacity admission, transactional publication, arithmetic
checks and step ordering are your implementation. Do not forward a whole
`va_list` to `vprintf`/`vsnprintf`. Python oracles are host verification tools.
The native campaign records GCC/Clang and optimization/sanitizer identities and
audits actual ELF dependencies. Hypothetical integer models are reasoning
exercises, not evidence of executing those targets.

Prerequisites: the T01 object/lifetime, bounds and workflow exercises; PR-T02-01
compatible prototypes, adjusted parameters, callbacks and unit wrappers. Bring
your implementations and a prediction notebook. Implement `learner.c` against
`format.h` and `expressions.h`; `scalars.c` is supplied permitted conversion
support. Public `tests.c` is an inspectable minimum, not a complete assessor.

Run `python labs/variadic_expressions/run.py --cc gcc --build build/my-tf`
inside the project export. Its default unfinished implementation must fail.
Submit your source, independent tests, build/diagnostic log, dependency manifest,
promotion ledger, typed expression trees, sequencing graphs and one minimized
wrong implementation for each distinct mechanism you claim to understand.

## Traversal is state, not a bag of values

Default argument promotions apply after the ellipsis: `float` becomes `double`;
integer types below `int` rank become `int` if its range contains every value of
the original type, otherwise `unsigned int`. Neither tags nor casts inside the
callee recover the actual types. Read an `I` payload with `va_arg(..., int)` and
a `D` payload with `va_arg(..., double)` only when the caller satisfied that
contract. Missing payloads and incompatible types are not malformed byte streams
that this interface can safely inspect. They are caller violations, analyzed
without execution. Account for the precise standard exceptions when classifying
corresponding signed/unsigned arguments; do not infer that all type differences
are equivalent.

`va_start` creates traversal state and `va_arg` advances it. A sizing traversal
cannot be replayed by assigning arbitrary `va_list` objects or reusing consumed
state. Construct independent copies with `va_copy`; every start/copy must have
its matching `va_end` in the same function on every return path. A helper that
borrows a pointer to the owner's list can copy it locally without consuming the
owner's traversal. Draw these lifetime arrows, including capacity, conversion,
allocation and empty-list exits. The observed array-shaped native representation
is an ABI fact; it is not an ISO C representation guarantee.

## Formatter and array contracts

`tf_format` takes an unsigned count in 0..4, followed by exactly that many pairs.
Each pair is tag `'I'` (type `int`) then an `int`, or `'D'` then a `double`.
Concatenate `I=<decimal>;` and `D=<fixed two decimal places>;` using the C locale.
For example, one `I` value 3 needs four text bytes and one trailing NUL. No spaces
or separators beyond those shown are added. `required` excludes the NUL.

`tf_array` takes the same logical values in `struct tf_arg` records and checks
`count <= available` before accessing them. The record has separate integer and
real fields; it does not pretend to validate an active union member. Count zero
permits a NULL array. A nonempty array must designate at least `available` live
records. That pointer extent and their lifetime remain caller obligations.

In both interfaces, a nonzero capacity requires a live writable destination of
that extent; NULL is permitted only at capacity zero. `required` designates a
distinct live `size_t`. The allocator callbacks and both scalar callbacks must
exist even for zero count. Buffers, array storage, `required`, and callback
contexts must not overlap incompatibly or mutate each other. Execution is
serialized. No cancellation, longjmp, exception-like escape or concurrent access
is admitted through a callback.

| Result | Required observation |
|---|---|
| `TF_INVALID` | Invalid known argument/count/tag, missing callback, or array count beyond available; destination and required unchanged. Reject count >4 before unnamed arguments. Reject an unknown tag before consuming its unknown payload. |
| `TF_FORMAT` | A scalar conversion reports negative length, or the writing pass reports a length inconsistent with its sizing pass; destination and required unchanged. |
| `TF_SIZE` | A text length plus final NUL cannot be represented by `size_t`; destination and required unchanged. |
| `TF_FULL` | Capacity <= text length; publish required length and leave every destination byte unchanged. No allocation or writing pass. |
| `TF_NOMEM` | Sizing/admission succeeded but staging allocation failed; destination and required unchanged. |
| `TF_OK` | Publish precisely the text and NUL, publish required length, leave bytes beyond the NUL unchanged, release all staging ownership. |

Known-argument admission precedes list access; tag/conversion/size failure during
left-to-right sizing precedes capacity admission, then allocation, rendering,
publication. Successful sizing at zero capacity is a query returning `TF_FULL`.
Count zero needs one byte to succeed; this implementation contract still
allocates and releases a one-byte empty stage. Do not silently special-case away
the allocation-failure exercise.

The allocator returns a fresh disjoint writable byte region of the requested
size or NULL, and `release` consumes it exactly once. Each scalar callback has
the corresponding scalar `snprintf` sizing/truncation contract: at capacity zero
the destination may be NULL; success returns the full length excluding NUL;
negative signals conversion failure. It never writes outside capacity. Under
ordinary operation it produces stable conversion text/length across passes.
Test doubles may report negative or changed length after sizing to investigate
containment. A callback that writes out of bounds violates an uncheckable
precondition. Length consistency checks cannot establish the callback's integrity.

First derive total length and prove that reserving a NUL cannot wrap. Allocate
one stage, render with independent traversal, and publish only after all
conversions succeed. If a second-pass callback fails, free the partial stage;
earlier callback context effects are observable and are not rolled back. Excluding
conversion-backend costs, work is O(count + total text bytes), temporary heap is
total+1 bytes, traversal metadata is O(count), with count bounded by four.
Discuss why replacing staging with direct destination writes changes the failure
contract. A finite stack buffer is not a repair for all representable `double`
outputs unless a justified format/target bound is part of the contract.

## Trees, conversions and sequencing

For T02.07, distinguish the syntax tree of `a+b*c` from the evaluation graph.
The multiplication result is an input to addition; this does not order the
evaluation of `a` before `b`. In `condition ? left : right`, the condition is
evaluated before the selected operand, only one operand executes, but both
operand types participate in selecting the result type. A cast after an
overflowing signed addition cannot retroactively admit it.

Annotate every node with operand promotions, common type, mathematical range,
conversion and possible failure. State rank as well as width. Compare a model
with 16-bit `int`/16-bit `unsigned short`, a native-style 32-bit `int`/16-bit
`unsigned short`, and a hypothetical 64-bit `int` with 32-bit lower-rank exact-width
types. The last is not merely a different typedef spelling on the same ABI.

`tf_narrow_signed` admits 0..UCHAR_MAX before publishing an unsigned char;
`tf_narrow_size` admits values <=UCHAR_MAX. NULL output returns `TF_INVALID`;
out-of-range returns `TF_SIZE`, preserving output. Explain what information the
second interface has already lost if a negative source expression was converted
to `size_t` before the call. Rejection of a large converted value does not restore
its provenance.

For T02.08, draw individual value computations and side effects. A comma operator
and `&&` supply sequencing edges; a comma separating arguments is not a comma
operator. Function bodies are indeterminately sequenced relative to other
otherwise unordered evaluations in the caller. Distinguish that rule from
unsequenced modifications in two argument expressions. Valid unspecified order
is different from undefined conflicting evaluations. Record permitted trace sets
before observing any compiler's choice.

`tf_steps(a,b,c,after_a,context,progress)` chooses the explicit order A, gate, B,
C and then computes A+B*C with checked `int` intermediate operations. Callbacks
return `TF_OK` and one valid int, or an error without a usable value. The gate
has no value. Validate function pointers and progress before any callback;
invalid admission preserves progress. After admission set completed=0; increase
only after each successful value callback. Propagate the first callback/gate
failure. Overflow returns `TF_SIZE`. Preserve the old result on all failures,
but retain completed count and callback effects. Success publishes the new result
and completed=3. Overflow admission occurs after all three value callbacks;
explain why this cannot undo their effects. Resource cost is constant apart from
callbacks, with no allocation in this component.

## Assessment and graduated hints

For each of the nine retained criteria, first predict its exact ordinary task,
then supply construction/analysis, a discriminating boundary or faulty specimen,
and a changed-assumption derivation. A score of 1 reproduces a supplied case;
2 requires correct mechanism, invariant and independent counterexample; 3 also
requires independent transfer with revised assumptions and honest evidence limits.
Passing the public tests alone cannot establish scores 2 or 3.

If stuck: (1) label the promoted types and owners before writing code;
(2) mark the exact point at which state/output becomes observable;
(3) for ordering, remove source-layout assumptions and add only standard-guaranteed
edges; (4) for checked arithmetic, prove each comparison is itself representable.
Use the retained assessment card's criterion-specific hints and tasks. Assessor
solutions and delayed prompts are distributed separately.

Verification must include empty and four-pair inputs; char/short/float calls;
all capacity boundaries around independently predicted text; count/tag rejection;
allocation and both-pass conversion failure; exact-once cleanup; array admission;
all arithmetic sign combinations and extrema; short-circuit and conditional branch
nonexecution; permitted ordering sets; gate and callback failure at each step.
Keep undefined examples compile-only. In this local original program, success is
observable text/state/resource correctness, not an exploitability claim.

Sources actually inspected: WG14 **N1570 (C11 draft, not the published C17 text)**,
5.1.2.3p3;6.3.1.1;6.3.1.3;6.3.1.8;6.5p1–5;6.5.2.2p6–10;6.5.13p4;
6.5.15p4–5;6.5.17p2;7.16.1.1–4;7.21.6.5. Exact locators and source bytes are
registered in `research/v5/expansion/legacy_variadic_expressions/resources.json`.
The active implementation baseline is C17; no C23 portability claim is inferred
from these experiments. Published-edition access, external review, independent
learner work and delayed performance remain separate evidence obligations.
