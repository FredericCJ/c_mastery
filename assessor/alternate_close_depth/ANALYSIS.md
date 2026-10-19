# Assessor: retained close authority

The implementation's state transition follows a selected QNX8 contract, not an
ISO C property. `EINTR` is the only error here carrying an inspected positive
retention guarantee. Success consumes the owner even if the seam supplies stale
errno. Other errors preserve diagnostic fd information but no operation authority.
Returning RETRY from a single attempt exposes caller scheduling instead of hiding
an unbounded retry loop. The backend can block; the wrapper proves no latency bound.

The independent oracle derives each output from the documented result table and
does not call the reference C implementation. Six controls separately consume
EINTR authority, retry twice per call, treat all errors as success, permit uncertain
handoff, invoke the callback from a terminal owner, and misclassify malformed
results as retained. Check exact assertion markers and successful compilation;
compiler rejection is not a semantic mutation rejection.

The real-fd host seam establishes a useful counterexample: after unknown completion,
the numerical fd can name a fresh pipe. A status query returning open cannot prove
identity. The QNX target must record actual EINTR and fstat identity before retry;
exclusive ownership and no intervening opens are necessary premises. If the target
does not produce EINTR, report inconclusive/failed trigger, not an executed contrast.

Cancellation belongs to the lifetime envelope. Temporarily disabling it inside
qc_once and restoring on RETRY could discard the only owner. The supplied thread
test disables before ownership, receives a pending cancellation, moves the fd to
surviving control, then restores and joins. That is a real host cancellation test;
it supplies no QNX-specific scheduling or signal proof.

The resource-manager source returns EINTR before its default close handler on one
real client request; synthesized rcvid0 disconnects use default cleanup. Single
dispatch-thread execution prevents a server-side data race on the injection flag.
This target probe is deliberately falsifiable: the unbuilt SDK/library path may
need repair after actual compilation or may expose different framework behavior.
Do not claim a passing result from source inspection or host header substitutes.

The separate signal pair deliberately does not return literal EINTR. Its first
close handler records the received request, sends SIGUSR1 to the kernel-reported
pid/tid and returns no reply. QNX8's unblock protocol retains the REPLY-blocked
client until the server answers. Default-first unblock handling plus current
MsgInfo identity and `_NTO_MI_UNBLOCK_REQ` prevent an old pulse from authorizing a
reply to a newer request. Only this admitted callback uses MsgError(rcvid,-1).
The client handler merely sets a sig_atomic_t, and ownership remains with the
disabled-cancellation caller until second-close success. Failure of that signal,
unblock, framework dispatch or actual libc-retention hypothesis must remain a
failed experiment. The signal traces in guard self-tests are explicitly synthetic
schema controls; actual target execution still requires the licensed environment.
