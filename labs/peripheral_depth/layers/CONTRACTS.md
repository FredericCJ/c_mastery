# T28.11 — effects across register, driver and service boundaries

This required successor implements the three existing T28.11 criteria in
PR-T28-06, including their original delayed variants. It supplements the
historical milestone and does not change its accepted hours or evidence.
Read `objective_mapping.json` in the corresponding research directory for exact
criterion identities. The learner files deliberately return `PL_UNIMPLEMENTED`.
Implement them; private solutions and transfer adjudication remain under assessor/.

Use C17 and exact-width unsigned types. The service, parser, posted device and
original byte driver use no allocator, libc or mechanism-providing library.
Host printing, Python polynomial arithmetic, compilers and QEMU are verification
dependencies only. The separately identified vendor experiment permits the
inspected STM32F4 HAL 1.8.3 UART source, CMSIS device F4 2.6.10 and CMSIS 5.9.0;
the changed-device experiment additionally uses the exact G4 commits in
`research/v5/expansion/legacy_advanced_layers/g4_provisioning.json`.
these are not dependencies of the original implementation. C23 does not turn
volatile accesses into a transaction, establish device ordering, or enforce
exclusive ownership. Exact published ISO edition access remains separately tracked.

## Prerequisites and route

First complete the T27 reset/clock/image-closure work and T28 MMIO, UART and DMA
ownership exercises. Reuse the actual `struct pm_model/pm_access`, `struct pu_driver/pu_step`,
`struct pu_clock/pu_rates` and constructed `sd_uart` artifacts. Bring their interface,
failure, physical-address and timebase assumptions into the integration dossier.
Do not replace a provider with a function of the same name that always succeeds.
The generic service has no register definitions in its public header. Build the
scripted transport first, then the real UART reference adapter, then the selected
F439 port. Moving to actual learner providers is a separate integration attempt.

For each part submit implementation, a state/ownership diagram, discriminating
tests, a rejected incorrect implementation and a source-bound evidence directory.
Every effect must have a named owner. Serialized calls and an identical transport
object/context throughout an active operation are preconditions. Callbacks must
be bounded, non-reentrant and obey the borrow contract. C does not enforce these
conditions; changing them requires a different protocol, not just `volatile`.

## 11.a — a framed service above a cancellable UART

Implement `service.c`, `uart_adapter.c` and `target.c` against their public headers.
The input command is two octets, sequence and value. The output frame is exactly
eight octets: F0, six nibbles encoded as 10..1F, F1. The six nibbles encode the
sequence, value and CRC-8 remainder, most significant nibble first. The CRC uses
polynomial x^8+x^2+x+1, initial zero, no reflection or final xor, over the two raw
octets. Derive this as polynomial division; do not define the expected CRC by
calling your encoder. For zero/zero the frame is F0 10 10 10 10 10 10 F1.

The stream parser has no allocation or buffering proportional to input length.
F0 abandons any incomplete prefix and starts a fresh frame. F1 accepts only six
valid nibbles with a matching CRC; malformed length, symbol or checksum returns
`PL_FRAME` without changing the output command. Other bytes outside a frame are
ignored. A valid frame yields exactly one command. The new marker is outside the
data alphabet, so any of the eight incomplete prefixes can be abandoned without
searching for an escaped delimiter. Derive why this is true. CRC is corruption
detection, not authentication or duplicate suppression.

`pl_send` borrows its own eight-byte storage to the transport. Tokens are nonzero,
strictly increasing uint32 values; exhaustion retires the service rather than
wrapping to an old identity. The duration is in (0,2^31) ticks. A failed submission
must retain the previous token/frame and create no borrow. An active submission
rejects a second send before any callback or buffer edit. The object and transport
context cannot move or die while active.

`pl_cancel` requests cancellation; it does not return the borrow. `pl_poll` retains
the borrow on BUSY and on unrecognized/invalid transport outcomes. Only an explicit
terminal OK/CANCELLED/IO/TIMEOUT outcome releases it and publishes a `struct pl_result`
once. A repeated poll is STALE. The transport must promise terminal means it will
never read the buffer again. `may_have_committed` is conservatively one: the peer
may already have accepted all eight bytes before cancellation. It is incorrect
to report that cancellation undoes remote effects. The minimal protocol cannot
determine delivery from a transmitter-side result.

Map the actual PuDriver states without mistaking `pu_poll`'s local polling-budget
exhaustion for physical quiescence; use `pu_step`. DMA EN clearing is only one of
the prior driver's completion premises. Its selected UART direct-mode profile,
memory lease and line lease remain distinct. F439 DMA frames must lie wholly in
20000000..20030000; the target adapter rejects CCM and crossing ranges before
publication. A numerical address check does not prove an actual C object exists.
The owning application must place the service in admitted SRAM and retain it.

Required tests include all 65,536 commands against an independent CRC oracle;
each prefix cut followed by a new frame; CRC corruption, malformed delimiters,
overlength and output sentinels; failed submission; stale/exhausted identities;
cancellation before and after full peer acceptance. Reuse the real PuDriver in
poll, IRQ-dispatched and DMA modes with every cut 0..8. Check that the old DMA
borrow survives a stop request and that a later frame is not corrupted. The
available target register model must run the same full frame and identity rules;
linked target code must retain both service and real port symbols.

Delayed transfer: after the original recorded minimum delay, give a fresh command
and request cancellation at an undisclosed byte boundary. Require a trace proving
whether the parser discards a partial frame or has already committed it. Change
the assumption to retry after a lost acknowledgement: demonstrate a duplicate
effect, then specify the additional durable sequence/acknowledgement protocol
needed. The implementation here intentionally does not claim exactly-once effects.

## 11.b — mocks must preserve effects and ordering

Implement the eight-entry posted FIFO in `posted.c` over the actual PmModel.
Only CTRL/STATUS/DATA writes are admitted; width is 32 bits, except DATA also
accepts 8 bits. Full admission returns BUSY without a write. `pl_post_read` does
not flush pending writes. `pl_flush` delivers FIFO order and stops at the first
device rejection. Prior delivered writes are irreversible; the failed write and
suffix remain queued. The model is a deliberately explicit interconnect contract,
not a universal assertion about ARM or every peripheral readback.

The exchange queues CTRL=1 and flushes before observing readiness. On TX-ready it
publishes one DATA byte, then waits for RX-ready and consumes DATA exactly once.
An error clears only W1C bit 1; it must preserve unrelated status bits. A finite
step budget produces TIMEOUT with output unchanged. Success changes output once;
later step calls are STALE. A failed publication/flush enters phase4 quarantine:
step must not enqueue another copy. A new exchange is forbidden while pending
writes remain. Explicitly resolving the queue may still cause the old effect;
there is no rollback instruction in this device. State when a higher layer must
refuse reuse because it cannot prove quiescence.

Use the immediate and posted versions of the same model with delayed readiness.
Reject double DATA reads, status read-modify-write on W1C bits, and omitted flush.
Demonstrate why an immediate-only mock accepts the missing-flush implementation.
Inject a second queued DATA write that fails after the first succeeds. Record the
delivered prefix, retained suffix and subsequent recovery, including the phase4
service quarantine. The finite model tests behavior under its premises, not bus
timing, cache maintenance or interrupt races on silicon.

Delayed transfer: replace the immediate publication assumption with the posted
queue. Identify the exact missing visibility edge and add the required flush,
not an arbitrary delay. Explain why C sequencing and volatile alone do not
establish that edge. Review the actual target manual before choosing DSB,
readback, a completion flag or a device-specific handshake.

## 11.c — compare equal contracts and change the register map

Implement `compare.c`. For the selected preconfigured 8N1 one-byte transfer,
wait TXE, write the byte, then wait TC. Timeout is unsigned elapsed ticks greater
than the supplied limit; limit0 times out on the first failed flag observation.
Flags win if already set at entry to the flag test. The tick callback must
progress and the entire operation must stay within the unsigned elapsed-time
assumption. A frozen tick defeats a finite timeout argument. HAL_MAX_DELAY is
not part of this bounded contract. On timeout memory is no longer borrowed but
the line may still be busy. Do not start another operation until TC or a proven
reset/quiescence protocol permits it.

Compare an original literal F439 map, the generated map from `registers.json`,
and the actual inspected HAL_UART_Transmit implementation. The changed-device
run also compiles and executes the separately pinned official G4 HAL, using its
ISR/TDR layout; direct and generated implementations run identical schedules. Keep the same byte,
readiness schedule, timebase, timeout rule and completion criterion. The ARM RAM
register guest supplies flags via HAL_GetTick; it is not a USART simulator and
does not validate baud, electrical behavior or implicit hardware flag clearing.
Report actual object imports, retained symbols, instructions, code size and
local stack frames. Distinguish shared helper cost, per-call context size and
whole guest harness size. Do not rank libraries using unequal contracts or
claim latency from instruction counts.

Inspect the actual HAL wait loop, READY transitions, macros, access widths and
interrupt completion path. Execute a separate callback test where TxCpltCallback
starts another transmit. The old operation must publish READY before the callback
and must not subsequently erase the new BUSY state. The blocking transmit has
no completion callback. Execute a frozen-clock witness bounded by an external
4096-call observer; explain the infinite-loop argument under permanently false
flags and an unchanged tick, without treating a bounded run as an infinite proof.

Changed-condition transfer uses the documented F439-to-G474 register-map change:
F439 SR/DR at 0/4 become G474 ISR/TDR at 1C/28. This is a different device register
generation, not a claim about a same-part silicon revision. TXE/TC remain bits7/6
in the selected FIFO-disabled G474 mode. Generate the two offsets from the schema,
test sentinels at old offsets (which now identify CR1/CR2), and reject a stale-map
mutant. The common transfer algorithm remains unchanged; the full G474 board port
requires clock, pin, FIFO/enable and errata work under the separately recorded
RM0440 source dependency. This variant claims only the supported preconfigured
register mapping and bounded effect model. Do not silently reuse the F4 HAL on G4.

## Hints, assessment and limits

Hint1: list which layer owns the frame, register effect, line and remote command.
Hint2: draw cancellation request, memory quiescence, TC and peer F1 as separate
events. Hint3: use a shadow candidate for fallible admission, and retain an
irreversible-prefix record instead of promising rollback after publication.

Passing requires the tests above plus an explicit argument for each criterion's
ownership, failure and transfer contract. A compiling solution with a happy-path
frame is superficial. A correct reference-equivalent implementation with justified
boundaries demonstrates the bounded construction. Independent mastery additionally
requires diagnosing an unseen fault and adapting the contracts without copying
the private solution. Author executions do not substitute for learner attempts,
external review or actual delayed evidence. None of these finite tests proves
absolute mastery or physical correctness.

Security scope is exclusively original local parser/device programs, synthetic
commands and dedicated emulated or development-board targets. A harmless success
condition is a synthetic command counter changing twice after replay, or a
different synthetic value accepted after an attacker recomputes the CRC. No
third-party device, network, real actuator or credential is involved. Repair
replay/authorization only with an explicitly specified protocol and trusted key
or policy boundary; CRC hardening alone is not a repair. Crashes and UB are not
claims of code execution. Board work follows `BOARD_CAPTURE.md` and is not run by
the author-side campaign.
