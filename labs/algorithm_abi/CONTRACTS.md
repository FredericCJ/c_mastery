# Paired lower bound and ABI transfer contract

Version 1. This required successor implements the six existing criteria
`AX-PR-T36-02-T36.03.a` through `.03.c` and `.04.a` through `.04.c`, including
their exact `-V1` delayed identities. The canonical project, leaf rubric and
seven-day delay remain authoritative. Workload is additive; accepted hours
and historical receipts are unchanged. Families A/B/C/D/F/G/H/I/J are selected
by the existing criterion. The controlled overread adds the local containment
rules below; it does not establish an exploitation capability.

## Prerequisites and supplied artifacts

Read the parent project, T36.03/.04 teaching, `labs/algorithms/SPEC.md`, the
scalar `algo.h` interface, and this contract. Bring the prior C lower-bound
learner implementation and its admission/invariant argument. Bring separately
authored SysV x86-64 and Cortex-M4 Thumb source files, not compiler-generated
assembly offered as independent handwritten implementations. The supplied
baseline assembly files are unsolved scaffolds. `bound.h`, `learner.c`, both
`learner_*.S`, native tests, original ABI probes, callback/poison scaffolds,
Thumb startup and linker script are public. Assessor implementations remain
withheld. The host Python fixture/oracle is not a target dependency.

## Environment and dependency boundaries

C17 remains the C baseline. GNU assembler syntax, GCC/Clang `ms_abi`, Linux
page mappings/fork/wait and Arm semihosting are explicitly outside ISO C.
The original kernels use profile C: no allocator, libc, OS call or implicit
runtime helper. The C certificate/admission layer may lower aggregate copies
to `memcpy`/`memset`; audit its actual undefined symbols and bind the original
byte-loop helper source when linking the freestanding image. The hosted test
process is profile A. The MPS2 Cortex-M4 image has a separate original reset,
linker and semihost harness, not a firmware board-driver claim.

Use SysV AMD64 Draft0.99.6 sections3.2.1–3.2.3, AAPCS32 2025Q1 core registers,
stack and argument rules, and Microsoft's x64 calling-convention defaults,
parameter passing, return and saved-register sections. Exact inspected
locators are in the successor resource register. The Microsoft variant is a
leaf native ELF function called through GCC/Clang `ms_abi`; it exercises the
register/shadow-store interface, not Windows image loading or exception
unwinding. Do not silently call it through an ordinary SysV function pointer.

## Problem and operations

Implement `algo_asm_lower` independently for both original architectures.
For sorted, live `uint32_t a[n]`, `n<=1024`, return the first index whose key
is at least `key`, or `n`. Empty input permits NULL and performs no load.
The array remains immutable throughout a call. For fixture
`[0,7,7,0x80000000,UINT32_MAX]`, keys0,1,7,8,UINT_MAX must return0,1,1,3,4.
The first occurrence, not any occurrence, is the contract.

Implement `ab_trace` to return the complete half-open interval certificate.
Its `upper=1` variant returns the first value strictly greater than the key;
the ordinary mode is lower bound. Stage output: invalid control, count>1024
or missing nonempty storage returns1 without changing output. `ABTrace`
holds12 steps, sufficient for the admitted count. Sortedness is a trusted
precondition of this certificate generator; it is not a sorting validator.

Implement `ab_checked`: validate nonnull outputs, `trusted`0/1,
count<=1024, count<=capacity and nonempty storage. In untrusted mode, reject
the first descending adjacent pair before invoking assembly. Success returns0,
publishes the result and number of sortedness comparisons; all rejection
returns1 with both outputs unchanged. With `trusted=1`, the caller has already
validated the same immutable array and extent; that flag cannot create this
fact. No validation comparisons then occur. Output objects are disjoint from
each other and input, have valid lifetime/alignment, and are not concurrently
accessed. An arbitrary pointer's allocation extent cannot be inferred from
its numeric value or from the caller's capacity integer.

Implement `ab_callback_lower` for both architectures. An `ABRead` callback
returns exactly `a[mid]`; it may update its own supplied context but must not
mutate the array, reenter the search, retain borrowed storage, throw or leave
nonlocally. Every iteration calls once, with `mid<n`; an empty input calls
zero times. Preserve array/key/bounds/midpoint/context/callback across the call
according to the ABI. The supplied original callback checks stack alignment
and poisons caller-scratch registers using either of two context patterns.

Implement the Microsoft x64 leaf `ab_ms_lower`, using the first three integer
argument positions and scalar return convention. Exercise the caller-reserved
home storage without altering nonvolatile state or making another call.

Implement `ab_records_lower` for sorted eight-byte `algo_pair` records and
`size_t` counts. Its checked facade rejects `n>SIZE_MAX/sizeof(algo_pair)`,
`n>capacity`, null nonempty storage or absent output before indexing; also
reject descending keys. Result and rejection semantics match lower bound.
This delayed interface uses the native size_t width (64 on this x86 ABI,
32 on the selected Arm ABI), not a silently truncated uint32 count. Live
storage and disjointness remain caller obligations even after arithmetic
checks. Never manufacture huge storage to execute a nominal overflow case.

## Necessity and required reasoning

Explain why a correct returned index cannot excuse register corruption,
stack misalignment or a load from `a[n]`. Derive `0<=lo<=hi<=n`, partition
predicates outside `[lo,hi)`, and strict decrease of `hi-lo`. Derive midpoint
without overflowing `lo+hi`; bind each four-byte load to `mid<n`. For records,
prove eight-byte scaling after the size admission. Empty and one-past result
are different from permission to load a one-past element.

Classify unsigned branch comparisons separately from bit patterns interpreted
as signed values. Distinguish argument registers, caller scratch, callee saved
registers, return state and the platform-specific r9 rule. Enumerate live
values at each nested call and explain all prologue/epilogue stack changes.
ISO C does not enforce hand assembly register discipline. Compiler diagnostics,
the selected ABI, disassembly review, runtime probes and programmer obligations
provide different evidence.

## Deliverables and verification

D1: A load-by-load interval proof, full fixture certificates and complete
register-liveness tables for both architectures and the callback transfer.
D2: Both original assembly kernels, admission/trace facades, callback kernels,
Microsoft leaf and eight-byte record kernels, plus owned source identities.
D3: Actual prior C comparison, independent linear/bisect oracle, all boundary
observations, unchanged failure outputs, and the first discriminators for
signed branch, register clobber, wrong stride and overread separately.
D4: Actual native and emulator executions, compiler versions, assembly/object
and final image inspection, unresolved-symbol reports and link maps. Show
what helper each layer requires. A successful cross-compile is not execution.
D5: Changed-poison and guard-layout results; equivalent-contract cost account
for one-time validation followed by repeated searches over immutable data.

Run every count0..1024 on the declared synthetic repeated-key workload with
five keys. Validate every supplied interval against an independent partition
check. Compare the original C algorithm with the original assembly and with
the independent linear result; do not describe two translated copies of
binary search as an independent mathematical proof. Run the guard queries
in separate children for empty and final-word input, at two page layouts.
Run the actual callback poison adapter and preserved-register probes at both
architectures. Inspect generated Microsoft call sites for home-space setup.

For costs, count sortedness comparisons and search decisions separately.
No timing result follows from these counts. Include validation in both APIs
when comparing one-shot untrusted queries; charge a reusable validation once
only if immutability/lifetime preserve it for every later query. Discuss the
cost of invalidating that trust if the array can mutate.

## Rubric and graduated hints

Use each existing leaf's four-level rubric independently. Level0 includes a
wrong relation or missing admitted case. Level1 reproduces answers without
the associated load/register/ownership argument. Level2 supplies D1–D5 and
the criterion's first actual discriminator. Level3 completes its exact delayed
variant unaided and constructs a new counterexample to an overstated claim.
Passing code alone cannot supply a missing proof or independent performance.

For .03a, assess independently authored sources and both executed boundaries;
for .03b, every live call-crossing value and restoration; for .03c, distinguish
the ABI failure from the protected-page fault. For .04a, check the entire
interval sequence and upper-bound change; for .04b, require the load/scaling
proof at both size_t widths; for .04c, require the minimal signedness example
and separately charged validation/search work. No shared test implicitly
passes all six criteria.

Hint1: Mark the invariant at the branch immediately before a load and the
live values immediately before a call. Hint2: Compare the arithmetic width
with the address scaling width; draw the caller's stack at call entry.
Hint3, after recording an unaided attempt: use one high-bit element versus
key zero, then test a zero-length interval immediately at a protected page.
Assessor technical answers remain in a separate private analysis.

## Changed conditions and security containment

The exact existing `-V1` tasks remain: .03a Microsoft ABI, .03b callback added
to a former leaf, .03c changed poison/layout, .04a upper bound, .04b eight-byte
records/size_t, .04c validated immutable trusted boundary. Administer after
at least seven days under the existing trust model; author execution of the
reference is not the learner's delayed assessment.

Only original intentionally flawed assembly is executed, on synthetic input
in a separate local child. The attacker model controls the query/extent for
the teaching target; the tested defect is an extra boundary load violating
memory confinement. Correct results and normal return are the repair oracle;
the flawed child must receive the expected SIGSEGV at its private protected
page. It does not demonstrate code execution or privilege escalation.
The target's own core dump limit is set to zero; no host security setting is
changed. Layout changes probe two placements only. Do not run arbitrary invalid
C pointer expressions or apply the defect to third-party programs. The target
emulator executes only the dedicated curriculum image.

## Validation status and limits

The source-bound `legacy_algorithm_abi/validation.json` and export receipt
record actual campaign status. Ordinary and delayed author implementations
are separate from external review, learner assessment and physical observation.
Assembly accesses are not automatically instrumented by C sanitizers. Page
guards, poison probes and finite oracles can falsify these defects but cannot
establish every ABI, all possible inputs or absolute mastery. Exact final
published C17/C23 text remains unavailable; the pinned ABI sources do not
replace ISO language references.
