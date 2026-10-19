# Assessor analysis: T39 normal ring and original changed tasks

Use the learner contract and exact original criterion/V1 text. The following is
assessor material and must not appear in learner exports.

## T39.06.a: whole interval and changed link placement

Only20000100/64 passes the four original cases. The adapter checks alignment and
imports actual `bu_f439_dma`; narrowing length occurs only afterUINT32 admission.
The selected real typed image section is20001000 with size12384. The diagnostic
CCM link deliberately fails EN_DMA_REACHABILITY; it never publishes OWN at an
invalid address. A learner checking only the first byte or treating the host
model base as a real pointer has not established this objective.

## T39.06.b: prerequisite order and two reset mechanisms

The cold target adapter asserts the peripheral reset before RMII selection,
while Ethernet clocks are still off; its pin/core-clock assumptions are stated.
The ring's SWR request comes later and must be observed clear. Bases only change
with both DMA processes stopped. The final engine start is the last fallible
write. All27 modeled start callback cuts stop before that effect; callback
failure-before-effect is an explicit model contract. A physical bus fault is
not covered by this convenience. A timeout while SWR remains set does not allow
other register writes: rearm only a finite read-observation window. No reset
routine may discard outstanding CPU borrows or assert device quiescence.

## T39.06.c: compiler, device, cache and reachability

Four distinct arguments are required: language/compiler access, CPU/device
ordering, physical bus reachability, and cache visibility when applicable.
DSB cannot connect CCM to Ethernet DMA. F439's selected uncached SRAM needs no
invented M7 cache recipe. In the changed two-line visibility model, clean moves7
to the device, device writes9, CPU stays7 until returned ownership plus
invalidation. The neighbor byte31 is unavailable while the same line is leased;
byte32 belongs to another line. Cleaning stale7 after return overwrites9.

## T39.07.a: capture policy and meaningful boundary changes

For retained FCS, FL64 exposes60. The selected ring requires complete one-buffer
FS/LS and error-free status. The changed capture adapter takes validated stored
bytes, not a fictional status-format conversion: CSTF1 strips only field>0600,
whereas length46 and0600 still retain FCS.60 stripped bytes expose60;64 stripped
bytes expose64. Rejecting changed-policy oversize and preserving output is
required. A hardware CSTF1 variant remains conditional on the real status and
capture contract, rather than extrapolating an ambiguous manual sentence.

## T39.07.b: final length, chain capacity and recovery

Retained `ed_rx_acquire` supplies the actual two-descriptor validator. A missing
final LS cannot publish; a late LS-only piece cannot become a new frame.
Twenty-four changed capacity splits for finalFL100 expose96 combined bytes;
the first descriptor's deliberately meaningless FL is ignored. AllOWN clear,
firstFS/finalLS, no intermediate boundaries/errors and combined capacity are
independent obligations. OWN still set is incomplete, not an error completion.
The hardware normal-ring profile may reject chains, as the original brief
permits; its separate complete-chain model cannot be omitted.

## T39.07.c: exactly-once release across an upper failure

Four held tokens force ring exhaustion. Releasing one permits precisely its
descriptor reuse; old/duplicate serials reject even at the same address. The
original badIPv4-version consumer increments one upper rejection while the
ring reports accepted1/dropped0/released1. A second release remains stale and
does not increment counters. A full parser is not required to demonstrate the
ownership edge, but must not be claimed from the version discriminator.

## T39.08.a: head is the publication boundary

The retained two-descriptor model rejects head publication before tail and
permits only one published request. EvChain's three-descriptor extension
requires both tails before head; each intermediate state remains unobservable
to the modeled head-fetch device. The actual normal ring instead stores one
whole frame in one descriptor and permits two device-owned entries; this
declared T41 profile difference does not erase the original T39 proof task.
TX controls must be rebuilt after status writeback. The modeled device clears
them, so32 successive reuses are a relevant regression rather than repeated
constant substitutions.

## T39.08.b: copied versus borrowed ownership

`en_tx_copy` completes its copy before returning. Mutating the original byte0
does not change the published frame. The EvChain variant retains the actual
pointer and forbids caller mutation/free until confirmed completion or stop.
Returned OWN with an error permits local reuse without delivery; an IRQ with
OWN still set does not. The borrowed variant's error-complete and abort paths
must clear its retained pointer exactly once. Raw C fields are not unforgeable
capabilities: callers must obey owner/lifetime rules and must not forge tickets.

## T39.08.c: stop is a proof obligation, not a timer outcome

Unknown stop retains every storage obligation. After DMA states are stopped
and CPU borrows returned, MAC is disabled and the component becomesBOUND. SWR
must then complete under a fresh epoch before admission; old tickets reject.
A TPDR callback failure afterOWN has been published remains an in-flight lease
inFAULT. Partial tail publication in EvChain likewise retains its borrow until
confirmed stop. Nonwrapping epoch/serial exhaustion retires admission; a wider
counter alone would not justify wrapping.

## Scope of the target and independent assessment

The real Thumb tests execute C/assembly and model actual selected register
addresses/effects. The non-model object proves selected access width and DSB
emission, not peripheral behavior. DWT's720 active cycles are a conservative
delay only with owned feature state, maximum180MHz core, and continuous RMII
clock. Debug halt can lengthen a delay, not supply a wall-time measurement.
External review, physical captures and independent seven-day transfer remain
separate gates. Reject any report promoting author tests into those dimensions.
For withheld transfer, select a different valid capacity split/fragment
boundary, a late error after link retirement, or a different live clock bound;
require a new invariant and falsifier, not edited expected constants.
