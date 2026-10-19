# H745 flash words, pending programming and a joint-image record

This required T59.01/.04/.05/.07 continuation uses the B06 shared exercise
contract and original project/objective rubrics. Families A/B/C/D/F/G/I/J.
Prerequisites: `H745_CACHE.md`, journal recovery, exact image/signature binding,
checked extents and the learner32-byte-unit allocator/ownership ledger. The
previous4/8-byte image model is **not** an H745 flash adapter: RM0399 describes
256-bit flash words with ECC and warns against rewriting nonvirgin words.
`PSIZE` is programming parallelism, not permission to rewrite part of a flash
word. A marker cannot be written later into an already programmed word.

## Environment and supplied artifacts

Use C17 host tests and Cortex-M7/M4 freestanding code generation. The original
`h745_flash_port.c` supplies volatile word access and DSB; implement the policy
in `h745_flash.c`. Select H745xI, unswapped bank1, and a **previously erased,
exclusively owned**32-byte word in0x08020000–0x080FFFFF, beyond the reserved
boot sector. The caller must keep source words in live immutable SRAM disjoint
from the session. The adapter does not erase or change option bytes. Clock,
supply, write-delay/parallelism, MPU and flash protection must already match the
selected manual and exact part. Execute code/interrupts from the other bank or
SRAM; map programmed data noncacheable until readback. No other core, interrupt,
DMA, debugger or flash client may program or configure this bank concurrently.
Readback ECC BusFault containment is a separate target fault-handler obligation.

## Concrete problem and state transitions

`fh_start` owns an initialized inactive session. Validate the callbacks,32-byte
alignment and allowed bank extent before MMIO. Read status/control and reject
an active queue/buffer/CRC/erase/program or unsupported control bits. Preserve
the original PSIZE/lock policy. Unlock only when LOCK is set; sending the key
sequence twice is not idempotent. Clear the old operation's flags, enable PG,
write all eight consecutive32-bit words, DSB, and return PENDING. No force-write
is permitted. The eight write-buffer bytes/words cannot be retried independently.

`fh_poll` reads status once. QW, BSY or WBNE means PENDING even if EOP is also set.
With no completion/error indication it remains pending. A freshly cleared EOP
followed by EOP witnesses completion even when software missed the short QW-high
interval. Once quiescent, error flags fail; otherwise compare all eight readback
words. Only then disable PG and restore the initial lock state. A software
deadline does **not** cancel programming: leave the session and exclusive bank
ownership live, continue polling or escalate to the separately qualified reset
policy. Never report unchanged flash on error. Quarantine an uncertain word;
an all-one readback alone does not prove its ECC word was never programmed.

`fh_record` creates128bytes from a quiescent, current joint-health readiness
snapshot and caller-verified image identities. Bytes0–95 contain LE32 magic
0x314A3642, nonzero sequence, boot generation, reserved0, two32-byte image
digests, two vector bases and two image versions. Bytes96–127 are a separately
programmed commit word: LE32 magic0x31433642, sequence, generation, complemented
sequence, CRC32 of the96-byte body, then12zero bytes. CRC is reflected
0xEDB88320 with initial/final all-one XOR. Build from disjoint live inputs; reject
unchanged unless both peers are healthy, confirmed and without outstanding work.
Program the three body words into virgin storage, confirming each operation
before the next; program the separate commit word last. Retain the old committed
record throughout. A failed program may have changed storage; do not reuse it.

`fh_append_begin` and `fh_append_poll` compose this ordering through the actual
session driver. The initialized append owner has state0 and owns four virgin
consecutive words. It copies the admitted record, starts word0, and advances
only after the previous driver's DONE. State1 retains pending ownership; state2
is completed and state3 quarantines any failed/rejected start or completion.
Neither terminal state can be reused by another begin call. The caller must keep
the append object/backend live until the hardware session is quiescent. The test
injects an error at each of the four words and checks that later words are never
started, earlier words are never reprogrammed, and a second begin is rejected.

This is an append-record primitive, not an erase/garbage-collection algorithm or
complete bootloader. Recovery validates format/CRC/commit consistency and then
**revalidates both images, exact digests, signatures, version floors and vector
admission** before boot selection. The record's CRC detects the specified cuts
and corruption; it does not authenticate an attacker-controlled record. The
health snapshot defines the admitted generation at commit time, not a promise
that either core will remain healthy later. Persistent policy and signed-image
composition reuse the earlier T59 artifacts under this stronger program-unit
contract; a full board startup/erase/recovery transport still needs integration.

## Deliverables and verification

D1: policy code, byte layout, no-reprogram invariant and sequence/epoch exhaustion
policy. D2: exact MMIO traces for locked/unlocked, ongoing/busy, errors and
indefinitely pending operation; show that1000 polls do not clear PG. D3: all1025
bit-prefix cut results and128 single-byte corruption witnesses, plus independent
Python struct/zlib comparison of the exact128-byte record. D4: M7/M4 generated
code, helper closure, stack reports and a table of separately pending board
preconditions. Run `python3 tools/run_b06_h745_flash.py`; require three host
profiles, three semantic mutants and the learner stub rejected by named
`B06_FLASH_CHECK` assertions. The model simulates status/register transitions;
compile/link evidence is not real flash execution or power-loss measurement.

## Assessment, hints and changed conditions

T59.01.a/b/c must distinguish byte/program/ECC/erase units, implement recovery
and falsify same-word late-marker designs. T59.04.a/b/c must bind both image
identities to the joint generation, preserve outstanding ownership and transfer
to a failed peer after first health confirmation. T59.05.a/b/c must classify
partial/indeterminate programming and derive append/recovery rather than retry
in place. T59.07.a/b/c must integrate cache visibility with flash readback and
retain actual H745 physical trace obligations. For each leaf: level0 conflates
PSIZE/word size or timeout/cancellation; level1 ordinary programming only;
level2 D1–D4 and every mapped invariant/counterexample; level3 independent delayed
transfer after at least seven days, preserving the original transfer IDs.

Hints: draw the ECC word first; separate starting from finishing an operation;
then place a commit marker in its own virgin program word. Changed conditions:
move to bank2 with swap enabled, then to F439's distinct erase/program geometry;
derive fresh numeric/register assumptions rather than substituting addresses.
Introduce a torn ECC read that faults and design bounded fault containment before
claiming recoverability. Add log reclamation without erasing the last valid pair.

Security scope is original synthetic firmware and local register models. The
runner never flashes, erases, resets hardware, changes option bytes or touches
deployed equipment. Exact-board errata, physical cuts, persistent dual-image boot
integration, independent learner and external review remain explicit gaps.
Source record R-B06-FOLLOWUP-RM0399-FLASH bounds the inspected manual passages;
`h745_flash_validation.json` holds actual author checks. Separate assessor
analysis: `assessor/firmware_lifecycle/targets/H745_FLASH_ANALYSIS.md`.
