# Required ABI changed conditions AP1-AP3

These continuations retain PR-T58-02,03,05 and the shared
`curriculum/v5/contracts/b06.md` evidence contract. Families A/B/C/D/F/H/I/J
apply. Bring integer context images, stack ownership/alignment proof, assembler
and linker-map reading skills, and the two original guest artifacts. Use C17
for hosted encoders and the freestanding aggregate oracle; ISA assembly is
explicitly outside ISO C. Selected targets are RV64IMAC/LP64 and Cortex-A53
AArch64 in QEMU virt, with no network, disk or host-kernel modifications.

## AP1: call preservation versus complete vector state

Map T58.02.a/b/c. Derive from the inspected AAPCS64 passage why preserving the
low64 bits of v8-v15 satisfies the base public-call rule while an interrupted
128-bit vector computation needs more. Seed v8 low/high lanes17/29, save/restore
only d8 after clobber, and observe17/0. Then save all32 q registers plus FPCR and
FPSR, clobber each, restore, and verify both lanes of every register and the two
control/status values. Use a544-byte16-aligned frame:512 vector bytes,16 control
bytes,8 saved-link bytes,8 padding. No SVE, SME or lazy-state extension is implied.
The private self-test deliberately sets its own floating environment, then
returns it to zero; a reusable API must instead restore the incoming environment.

D1 is the frame layout, explicit caller/callee/asynchronous distinction, source
and actual guest trace. The omitted-high-half mutant must fail a value check.
Explain why a passing ABI-call test cannot certify a full interrupt frame. Hints:
first name which bits the ABI protects; then count the interrupted computation's
live bits; finally include control/status state. AP1-DELAYED adds a selected
scalable-vector regime only after inspecting its independent size/ownership rules.

## AP2: hidden aggregate result pointer

Map T58.03.a/b/c. Compile a separately defined C function taking eight unsigned
64-bit values and returning three unsigned64 fields: sum, first xor last, last.
Supply arguments1..8 and derive36,9,8 independently. Write assembly callers with
distinct24-byte result storage and16-byte aligned call boundaries. AArch64 passes
the result pointer in x8 without consuming x0-x7; RV64 passes an implicit first
argument in a0, placing the eighth explicit argument on the stack. Neither caller
may assume the hidden pointer's register is unchanged on return.

D2 is the source/layout assertion, register/stack ledger, two actual guest
executions and a wrong-hidden-register mutant for each target. Preserve all live
callee state and return addresses. Level2 requires the correct memory result
and a causal ABI explanation; merely shifting the same argument table on both
architectures fails. Hints: classify the result before assigning explicit
arguments; inspect the first stack argument's offset. AP2-DELAYED changes to a
homogeneous floating aggregate and requires a new classifier, not a size-only rule.

## AP3: scattered offsets and linker veneers

Map T58.05.a/b/c. Implement `pe_branch` for RV BEQ with registers0..31 and even
signed byte displacement -4096..4094; implement `pe_jump` for JAL with even
displacement -1048576..1048574. Reject range, parity and register errors before
publishing the uint32 word. Pack explicit fields without signed shifts or native
bit-field layout. `encoding.h` and learner stubs are supplied. D3 derives the
instruction-bit map, sign extension and PC-relative interpretation, and compares
26 boundary/cross-field words with an independent assembler. Round-tripping one
encoder/decoder pair is insufficient because both can share the same bit error.

Link the supplied AArch64 call at0x400000 to a target at0x50000000. Inspect the
actual thunk and its target literal. The selected LLD emits a literal load into
x16 followed by BR x16; verify raw instruction words and the64-bit target. IP0,
IP1 and flags are permitted veneer clobbers even when this specific thunk uses
only IP0. Do not confuse a successful link with execution of the distant call.
Hints: isolate every immediate subfield; then compare bytes without calling your
decoder; finally distinguish permitted clobbers from observed clobbers.
AP3-DELAYED changes reach/alignment and linker and requires a new counterexample.

## Verification, rubric, references and limits

Author commands are `python3 tools/run_b06_abi_guests.py` and
`python3 tools/run_b06_encoding.py`; reports are `abi_guest_validation.json` and
`encoding_validation.json`. Learner construction uses the public scaffolds and
brief, never the separate assessor guest implementations. Require no undefined
link symbols in freestanding guests. D4 retains maps, raw commands, source hashes,
code size/frame costs, expected failures and result interpretation.

For each AP criterion, `.a` assesses derivation, `.b` implementation/boundaries,
`.c` transfer and evidence limits. Level0 confuses ISO/ABI/ISA guarantees; level1
matches one output; level2 satisfies each named invariant and negative control;
level3 solves the unaided delayed variant after seven days. Inputs and fault
targets are original synthetic guests only. No physical timing/cache result or
complete operating-system claim follows. Inspected bindings are
R-B06-FOLLOWUP-AAPCS64 (SIMD/FP, result return and veneers), R-B06-FOLLOWUP-RVABI
(implicit result parameter), and R-B06-FOLLOWUP-RVENC (B/J formats). Exact bounded
passages remain in the source register; full architecture coverage is separate.

## AP4: two stack-owned continuations

Map T58.04.a/b/c. Extend the AArch64 guest with two256-byte16-aligned task stacks,
each preceded by16 guard bytes. Each task owns its context image, stack-local
sentinel and counter; both yield twice and terminate after their third step.
The scheduler resumes them alternately through saved return continuations.
States are ready, running and dead: resume admits ready only and marks running
before restore; yield returns to ready; termination becomes dead before returning
to the scheduler. A second resume of dead state must reject before restoring any
register or SP. Never use the same stack for two suspended continuations.

D5 contains state/ownership invariants, all six actual resumptions, two stable
stack-local sentinels, final counters3/3, intact guards and two refused dead
resumes. After reserving the initial16-byte local frame, demonstrate that the
remaining240-byte reservation fits exactly and241 rejects before SP moves; no
deliberate out-of-bounds stack write is needed. Kill shared-stack and omitted
admission mutants in the actual guest. Distinguish cooperative preserved-register
state from the full live state needed by asynchronous preemption. AP4-DELAYED
adds a third task and cancellation of a suspended task with explicit frame
cleanup. Hints: make ownership/state transitions explicit before assembly; then
identify which saved PC resumes after yield and which SP owns its locals.
The AP1-AP3 rubric also applies: each leaf is assessed by derivation, boundary
execution and independent changed-condition reasoning; author traces are not
independent learner evidence.
