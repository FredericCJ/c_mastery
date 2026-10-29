# Entry diagnostic: traces, invariants and finite representations

This is the executable successor for the six original PR-T01-01 criteria and
their six changed-condition assessments. Their AX identifiers, tasks, seven-day
delay, baseline fixture and historical hours are retained. Read the original
project and `curriculum/v5/expansion/legacy_assessments/shared-contract.md` first.
Use the corresponding criterion ID as the heading of each submitted dossier.

## Environment, prerequisites and permitted mechanisms

Use C17, hosted Linux, eight-bit bytes, available exact-width `int32_t` and
`uint32_t`, and `INT_MAX == INT32_MAX`. The reference enforces these assumptions
with static assertions. This is an explicit execution profile, not a claim that
ISO C requires that data model. The earlier synthetic sixteen-bit fixture remains
a separate mathematical exercise. No C23 feature is needed or inferred.

Prerequisites: integer comparisons, array bounds, function calls and the original
PR-T01-01 prefix specification. Supply your own `f_prefix` implementation through
`--foundation`; do not substitute a library sum or allocate inside that fixed
primitive. Supply the functions declared in `entry.h` by editing `learner.c`.
The public runner does not discover assessor implementations. Host stdio and
Python are harness dependencies. The component uses loops, local objects,
checked arithmetic and the explicitly supplied allocation callbacks only.

Run from the exported workspace:

```
python labs/foundation_entry/run.py --foundation /absolute/path/to/your/foundation.c --build build/entry
```

The default scaffold compiles and fails a semantic assertion. Implementations
must pass the public tests, independent cases and the reasoning obligations
below. Retain compiler versions, exact commands and source identities. Tests
cannot assess the truth of a complexity or language explanation by themselves.

## Representation and ownership contracts

`enum fe_status` separates capacity, arithmetic, invalid-argument, allocation and empty
results. All status-returning operations preserve the caller's result on failure.
Trace records are explicitly observation outputs: after admission they reset and
may contain a partial history when unwinding fails. A failed call before admission
preserves the old trace. Trace entry has `entering=1`, current index and value zero;
a successful return has `entering=0` and the suffix value. Failed activations do
not invent successful return events.

Input and output objects must be alive and sufficiently large after scalar
admission; C cannot validate arbitrary pointer extents. Control objects and the
trace are disjoint from arrays. Null arrays are permitted for zero-length sums.
No concurrency, signals or callbacks may modify the input during an operation.
Caller ownership lasts throughout the call; no borrowed pointer escapes it.

* `fe_fixed`: call the supplied `f_prefix`, preserving its sixteen-element limit,
  checked prefixes, arbitrary overlap and publish-after-success contract.
* `fe_recursive`: for at most sixteen values, evaluate
  `S(i) = input[i] + S(i+1)` with `S(n)=0`; check each addition before evaluation.
  The base case is one live activation. Publish the sum only on full success.
* `fe_explicit_stack`: implement that same evaluation and trace with at most
  seventeen explicit records and no C recursion. Relate each record to one
  suspended activation. Merely reversing a loop without this relation is not
  sufficient evidence for the transfer criterion.
* `fe_two_pass`: remove the sixteen-element bound. First check all prefixes;
  only then compute and publish. Arrays must be disjoint or start at exactly the
  same address. Arbitrary partial overlap is outside this contract. Explain why
  exact in-place publication does not destroy a future unread element.
* `fe_staged`: remove the bound while allowing arbitrary array overlap. Reject
  `n > SIZE_MAX/sizeof(int32_t)` before allocating or accessing array elements.
  For admitted nonempty input, `take(context,n)` returns null or a disjoint live
  array of at least `n` `int32_t` objects. On success the component owns that
  temporary allocation until exactly one `release`, including arithmetic-failure
  paths. Callbacks must return normally and preserve the input. Empty input
  makes no allocation; failed allocation preserves all output. The classroom
  test allocator has 64 elements, but this is not an API capacity limit.
* `fe_maximum`: empty input returns `FE_EMPTY` and preserves the result; nonempty
  input returns its greatest element. A null result pointer is invalid even
  when empty. Zero is not an invented maximum for a negative-only range.
* `fe_encode24`: admit a nonnull destination of at least three octets and a value
  at most `0xffffff`, then write exactly three most-significant-first octets.
  Invalid pointer, insufficient capacity and excessive value are checked in
  that order. Failure preserves storage. Never serialize a C object by casting
  its address; object byte order is not this wire contract.
* `fe_any`, `fe_all`, `fe_forbidden_clear`: return exactly zero or one. Respect
  the empty mask; “all selected bits present” and “no forbidden bit present” are
  different predicates even when their mask argument is the same.
* `fe_saturating_add`: mathematical addition clamped to the signed 32-bit bounds.
  Check bounds before performing C addition. Saturation is a specified policy,
  not the default behavior of signed C overflow.

## Criterion progression and required evidence

| Objective / original AX suffix | Construction and diagnosis | Evidence and changed assumption |
|---|---|---|
| T01.01.a | For `[3,-1,4]` and empty input, write iterative states before and after each transition, recursive enter/return events and live activations. Implement the trace convention. | Compare hand predictions with both implementations, including all 34 events at length 16; after the recorded delay derive the bounded explicit-stack correspondence without copying the prior source. |
| T01.01.b | Separate input/output, sixteen-element staging, scalar loop state, trace output and recursive storage. Count additions and traversals before using asymptotic notation. | Implement the two unbounded strategies. Test length 17, allocation refusal, arithmetic rollback, exact in-place and overlapping storage under their different contracts. Explain the time/space/aliasing tradeoff. |
| T01.01.c | Prove initialization, preservation, exit and a decreasing measure for staged prefixes. Diagnose `[INT_MAX,1,-1]` without executing signed overflow. | Retain sentinel bytes after rejection; derive and implement a total maximum-selection contract for empty, singleton, negative-only and extreme-valued ranges. |
| T01.02.a | Convert decimal 173 to binary and hexadecimal, reconstruct by positional weights, distinguish numerical value from two-octet order. | Derive the three-octet big-endian codec, including `0x1234ad`, zero, maximum, capacity two and one-past-maximum refusal. Explain why a host byte-order change cannot change the specified wire bytes. |
| T01.02.b | Complete the mask-12 truth table for neither, each and both selected bits; give a plausible incorrect any/all explanation and its smallest counterexample. | Implement forbidden-permission rejection, including mask zero and combinations of allowed/forbidden bits. Construct an independent per-bit oracle rather than reusing the implementation's bitwise expression. |
| T01.02.c | Compare unbounded mathematical cancellation, every bounded signed prefix and unsigned arithmetic modulo its range. | Implement checked saturation and exhibit unequal results from reassociation. Explain exactly which integer model permits each transformation. |

For each row submit (1) predictions and the source-level argument, (2) code or
explicit mathematical analysis, (3) raw observed tests with an independently
chosen counterexample, and (4) the changed-contract derivation. Include a failed
candidate and explain the first violated invariant, rather than only its symptom.
The trace of a suffix sum need not share the overflow admissibility of a prefix
sum: their intermediate values differ. This is a useful falsification exercise.

The public corpus includes all 65,536 eight-bit value/mask pairs using a separate
per-bit membership oracle, and 343 triples whose prefix/suffix expectations are
direct 64-bit formulas. These finite results do not establish arbitrary-length
correctness. Source proofs must justify the remaining ranges and allocator
contract. Width changes require revisiting the implementation assumptions.

## Rubric and hints

Each ordinary criterion needs level 2; its delayed criterion needs level 3.
Level 0: wrong result, undefined operation, lost output, leaked stage or false
language rule. Level 1: supplied examples pass but explanation omits the row's
mechanism (activation, aliasing, induction, weights, mask quantifier or arithmetic
model). Level 2: derives that mechanism, implements the admitted contract and
independently falsifies the named misconception. Level 3: independently derives
the changed condition, revises the invariant/resource account and demonstrates
where an old proof no longer applies. A green executable with a false causal
explanation fails the affected criterion. See the original exact rubric too.

Hint 1: label a state as “before” or “after,” and keep a count of completed work.
Hint 2: ask whether a write can destroy an input not read yet, and whether the
final mathematical result says anything about each intermediate C expression.
Hint 3: separate output storage, work space and observations; separate the set
selected by a mask from the predicate applied to that set. Work out an empty
case before writing a general loop.

## Sources, containment and evidence limits

The source register is
`research/v5/expansion/legacy_foundation_entry/resources.json`. Inspected N1570
is the C11 committee draft, specifically 6.2.5p9, 6.5p5 and 6.8.5.3p1–2, not
the published C17 standard. These passages support the unsigned, exceptional
expression and loop distinctions. Contracts, proofs and finite fixtures here are
original teaching material; no claim of having read CLRS or EWD249 is inferred.

Run only original local teaching or learner programs with synthetic data.
Signed-overflow counterexamples are reasoned about and rejected before the
operation; no undefined-output golden result or exploitability claim is used.
This hosted package makes no freestanding closure, physical measurement,
independent reviewer, learner mastery or completed delayed-assessment claim from
its native tests alone. The separately selected M4 profile below supplies bounded
cross-target evidence. Current execution is recorded in source-bound reports.

### Component dependency and changed-target audit

For profile C, the component's named predecessor call is `f_prefix`; explicit
allocator callbacks have the ownership contract above. Compiler-generated memory
calls are allowed only when resolved to the original inspected byte routines in
`assessor/execution/support.c` for author execution, or an explicitly supplied
equivalent learner runtime. The public target builder requires `--source`,
`--foundation`, `--runtime` and `--driver`, never guesses these providers.

The changed target is Cortex-M4 soft-float, ILP32, little-endian C17 on QEMU
`mps2-an386`, with 256KiB ROM and64KiB RAM. Original reset code masks interrupts,
copies data and clears BSS; semihosting is a named observation/exit boundary.
Direct `arm-none-eabi-ld` links five explicit objects and no runtime archive.
Inspect every object's undefined symbols, final map, disassembly and remaining
undefined symbols. The private driver calls every API, exercises recursion at
length16 and checks rollback; it does not execute the whole hosted oracle corpus.
The stack audit includes at most17 `recursive` frames, all other live C frames
once, and1024 bytes allowance, under the selected callback/IRQ contract and32KiB
reserve. This conservative compiler-frame sum is not a measured high-water mark.
Compiler instrumentation changes the native diagnostic dependency profile; do not
claim its sanitizer runtime is part of the portable component.
