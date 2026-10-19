# PR-T24-03 assessor analysis and delayed package

Keep this file, reference C, native drivers, oracle and campaign out of learner
exports. The six existing IDs `AX-PR-T24-03-T24.05.a/b/c` and
`AX-PR-T24-03-T24.06.a/b/c`, with their exact `-DELAYED` successors, are the
assessment units. All original minimum-seven-day requirements remain. Reference
execution does not supply an independent learner attempt, reviewer or delay.

## Distinguishing understanding

For each leaf, score 0 for an incorrect mechanism or violated contract, 1 for
matching an ordinary output without the causal/ownership argument, 2 for a
correct ordinary implementation plus its discriminating boundary and repair,
and 3 only for an independently derived changed-condition solution after the
recorded delay. A level-3 reference implementation is an assessor resource,
not evidence that any learner attained level 3. Ask the learner to predict a new
trace before showing expected results. Record source hashes, prediction time,
attempt date, hints used and which rubric clauses the artifact supports.

| Criterion | Level-2 discriminator | Withheld changed-condition discriminator |
|---|---|---|
| T24.05.a | Consumes each octet once, preserves suffix/output, classifies EOF at actual frame boundaries; complete-buffer oracle differs in structure | Two-octet partial header; independently bounds 65535 before any body store; enumerates all lengths |
| T24.05.b | Response after write half-close; separate truncated header/body; fresh reconnect state and descriptor ownership | Lose reply after effect; same session/ID/payload returns cached original result; wrong payload/full cache reject unchanged |
| T24.05.c | Queue and global invariants survive EAGAIN, short prefix, cancellation and full queues; ownership never releases early | Variable lengths under a faster producer; independent global byte accounting including empty-frame count bound |
| T24.06.a | Distinguishes copied prefix/original length/zero datagram; preserves and interprets flags | Complete payload with truncated control still rejected; received references accounted for, OS flag namespace not confused with application flags |
| T24.06.b | Ordered emissions 7,8,9 and explicit duplicate/stale/window policy; gap is not proven loss | New session at zero rejects old traffic; finite sequence space retires rather than aliasing; discusses forged session limit |
| T24.06.c | Separates local send, peer receipt and effect using observable counter states | Lost effect acknowledgment produces duplicate effect in original flawed specimen; bounded cache repairs only its declared live-session property |

Different internal layouts are valid if they preserve all operation contracts.
An array/set implementation may replace the three-bit reorder representation;
a frame-at-a-time parser may use separate header/body enums. A linear queue may
replace the ring if fixed storage and stable ownership hold. Compare costs under
the same contract, not by omitting duplicate detection or global accounting.

## Derivations and expected results

For a wire of n octets, every one of its n−1 interior gaps either splits or does
not split a chunk, giving 2^(n−1) partitions. Sum over both widths and 81 adjacent
length pairs: the public campaign checks 2,611,210 partitions. The independent
decoder advances by absolute header+length extents; it does not reuse parser
state. Every proper prefix is independently a boundary only at zero, after the
first complete frame, or after the second. Two-byte malformed lengths latch
failure; truncation is not repaired by receiving a new connection's bytes.

With four connections and eight frames each, at most 32 message objects are
queued. The maximum encoded wire is 320 octets for two-byte headers and eight
payload octets, but payload budget is 256. Generalized invariant checking scans
K objects and pairwise duplicate references/IDs, O(K²); ordinary cursor advance
is fixed work except for any invariant checks called by the reference. The
independent admission ledger covers 257 limits times nine lengths = 2,313 cells.
The deterministic writer commits one octet on odd calls and would-blocks on even
calls: five commits in nine calls complete `[0,3,7,0,9]`; four of those commits
are shorter than the offered span. Advancing by the offered length is rejected.

The actual stalled-peer probe has four owned nonblocking AF_UNIX stream pairs,
bounded prefill and 32 admitted frames. It rejects four ninth frames, preserves
state on EAGAIN, and finishes 320 one-byte frame progress steps after draining.
The pilot observed zero short-positive prefill syscalls. That is not converted
into a claim that positive short kernel writes occurred: the short-return model
is separate. The zero-drain producer admits 32 of 256 attempts with a 64-byte
global budget and rejects 224, while varying payload length. Retained object and
kernel storage are additional costs, not hidden inside the payload number.

For the sequence script, return codes are accept, accept, accept, duplicate,
stale; emitted counts are 1,0,2,0,0. Pending 9 cannot emit until 8 arrives.
After buffering 9 at next=7, two policy expirations report 7 then 8, with 9
emitted only after the second. The independent Python oracle stores a set and
recent history, enumerating 25 reachable states and all 175 outgoing edges for
the selected alphabet. This is a finite model, not a proof for all histories.
Retirement at next=65536 prevents arithmetic wrap from silently accepting zero.

The datagram probe expects eight copied octets plus MSG_TRUNC, or original length
12 when input MSG_TRUNC is used. A zero datagram is consumed and the next receive
gets EAGAIN. With a one-byte ancillary buffer and two SCM_RIGHTS references, a
complete payload and MSG_CTRUNC coexist. Linux closes excess undelivered rights;
the complete-control branch receives two new references and closes both.
An initial diagnostic asserted `msg_flags == 0` after MSG_CMSG_CLOEXEC. The
observed flag was 0x40000000, and the inspected current recvmsg manual explicitly
documents this returned flag. The repair checks truncation bits and descriptor
properties instead of assuming all returned bits are zero. This is a corrected
test assumption, not a relaxed completeness rule.

The live cache tests first delta 3, then delta 5 with its acknowledgment lost.
The repaired total remains 8 on retry; a small original no-cache specimen yields
13. There is no process crash in that demonstration. An alternative cache which
returns the current total instead of the first request's result fails after an
intervening request, even if it avoids a repeated increment.

## Rejections, hints and transfer administration

Compiled semantic controls intentionally accept malformed lengths modulo nine,
accept partial EOF, ignore a global budget, repeat an effect, return the current
total instead of the cached original result, admit another
session, advance by requested bytes, or ignore payload truncation. Each must
reach its named assertion and exit 1. Public scaffold controls are compiled and
rejected independently in core, stream and datagram drivers. A crash, missing
binary or compiler diagnostic cannot substitute for those semantic outcomes.
Sanitizer profiles run correct references; absence of a finding proves neither
portable behavior nor exhaustive memory safety.

Use the learner-facing graduated hints in order and record every hint given.
First ask for the state/ownership diagram; then reveal the relevant boundary;
only finally identify the missing field or acknowledgment distinction. Do not
show `stream.c`, the oracle or mutation outputs before the independent attempt.

At least seven days after ordinary assessment, provide only the unchanged
`-DELAYED` task and necessary public API contract. Separate answer keys are the
two-byte parser branch, cached reconnect, global-budget model, ancillary driver,
session restart oracle and dropped-effect-ack branch. Require a new prediction
and learner-generated counterexample, not a rerun of this campaign. Probe
transfer with two simultaneous complete frames plus a partial next header,
an empty frame at a zero-byte budget, duplicate ID with altered payload,
a stale packet just after session rotation, and a complete datagram with
mandatory control absent. If adding any new policy premise, state it explicitly.

## Evidence limits

Run `python3 tools/run_streams_followthrough.py --report
reviews/v5/expansion/legacy_framing/current_validation.json`, then the separate
report guard and real CLI export check. Keep pilot and failed diagnostic records.
The final report pins source and retained objects; nm/size are hosted generated
object inspections, not a strict-freestanding closure claim. Linux local sockets
do not validate Internet delivery, physical timing, distributed crash durability
or another operating system. Published ISO edition access, independent review,
learner artifacts, delayed attempts and physical extensions remain separately
unobserved. No finite passing set proves absolute mastery.
