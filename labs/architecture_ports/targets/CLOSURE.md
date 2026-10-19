# AP7: atomic width, ISA and runtime closure

Required PR-T58-08 transfer, objectives T58.08.a/b/c, families A/D/F/H/I/J.
The B06 shared contract `curriculum/v5/contracts/b06.md` supplies D1–D6, graded
rubric, separation and delayed-assessment policy. Bring original linker scripts,
ABI guest runs, helper ledger and C17 atomic-ordering knowledge. Inspect
R-B06-FOLLOWUP-GCC-ATOMIC plus the selected ABI/ISA source records.

## Experiment contract

`closure_probe.c` is a supplied original compiler experiment. Compare one
acquire-release atomic fetch-add and one unsigned64 division with nonzero divisor
under three widths32/64/128, optimization0/2/size, and four ISA configurations:
RV64I, RV64IMA, Armv8-A without outlined atomics, and Armv8.1-A+LSE without outlined
atomics. `_Atomic` has ISO C semantics; `unsigned __int128`, noinline attributes,
ISA flags and runtime helper names are compiler extensions/contracts. The128-bit
case is deliberately an extension comparison. Do not infer lock freedom from
spelling or ordinary hardware width. No runtime provider is silently linked.

For each of36 configurations record compiler/linker identity, flags, object
undefined symbols, actual disassembly, section sizes and static stack report.
Attempt a final link with only the object and explicit entry/text placement.
Every unresolved helper must cause the intended link failure and appear in its
diagnostic; helper-free cases must link and have no final undefined symbols.
The supplied entry is for closure analysis, not an initialized reset runtime:
these images are not executed. Original ABI guests independently establish their
own target execution. Do not suppress unresolved references or add an ordinary
non-atomic C replacement for an atomic helper merely to obtain a successful link.

## Deliverables and acceptance

D1 predicts lowering and names ISO/compiler/ISA boundaries before compilation.
D2 is the reproducible36-row experiment; D3 retains commands, maps, symbol and
instruction evidence. D4 explains each negative final-link control and proves
that an object compile is a weaker result. D5 compares every emitted helper,
inline sequence, code extent and frame change under equal source semantics;
helper frames and exception/runtime frames remain unknown unless inspected.
D6 states which changed assumptions invalidate the resource argument. Use
`python3 tools/run_b06_closure.py`; its source-bound report is
`closure_validation.json`. Assessor observations are separate in
`research/v5/expansion/b06/architecture_codegen_review.md`.

Assess `.a` by memory-order/dependency reasoning, `.b` by exact actual-object/link
records, `.c` by causal explanation and AP7-DELAYED: choose an outlined-atomic
runtime implementation, inspect its ISA dispatch and progress requirements,
then determine whether it is permitted on an interrupting single-core target.
Run this unaided after seven days; an unresolved runtime is a genuine dependency,
not a reason to guess its behavior. Level1 lists symbols; level2 explains all36
cases and their limits; level3 transfers the proof to the changed environment.
Hints: split object references from final providers; inspect atomic and division
functions separately; an LL/SC retry loop has different progress properties from
one instruction. All outputs are private objects/images; no target system or
host security configuration is modified. Static compiler evidence and finite
guest tests do not certify arbitrary runtime contexts or independent mastery.
