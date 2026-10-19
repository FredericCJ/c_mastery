# Assessor: PR-T41-08 actual packet-stack HTTP

## 08a

Grade framing independently of packet boundaries. The32 raw cases include27
crossed chunk/MSS/window conditions, three maximum-body/interim cases, one
missing trailer terminator and one complete trailing request. The expected
binary entity is generated independently from byte index; HTTP headers are
literal expected bytes, not `hf_response` output reused as an oracle. The
12-byte NUL/CR/LF body survives every split;1024 bytes uses the exact length1024.
The EOF case is400 with `bad request\n`, length12. A trailing request remains
in the receive queue with `requests==1`; no optimistic parse reset dispatches it.

An incomplete trailer is not repaired by FIN. Mutant1 converts that error into
200 and fails the independent response comparison. A valid alternate state
machine may stream output under a different explicit application contract,
but cannot assert this echo result before the required complete request or
discard body bytes after an arbitrary NUL. Preserve predecessor parser limit,
overflow, header/security and RTO investigations; these32 cases cannot replace
their distinct objectives.

## 08b

NULL produces no Date field. The trusted synthetic callback produces exactly
`Sun, 18 Oct 2026 01:02:03 GMT` once for the final response. Both paths are raw
byte checked. Mutant2 restores the historical stale literal and fails even
where Date must be absent. Callback syntax checks do not prove its UTC source
or weekday/calendar consistency. The transport receives only the monotonic
argument, independently of callback text. A candidate claiming wall-clock
jumps may alter retransmission deadlines has changed the contract incorrectly.

## 08c

The link fixture stops between actual descriptor publication and modeled DMA
return. Application abort leaves `tcp.dma` set. A stuck process prevents stop
completion. After positive stop, the old local ticket is stale; actual PHY
requalification and ring reset precede a new TCB epoch/ISN5000. An old wire ACK
does not open LISTEN or acknowledge SYN5000, whereas ACK5001 does. An old
application object cannot abort the newly bound LISTEN TCB.

This is selected old-packet rejection, not authentication. A peer capable of
constructing acceptable new sequence numbers can still send accepted traffic;
epochs are local metadata. Physical qualification and quiet-time/tuple policy
remain explicitly assessable in the separate capture protocol. Do not award a
physical pass from the modeled link bit or claim host interoperability from the
original raw peer.

## Failure, cost and enforcement

The zero-window case retains the final copied application output until the
absolute15s deadline, then starts stop. An interim response temporarily pauses
parsing; its transmission/ACK does not renew the request's total allowance.
All fixed storage, parser capacity and encoder scratch count toward the target
budget. Emitted nonrecursive frames and known callback paths support a bounded
author stack estimate; no physical watermark or WCET was observed.

ISO C17 enforces neither unique application ownership nor non-reentrancy.
Runtime epoch/state checks cover named errors; the remaining ownership,
callback lifetime and correct UTC authority are explicit programmer contracts.
Accept an alternative implementation only with equivalent byte/failure/closure
contracts and new discriminating tests. Retain separate implementation,
author-execution, reviewer, independent/delayed learner and physical statuses.
