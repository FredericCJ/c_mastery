# T31 connection and optimized-state notebook

## Identity, families and objective boundary

This successor implements the six original `AX-PR-T31-01-T31.01.a` through
`T31.02.c` criteria and their existing `-DELAYED` variants. Families A/B/C/D/F/I/J
are assessed through the distinct tasks below. It preserves the original project,
unit identities, six-hour historical description and historical receipts; the
added work has no time ceiling. Current support is in
`research/v5/expansion/legacy_embedded_debug_connection/support.json`.

The deliberate misconception is that a saved debugger command, generic Cortex-M
configuration or source-level backtrace establishes physical device identity,
Flash support or exact machine state. None of those inferences is valid.

## Prerequisites and inspected references

Bring learner-owned PR-T27 bring-up evidence, PR-T31-01 fixture predictions and a
bounded byte/checksum routine; know object lifetime, unsigned arithmetic, ELF
symbols, Cortex-M reset vectors and the distinction between host and target code.
Public sources here are unsolved. The original helper implementation and assessor
routine are not learner dependencies; `build.py` accepts explicit completed
routine, runtime-helper and linker paths.

The source register identifies actual passages of GDB's Optimized Code,
Watchpoints, Inline Functions, Tail Call Frames and Remote Configuration manual,
OpenOCD Reset Configuration and QEMU's GDB documentation. Their exact versions
and inspection limits are recorded separately from installed tool versions.
Published C17 and physical probe/board manuals remain distinct reference needs;
the current GDB documentation is not a statement that every installed release
behaves identically.

## Environment and dependency profile

Retain profile E. The connection core is C17, caller-owned fixed storage and no
library calls. Hosted tests and Python oracles are verification dependencies.
The original freestanding image is Cortex-M4, soft-float, `mps2-an386`, explicit
startup/vector/runtime/linker sources, no standard library or implicit runtime
archive. Audit maps, undefined symbols, assembly, stack-usage and DWARF outputs.
GCC debug/release/ASan+UBSan and Clang equivalents exercise the connection core.
Target configurations are GCC O0, GCC O3, GCC O3+LTO and Clang O3; actual debugger
capabilities are observations, not ISO C guarantees. `__attribute__((noinline))`
and LTO are toolchain mechanisms, not ISO C syntax or enforcement.

`probe.py` runs only a fresh original QEMU process via an owned private UNIX
socket. It never programs Flash, opens an external target, changes host security
or exposes a TCP listener. A private extracted `gdb-multiarch` is supported.
The optional relay delays each chunk by 1 ms and records actual host elapsed
command times; it does not model a particular physical network or probe latency.

## Concrete inputs, outputs and supplied artifacts

Implement `connection.c` and `routine.c` from the headers. `tests.c`, `replay.c`,
`oracle.py`, `notebook.py`, `m4.c`, `build.py` and `probe.py` are supplied.
The three original connection fixtures classify as connection, Flash and
reset/application. Add inconsistent Boolean combinations, wrong target ID,
unknown power, changed board/reset identity, and vectors at/exceeding extent.

The target input bytes are `[1,2,3,4]`; checksum is the sum of each byte multiplied
by three, giving 30. Changing the second byte to 7 gives 45. The routine performs
an observable volatile input read into an otherwise dead local and uses a loop
invariant. Later stores put 7 into the counter twice, then `dc_tail(9)` calls a
leaf which stores 9 and returns 10. The final marker is `dc_done=1`. All values are
synthetic. A checksum of an empty range is zero; a live extent of `n` bytes is
required for nonzero `n`, and null is permitted only for the empty range.
Unsigned sums follow `uint32_t` modular arithmetic; no security checksum is claimed.

## Representation and operations

`struct dc_manifest.known` has exactly ten bits, one per nonzero identity handle. A zero
handle is unknown; unknown transport requires zero stored latency. Handles name
immutable caller-owned records: the configuration record contains its hash and
actual command, and the probe/device/power records distinguish observations from
intent. Handles alone are neither authentication nor a device detector.

`dc_init` requires a fresh/discardable destination. Other operations require a
live successfully initialized session; the caller cannot mutate it behind the
API. Session, manifest and output objects are disjoint, live and correctly typed.
Calls are externally serialized. Null control/manifest/output or invalid shape
returns INVALID. This API does not try to validate arbitrary corrupted pointers.

Identify requires phase0, known board/probe/firmware/server/config/target/reset/
power and exact observed target identity. Halt requires phase1 and an actual true
observation; reset-test requires phase2 and true reset/halt observation. False
observations return UNKNOWN without advancing. Programming authorization requires
phase3 **and separately known Flash driver support**. Authorization emits no
programming command and does not establish that a physical driver is correct.

Any changed environment identity or latency revokes prior phase and target
observations. Identical reconfiguration preserves them. Board/reset changes must
therefore reacquire all evidence. Failed admission preserves state/output. All
operations have bounded constant storage and work; they perform no allocation,
I/O, retries or unbounded wait. `dc_classify` refuses impossible sequences
programmed-without-connected or reset-without-programmed. Its successful class is
only the next investigative stage, not a root-cause diagnosis.

`dc_vector` accepts a nonempty 32-bit numeric extent with representable end,
checks a Thumb low bit and at least two bytes available at the masked entry.
It does not read memory, validate an MSP or establish executable access. A wrong
vector can explain failure after programming, while a power fault is an
independent hypothesis requiring electrical evidence.

## Connection and reset investigation

Keep disconnected/identified/halted/reset-tested transition logs. Do not infer
Flash programming support from the CPU name. For a changed board revision or
reset wiring, revoke the old manifest; record actual pin mapping, voltage domain,
drive type, reset gating and delays only after independent inspection.

The reset-loop variant is a **supported experiment plan**, not an executed board
claim: first confirm device/probe/board documentation and electrical compatibility;
identify whether SRST is available and whether debug access remains possible while
asserted; then select documented connect-under-reset support. OpenOCD's
`connect_assert_srst` requires `srst_nogate` and target support. A missing wire or
gated debug path is not repaired by copying those tokens. Plan a bounded assertion,
attach/identify/halt observation and release check, with explicit abort conditions
for unknown wiring/power or unsupported behavior. No Flash operation is needed to
classify this experiment. Physical execution requires a dedicated teaching board
and fresh authorization; it is outside this author campaign.

## Machine-state and failure reasoning

Predict 30 and 45 before execution. At each stop retain exact ELF/source/config
hashes, PC/SP/LR, memory, symbol/address, source-line and assembly observations.
Separate the source expression, an executed instruction and the debugger's
reconstruction. Do not invent a dead local's contents when GDB reports unavailable.
At O0 a local shown before initialization is not a trustworthy source-level value.
Inspect instruction position, not only the displayed line.

Compare ordinary O3 and LTO attribution. A source breakpoint can land in the
caller when a function is inlined. A tail branch can reuse the caller's return
address without pushing the expected physical frame; GDB may still display a
virtual tail-call frame using DWARF. LTO need not preserve the non-LTO decision.
No expected optimizer transformation is a portable C guarantee.

The deliberately flawed reference controls omit a byte or change the factor.
They are defined, inspectable wrong algorithms. Identify the first incorrect data
contribution and repair the actual routine; rerun the same optimized configuration.
Changing only to O0 is not a repair, and compiler/sanitizer failure is not the
intended semantic rejection.

## Watchpoint investigation and changed assumptions

Request access observation of the live four-byte counter. Record actual stop PC
and assembly; do not assume it denotes the address of the store rather than the
next instruction. Compare value-change `watch` with `awatch`: the second store
of 7 is an access even though the value does not change. Preserve the two actual
transcripts and demonstrate their different stopping behavior.

QEMU TCG system emulation advertises effectively unlimited emulated hardware
watchpoints. This lab explicitly sets GDB's remote resource policy to two regions
and four bytes, then requests an eight-byte region, two supported regions and a
third. Record refusal and exact error, not a fictional physical comparator count.
Do not assume software fallback: read/access requests can fail outright. A
physical target's capability query and a separate fallback/disturbance experiment
remain required before claiming hardware-specific behavior.

## Deliverables and independent verification

D1: predict state/failure classes and outputs; derive phase revocation, extent
arithmetic and the optimization/stack distinction. D2: completed C implementations,
actual debugger notebook and supported reset experiment plan. D3: six-profile
tests, 4096 independent four-action histories per profile, exact strict target
builds and observed debugger transcripts. D4: all six original changed conditions
with changed identity, reset-loop plan, delayed local transport, LTO attribution,
same-value access and actual tail-frame comparison.

Use `oracle.check_replay` for the complete history output. `oracle.check_probe`
checks actual state/stop evidence; `notebook.validate` refuses changed ELF/config
or invented physical fields. Keep raw commands, logs, maps, DWARF and binary
hashes. Require clean compilation, intended semantic failures for the three
connection mutants and unsolved stub, and actual wrong checksum results for two
target mutants and target stub. No crash, unrelated diagnostic or absent target
is accepted as a negative control.

## Per-objective rubric and graduated hints

Each row is separately assessed. Level0 is missing/incorrect mechanism; level1
replays the demonstration but cannot derive the distinction; level2 satisfies the
listed criterion with independent counterexample; level3 derives and executes or
properly scopes the named changed condition without copying.

| Objective | Level2 required evidence | Level3 changed condition |
|---|---|---|
| T31.01.a | All phase/identity/driver gates, failure preservation and unknown-state refusal | Board/reset identity change invalidates every previous session observation |
| T31.01.b | Three exact fixture classes, extent/vector boundary and separate power hypothesis | Supported reset-loop plan with wiring/gating prerequisites and abort conditions |
| T31.01.c | Actual command/config/ELF/tool identities and honest unknown physical fields | Owned remote transport version and measured elapsed commands; no credentials or invented board state |
| T31.02.a | Input30→45 through matching memory/registers/assembly/DWARF | LTO source breakpoint attributed to actual instruction/caller |
| T31.02.b | Supported4-byte access and exact width/capacity refusal, policy vs silicon distinction | Equal-value store observed by access but skipped by value-change watch |
| T31.02.c | Unavailable local/invariant explained, wrong algorithm repaired under same flags | Physical tail branch/return authority distinguished from reconstructed debugger frame |

Hints progress from (1) write the evidence ownership boundary, (2) compare the
current PC and exact ELF before interpreting a line or variable, to (3) check
whether the claimed capability belongs to ISO C, compiler, debugger, server or
physical target. Private technical analysis is packaged separately. A successful
author run is not an independent learner attempt; delayed variants retain their
minimum seven-day assessment interval.

## Security boundary

Only original curriculum processes and synthetic bytes are used. The debugger
socket is private to the owned temporary directory and QEMU has `-nic none`.
No real credential, host security change, production image, external target,
hardware Flash or electrical reset is part of this campaign. A physical plan
does not authorize hardware interaction.

## Validation and limitations

The current support record, not this prose, states exact observed campaign status.
Authored/reference/model execution, private QEMU execution, external review,
independent delayed assessment and physical observation remain separate. The
finite history set does not prove arbitrary mastery, all compiler behavior or
every physical debug failure. Physical SWD/JTAG/reset/Flash support, actual
comparator counts/fallback disturbance and electrical measurements remain open.
