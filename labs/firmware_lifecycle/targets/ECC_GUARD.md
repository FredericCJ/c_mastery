# FW-ECC: fault-contained journal snapshots

This required B/C/D/F/G/I continuation supplies the missing read boundary for
G-B06-T59-TARGET and G-B06-T59-04-TRANSFER. Use the B06 shared contract and
FLASH_MAINTENANCE.md; the original objective/transfer identities stay unchanged.
Prerequisites are basic exception frames/EXC_RETURN, precise versus imprecise
faults, original FH record validation and the two-sector recovery invariant.

## Exact problem and environment

Implement ecc_guard.h from the stub. Hosted C17 tests exercise the policy and
scanner. The target adapter uses explicit Armv7-M assembly/MMIO, privileged
startup thread mode on MSP, a basic non-FP frame, eligible enabled BusFault
priority, no concurrent IRQ/NMI/other caller, and resident fault-handler code,
data and stack outside the flash bank being inspected. No PRIMASK/FAULTMASK
may suppress BusFault. Caller-supplied output is live writable aligned SRAM;
input addresses are admitted by trusted slot geometry, never arbitrary user
addresses. This API is not a general memory probing facility.

RM0399rev4 section4.7.7 says uncorrectable double ECC errors generate a bus
error and record flash status/address. PM0214rev10 distinguishes a precise
data fault's stacked PC/BFAR from imprecise, instruction, stacking and lazy-FP
faults. Recover only the exact expected basic-frame EXC_RETURN0xFFFFFFF9,
CFSR0x8200, Thumb thread xPSR, admitted address and the designated load PC.
Replace stacked R0 with0 and PC with the designated abort label; preserve all
other frame words. Any mismatch returns rejection without changing the frame.
The installed handler enters the fail-stop hook on rejection, never skips an
arbitrary instruction. Do not turn a broad fault-handler catch into success.

eg_read_word marks one active admitted load, executes one word LDR, stores to
output only after success and returns1. A recovered load returns0 without
changing output; the next unrelated valid load can succeed. Clear only the
observed CPU fault bits after recording the exact frame. H745 flash ECC status
and first-fault address remain available for separate diagnosis; the manual
does not require clearing them before a subsequent distinct read. Do not retry
a quarantined torn word in place or infer virgin ECC from visible all-ones.

eg_snapshot reads32words into private128-byte staging, and publishes only when
all32 reads return exactly1. On any failed word, preserve the caller's output.
eg_scan_pair obtains separate fault-contained snapshots, marks an unreadable
candidate unavailable, and calls the actual fm_recover record selector. Return
0 if neither is readable/valid,1 or2 for the selected record, and-1 for a
conflicting equal-sequence pair. Publish only for1/2. A fault on one candidate
must not discard a valid record in the other sector.

## Deliverables, assessment and variants

D1: derive exact exception admission, stack-layout assumptions and the relation
between CPU CFSR/BFAR and H745 flash ECC registers. D2: implement the scanner
and target load/handler; supply the no-reentry and output-ownership argument.
D3: test every single-bit change of the accepted CFSR, wrong address/PC,
extended/PSP frames, all32 failed-read positions and64 candidate/word faults.
Verify unchanged rejected frames/snapshots and exact surviving-record selection.
D4: execute original MPS2 M4/M7 images with an actual precise BusFault at a
deliberately invalid teaching address, then a valid read. Separately execute
an unguarded read and a faulting output store; both must reach the refusal hook,
not the successful recovery path. Inspect target assembly, linker/helper
closure and bounded stack records. D5: kill the wrong-PC/address and early-
snapshot-publication mutants; reject the learner stub semantically.

Level0: treat any exception as a recoverable missing record. Level1: ordinary
snapshot or one synthetic fault only. Level2: D1–D5 with exact frame identity,
transactional staging and actual CPU evidence. Level3: independently redesign
the original delayed boot-recovery variant after seven days for PSP/extended
frames or a pending imprecise fault; do not broaden admission by deleting
checks. Hints: distinguish the instruction that faulted from its successor;
write the allowed frame edits before assembly; stage bytes before record
selection. The separate assessor analysis explains the intended negative
controls and limits.

## Isolation and validation limits

All faults target the original teaching program in disposable QEMU. No real
flash corruption, hardware fault injection, external device or host security
setting is involved. The harmless success condition is a recovered exact load
with preserved output, or refusal of a different fault. No privilege-escalation
or arbitrary-code-execution claim follows from either.

This code observes actual Cortex-M fault entry/return; QEMU's invalid address
is not an emulated STM32 ECC controller. Physical H745 ECC routing, cache/
prefetch effects, handler placement under actual linker/clock configuration
and silicon errata remain target qualification requirements. Source and exact
commands are recorded in ecc_guard_validation.json. Author executions do not
establish independent review, learner transfer or physical fault tolerance.
