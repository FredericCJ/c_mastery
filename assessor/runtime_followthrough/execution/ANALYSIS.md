# Exact T46.03/T46.04 assessment and delayed variants

Assessor-only. Preserve the original six task texts and seven-day minimum. A
learner must produce the changed argument before seeing this implementation.
Record genuine attempt dates; current executions are author-side validation.

<a id="03a"></a>
## T46.03.a

For checked arithmetic the intermediate fits int64: each input is int32, so
even the extreme product fits. Admission is before signed division, and before
int32 narrowing. -7/2 is -3; floor division is a different operation. The public
matrix includes max+1 versus max-1, min-1 versus min+1,46341 squared versus46340
squared, min/-1 versus min/1, zero divisor and both negative-sign directions.
The actual original VM is executed for every row, while the successor exposes
the individual two pushes, operation and RET. A failed operation consumes its
operands diagnostically but leaves result91.

WrapV2's high unsigned value u maps to `-1-(UINT32_MAX-u)`, whose cast operand is
representable. Do not use `(int32_t)u` as a portable proof. The independently
written Python oracle uses unbounded mathematical integers, explicit quotient
sign and mathematical modulo. It checks18696 cases/profile:2modes x4operations
x(17^2 extrema pairs+2048 seeded pairs), seed4603. Mutant1 silently retains checked
semantics in wrapping mode and fails the first overflow comparison. Test success
establishes only that finite corpus; the derivation establishes the general model.

Score0 incorrect arithmetic or UB in the implementation;1 nominal answers only;
2 derives guards/conversions and explains every boundary trace;3 independently
defines the changed division edge and wrapping representation with a fresh
counterexample against claimed equivalence. Missing source-edition distinctions
or replacing admission with a sanitizer observation fails the semantic argument.

<a id="03b"></a>
## T46.03.b

TRUE/FALSE/ADD fails TYPE after two operands are removed; max+1 fails RANGE;
63 caller temporaries plus a callee's second push fails STACK_LIMIT at sp64;
POP at empty entry is refused VERIFY before admission. A pending service is
issued only after valid20+22. Its completed failure yields HOST_ERROR, retaining
the distinction from arithmetic failure and preserving result91. Capability
denial issues no request. No native external service or unbounded callback runs.

Score0 conflates status layers;1 inspects only return values;2 gives minimal
images and admissible diagnostic mutations with output preservation;3 derives
the added service failure ordering, actual issued-request count and a new denied
service counterexample. Alternative service designs may charge denial differently
only under a separately specified contract; they cannot claim this one's evidence.

<a id="03c"></a>
## T46.03.c

The four-instruction arithmetic image runs with fuel0..5. Fewer than4 stops at
the next unexecuted instruction, preserving result;4 completes with zero fuel;
5 leaves one. Mutant3 writes the result during progress and fails immediately.
The original run API resets execution state each invocation and cannot be used
as a one-instruction resume operation. This successor owns its image and explicit
continuation; changing the caller source after start proves the borrow ended.

HOST creates one pending request. Take-before-complete, exact epoch/serial,
single take, duplicate completion, pending restart refusal and old-epoch-after-
restart are distinct transitions. Mutant2 accepts a wrong identity, mutant5 resets
a pending continuation, mutant6 takes twice. All compile and fail semantic checks.
Completion adds no fuel/host charge. A failed operation is terminal until restart.
No resumption after arbitrary mid-instruction error is supported.

Score0 exposes invalid continuation or replay;1 merely saves an IP;2 explains
failure-state diagnostics and repeatable independent replay;3 derives retained
ownership and all pending transition invariants, distinguishing quiescence from
the identity check. A promise that the API cancels real I/O is incorrect.

<a id="04a"></a>
## T46.04.a

The recursive trace has main invocation1 and sum invocations2..7 with n5..0.
The entry's100 remains below the sum base1. Each non-base sum retains n before
calling n-1, so successive bases are1..6. Returns unwind in reverse order;
sum0 returns0 and sum5 returns15, then main returns115. Instruction count is
5 for main+5*11 for non-base sum+6 for base sum=66. The raw trace records all
13 call/return points with function, invocation, IP, base, sp and initialized
local0. Tests compare the retained predecessor's exact result/fuel as a second
implementation, but the66 calculation is independent of either execution.

Mutual recursion has two descriptors whose locals0/1 mean n/acc in each frame.
The tail variant checks alternating function identities and decreasing n at every
step; invocation identity changes even when the physical frame slot is reused.
An implementation that accidentally always calls function1 cannot pass this
check merely because its numerical answer agrees.

Score0 conflates function with invocation;1 traces only n;2 derives the full
return/base/initialized trace with live caller temporary;3 independently traces
mutual calls with reused local numbers and shows which identities remain stable.

<a id="04b"></a>
## T46.04.b

Wrong arity fails the verifier and leaves the old session intact. Ordinary direct
and mutual accumulator recursion reaches16frames and refuses the next call
without advancing next_invocation past16. The actual compiler-instrumented
`vs_step`/original`cvm_run` call depth is1 while the guest frame depth is16.
This is actual function entry/exit evidence, not just a source search for self-call.

TAIL24 is admitted only with the terminal CALL/RET-compatible stack equation.
Copy args before clearing frame locals; restore the caller base rather than the
temporary current top. For n30 the result is465 at depth2 and invocation32.
Replacing tail behavior with ordinary CALL is mutant4: it fails at the frame
bound. Do not compare performance without acknowledging changed instruction and
frame contracts. Caller traces remain well-defined, but eliminated frames are
not present for a later guest backtrace.

Score0 wrong admission/resource failure;1 nominal call tests;2 correct exact
arity/frame boundary and C-stack evidence;3 derives tail admissibility, copied
arguments and preserved return base, then predicts changed resource behavior.

<a id="04c"></a>
## T46.04.c

The host probe separately records sizeof(CvmFrame), operand array, Cvm, Vs and
native recursive stack span. The native experiment recurses only17 levels with
96volatile bytes per frame and post-return work to prevent tail elimination.
It measures an ABI-specific address span, not the full usable stack or portable
growth direction. Instrumented interpreter nesting stays1. Compiler `.su`
records distinguish automatic session placement from caller-owned static state.
The public interleaving test runs disjoint automatic/static sessions to42/56,
using scratch only during admission. Neither storage duration permits concurrent
unsynchronized access or callback reentry into a live session.

The target pilot exposed actual large compound-literal temporaries at-O0. The
reference start now clears its owned integer storage without an automatic whole-
Vs temporary, then `cvm_init` assigns actual null pointers before reading them.
The test driver similarly avoids repeated automatic reset temporaries. No stack
reservation was enlarged. This is a compiler-generated resource lesson, not a
claim that aggregate assignment has inherently unacceptable cost.

Score0 confuses guest and native stack;1 reports one aggregate number;2 separates
objects, frames and measured call chain with actual generated evidence;3 derives
automatic/static lifetime, reentrancy and concurrency limits under changed storage.
The target bound sums common live C frames plus the maximum of five sequential
test-entry frames and1024reserve. It applies only to this selected call tree.

## Cross-component boundary and validation limits

The successor uses actual frozen verifier and VM heap/value primitives. Strict
builds name all10objects, inspect imports and close original compiler helpers.
The host stack probe uses compiler instrumentation extensions only on the host.
PPC64 big-endian and M4 execution are emulated observations, not physical results.
The pending primitive is ready for T46.09/.10/.12 composition; it does not by
itself close those complete resource/store/provenance criteria. Their remaining
author and external evidence remains in the inventory.
