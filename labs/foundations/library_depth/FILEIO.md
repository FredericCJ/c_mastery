# T12.11: stream state, committed prefixes and close ownership

Required PR-T12-04 T12.11a/b/c and the original changed conditions, families
A/B/C/D/F/G/I/J. C17 hosted core; named POSIX2024 and GNU custom-stream adapters
are separate platform investigations. Prerequisites: FILE orientation, lifetime,
caller-owned buffers, partial I/O, actual f_write_all learner artifact and the
preceding finite scanner. Read R-LLIB-N1570-STREAMS, R-LLIB-POSIX2024-HANDLES
and R-LLIB-GLIBC241-COOKIE. Complete fileio.c; the reference and platform solution
are assessor-only. All streams/pipes/cookies are created locally with synthetic
abc data. No external target or host security setting is involved.

Draw unopened, open-unoriented, byte-oriented and closed lifecycle states.
EOF/error are independent sticky observations on a live stream, not ownership
states. A first byte operation chooses byte orientation; a wide operation is
then forbidden. Analyze the invalid mix without executing it. Supply a setvbuf
buffer before other stream operations and keep it alive until fclose. A FILE
object cannot be made independent by copying its representation. The stream
pointer's lifetime ends at close; a close failure does not permit a second close.

The copy core owns a two-byte staging window and cumulative read/written counts.
Its reader writes at most the requested two bytes and returns count plus MORE,
EOF, ERROR or WAIT. MORE requires positive progress; WAIT requires zero bytes.
EOF/ERROR may accompany valid bytes, which must be drained before terminal
status is returned. Callbacks borrow their spans synchronously, never retain or
reenter, and must obey their count contracts. The original FWriter contract
accepts a positive prefix, -2 retry, and terminal zero/other negative results.
This adapter preserves that contract: it does not reinterpret zero as resumable
backpressure. Actual nonblocking POSIX EAGAIN is translated to -2.

Implement li_pump with a total callback budget per call. Retain an incomplete
window and its write offset across calls; never read a new window over unwritten
bytes. Call the actual f_write_all provider with budget1 for each write step.
Maintain0<=offset<=length<=2 and written_total<=read_total. Only the offered suffix
may be written; a committed prefix must never be replayed. A terminal state makes
no further callbacks. A read error after ab reports READ_ERROR with ab committed.
A write error after a reports WRITE_ERROR with one byte committed, even if two
bytes were already consumed from the input. No seek-based rollback is promised.
Arithmetic overflow or an invalid reader result is a protocol failure, not
permission to wrap counters. Initial/callback storage and output are live and
disjoint; C does not enforce these borrow rules.

For the ab,c,EOF fixture the output is exactly abc in two data chunks and the
EOF observation follows an attempted read. Exercise quotas1/2, budgets1..4,
an initial retry, read-error-after-ab, and each output failure prefix0..2.
Resumption is valid only with the same live copy state and endpoints. Recreating
state after a terminal error can duplicate prior output or lose consumed input;
the result is evidence about a prefix, not a transaction that can roll back.
Memory is constant; work is O(bytes+callback attempts), with at most the declared
budget callbacks per pump. Callback blocking time is outside that bound.

Implement li_release independently of copy progress. An owned handle is consumed
before calling close, even if close fails. A borrowed output is flushed but never
closed; a borrowed input is neither flushed nor closed. Clear the local handle
and make repeat release return CLOSED without another callback. Missing required
callbacks are argument failures and preserve ownership. Test all owned/borrowed,
input/output and success/failure policies. In a tiny child borrow stdout, flush,
then prove the caller can still write. If flushing fails, the external owner
still owns the stream and must decide recovery; this wrapper has relinquished
its borrow. Do not silently convert a close error into copy success.

Run actual FILE adapters with GNU fopencookie original read/write/close hooks:
read ab then fail, short-write one byte, and fail close. Its writer error return
is zero, unlike a POSIX write error; inspect the named GNU contract. Keep these
observations distinct from ISO guarantees and from the core callback model.
An API that can report accepted bytes and a simultaneous diagnostic at the
completed boundary needs the explicit status-plus-count extension from the
T19 stream successor; FWriter alone cannot encode both. The supplied FILE
adapter checks a previously set error before another write. It does not claim
the stronger arbitrary transport contract.

For an ISO update stream, output-to-input needs flush or positioning; input-to-
output needs positioning unless the input met EOF. Do not call ISO fflush on
an input stream and assume portability. Run permitted read/write transitions
and retain the resulting aXc. Flush is not a durable-storage commit.

Under the explicitly named POSIX adapter, distinguish descriptor from open-file
description and FILE's private input buffer. Explain an invalid uncoordinated
trace in which application consumption1 coexists with buffer read-ahead6; do
not execute it to obtain a portable expected result. Select one active handle.
Before a seekable input stream yields activity, apply the inspected POSIX input
fflush rule; this is an extension to the ISO contract. Switch to raw read for b
and back to stream read for c with the required coordination.

For the changed condition use dup/fdopen to create a second FILE over the same
open-file description. Serialize activity, synchronize the old stream and
reposition the new stream when the prior activity requires it. Preserve the
exact offset transitions and d/e results. Two distinct FILE buffers do not imply
two independent underlying offsets. The rules also matter across fork; this
exercise does not authorize unsynchronized forked stdio experiments.

Finally use two owned nonseekable pipes. Fill the nonblocking sink to actual
EAGAIN, retain the pending ab window, drain only the known filler, then resume
to exactly abc without any seek. Close only the owned pipe ends. This tests a
real kernel backpressure event, not merely a serialized scripted port. It does
not establish durable delivery, peer acknowledgment or a general cancellation
protocol. Those require their later component contracts.

Deliver state/ownership diagrams, invariant and cost arguments, adapters,
failure-prefix traces and current source/tool evidence. Six compiler/optimization/
sanitizer profiles must pass ordinary and platform checks. Reject replay,
error-as-EOF and retained-close-ownership mutants at designated assertions.
Rubric:0 loses/duplicates bytes or misowns handles;1 nominal copy only;
2 explains every relevant state and layer;3 independently derives the original
borrowed-stream, nonseekable and two-handle changes after at least7days. Hints:
separate consumed from committed; drain before reading; close consumes ownership
even on error. No reviewer, independent learner or physical evidence is inferred.
