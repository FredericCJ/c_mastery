# PR-T19-04 concrete short-write stream successor

This newly authored implementation supplies the previously unspecified concrete
io.stream export used by T72. It is attached to PR-T19-04, particularly
T19.08.a/b/c and their existing AX criteria. It is not a claim that historical
acceptance included these files. Stable handles also exercise the parent's
T19.09 lifetime contract; complete reference-count-cycle analysis remains in
the original assessment. Use C17 profile C; the component needs no allocation,
operating system or standard-library copying routine. The host harness uses
stdio/string routines. Any strict freestanding claim needs its own object/link
audit, including compiler-generated structure-copy helpers.

Prerequisites are byte spans, unsigned arithmetic, short-write progress and
live-owner generation handles. Implement stream.h from the compiling stub.
SFStream is caller-owned storage for at most64 payload bytes plus one-byte
length and modulo256 additive checksum. sf_start stages [length,payload,sum],
then publishes a handle bound to that live stream and a new nonzero generation.
It rejects an active stream and refuses generation wrap. Failure preserves
the old stream and output handle. Input and control/output storage are distinct.
Initialize fresh storage only after all old handles have been discarded.

sf_pump calls a borrowed nonreentrant transport with the remaining suffix and
an invocation budget. Positive results accept at most the offered count; zero
means backpressure and immediately returns PENDING; -2 is retryable and spends
one budget unit; other negative values terminate with IO. accepted reports the
total prefix, including on terminal error. It never authorizes replay of that
prefix. A too-large callback result is INVALID and terminal, but checking the
count cannot repair a callback that already accessed outside its borrowed span.
DONE is stable without another callback. Cancel prevents further writes and
retains progress. Starting another request invalidates old live-owner handles.
Handles cannot validate a freed stream pointer or synchronize concurrent calls.

The supplied [1,2,3] becomes [3,1,2,3,6]; quotas2,1,2 yield exactly that frame.
The tests also enumerate every payload length0..64 with quotas1..7, zero progress,
retry, terminal failure, cancellation, stale handles and retirement. Derive
expected bytes independently and reject both offset-replay and ignored-cancel
mutations. No external peer, credential or deployed system is a test target.
The harmless adversarial witness is duplicated synthetic bytes or a callback
after cancellation, not a claim of code execution.

Deliver implementation, invariant0<=offset<=length<=66, ownership/alias ledger,
exact reconstructed bytes and progress trace, commands/hashes, and a cost
comparison with an equivalent direct writer. Start is O(n), each pump has at
most budget callbacks plus accepted-byte transport work, and storage is one
fixed frame. Callback latency is external. Hints: retain an offset; separate
zero progress from retry; never reset generation to reuse a request.

Assess the existing T19.08 criteria separately: level1 nominal frame output,
level2 complete admission/progress/failure argument with independent cases,
level3 a delayed changed transport that can report a diagnostic after accepting
a prefix and an explicit revised result type. Whole-domain learner mastery and
independent reviewer acceptance are not inferred. The withheld stream analysis
and reference live under assessor/api_depth; the learner gets only stub/tests.
