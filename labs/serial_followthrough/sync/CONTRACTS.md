# Synchronous USART extension, revision 1

This is the required implementation successor for PR-T38-02 and its exact
AX-PR-T38-02-T38.03.a/b/c criteria and V1 variants. The original project, legacy
shared assessment contract, serial_depth contract and PuDriver UART contract
remain in force. All original seven-day transfer delays remain. The additions
do not rewrite their historical acceptance or make physical gates optional.

## Prerequisites and environment

Bring the learner's `sd_uart`, `sd_sync`, `PuDriver`, `PuRx`, F439 pin/clock
adapter and T27 reset/linker artifacts. Read the actual selected clauses in
R-SU-RM21 (RM0090 Rev21 pp973–977,981,991,997–999,1010–1017) and R-SU-DS14
(DS9484 Rev14 Table12 pp78–79), registered in
`research/v5/expansion/legacy_serial_sync/resources.json`. Understand volatile
register side effects, SR-before-DR reads, TXE versus TC, half-range timestamps
and exclusive caller ownership before entering this exercise.

Use the existing ISO C17 baseline, profile E, eight-bit octets and uint32_t.
The core uses fixed caller storage, scalar integer operations and the previously
implemented components. It has no heap, library serial engine, operating system,
HAL or hidden timer. Host Python, stdio and sanitizers are verification tools.
The target is the selected STM32F439; the executable guest is separately named
MPS2 AN386 Cortex-M4 with an original RAM register model. It does not model a
USART peripheral or establish wire timing. The supplied compiler byte helpers
must be explicit in a target image map; inspect unresolved symbols and generated
code. Compiler GNU inline assembly is confined to the guest's semihost adapter.

## Problem and necessity

Extend the actual asynchronous producer into a synchronous master without
equating USART character conventions to every SPI device. Implement `sync.h`
from the unfinished `learner.c`; keep supplied tests unchanged and add your own.
Use 0xA5 and 0x96 to derive all CPOL/CPHA/LBCL traces, then transfer one through
eight bytes to an original matched peer. Its declared response is TX xor0x5A.
The peer supplies exactly one response per transmitted character, no unsolicited
traffic, eight LSB-first data bits, matching capture edges and LBCL1. Its timing
must satisfy the selected setup/hold contract before any physical claim.

The input includes ordinary frame data, a clockless RE-only request, a live
configuration change, an omitted last clock, a phase-shifted trace, stale tokens,
clock-generation mismatch, errors and an absolute deadline. Outputs are concrete
register writes, predicted edges, preserved received byte/error pairs, the
received-prefix count and a distinct success/error/busy/deadline result. No
dummy byte is implicit: a receive-only client supplies its own clock-generating
transmit value. This is necessary because an empty receive buffer and RE1 alone
cannot make an F439 synchronous master produce a clock.

## Operations, invariants and failure behavior

`Su` starts zeroed. All controls, TX/RX spans and callback storage are live,
adequately sized, disjoint and exclusively owned. Only these APIs may mutate
the state while a request is active. `PuIo` callbacks terminate, obey the
register contract, do not reenter the component and retain no caller pointers.
An ISR or concurrent adapter is not supplied by this serialized contract.

First call the retained `pu_open(&s.byte,...)` with a truthful stable clock
record. With no pending transfer/character, explicitly disable TE and RE.
`su_configure` admits only boolean CPOL/CPHA/LBCL, ready state, no outstanding
buffer/line ownership and both TE/RE clear. It writes CR3=0, then CLKEN plus the
three selected clock fields in CR2. No physical effect rollback is promised
if a callback violates its contract. Rejected inputs publish no configuration.
`su_enable` sets the requested TE/RE together in one CR1 write while idle; the
caller must have established no in-flight receive before disabling them.

`su_begin` admits 1..8 immutable TX bytes and at least that many RX `PuByte`
slots (capacity<=8). LBCL1 and UE/TE/RE enabled are necessary for the matched
eight-bit exchange; LBCL0 remains useful for trace analysis but is rejected by
this peer adapter. A pending RXNE rejects admission rather than treating an old
byte as the new response. The clock generation must match the retained producer.
The external request token increases strictly without wrap. Reserve one new
internal PuDriver token for each character before admission; exhaustion retires
that control until no old events can exist, not reset-and-reuse.

Timestamps are uint32_t modulo arithmetic with a positive budget below2^31.
Consecutive observations advance by less than2^31 ticks. Store one absolute
deadline for the whole request. Each `su_step` observes at most one receive
event, calls the actual producer's bounded progress operation, and starts at
most one new one-byte subrequest. A received prefix is visible through
`received`; later RX cells remain untouched. Errors retain the older RDR byte
and raw PE/FE/NF/ORE mask and terminate further transmission. Preserve the first
error while cancelling the lower layer. A stop request is not stop acknowledgment.
TC from before a DR write does not acknowledge that write. The RX/TX spans remain
owned until `active` becomes zero, including a timeout waiting for final TC.

On success all requested responses and final TC have been observed. On error,
cancellation or deadline, wait for lower-layer memory and line ownership to
drain, disable TE/RE, drain any residual RXNE using SR then DR, and publish the
terminal reason. A missing TC can keep the operation BUSY indefinitely; finite
polling is not a license to free storage or fabricate completion. Caller-level
reset/isolation needs a separate hardware quiescence contract. No hidden
retry, thread scheduling guarantee, DMA backend or full-duplex streaming is added.

`su_trace` checks a complete synthetic trace against the independently declared
mode: first divergent one-based edge, including the first missing/extra edge,
or zero on a match. Malformed scalar inputs preserve the output. It does not
infer analog setup/hold or decode an arbitrary noisy instrument capture.

## Target route and changed assumptions

Reuse the actual F439 USART3 PD8/PD9 AF7 adapter. `su_f439_ck` adds PD10 AF7,
push-pull, low-speed, no pulls, preserving every other GPIO field. Clock enable
uses the existing readback rule. Actual package, board revision, competing FMC
or LCD pin ownership, supply and connection routing must be verified first.
The host and MPS2 mapped test uses RAM in place of register addresses; its
readback is a statement about the original address/bit code, not hardware.

For .a-V1, enable LBCL and show why a peer requiring all eight capture strobes
rejects the seven-strobe trace. For .b-V1, compare an externally clocked SPI
slave: an external clock can drive its sampling without a local USART TX,
changing clock ownership. Do not conclude that F439 USART CK can become input.
For .c-V1, diagnose an unseen trace with the correct idle level but wrong phase;
compare its first divergent edge to an otherwise correct missing-last-clock
trace. Retain an unaided attempt after the original minimum seven days.

## Deliverables and verification

D1: derive the bit/edge tables, state machine and ownership sets; identify the
source of every claim (ISO C, library API, vendor register rule, model or wire).
D2: implement the real composed adapter and CK mapping; supply source identities,
target object disassembly, link map, unresolved-helper ledger and stack account.
D3: record full progress/error prefixes and first divergence; test all widths of
the bounded request, deadline equality/wrap, clockless/live-change rejection,
stale request, source/token exhaustion, parity/framing/noise/overrun and delayed
TC. Check outputs and untouched suffixes, not only return values.
D4: independently derive expected edges and register masks, compare the public
fixtures and add a defect which the ordinary happy path cannot reveal. Run GCC
and Clang at changed optimization and sanitizers. A compiling stub must fail;
reject live-change, clockless, premature-release and lost-error mutations.
D5: deliver the changed-condition artifact and the separately scoped physical
capture dossier in BOARD_CAPTURE.md. Missing equipment leaves that dimension
open; a software result does not replace it.

For native checks compile `learner.c`, `tests.c`, `native.c` with the learner's
UART/serial sources and their include paths; run selectors0..3. The mapped
executable additionally uses `mapped.c`, `mapped_native.c`, the learner's target
adapter and PU_TARGET_MODEL/SU_TARGET_MODEL hooks. `target_build.py` takes explicit
source, target, UART, serial, boot, linker and runtime inputs; no assessor source
is silently selected in an exported workspace. The actual source-bound author
campaign and limitations live in the research successor's validation.json.

## Rubric and hints

Level0 violates ownership/configuration or claims absent physical evidence.
Level1 predicts ordinary edge counts but cannot explain first divergence or
clock ownership. Level2 supplies the criterion-specific derivation, working
composition, error preservation, generated-code audit and discriminating
counterexample. Level3 independently transfers under the exact delayed variant
and states which old assumptions and measurements no longer apply. Finite
passing tests are bounded falsification evidence, never absolute mastery.

Graduated hints: (1) Draw the transmit and receive ownership intervals separately.
(2) Number edges from the first transition away from idle, then mark captures.
(3) Write TE/RE beside every CR2 change and write the source of TC beside every
release; compare a pre-write status snapshot with a later observation.

All faults use original local targets and synthetic bytes/registers. Harmless
success of a defect is a wrong trace, false completion or corrupted local
result; no arbitrary-code-execution claim follows. No external bus, operational
vehicle, production data, host setting change or flashing is authorized here.
Assessor implementations and technical analysis are kept outside learner exports.
