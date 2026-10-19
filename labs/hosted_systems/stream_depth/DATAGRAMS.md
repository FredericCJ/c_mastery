# T24.06: completeness, order and effect acknowledgments

The inspected Linux `recvmsg` and `unix` passages define payload truncation,
ancillary truncation and descriptor transfer. ISO C alone supplies none of these
interfaces. Keep application policy separate: this lab requires both complete
payload and complete mandatory control metadata. A different application may
legitimately choose another policy, but must not silently weaken this one.

## T24.06.a — two independent truncation channels

Send twelve synthetic octets through an owned local datagram socket pair and
receive into eight octets using `recvmsg`. Preserve `msg_flags` and reject the
prefix as a complete application message. Repeat with input `MSG_TRUNC`, which
on the selected Linux socket interface reports the original datagram length.
Treat reported length and copied capacity as different quantities. Then send
and consume an actual zero-length datagram; a subsequent nonblocking receive
must report no queued datagram. Explain why zero here is not stream EOF.

Map the relevant output flags explicitly to `rf_datagram_accept`. It accepts
complete zero-length data, rejects payload/control truncation and oversize, and
rejects unknown bits in its own two-bit policy interface. Do not feed raw Linux
flag values into that private enum. Also do not require every unrelated OS flag
to be zero: for example the inspected Linux interface reports
`MSG_CMSG_CLOEXEC` when the corresponding receive option is used.

In the delayed task, send two owned pipe descriptor references through
`SCM_RIGHTS`. Receive with a control buffer deliberately too small and retain
`MSG_CTRUNC` independently of the complete one-octet payload. Reject that
application message and verify the descriptor count after rejection. Then use
properly aligned, adequately sized control storage; inspect each control record,
validate its level/type/length and close every received descriptor. Record the
close-on-exec flags when requesting `MSG_CMSG_CLOEXEC`.

A received descriptor denotes a new reference in the receiver's table, not a
portable transfer of the sender's integer. Linux closes excess rights that cannot
fit the receiving ancillary buffer; that rule does not relieve the application
of closing references it did receive. Keep this Linux behavior distinct from
any claim about another OS. These exercises transfer only lab-owned descriptors.

Hints: preserve both `msg_flags` and the return value; next maintain two capacities
for data and control; finally reconcile descriptor incarnations, not just numbers.

## T24.06.b — ordering needs a declared policy

Implement `RfSequence` with session IDs 1–65535, sequence values 0–65535, a
three-value forward reorder window and the last three delivered values for
duplicate classification. There is no modulo reuse: after sequence 65535 is
delivered, the session retires. Inputs from another session never enter the
current reorder set. Reject outside-window arrivals without changing it.

Before running, derive all return codes, emissions and retained values for
arrivals `7,9,8,9,6` starting at 7. Distinguish a buffered duplicate, a recently
delivered duplicate, an older stale value, a wrong-session value and an
outside-window value. Delivery is ordered; arrival and delivery are different
events. Output arrays have three slots and a separate initialized count.

Specify an explicit gap-expiration policy. This lab permits reporting one missing
value only when later data is already buffered, then drains any newly contiguous
values. A policy timeout or observed gap is not evidence of permanent network
loss. Demonstrate an alternative history where the missing datagram is delayed
and would have arrived later. Do not invent a portable packet-loss probability.

Use a separately represented set/history oracle, not the implementation's bit
mask, to enumerate reachable histories under arrivals 6–10, expiration and a
wrong session. Compare return code, delivered sequence, missing report and final
logical state. The changed-condition task restarts at 0 under session 2 and
injects delayed session-1 data; also test maximum-sequence retirement. Explain
why an unauthenticated session number prevents accidental mixing but not a
malicious peer that can forge the new session ID.

Hints: list emitted values separately from pending ones; write the ordering
invariant as a set relation; then decide what happens at the finite maximum.

## T24.06.c — local send, receipt and completed effect

Use an original receiver and a synthetic integer total. First send successfully
and let the receiver discard the datagram. Observe unchanged total and no
application reply. In a separate request, return a receipt acknowledgment before
updating the total; then return an effect acknowledgment containing the request
ID and result. Record each observable state. Neither successful send nor receipt
acknowledgment alone establishes that the requested effect completed.

For the delayed task, commit an effect and lose only its effect acknowledgment.
Retry the same request. Demonstrate the incorrect duplicate increment in a small
original specimen and compare with the bounded result cache from T24.05.b.
For example, after a prior delta 3, a delta-5 request and its retry can produce
13 in the flawed specimen but must retain 8 in the repaired live-server cache.
Show that the replay returns the original result even if later requests exist.

State attacker capabilities and limits for the harmless replay demonstration:
the local test controls request IDs, deltas, receipt/reply loss and retry order;
it cannot invoke another service or access real data. The violated property is
at-most-once effect within the declared live session. A repeated synthetic
increment is the success condition, not a crash or privilege escalation.
The repair binds session, ID, payload and cached result, refuses ambiguous ID
reuse and cache exhaustion, and has regression tests for overflow and wrong
session. Volatile state, unauthenticated identities, concurrent processing and
crash recovery remain limits; deduplication alone is not a durable transaction.

Hints: draw three acknowledgments on different timeline points; move only the
last reply into the loss channel; then decide what a retry needs to identify.
