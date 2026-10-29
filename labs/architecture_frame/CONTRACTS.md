# Reset, calls and exception ownership: PR-T37-07

This required successor supplies the missing executable composition for the
three original T37.09 criteria and their unchanged seven-day variants. Read the
parent project and the incorporated `labs/interrupt_depth/frame/CONTRACTS.md`.
The decoder, priority rules, startup/vector table and IRQ handlers are the
actual T29 producer; do not replace them with a second decoder or vector table.
The learner must implement `ledger.h`, complete or import their own T29
producer, and supply their own runtime helpers. The author reference remains
in the assessor package. Author execution is not a learner assessment.

## Prerequisites and environment

Bring the T08 AAPCS call dossier, T27 declared map/startup artifact and T29
`struct xf_frame` decoder/handler. Read the actual boot and frame passages registered in
`research/v5/expansion/legacy_architecture_frame/resources.json`: R-MF-ARM,
R-MF-DUI, R-MF-DDI and R-MF-RM21. The register records name precisely inspected
passages; older book or final published C17/C23 access gaps remain separate.

The pure component uses ISO C17, `uint32_t`, borrowed live input and distinct
output objects, no allocation, I/O, library decoder or target libc. Hosted
printing and Python certificates are verification tools. Cortex-M4 execution
uses GNU-compatible assembly, Armv7-M and base AAPCS32 soft ABI; an explicitly
selected FP callee uses softfp instructions. C neither defines exception entry
nor enforces this ownership discipline. Do not confuse a compiler diagnostic,
test, or instruction barrier with a complete language-level safety guarantee.
Changing the ISO C edition does not change the hardware frame layout.

## Concrete problem and admission

`mf_reset` is an inspected **documentary mapping model**, never a reset command.
For F439 Rev21, select main Flash bank1 or SRAM1. BOOT pin/option-byte and silicon
qualification are caller evidence; this model neither reads nor changes them.
Its reset fetch labels are always 0 and 4. Backing labels are 08000000/08000004
for the Flash alias or 20000000/20000004 for the SRAM1 alias. VTOR is a separate
post-reset table location: admit zero or the selected backing base. Never use
VTOR to rewrite the architecture's reset fetch. Bank2/system-memory/external
boot and live remapping are excluded pending their own contracts.

Supply a stable table of up to 256 vector words, an IRQ0–239, the chosen boot
source, an owned half-open executable extent within its real source window,
and an owned stack interval. The table must contain at least the first two words
and the selected IRQ entry. Validate the declared extent before reading fields.
Initial MSP must be greater than the stack's low bound, at most its high bound,
and eight-byte aligned under this **application admission policy**. Hardware
reset masks the low two MSP bits; it does not itself enforce this stricter policy.
Require reset-vector Thumb bit and even PC within the owned executable extent.
Return a staged `struct mf_reset` or an error preserving the complete previous output.
An IRQ lookup address is not permission to invoke its entry. Table content,
copy/zero initialization and exception vector validity remain startup duties.

Original input: MSP20002000, reset08000101, IRQ5, VTOR08000000,
code08000080..08000200, stack20000000..20030000. Report reset PC08000100,
exception21 and IRQ vector08000054; derive each calculation. Changed input
selects SRAM1, reset20000101 and code20000080..20000200. Compare VTOR0 and
20000000 while preserving reset-fetch labels. Explain which board and boot
observations must be repeated and why running MPS2 does not supply them.

`mf_capture` adds an explicit policy around **the imported `xf_frame`**. The
original policy admits only FFFFFFFD, eight core words and no padding. Reject an
unsupported return before reading frame fields; reject short input before
reading xPSR. A declared stacking fault rejects without reading the capture.
The changed policy admits only the producer's six documented return encodings
and its checked FP/padding/alignment rules. Keep extended reservation distinct
from actual FP materialization. Input length must describe genuinely readable
storage; no routine can repair a dishonest length or an unreadable fault stack.

Use the exact original fixture's words at base20000FE0. Publish restored
PC08000200, LR08000111 and PSP20001000. The handler's live FFFFFFFD is a different
value from the stacked LR. In the changed cases use 8/26 words plus optional
one-word padding; core offsets remain 0–28, FP words begin at32, and reserved
storage is not automatically valid saved data. Do not read lazy FP storage.

`mf_ownership` returns masks for general registers0–15 and xPSR at bit16.
Specify AAPCS callee preservation, call-clobbered state and automatically
stacked exception state. Select whether r9 is the platform's additional
callee-saved variable register; otherwise report its platform-specific duty.
An exception does not automatically save r4–r11, but conforming handler software
must preserve the registers it changes. BL replaces LR; a nested call's LR and
the interrupted context's LR have distinct lifetimes. Masks are a derivation
aid, not a runtime enforcement mechanism.

`mf_nested` consumes stable, already validated `struct xf_frame` records plus their
actual captured stack-address labels. Require disjoint occupied intervals, an
outer Thread/PSP frame, inner Handler/MSP frame whose IPSR identifies the outer
exception, and a strictly more urgent inner **group** derived through the real
`xf_priority`. Return the restoration order MSP-handler then PSP-thread. Reject
wrong ownership, overlap, unrecognized configuration or equal group without
publishing. It cannot authenticate fabricated `struct xf_frame` records or replace
hardware exception-return integrity checks; the caller supplies their provenance.

Use outer words from the fixture and inner words
`101,102,103,104,112,FFFFFFFD,08000300,01000010` at20002FE0. With four implemented
bits/group3 and priorities60/50, derive which saved LR names a code return and
which names an exception return. Group6 prevents that preemption despite the
same raw priority bytes. Runtime and storage of each adapter are bounded O(1).

## Deliverables and verification

D1: source and an authority/ownership table distinguishing ISO C, AAPCS, ISA,
selected MCU aliasing and caller evidence. Include pre/postconditions, output
atomicity, readable-storage assumptions, reserved/lazy words and constant costs.
D2: retain original and changed packet traces, the full output before every
rejected case, and independent arithmetic/set/layout derivations. Test short
tables/captures, range edges, missing Thumb, FP-policy mismatch, padding,
unmaterialized FP, incorrect nested IPSR, overlapping extents and group changes.
D3: compile native GCC/Clang debug, optimization and sanitizers, audit unresolved
component calls and final target closure, and submit the generated assembly and
static stack records. The unresolved `xf_frame`/`xf_priority` edges must link to
the actual producer, not a copied answer table.
D4: use the existing IRQ driver and startup to run grouping3/6 × threadFP0/1 ×
lazy0/1 × padding0/1. Capture the producer's sixteen `XF_TRACE` records and
interrupt log, actual ELF/symbols, initial vector bytes, priority readback and
the new preflight marker. Show the nested MSP frame versus the interrupted PSP
frame and explain the priority-driven absence of nesting in group6. With an
FP-using callee, show when lazy storage becomes materialized. This actual MPS2
experiment is separate from synthetic F439 Flash/SRAM reset labels.
D5: construct one rejected implementation per misconception, including using
VTOR for reset fetch, forgetting the16-exception offset, silently widening the
basic policy, conflating LRs, adding r4 to the hardware mask and returning the
inner frame through PSP. Show the first discriminating assertion, not only a
nonzero process result. Retain all exact commands and evidence limitations.

`target_build.py` accepts explicit source/frame/driver/boot/runtime paths. Its
default implementations are unsolved public sources. It compiles the imported
driver with only its main symbol renamed, runs your adapter tests first, then
enters that same driver's experiment. The startup/vector source is unchanged.
Do not substitute the synthetic ledger for real architectural observations.

## Assessment, hints and variants

Each of .09a/.09b/.09c is scored independently: 0 wrong or absent mechanism;
1 memorized output; 2 derived contract plus relevant rejection and current
source-bound execution; 3 an independently completed changed-condition attempt
with a new counterexample and justified limits. .09a must justify bounds and
FP/padding admission; .09b must assign every saved value to its owning context;
.09c must distinguish synthetic labels, actual emulator evidence and unobserved
board boot configuration. Passing finite tests does not prove absolute mastery.

Hints: first draw reset-fetch, backing and current VTOR as separate addresses;
then label each LR and SP by context; finally compare the actual core offsets
in both frame diagrams before deriving the FP extension. Record hint use.
The original `AX-PR-T37-07-T37.09.a-V1` adds inspected FP/padding, `.b-V1` adds
the nested other-stack context, and `.c-V1` changes boot remapping. Keep each
at least seven days after study, with an unaided learner record. Author variant
execution supplies support readiness and never satisfies that delayed gate.

## Containment and evidence limits

All payloads/register values are original synthetic teaching data. Invalid
frames are inspected as arrays, never used as actual exception-return requests.
The executable target is the dedicated MPS2 emulator with software-pended IRQs,
no network, host security change or hardware flashing. Physical F439 boot,
silicon/errata qualification, stacking latency and timing remain unobserved.
An emulator tail-chain event does not establish physical late-arrival timing.
Author validation, external review and independent learner evidence stay
separate; current exact checks live in the successor validation report.
