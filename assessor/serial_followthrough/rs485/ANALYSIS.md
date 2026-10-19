# Withheld RS485 assessment analysis

T38.09.a: 16/8+2=4 unit loads; driver current, two end resistors, common reference,
fault behavior and peer exclusivity are independent premises. Distinguish actual
THVD1410 600ns/11us driver-enable columns and THVD1400 650ns/10us. A receiver wake
bound is a different transition. Reject the convenient but incorrect use of600ns
for every part or shutdown mode; R5_MUTANT1 exercises that error.

T38.09.b: two 8N1 bytes consume20bits/100000=200us. With600ns startup final TC is
200600ns, then200+2000 yields202800ns. The actual producer clears the earlier TC
snapshot after a DR write. Both the channel ownership mutant and retained producer
TXE mutant release too early and fail the full state assertion at the second
write. Counterexamples must explain why a later timing adjustment cannot repair
early storage/line release. Unsigned subtraction is valid only with the stated
half-range observation/budget assumptions; the test beginning UINT32_MAX-300
crosses the wrap and retains the same contract. A gap exactly2^31 is rejected.

T38.09.c: input33 with FE|ORE yields BYTE(error mask), FRAMING and OVERRUN exactly
once. Full software storage preserves the supplied observation and consumed output;
it does not prevent physical UART overrun. Actual DR acquisition is preflighted
conservatively. Two event reservations are necessary because unknown stop may emit
UNAVAILABLE before the one terminal event; a queued LOSS cannot steal either.
Cancellation retains copied storage while DMA EN or line TC remains unresolved.
The retained producer's early-memory-release mutant is caught before any second
chunk. A matching cancelled transfer can only emit ABORTED, never TX_DONE, and
a stale external token cannot act on a fresh transfer.

The changed .c packet composes the actual byte/event channel and retained EDU7
decoder, injects a LOSS at every prefix boundary, then a complete fresh frame.
Boundary0 precedes opening delimiter, so it need not lose the frame; a notice
after final publication cannot undo it. Interior loss invalidates the prefix.
Destination/grant checks happen before copying the published result; malformed
or ungranted traffic leaves output unchanged. The test is actual author reference
composition, not an unaided learner's T40 solution or a full layered-stack gate.

Port analysis: imported USART3 PD8/9 AF7, DE on PD10; initialize latch low before
mode change, BSRR direction writes and DSB. Original mapped-register tests check
the actual adapter, not a copy of its result. Cortex builds exercise numerical
DMA reachability for a real allocated channel, while a CCM linker placement must
fail. The object audit requires32-bit stores, barrier and producer calls. F439
hardware behavior and analog margins are outside emulator evidence.

The exact compiler/runtime, map/disassembly, stack frames, positive tests, rejected
scaffolds and semantic mutant commands are in the source-bound campaign. Finite
tests omit arbitrary callback dishonesty, concurrent ISR mutation, multi-wrap
unobserved clocks, physical cable dynamics and external attacker control. Keep
those limits explicit in every score; physical and independent dimensions cannot
be promoted from the author's report. Use a fresh delayed byte sequence/schedule
and require explanation of invariants rather than distributing this solution.
