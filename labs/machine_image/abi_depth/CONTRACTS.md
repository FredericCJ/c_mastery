# T08: machine state, representation and callable boundaries

This required successor implements the thirty T08 supplemental objectives and their
changed-condition assessments. Keep the original project IDs and criterion IDs in
`curriculum/v5/expansion/legacy_assessments/PR-T08-*.json`. Their historical receipts
are evidence of the original scope. This package adds separate observations.

The baseline is ISO C17. GNU extended assembly, packed layouts, vector types,
`-fwrapv`, ELF, DWARF, SysV AMD64, AAPCS32 and Linux system calls are explicitly
different contracts. C23 does not make signed overflow, incompatible function
calls or arbitrary machine-state restoration valid. The inspected AMD64 document
is **draft 0.99.6, July 2012**. The source register gives exact inspected sections
and distinguishes this draft from a current ABI or published ISO standard.

## Entry, progression and supplied artifacts

Complete T03 control/lifetime, T04 unsigned arithmetic, T05/T06 representation and
pointer bounds, and the T07 build/provenance supplement first. Import your existing
`mi_asm_sum`, `mi_mix` and `machine.h` components with their original contracts.
`api.c` and `boundary.S` deliberately compile but remain unsolved. `tests.c` and
`hostile.S` supply inspectable checks and a caller, not the implementation.

Predict the result and draw the state boundary before compiling. Implement the
typed C layer, then the assembly caller/callee layer, then diagnose the flawed
counterexamples. Retain source, actual compiler/linker versions and argv, object
headers, undefined symbols, relevant disassembly, per-case output, and a causal
analysis. Finally repeat with the changed assumption assigned in the original
criterion. At least seven days later the assessor assigns that transfer without
the earlier worked analysis. Author reference runs are not learner attempts.

```text
python labs/machine_image/abi_depth/run_core.py \
  --sum-source my_sum.S --mix-source my_abi.c --machine-include my_include
```

Use Linux x86-64 SysV GCC and Clang at O0/O2; inspect optimized and sanitized builds
separately. The command accepts explicit API/boundary sources for alternative valid
implementations. It never searches for an assessor solution. For the Arm portion,
use Cortex-M4, Thumb, `fpv4-sp-d16`, and each named float ABI. Cross-object evidence,
QEMU MPS2 AN386 observations, and physical board results have separate labels.

## Values become representations under stated assumptions

A load's width and extension are part of the algorithm. For `uint32_t` input, first
convert each value into the wide accumulator's domain. Summing three elements in
a narrow temporary and widening afterwards implements a different operation.
Use the independent mathematical sum, then reduce modulo the destination width
where the unsigned contract requires it. Do not test a signed-overflow substitute
as though its output had a portable golden value.

Derive `base + index * sizeof(element) + offsetof(member)` before reading the
addressing mode. Synthetic address calculations do not authorize a dereference.
Packing is a compiler extension; it can change alignment and legal access strategy.
Reordering fields may reduce space while breaking an already published layout.
Member equality does not imply equality of padding bytes. A serialization format
needs specified widths, order and encoding, rather than a dumped `long` or struct.

An ISA mode does not determine C's data model. Record width and alignment for
`int`, `long`, pointers and `size_t`. An ILP32 ELF object and LLP64 COFF object can
be inspected without claiming they executed. Before two separately compiled
components exchange an exposed record, compare interface version, size, alignment
and every relevant offset. Reject disagreement before access. An opaque handle
hides layout, but still needs compatible calling conventions, ownership and
destruction rules.

## Calling is a resource and ownership protocol

The scalar AMD64 boundary distinguishes argument registers, call-clobbered
temporaries, preserved registers and stack arguments. A routine that makes another
call has both roles. It must retain its own live arguments before the nested call
destroys permitted scratch registers. Preserve each designated caller value even
when the numerical answer is correct. Independent sentinels prevent a fixed
initialization pattern from concealing corruption.

For the declared non-vector call, account separately for the pre-call stack and
the pushed return address. Derive the seventh and eighth argument locations on
both sides. Do not generalize this rule to a stack-passed 256-bit vector without
reclassifying the interface. Classify each aggregate's fields and register classes
before deciding whether the return uses registers or caller-provided storage.
Never repair a mismatch by casting incompatible function pointers.

Source locals do not each imply permanent stack slots. An escaped array, a scalar
eliminated by optimization, and a value live across an opaque call impose different
requirements. Source recursion gives a logical depth; optimized tail calls are a
compiler observation. A post-call observable action changes that optimization
opportunity. Keep a conservative bound when machine-code assumptions may change.
The userspace red-zone contract does not authorize use on arbitrary interrupt
stacks. Adding a helper call requires reconsidering every temporary below the
current stack pointer that must survive.

The Arm float ABI separates where arguments travel from how arithmetic is
implemented. Inspect attributes and instructions independently. A soft-float add
can introduce `__aeabi_fadd`; its successful object compilation does not satisfy a
strict final-image closure. Same-profile identity links give a positive control
for mixed-profile rejection. A variadic boundary uses different marshaling from a
fixed-prototype VFP candidate. Preservation of an available VFP register is a
separate obligation from whether the target implements that register at all.

## Metadata and compiler-visible effects

A callable symbol needs the correct code, extent, function metadata and stack
policy. A shared object also needs an intentional dynamic export boundary. CFI
describes how an unwinder can recover the preceding activation at each code
location; it does not execute the restoration. Derive the CFA rule after each push
and stack adjustment. An early-return branch needs its own valid state. Capture a
debugger backtrace from a nested call, then inspect a metadata-only corruption.
Passing ordinary return-value tests cannot validate that corrupted description.

An extended-asm template is opaque to the compiler's C dataflow analysis. State
read/write values, tied operands, early scratch writes, flags and memory effects.
When a scratch output is written before a remaining input is consumed, an omitted
early-clobber can permit a destructive register overlap. Equal input values can
make overlap attractive to allocation, but a particular compiler need not choose
it. A concrete allowed-overlap counterexample disproves the contract without
claiming a universal miscompilation.

Use precise memory operands when the touched object is known. An opaque pointer
access needs a suitable memory effect declaration. `volatile` alone does not
describe that effect. Distinguish three layers: a compiler constraint, an ISA
memory-order instruction, and an ISO C synchronization relation. Inspect the empty
compiler barrier, an actual processor fence, and an atomic publication separately.
Do not run an ordinary-data race to try to establish portable synchronization.

## Experiments and assessment evidence

Each row retains its three original leaf objectives and separate delayed tasks.
Submit a prediction, implementation or derivation, a discriminating counterexample,
and an evidence limitation for **each** leaf. A row-wide pass cannot replace them.

| Original topic | Required ordinary evidence | Changed-condition evidence |
|---|---|---|
| T08.01 | Width-aware sum; measured member address; declared byte encoding | Narrow accumulator; packed/reordered address; dependency versus independent-operation measurements |
| T08.02 | FP/control-state boundary; independently varied preserved registers; signed/unsigned flags | Nested external call; both caller/callee roles; carry versus signed-overflow counterexample |
| T08.03 | Eight-argument map; three typed returns; pre-call/entry alignment | Ninth plus FP argument; mixed-class return; actual stack-passed over-aligned vector |
| T08.04 | Escaped/scalar frames; factorial 0..12; red-zone leaf | Live value across call; post-call work; helper with live temporary |
| T08.05 | LP64/ILP32 manifests; padding; separate component layout gate | LLP64; reordered binary interface; opaque-handle client |
| T08.06 | Soft/softfp code; nine-cell PCS link matrix; d8 preservation | Hard PCS; variadic marshaling; absent-register rejection and actual M4 guest |
| T08.07 | Sum symbol/stack metadata; CFI and backtrace; hostile boundary checks | Shared exports; no-frame-pointer early return; optimized separate consumer |
| T08.08 | Full asm constraints; two memory declarations; three ordering layers | Concrete bad allocation; narrower operand effects; weaker-ISA barrier argument |
| T08.09 | Call/syscall/fault/interrupt table; privileged bytes only; valid longjmp | Signal contract; disposable-guest protocol; cooperative versus preemptive ownership |
| T08.10 | Source/IR/object mapping; equal-contract cost comparison; overflow classification | Signed precondition; actual LTO; explicit compiler wrapping extension |

Benchmark only computations whose results are independently checked. Record paired
order, repetitions, elapsed-clock source, instruction counts and disassembly. The
dependency experiment keeps multiplication count fixed but changes dependence;
its wall time includes loop, scheduling, virtualization and clock effects. It is
not a measured instruction-latency constant. Inlining can change footprint and
register pressure; LTO can change a formerly external boundary. No speedup is a
pass criterion.

## Transfer, containment and acceptance

The target is exclusively the original learner program or dedicated QEMU guest.
Synthetic values and registers are the only data. A flawed assembly routine can
violate the caller's state while returning a correct value; the harmless success
condition is a named sentinel/alignment failure, not a crash or an exploitation
claim. Repair the violated contract, rerun the distinguishing control and the
correctness cases, and retain the remaining portability limitations.

Privileged CR3/CLI/STI examples are **assembly/disassembly only** on the host.
The original transfer asks for a disposable-guest experiment specification, not
host execution. State guest CPL, page-table/IDT ownership, captured exception
fields, serial output, timeout and reset procedure. Never infer escalation from a
fault. A harmless local `getpid` syscall is the executable comparison. Exception,
signal and coroutine tables must retain their distinct OS, ISA and C contracts.

Rubric level 0: missing/incorrect contract or unsupported claim. Level 1: matching
output with no explanation of the first state that could fail. Level 2: every
ordinary criterion clause, independent expected reasoning, a specifically rejected
incorrect implementation, and honest environment limits. Level 3: independently
derive and validate the changed condition after the recorded delay. An alternative
valid representation or instruction sequence earns full credit when it satisfies
the same ownership, state, arithmetic and resource contract.

Hints progress from locating the first boundary where the claimed value/state is
created, to drawing its before/after state, to varying one assumption while keeping
the other contract inputs fixed. If a mutant crashes, first establish that it
reached the intended defect; a crash alone is not the required semantic rejection.
Current author execution status lives in the source-bound successor support, not
in this teaching text. Non-author review, independent attempts and physical
observations remain separate evidence.
