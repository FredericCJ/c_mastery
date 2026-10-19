# Diagnostic-ring technical analysis

The old DG path called EDRx/EDTx. Its tests alone did not execute EnRing's real
normal-descriptor ownership or epoch behavior. This bridge calls those actual
providers, retaining the original raw length-frame protocol.

EnRing publication writes metadata, publishes OWN, then performs a fallible
doorbell. Failure therefore needs quarantine, not a clean retry or cleared phase.
The ring's stopped-device witness and new reset epoch are necessary before reopen;
an application timeout cannot substitute. Copied send means caller mutation after
admission is harmless, but the bridge still owns its ticket until completion.

RX stages the replay counter and message. Returning a borrow may fail after OWN
was returned, so publication waits for release success. The failed frame is
consumed, application state remains uncommitted, and the ring faults. A later
retransmission after genuine recovery can still use the same application sequence.
There is no external side effect before this atomic application publication.

The finite sequence acquires1+8+1+1=11 leases; the hardware-error descriptor is
dropped by EnRing before acquisition. The synthetic minimum frame has14-byte
header plus46-byte stored payload, with meaningful DG length separate from pad.
The final four RX bytes are stripped under supplied hardware-valid status, not
verified as a real cable CRC.

Five defects are compiled separately and must fail named assertions. Withheld
variants move failure before copy, after OWN, or after logical release; compare
which ownership facts survive. None of these observations replaces actual PHY,
controller silicon or delayed independent learner evidence.
