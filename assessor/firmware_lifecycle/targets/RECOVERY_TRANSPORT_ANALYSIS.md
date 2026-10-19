# FW-RX assessor analysis

The packet selector is not authenticated independently; it is constrained by the
signature-covered image base/core and fixed trusted layout. Moving an otherwise
valid envelope to a different packet slot must fail actual admission. The bounded
frame is only an input artifact until rt_admit succeeds. On a UART error the byte
may be present but it is not trusted framing input; continuing after silently
dropping it would shift the whole message and encourage accidental resynchronization.

Model controls distinguish TXE from TC and observe ICR/RQR writes without reads
of erroneous RDR. The actual H7 clearing mechanism differs from F4 SR/DR clearing.
A superficial port of the older UART driver is wrong even though both contain a
register named USART3. BRR556 follows fixed64MHz/115200 nearest rounding; pin AF7
comes from DS12923Rev3 table9, not guessed GPIO semantics. Physical baud and
loading still require measurement.

Mutation1 publishes a dummy image after a complete corrupt frame without crypto;
mutation2 forwards RXNE even when error flags are present. Both must fail RT_CHECK
under every profile. All684 truncated positions and684 single-byte corruption
positions are bounded probes, not a parser proof. The min/max cases check546 and
2080 envelope lengths. Unknown or reused parser storage is outside the typed API
precondition; rt_reset does not erase historical bytes or confer freshness to a
previously returned borrowed pointer.

D6 cannot be closed by this campaign. No flash commit, trial health or recovery
reset happens in recovery_probe.c. The next coordinator must authenticate both
members, own inactive erase sectors, complete program+fresh readback, preserve
old committed metadata, trial/health the same generation, then commit. A link or
one-byte UART status reply proves none of those persistent steps.
