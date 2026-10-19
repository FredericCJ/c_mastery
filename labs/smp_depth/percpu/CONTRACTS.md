# T53.03: actual per-CPU storage, frames and controlled nested faults

This required successor preserves PR-T53-03 and its three original criterion
IDs and delayed variants. Prerequisites are the actual completed PR-T53-02
startup adapter, CMK kernel image and the original table/interrupt/ABI teaching.
The shared legacy assessment contract supplies the seven-day assessment rule,
evidence format and distinction between author tests and learner performance.
Families A/B/C/D/F/G/H/I/J apply to the concrete investigations below.

## Representation and learner inputs

Implement `percpu.c`, `percpu_guest.c` and `percpu_frames.S`. Public files remain
unsolved. The header, tests, trace adapter, independent Python oracle, explicit
guest composition runner, ABI probe and equivalent-workload cost driver are
supplied. Completed implementations and causal answers are assessor-only.

The C17 numeric model lays out one to four CPU records in a caller-described
half-open arena. Nine distinct spans per CPU represent task0/kernel, idle,
interrupt, emergency, GDT, TSS, IDT, task1 and controlled-fault stacks. The
requested stack size is at least512 bytes and a multiple of a power-of-two
alignment8..4096. Table sizes are56,104,4096 bytes; their allocation alignments
are8,8,16. These are explicit x86 teaching-layout choices, not ISO C guarantees
or universal ISA alignment requirements. All additions and alignment rounding
must be checked before publication; extent overflow is RANGE, insufficient
space SMALL, malformed parameters INVALID. Any refusal preserves output.

`sp_validate` accepts at most36 spans, rejects empty/reversed extents, unknown
CPU/kind, duplicate owner-kind pairs and any pairwise overlap. Equal touching
endpoints are valid. Valid numeric spans do not prove mapping, ownership, cache
attributes or physical availability. The guest separately validates its actual
object addresses. Sharing immutable code/page tables is a separate ownership
argument; writable TSS records and active stacks are not shared.

`sp_budget` accepts up to16 supplied frame sizes, rounds each to the declared
ABI alignment, and adds explicitly supplied hardware, saved-register, alignment
slack and extended-state costs with checked arithmetic. It is a calculator for
a declared finite call/entry model. It does not discover recursion bounds or
prove that unlisted asynchronous paths fit. Use separate budgets for separate
IST stacks. An intact32-byte canary is a boundary observation, never a proof of
a complete worst-case stack bound.

## Concrete target work

The target is the original disposable x86-64 CMK image in QEMU TCG. Its retained
source is copied only into a private build directory. Explicit completed startup
paths supply its real discovery, INIT/SIPI, mode transition and permanent-stack
handoff. The new adapter replaces only the declared post-handoff worker and BSP
entry hook in that copy. It never silently searches for assessor solutions.

Every CPU, including the BSP, executes its scheduler on its own16KiB permanent
stack. Each owns two8KiB task stacks,8KiB interrupt stack,4KiB controlled-fault
stack,4KiB emergency stack, GDT, TSS and IDT. Record every actual half-open span,
CPU index and APIC ID. Verify loaded GDTR/IDTR/TR identity and TSS stack choices.
Use the sparse four-CPU topology0,1,2,4 as well as one/two CPUs.

Each CPU alternates two original cooperative tasks. A five-frame bounded nested
call tree keeps distinct volatile patterns live across a context switch and
software interrupt244. Assembly checks caller-saved registers across interrupt
return and callee-saved registers across the cooperative switch. All C target
translation units use `-mno-red-zone -mgeneral-regs-only`; inspect emitted code
and exact stack-usage records. The existing context-switch primitive is imported
unchanged, and its register-save set must be explained.

Vector244 uses IST1. Exactly one selected CPU executes a known `UD2` instruction
inside that handler; its #UD handler uses IST2. While there, it causes a known
invalid segment-selector load, whose #GP handler uses IST3. Distinguish the
hardware error-code frame from the placeholder for vectors without one. Inspect
the saved RIP and error class before redirecting only these two known teaching
sites to their explicit recovery labels. Unknown faults remain fatal. Reusing
one IST top for arbitrary same-vector nesting would overwrite active frames;
this experiment permits exactly the declared vector244→#UD→#GP chain.

The controlled-fault CPU retains records of its own identity, nesting depth,
frame addresses and peer progress before/after the held fault. With multiple
CPUs it requires at least four actual peer progress increments before returning.
It must then complete both tasks and preserve canaries. One CPU has no peer
progress claim. The original CMK whole-image fault path remains unchanged; this
new known-site recovery is not general kernel fault isolation, task termination,
double-fault recovery or arbitrary memory-corruption containment.

## Changed assumptions and experiments

For `.a-DELAYED`, change stack size8192→2048 and ABI alignment16→8, regenerate
all spans and checked budgets, and compile the identical finite `abi_probe.c`
call tree under SysV AMD64 and AAPCS32. Inspect actual generated frames and stack
usage instead of assuming that changing an integer makes an ABI port. AAPCS32
requires word alignment at all times and double-word alignment at public calls;
this comparison's optional hardware-frame inputs are explicitly supplied model
parameters, not invented Arm exception-frame evidence. The Arm artifact is a
compiler/analysis result unless separately executed on an appropriate target.

For `.b-DELAYED`, enable the selected x87/SSE state profile after actual CPUID
checks for FXSR/SSE/SSE2 and explicit CR0/CR4 initialization. Retain distinct
512-byte,16-byte-aligned FXSAVE images for each task and idle context. Interrupt
entry saves/restores the interrupted state even though the handler deliberately
clobbers XMM0/XMM15/MXCSR. Task switches save and restore their own images.
Distinct task/CPU patterns and rounding modes must survive both paths. Omitting
restoration must cause the exact preservation assertion. FXSAVE is not sufficient
for AVX/YMM/other enabled extended states; an AVX-capable CPU does not authorize
using them. Derive CPUID/XCR0/XSAVE sizing and ownership requirements before
extending the profile. This laboratory does not implement an asynchronous SIMD
scheduler or claim lazy-save safety; preemption requires preserving every live
architectural state at each allowed preemption boundary.

For `.c-DELAYED`, the nested #GP above is the separate emergency-stack transfer.
State why an invalid emergency stack, fault in an unrecognized handler, repeated
same-IST reentry or failure during double-fault delivery remains unrecoverable.
The TSS-alias control points CPU1's interrupt stack at CPU0's and must be refused
by a concrete ownership check. A reboot or host timeout is failure, never a
successful control. A separate control suppresses nested-fault accounting and
must fail its accounting assertion.

The retained project also requires a padding transfer experiment. `cost.c`
executes identical relaxed atomic increments on two allowed logical CPUs with
adjacent8-byte counters versus128-byte strides, alternating order across ten
pairs. Record affinity, reported cache-line size, counts and raw monotonic times.
Both counters must reach500000 in every trial. No speedup threshold is a
correctness criterion; logical CPU affinity does not prove separate physical
cores, and contention results are specific to the actual host/environment.
Host pthreads, libc and performance tools never become target dependencies.

## Verification, rubric and failure semantics

Predict every test and negative control first. Required evidence includes the
transactional layout/budget implementation, independent numeric-grid comparison,
actual frame/descriptor/CPU traces, changed-ABI compile/disassembly, image helper
closure, stack-usage derivation, equivalent-workload samples and separate learner
analysis. Exhaustive means only the explicitly enumerated finite grid. Testing
the positive path with one CPU cannot assess cross-CPU aliasing or peer progress.

For `.a`, level2 requires actual disjoint extents, checked arithmetic, ownership
and immutable-sharing distinctions; level3 regenerates the changed ABI/layout
and finite call-frame calculation. For `.b`, level2 requires actual frames,
ABI/register/canary reasoning and controlled-fault limits; level3 preserves the
selected SIMD state and explains excluded extended state/preemption cases.
For `.c`, level2 requires multiple CPUs, exact fault identity and peer evidence,
and a discriminating TSS-alias control; level3 demonstrates the known nested
emergency path and identifies unrecoverable cases. Level1 only reproduces a
marker. Unsupported universal stack bounds, general fault recovery or ownership
enforcement claims fail even if numbers match.

Hints: enumerate actual writable ranges before writing assembly; draw both
error-code frame shapes; trace the interrupted RSP through each IST transition;
then distinguish a compiler call-save rule from a scheduler's asynchronous state
ownership. Assessor causal analysis is separate. Fixed arrays and bounded scans
are required: no allocator, libc, libgcc, compiler-rt or libatomic in the guest.

## Source, containment and validation limits

Sources are the actually inspected historical Intel SDM Vol3A253668-060US
§§6.14.1–5,7.7,13.1.1–3,13.3–4; scalar AMD64 draft0.99.6; AAPCS32 release2025Q1
stack constraints; and the explicitly imported startup contract. Keep ISA,
ABI, C17, compiler extensions, OS and emulator behavior distinct. Source records
and exact campaign status reside in the linked successor envelope; prose is not
an execution receipt. All vulnerable/negative cases are original local teaching
images and synthetic state, with no networking, host disks, deployed targets,
flashing or host-policy changes. Physical timing, arbitrary workloads, independent
human review and delayed unaided learner results remain separate obligations.
