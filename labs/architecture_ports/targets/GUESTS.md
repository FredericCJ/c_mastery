# T58 privileged guest continuation

This required B/C/D/F/G/I/J continuation of PR-T58-01–08 uses the shared B06
contract, the learner's aligned context buffer and linker/reset artifacts. The
target contract is RV64 QEMU virt without firmware, and AArch64 QEMU virt
Cortex-A53. C17 governs any C harness; assembler, privilege registers, translation
tables and traps belong to the named ISA, while callee preservation belongs to
psABI/AAPCS. No library implements the context switch or page walker.

Implement original reset, linker and trap code. Zero BSS, establish separate
aligned main and alternate stacks, and preserve the specified integer ABI
registers. Initialize RV M-mode PMP/delegation before S-mode execution; establish
the actual AArch64 exception level rather than assuming reset arrives at EL1.
Save a context, scramble preserved registers, move to the alternate stack and
restore. Then install a synchronous trap vector, issue one service trap, and
verify exact saved PC/status and return. Service return PC conventions differ
from access-fault return PC conventions; compare the actual architecture rules.

Build two real translation roots mapping one test VA to physical cells17 and29.
RV64 uses three Sv39 levels; AArch64 uses a39-bit VA and4KiB granule. Map code,
stack and required emulator interfaces separately. Publish tables with the named
barriers/invalidation sequence. Switching root must change the observed value.
Store through a read-only mapping and load an unmapped VA; require exact cause,
fault address and saved PC, then resume at the known following instruction. A
wrong cause, unexpected vector or extra trap must fail. Original fixtures expect
three synchronous traps. A marker printed before the fault is not completion.

D1 is source/link map/nm/disassembly and dependency closure; D2 is an ABI versus
exception-frame register table and stack-lifetime proof; D3 is a hand-derived
table walk and both root observations; D4 is exact cause/address/PC traces and
the missing-register and stale-root negative controls. Run
`python3 tools/run_b06_abi_guests.py`. Require both reference guests and all
negative controls to produce their defined status. GPA1 maps T58.01.a/b/c and
T58.04.a/b/c to reset/context/stack evidence; GPA2 maps T58.06.a/b/c to trap and
return; GPA3 maps T58.07.a/b/c to translation/permissions; GPA4 maps T58.08.a/b/c
to binary/helper audit. Level0 misidentifies ABI as ISA; level1 runs only happy
path; level2 provides the listed independent observations and invariants; level3
derives an unseen mapping/return defect and adapts the implementation after
seven days without copying the assessor.

Hints progress from register ownership, to saved-PC distinction, to a table-walk
worksheet before bit encoding. Changed conditions add an FP/SIMD-preserving
context, two independently owned task stacks, hidden aggregate return, and
branch/veneer reach. These remain required separate variants, not inferred from
integer context success. Assessor implementations/analysis live under
`assessor/architecture_ports/guests/`. Only original code in isolated QEMU with
no disks/network is targeted. No physical board, multicore TLB shootdown, general
OS, asynchronous complete exception frame or learner mastery is claimed.

Inspected reference bindings: R-B06-FOLLOWUP-AAPCS32, R-B06-FOLLOWUP-RVABI, R-B06-FOLLOWUP-RVSUP, R-B06-FOLLOWUP-AAPCS64, R-B06-FOLLOWUP-AAMMU, R-B06-FOLLOWUP-AAEXC.
Exact passages and inspection limits are in
`research/v5/expansion/b06/followup_resources.json`; source titles alone are not
inspection evidence. Retain those locator bounds when changing assumptions.
