# Required firmware capstone composition

This successor supplies the twelve ordinary and twelve named delayed tasks in
PR-CAP-MCU-01..04. Their exact original objectives, rubrics, prerequisites and
seven-day assessment interval remain normative. The original capstone shared
contract and `../core/CONTRACTS.md` apply. Source locators and execution limits are
in research/v5/expansion/legacy_capstone_firmware; no prior acceptance is rewritten.

## Environment and concrete inputs

Use active C17 with the actual learner P3 protocol, P4 CPFRing/CPFSample/CPFClock,
T28 PdTransfer/PdIo and T29 XfFrame contracts. Original assessor sources are
separate and explicitly selected by the author campaign. Host GCC/Clang and
Python provide verification. Target builds use Cortex-M4 Thumb soft-float, explicit
original byte/AEABI helpers, no libc, compiler runtime archive, allocation, HAL or
libatomic. Inspect final symbols, linker map and actual generated instructions.
CPU startup/MPU execution is MPS2-an386; the F439 image is separately cross-built
and audited. Neither is a physical F439 observation. No tool here flashes a board.

`run.py` takes explicit `--source` and `--include` arguments for your Cf, Cs, P3,
P4, Pd and Xf implementations; it compiles `checks.c`/`tests.c` and executes all
eight named case numbers. No assessor path is selected implicitly. Unfinished
public scaffolds compile but deliberately fail a semantic check. For the target,
compile these same providers plus your target driver and supplied `startup.S`
with the flags above, provide linker symbols `__stack_top`, `__data_load`,
`__data_start/end`, `__bss_start/end`, and implement the declared target entry and
trap/PendSV/fault/SVC callbacks. Use the source-derived ranges and ownership
contracts below; the private reference target is withheld.

The ordinary service inputs are six samples with sequence100..105, valuesA500..A505
and16-bit ticks65530..65535. The sequential and three-at-a-time acquisition paths
must encode identical108 bytes. A capacity4 CPFRing has three usable slots; a
fourth paused offer refuses. Candidate/core queue capacity is3. Once that queue is
full, another acquired frame is explicitly rejected and counted. A timeout
produces no sample and no watchdog progress. The synthetic command is a validated
P3 message with one payload byte0/1, a16-bit sequence and an independent trusted
authorization argument. Its effect is only an integer actuator state.

## Representation, operations and ownership

`struct cf_firmware` owns a portable CsService, a CPFRing, a PdTransfer and two64-byte DMA
buffers. It also owns finite recovery/replay counters and first-fault state. It
does not expose Linux descriptors or register addresses through the core API.
BOOT reaches RUNNING only after bounded clock readiness; a two-observation
timeout enters SAFE_STOP. Recoverable faults enter RECOVERING and force actuator0;
the third fault after two attempts enters SAFE_STOP. Later faults cannot overwrite
the first identity. External bytes already emitted cannot be rolled back.

One acquisition ISR is the sole ring producer; the foreground is its sole consumer.
Other Cf operations are serialized foreground operations. Before `cf_recover`,
the caller must disable/quiesce the acquisition source and all old notifications;
the function cannot infer that fact from elapsed time. It refuses while a DMA or
core output lease remains outstanding. At quiescence it accounts the old core's
accepted/delivered/discarded totals, resets the ring/core for a fresh epoch and
retains command replay history. At most three1000-admission epochs keep those
totals within uint32_t. Reinitialization while a producer/borrower can run is a
contract violation. All object/pointer lifetimes and exclusive access are caller
obligations; C does not supply affine ownership or validate arbitrary pointers.

`cf_dma_begin` imports actual PdTransfer with one8-byte transfer, full64-byte source
and destination capacities, width1 and increasing nonzero tokens. The selected
F439 addresses must be owned AHB SRAM, not CCM. The host model maps numerical
addresses to the two original arrays. Cancellation requests stop and retains both
buffers. The model writes more bytes after the stop request and after EN clears
while FIFO is nonempty. Only matching token, EN0, FIFO empty and TC permit Pd
finalization. A cancelled/error result publishes no sample. A stale snapshot must
not complete a newer token. Numeric range checks do not establish ownership or
physical reachability. A timeout that never obtains quiescence quarantines state;
it cannot safely recycle it.

The F439 adapter consumes the frozen original P4 setup/transfer implementation
directly, with its original entry renamed and discarded. It connects actual
PendSV samples, Pd DMA2 memory-to-memory acquisition, Cf foreground framing and
P4 polling/DMA1 USART2 transmission. Three of five paused sample offers survive;
each transmitted frame retains its capstone lease until P4's selected completion
contract, and only then supplies progress mask3. The port assumes dedicated cold
reset, reset HSI, PA2 AF7 USART2 and exclusive DMA streams. These are source/build
contracts until a board protocol is executed. On timeout it retains ownership,
records a fault and stops; no failed prefix is silently replayed. Its actual
privilege configuration remains privileged with MPU disabled. The separate MPU
guest is an evaluated alternative, not an undisclosed change to that board image.

## Authority, recovery and persistence

CRC is only wire validity. Unauthorized commands have no accepted/effect change;
a trusted command is applied only after bounded admission. Repeating the exact
latest sequence/value returns REPLAY without another effect. A different value
under the same sequence and all older sequences refuse. Sequence65535 retires
that command namespace until a separately authenticated reset/epoch policy;
there is no implicit wrap. High-rate telemetry, malformed bytes, transport replay,
clock timeout, processing stall and reset are different cases in the threat table.

Each watchdog observation is the current complete epoch mask, not accumulated
across calls. For1,3,2,3 the results are false,true,false,true. A timeout or an
acquisition-only epoch cannot stand in for processing progress. The fixture models
the feed decision, not a physical IWDG oscillator or reset deadline.

The update model keeps committed slotA generation7 untouched while B generation8
passes payload, metadata, CRC, trusted verification and commit. Cuts0..4 selectA;
cut5 selectsB only if compatibility and trusted verdict are true. `verified` is
an external trusted model predicate, never an attacker-writable signature flag.
This exercise does not implement signature verification or flash erase/program.
The changed commit has two separately written complementary marker words: either
incomplete marker rejects. The admitted generation ordering is a16-bit serial
window strictly smaller than32768; equality and half-range ambiguity refuse.
65535→0 can be newer in that bounded window, unlike naive numerical comparison.
This encoding does not create physical atomicity or protect arbitrary corruptions.

Crash records separately contain sequence, first cause, CRC and commit. A reset
at each of the four stores preserves the previous committed record until all
fields validate. No valid record means absent/incomplete capture, not no fault.
The32-bit crash sequence retires before wrap; the caller cannot reuse1 after
UINT32_MAX and claim freshness. Publication/storage assumptions must accompany
each result. Actual reset electrical behavior and flash atomicity remain separate.

## Budget, startup and isolation

Reproduce the exact synthetic RAM sum1896, leaving2200 of4096. With2048, adding160
requires2056 and must reject before publishing enable. `cf_budget` preserves both
outputs on refusal and checks arithmetic before adding. The synthetic ledger is
not sizeof(CfFirmware). Replace it with actual target static sections, buffers,
metadata, stack-usage records, selected call paths and exception/nesting allowances.
The linker reserves a computed conservative stack account for multi-instance test
frames and the selected nesting bound; that must not be called a4096-byte deployed configuration. Report
admission/refusal under the actual selected image ledger rather than forcing it
to fit the historical synthetic number.

Actual XfFrame distinguishes basic8-word and extended26-word frames; FP adds72
hardware bytes before alignment/software-saved state. Lazy FP reservation is not
permission to read unmaterialized FP values. A new nesting or FP policy requires
new call-path, ABI/FPU and interrupt-exclusion evidence, not just increasing a
constant. Current soft-float builds do not establish physical FP latency.

Startup audits include vector entry/Thumb bit, aligned in-range initial stack,
data LMA/VMA and zero region. The guest poisons RAM before actual reset. A wrong
copy bound and omitted zeroing reject at predeclared startup checks; a wrong reset
vector is rejected from the image before execution. Reordering data and adding an
aligned initialized object recomputes the linker-derived ranges. A successful
link alone cannot detect a wrong copy loop.

The host pointer counterexample writes through a live ordinary pointer despite
an alleged C module boundary. The actual MPU guest maps code RX, RAM RW/XN and a
higher-priority32-byte privileged-only region; an unprivileged store causes a
specific MemManage fault and leaves the synthetic guard unchanged. The handler
resumes only the original known instruction. SVC copies a validated offset/count
from a fixed live8-byte mailbox, then returns to privileged mode. Another writer
or DMA master must be excluded during copy-in; mutable request bytes must not be
re-read after validation. The handler's privilege does not authorize arbitrary
untrusted pointers. MPU protection here covers CPU accesses, not DMA/debug/NMI
authority. This bounded teaching continuation is not a generic fault recovery API.

The register model has W1C bits1/2 and a read-clear bit4. Direct acknowledgement of
bit1 preserves bit2 and does not read/consume bit4. A generic read/OR/write clears
both other observations. This is an original exact model, not a claim that every
F439 status register has this layout. Imported Pd/P4 ports retain their actual
selected-register semantics.

## Evidence, release and assessment

Produce D1 architecture/context/threat/region diagrams and assumptions; D2 original
C implementation plus exact unchanged provider identities; D3 independent fixture
derivations, all failure traces, maps/disassembly/stack records and source-bound
commands; D4 the exact named delayed variant and revised decision record. Hints:
separate stop from quiescence; mark every external prefix before rollback; label
each byte CPU/ISR/DMA owner; draw commit-last order before selecting a record;
put timer epochs beside modulo arithmetic before converting ticks to time.

Compare actual errors.result on the host under equal success/failure behavior,
including its size and skipped success callback. Inspect its target header/object
closure before import: the family header also introduces setjmp declarations.
The selected strict core uses plain status/out-parameters. Runtime tags reject
only checks actually implemented; C diagnostics do not prove ownership, exhaustiveness
or temporal safety. Apply all13 modern-cost dimensions through the original shared
cost contract; allocation is zero in this core, and synchronization/context/stack
costs are explicit rather than hidden by the representation comparison.

The release dossier binds exact core source, compiler flags, ELF, derived binary,
map, vector and symbol artifact. Re-derive binary bytes from the declared ELF and
reject a stale ELF/new-binary pair even when both have valid individual checksums.
The stripped image links an external symbol file by its recorded hash and actual
debuglink CRC; filenames alone are insufficient. This establishes build-artifact
association, not that a device contains those bytes. The dedicated-board protocol
must separately qualify board/manual/errata/schematic/power, obtain authorization,
verify deployed bytes, power-cycle, capture UART/DMA/safe outputs and bind instrument
and image identities. No such physical result is supplied by this author run.
The concrete later-session protocol is `BOARD_CAPTURE.md`; it separates required
byte verification, cold power-cycle, raw captures and unresolved target extensions.

For irq.json,65530→6 is12 ticks only with a bounded wrap assumption. At16MHz this
is0.75us; at8MHz it is1.5us. Integer microsecond truncation loses information.
Two wraps give65548, which the raw16-bit pair cannot distinguish. Use an explicit
observed epoch or a justified maximum interval. Keep measured maximum, timer
resolution/clock, analytical interference assumptions and WCET claim separate.
Host timing is not physical GPIO timing. A CPU-only emulator cannot support the
same peripheral claims as a validated selected-device model.

The property matrix rejects host timestamps for physical timing, ELF hashes for
deployed-byte verification, and category substitution between parser, helper,
DMA, startup, reset and output claims. Category validation does not independently
verify arbitrary evidence. Unsupported physical claims remain required open gates.
The separate assessor analysis and private changed schedule preserve original
rubrics. Competent work explains invariants, diagnoses every discriminating fault,
and preserves limits; independent mastery additionally reconstructs changed tasks
after the original delay without the solution. Author tests are not that event.

All defects target original bounded synthetic firmware/models; harmless success
is a detected counter, stale lease, lost flag or forbidden synthetic store. No
third-party service, operational vehicle, real credential or deployed equipment
is involved. Repair and regression controls must retain ordinary valid behavior.
Validation status is the exact current campaign/export/guard receipt, not this
prose. External review, independent learners and physical observations remain
unperformed, and finite passes do not prove absolute mastery.
