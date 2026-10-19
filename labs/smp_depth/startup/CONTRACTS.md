# T53.02: startup generations, retained resources and real AP handoff

This required successor serves all three original PR-T53-02 criteria and their
named delayed variants. Retain original prerequisites PR-T53-01/PR-T48-03,
learner-owned completed kernel, seven-day independent assessment delay and
historical records. C17 model semantics, GNU x86 assembly, Intel ISA rules,
SysV AMD64 calling convention and QEMU behavior are separate assumptions.
The guest uses GNU as for its GNU assembly, including when Clang compiles C;
the actual command records disclose `-fno-integrated-as`. Clang's integrated
assembler rejects a retained Multiboot checksum literal accepted by GNU as.
This supported toolchain choice does not change the retained boot source.
The Clang guest profile compiles the complete adapted `smp.c` and `ap.S`
translation units with Clang; frozen CMK boot/kernel/user prerequisites remain
GCC-built because their `_Atomic`/GNU-atomic combinations are GCC-specific.
Record this mixed compiler dependency explicitly. It is a changed compiler
experiment for the new startup mechanism, not whole-CMK Clang portability.

## Concrete construction

Implement `startup.c`, `startup_guest.c` and `startup_trampoline.S`. The public
files are unsolved; completed implementations are private assessor material.
`startup.h`, `tests.c`, `trace.c`, `oracle.py`, `audit.py` and `guest.py` are
supplied verification scaffolds. No existing startup/ACPI/CPU runtime library
may replace the mechanisms. The completed CMK prerequisite supplies its actual
boot image, ACPI discovery, local APIC MMIO/PIT, per-CPU GDT/TSS/IDT and linker
layout. The explicit public runner imports those routines into a private copy;
it never finds or copies an assessor directory implicitly.

The finite lease controller has three slots. Each stores a32-bit monotonically
increasing generation, phase, dense CPU index1..3, xAPIC ID0..255, one aligned
low4KiB page, and a16-byte-aligned bootstrap stack extent of at least4KiB.
Initial worlds are zeroed except `parallel`0/1. The caller serializes controller
calls and owns a valid live world; tickets/results are disjoint from it.
There is no claim of automatic C ownership enforcement or lock-free control.

`ss_begin` accepts only an empty, nonexhausted slot and validates all extents
before publication. Pages must be within `[0x1000,0x100000)`. A page cannot
overlap its stack; live resources cannot share CPU index, APIC ID, page, stack,
or cross-overlap another page/stack. Serial mode admits one live slot; parallel
mode permits disjoint per-AP resources. Refusal preserves world and output.
Generation `UINT32_MAX` may be used once, then further admission refuses FULL;
cleanup remains legal and generations never wrap. Port ownership of these low
pages is an additional prerequisite, not established by numeric alignment.

`ss_timeout` changes PENDING to TIMED_OUT and retains every reservation.
`ss_ack` checks ticket generation, actual APIC ID, all four verified-state bits
and entry stack `stack_high-8` before READY. A matching late acknowledgement
becomes LATE_READY and returns LATE; it is not an on-time successful startup.
Duplicate acknowledgements are rejected separately from old-generation tickets.
The observed bits mean long-mode/protected/PAE/paging state, selected CR3,
aligned/private bootstrap stack, and actual processor identity respectively.

`ss_quiescent` requires a separate truthful port attestation after READY or
LATE_READY. A flag supplied by an arbitrary caller does not establish safety:
the caller must show there is no possible remaining access to the attempt's
code page, mailbox or bootstrap stack. `ss_release` requires QUIESCENT, removes
reservations and retains the generation counter. The controlled host model can
assume a supplied attestation to examine transitions; the emulator must produce
an actual handoff witness. Timeout, ICR delivery status and acknowledgement are
three different observations and none alone is that witness.

## Actual emulator adapter

Derive16-bit real mode ->32-bit protected mode ->64-bit long mode. The supplied
assembly worksheet must account for CLI/CLD and segment bases, GDT/far targets,
CR0.PE, CR4.PAE, low32-bit CR3/PML4 address, EFER.LME/NXE, CR0.PG/WP and the
final64-bit code descriptor. The identity mapping must cover transition code,
mailbox, stack and page tables. Do not treat a compiler function pointer as a
real-mode startup address. Establish SysV entry stack alignment before C.

The original adapter performs directed INIT assert/deassert with retained PIT
delay, then one SIPI for the selected page. This selected QEMU teaching policy
has no retry if startup fails; the typical Intel two-SIPI sequence and physical
timing/errata must be assessed separately before a hardware claim. A finite
spin budget is not a calibrated100ms deadline. Host harness timeout remains a
failure, not missing-ack success. The ICR busy condition only gates another send.

The AP records its actual CR0/CR3/CR4/EFER, initial/current stack membership,
LAPIC identity and dense map before publishing its generation acknowledgement
with a release store. After local table installation, assembly switches to the
permanent per-CPU kernel stack and never returns to the bootstrap frame.
Only `ss_parked`, running on that permanent stack, publishes quiescence. The BSP
uses acquire observations before reading fields/attesting/releasing. Permanent
kernel stacks, GDT/TSS/IDT and all CPU state remain owned until VM teardown;
they are not reclaimed by `ss_release`. This is an explicit resource transfer,
not a claim that a timed-out or halted CPU cannot access its old memory.

Serial launch uses one code/mailbox page and successive generations1,2,3. Each
AP has a private bootstrap stack; the next AP is launched only after the actual
permanent-stack handoff. Parallel launch uses one code/mailbox page per AP and
independent tickets, allowing initialization order to differ. Verify every AP
before reporting completion. A delayed AP1 waits until after BSP timeout;
admission/release must still refuse while it can use old resources. The BSP then
allows it to complete, records LATE, waits for permanent-stack quiescence and
only then releases. Deliberately missing and stale acknowledgements must reach
specific guest failures, never pass on an old print marker.

## Boundaries and changed assumptions

Run1/2/4 CPUs. The four-CPU topology declares two sockets/three possible cores
per socket and max6, yielding actual enabled APIC IDs0,1,2,4. Index3 therefore
does not mean APIC3. Record each actual ID, ticket generation, page and bootstrap
versus permanent stack address. A one-CPU run has no AP acknowledgements and is
a valid empty startup set, not evidence for AP code.

Move base0x8000 to0x9000 by rebuilding all three macro-generated trampoline
images, including far targets, GDT bases and mailbox addresses. `audit.py`
extracts the actual section, checks relocated fields and emits separate16/32/64
disassembly. Auditing the whole mixed-mode blob as only64-bit instructions is
incorrect. Retain zero undefined-symbol/no-dynamic-runtime closure of the final
kernel and user images. Compiler helpers count as dependencies even if absent
from source calls. No hosted libc/allocator enters the freestanding guest.

Change QEMU CPU model from `qemu64` to `max`. Before loading the teaching image,
query the actual selected TCG/machine model using QMP full expansion, without
conflicting `-cpu` options. Required APIC, PAE, long-mode and NX features must
be present; a `qemu64` model with NX explicitly disabled must be reported
unavailable, with no teaching-image execution. This gate is intentionally narrow:
it does not inspect every vendor erratum or prove general model compatibility.

## Evidence, reasoning and assessment

The hosted trace protocol accepts `parallel initial_generation event_word`.
B/C begin first/second slots; T times out first; A/D acknowledge first/second;
S supplies the previous first generation; P/K attest quiescence; Q refuses
attestation; R/J release. `-` is empty. Report each result and final phase/counter
pair. The independent Python model enumerates15,544 declared short histories
including near-exhaustion; it shares the contract but no C implementation.
State the depth bound and external port premise. Native tests additionally
probe overlapping resources, incomplete state, duplicate/stale/late outcomes
and cleanup at exhaustion. Distinct mutants ignore generations, release on
timeout, or permit overlapping stacks; all must compile and fail semantic tests.

Provide a state-transition/lifetime argument, mode-transition annotation,
low-memory reservation rationale, exact source/image/command identities,
independent model output, raw startup traces and the regenerated assembly audit.
The full controller uses fixed storage and bounded scans over3 slots; no heap.
Serial low-page storage is4KiB versus up to12KiB for parallel launch, plus one
4KiB bootstrap stack and retained permanent stack per AP. Initialization polling
cost depends on the target, not only the O(3) admission scan. Explain this cost
under equivalent acknowledgement/quiescence contracts before comparing speeds.

`.a` level2 requires a correct ISA/ABI transition derivation; level3 independently
rebuilds the relocated images. `.b` level2 requires delayed timeout retention and
causal quiescence, with old/duplicate ticket counterexamples; level3 implements
parallel ownership and acknowledgements. `.c` level2 requires actual1/2/4 traces,
sparse identity, missing/stale controls and final image closure; level3 changes
CPU model, performs preflight and explains unavailable conditions. Level1 merely
repeats a successful marker. False resource-reuse, ISO-C-enforced ownership,
physical-timing or universal-startup claims fail regardless of output numbers.
Hints: identify the last possible low-page/bootstrap-stack access; distinguish
delivery from initialized state; then add a permanent-stack handoff witness.

## Source status and containment

Only original CMK teaching images and synthetic state are used, under disposable
QEMU TCG with no host disks, external targets, networking or host policy changes.
No hardware flashing occurs. Read Intel SDM Vol3A253668-060US §§8.4.4.1–2/Table8-1,
8.4.5 and9.8.5–9.8.5.3, and actual QMP CPU expansion documentation; exact inspected
passages/edition are registered separately. This historical SDM is not current
CPU errata. The retained boot's mapping/cache assumptions and unsupported
physical-platform observations remain explicit. A finite campaign provides
author evidence only; no independent reviewer, learner attempt, delayed-transfer
completion or absolute mastery is manufactured.
