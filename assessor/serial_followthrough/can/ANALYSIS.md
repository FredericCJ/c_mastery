# Assessor-only Classic CAN analysis

The parent criterion objects and V1 tasks remain unchanged. This successor supplies actual implementations and observations, not learner success. The published contract deliberately narrows the codec to strict standard-data0..8; current ISO receiver reserved-bit and final-EOF tolerances must be explained. A submission claiming full CAN conformance from this codec fails the scope criterion.

## T38.10.a

For a shared base identifier, standard data wins against standard remote at RTR; standard remote wins against extended at IDE after RTR/SRR tie. Numeric extended identifiers combine29 bits and therefore do not sort against11-bit values as bus prefixes do. Extended base(base−1) beats standard base despite its larger numeric identifier for all supplied2047 cases. Equal arbitration prefixes followed by different payloads can create bit errors; arbitration tie is not two successful transmissions. Require the independently marked decisive position, not only a winner table.

## T38.10.b

CRC validity and fixed-form validity are separate. Re-stuff the modified CRC to avoid unintentionally using a stuffing failure as the CRC counterexample. At a chunk boundary after five equal transmitted bits the next bit belongs to the stuffing rule and cannot become payload. The zero-filled8-byte frame has124 wire bits; all ordered three-chunk cuts number7875, with1187 first-boundary cases requiring a stuffed next bit. The independent oracle derives these values using a separate recurrence. ACK0 is reported but cannot grant application authorization. Successful parsing publishes only after the full admitted tail; all errors preserve the output object.

## T38.10.c

Inspect every rule's premises before its delta. Receiver120→121→129 exercises two applicable updates and the passive transition. A successful receive above127 may reset into119..127; this model chooses127 while the selected RM describes120 for its stated condition. A real adapter reads ESR rather than simulating its counters. Missing-ACK errors reach TEC128 then the passive/recessive-flag exception prevents further increments for that particular history. A dominant observed during that passive flag removes the exception. Pre-arbitration stuffing has its distinct exception. Active/overload dominant episodes first charge at14 bits; passive episodes first charge at8, then every extra8. Recovery requires explicit restart and128 completed11-recessive observations; no software write of zero substitutes for observation. Reject repeated cumulative episodes, blanket “every error +8” and claims of health from an active state.

## T38.11.a

Prescaler and segment register fields encode one less than their actual values:5,10,1,0. Total14quanta yields500kbit/s and sample12/14. At48MHz, the search finds31 exact-rate segment tuples, and the first exact3/4 sample match uses pre6,BS1=11,BS2=4. BS1 then lasts1375ns; a2000ns minimum allowance is impossible at500kbit/s. Revisit oscillator tolerance, cable and transceiver delays separately. The target image must contain no unaccounted arithmetic helper; the reference uses64-bit products, not64-bit division.

## T38.11.b

Mask bits include IDE/RTR, preventing standard data selection from silently admitting remote or extended frames. Hardware unlocked overflow replaces the last stored frame, unlike an oldest-drop software queue. The driver copies before exactly one RFOM write and keeps FOVR sticky until explicit acknowledgement. Reading RF0R then writing status|RFOM accidentally acknowledges observed W1C flags. The mutant is rejected by the second popped frame's lost overrun evidence. A corrupted out-of-subset DLC is released once and rejected to avoid an indefinitely wedged foreground loop. Coalesced hardware FOVR cannot support an exact lost-message count.

## T38.11.c

An abort request is a command, not completion. The implementation retains the lease until RQCP0 and TME0 are both observed. TXOK takes precedence if successful transmission wins the race. Automatic-retry ALST without RQCP stays pending; NART ALST+completion becomes terminal. A live software token binds a continuously owned mailbox; hardware offers no token field, so arbitrary late completion after an unauthorized reuse cannot be made safe merely by comparing software integers. Generation exhaustion rejects admission. Bus-off blocks sends and preserves outstanding ownership. Recovery requires no live/unconsumed lease and bounded enter/leave-init observations; BOFF must clear by hardware. The unavailable result makes no promise that the device stopped or that the bus is healthy.

## Changed-condition and independent assessment

Keep all six original delayed V1 IDs, minimum7days and exact tasks. Use a new competing identifier, chunk schedule, exception history, sample ratio/propagation premise, queue policy or NART/cancellation ordering after the delay. Do not award level3 merely from matching supplied packet constants. Ask the learner to show which argument fails if another ISR owns the mailbox, the filter partition gives bank0 to CAN2, the clock changes, EOF tolerance is widened or a caller assumes physical measurements exist. The ordinary/changed author campaigns are source-bound finite observations only. Independent external review, successful learner submissions and physical two-node work remain unperformed.
