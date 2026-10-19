# Selected flash erase, program and joint-record reclamation

This required T59 continuation composes the existing H745 program-word backend
and joint journal. It supports original G-B06-T59-TARGET and
G-B06-T59-04-TRANSFER; a compiled port is not a physical flash experiment.
Use the B06 common contract and original project rubrics together with this
concrete specification and the separate assessor analysis.

## Prerequisites and exact environment

Explain the existing FHSession/FHAppend pending/quarantine rules and construct
the two-image FWPeer health/loan predicate. Supply the learner's H74532-byte
program backend, journal CRC and peer-generation implementation. Read the
registered RM0399 revision4 erase/control passages and RM0090 revision21
F42/F43-specific geometry and registers. Do not reuse an F405 register layout.

Language is ISO C17 with explicit GCC/Clang Arm inline-assembly extensions only
in the target port. Host models use stdio for diagnostics, outside target code.
Targets are STM32F439 2MiB, unswapped, and STM32H745xI 2MiB, unswapped. The
exercise owns only bank1's last two128KiB sectors: F43910/11 and H7456/7 at
0x080C0000/0x080E0000. No option bytes, mass erase, protection removal, bank
swap, or real hardware programming is authorized by running the host lab.

For a dedicated target run, supply qualified stable supply/clock/latency and
device-revision evidence first. F439 uses32-bit parallelism at2.7..3.6V without
external VPP. H745 preserves the caller's qualified PSIZE. The flash operator,
all callees/constants/vector handlers used while busy, stack and immutable
input live in SRAM or the other nonbusy bank. The other core, interrupts and
DMA must not access the owned bank. Privileged callbacks cannot reenter, fail,
retain temporary buffers, or access unowned addresses. No dynamic allocation,
libc, implicit helper or floating-point dependency is allowed on target.

## Problem and necessary distinctions

Implement flash_maintenance.h from the supplied stubs. An initialized-zero
FMOperation starts one sector erase or one aligned F439 word program. Reject
unknown profiles, other sectors, busy/unsupported register state and invalid
addresses before side effects. Return FH_PENDING after issuing the operation;
poll retains ownership until quiescent, then restores the original lock and
parallelism controls, refreshes the read view, and verifies at most32words per
call. Every readback mismatch or hardware error is terminal FH_ERROR; uncertain
cells must not be reused in place. A new operation requires a fresh operation
object after the old one reaches a terminal state. A timeout cannot cancel a
flash controller command or permit early register restoration.

F439 EOP is not a mandatory poll witness: RM0090 says it is set only when
EOPIE is enabled. This polling profile leaves interrupts disabled and uses
post-command BSY clear, error flags and actual readback. H745 retains the
existing fresh-EOP/error rule after clearing old status, checks all busy flags,
and never mistakes a queued erase for cancellation. In F439 bank2 the encoded
sector numbers have a hole; these APIs reject that unsupported profile instead
of applying a guessed linear encoding. Explain why all-ones bytes alone do not
prove H745 virgin ECC after a torn program; only an admitted completed erase
establishes reuse under the digital model.

FMIO.fresh establishes a coherent subsequent read view. The supplied F439
port disables the flash-interface I/D caches, resets them through FLASH_ACR,
then restores their previous enables/latency/prefetch; resetting enabled caches
violates the source contract. The H745 port invalidates whole32-byte data-cache
lines at PoC with DSB/ISB. The owned log range must contain no dirty CPU data and
no executed instructions: this is not a general instruction-cache update API.

## Joint-record transaction and recovery

fm_recycle_begin requires a valid old record at offset0 of one owned sector,
an authenticated new image pair, strictly increasing nonzero sequence, and a
confirmed healthy FWPeer with no outstanding/reserved loans. It snapshots and
validates the old record, constructs the new record, then erases the other
sector. Program the three32-byte body words and final separate32-byte commit
word once. Refresh/read back the complete new record; only after exact
validation erase the old sector. Recovery accepts one valid record, chooses
the larger sequence if both are valid, and rejects unequal records with equal
sequence. Counter wrap retires the sequence space. This bounded two-slot
reclamation transaction is not a multi-record search engine.

At every interruption there must be at least one valid old-or-new record,
assuming an interrupted operation affects only its designated sector/word.
Real torn H745 reads may raise ECC faults: fm_recover accepts already readable,
fault-contained snapshots; it does not recover from a BusFault. Sequence/CRC
are not authentication or antirollback security. Image signature, version
policy and startup admission remain separate composition obligations.

## Deliverables and verification

D1: derive both register traces, supply/wait-state and ownership assumptions,
state-transition diagram, pre/postconditions and finite-loop resource bounds.
D2: implement the APIs and actual MMIO ports; inspect target ELF undefined
symbols, stack-usage records and generated access/barrier instructions at
O0/O3 for Cortex-M4/M7. Host tools are not linked into the target.
D3: run the public model with busy, protection-error, failed-readback,
programming0-to1, wrong-address and invalid-state controls. F439 must complete
with EOP remaining0. A callback never reads owned flash while busy.
D4: run40 original digital-cut histories: first/second/middle/last inactive
erase granules, every32-bit store of the128-byte new record, selected old erase
granules, and uninterrupted completion. Recover old or new, never neither.
Inject a post-program verification corruption and prove old sector is not
erased. Reject equal-sequence conflicting valid records.
D5: falsify the busy-skipping, readback-skipping and pre-reclaim-validation
mutants. Compare H745 ECC-word and F439 word/row rules using a changed
assumption, not a substituted constant. Record original command output and
state exactly which failures remain physical/model-only.

## Rubric and hints

Level0: an API compiles or an ordinary erase returns success without evidence.
Level1: ordinary traces work but busy ownership, freshness, failure or committed
record preservation has no argument. Level2: all D1–D4 obligations have traced
source/contracts, meaningful positive and negative evidence, and scoped
limitations; no error is reported as safely cancelled. Level3: independently
derive D5 changed-profile behavior and expose an invalid equivalence, keeping
the same correctness contract and accounting for retained resources.

Hint1: distinguish the configuration interface, flash memory writes, hardware
queue completion and the CPU cache view. Hint2: draw which sector owns the last
valid commit before every destructive action. Hint3: use separate operation
and verification phases; bounded polling need not scan an entire sector at
once. Assessor details are withheld under assessor/firmware_lifecycle/targets.

## Changed conditions, security and evidence

Delayed transfer uses the original project transfer identity and seven-day
policy: derive an F439 bank2 extension using its encoded sector-number gap;
reject1MiB/DB1M geometry until its separate source/profile is qualified. Then
change the flash model to fault on torn ECC reads and design a bounded
fault-contained scanner before claiming complete boot recovery. Compare the
two-slot algorithm with a rolling multi-record log under a stated endurance
budget. These are author investigation and independent learner obligations,
not claims that the present implementation covers the changed hardware.

The original host model uses synthetic image digests and memory only. Its
attacker may corrupt staged journal bytes or choose untrusted sequence/record
inputs; success means rejecting invalid state or observing an original mutant
erase its last valid record. No third-party firmware, credentials, production
target, physical flashing, host setting or external service is involved.

Authored and executable; actual source-bound commands/results are in
research/v5/expansion/b06/flash_maintenance_validation.json. Physical voltage,
errata, retention, power cuts and complete signed reset-to-image handoff remain
separately registered. Finite digital cuts do not prove physical atomicity,
universal safety or independent learner mastery.
