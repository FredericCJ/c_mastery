# QNX 8 close ownership and the interrupted-close transfer

This required supplement implements the author-support portion of
`AX-PR-T23-02-T23.04.b-DELAYED` and `IW-e2cdc87b21fb7b31` without changing that
seven-day learner task. Its families are B/C/D/F/G/I/J. Prerequisites are the
PR-T23-02 learner descriptor ledger, operation-specific retry analysis, and
ordinary copier/wait evidence. The supplied C17 representation needs no allocator
or mechanism library; the target adapter deliberately uses QNX libc and pthread
contracts. Host fault injection is a separate Linux test environment.

## Source authority and question

Read the exact QNX SDP8 `close` Errors and Safety passages registered in
`research/v5/expansion/alternate_close_depth/resources.json`. EINTR retains the
descriptor in that implementation. The other listed error rows do not state
ownership disposition. This exercise does not infer retirement for them or
generalize Linux's descriptor-release rule. The POSIX Issue7/8 edition model in
`labs/posix_edition` remains a distinct exercise. ISO C17 enforces local object and
integer rules; it cannot enforce descriptor ownership, signal or cancellation
rules. A successful close also does not establish data durability.

## Representation, ownership and operations

`QCOwner` is live, initialized, exclusively accessed, noncopied storage containing
an fd and state. Init is only for fresh storage, not a way to discard an unresolved
owner. Caller transfers a live nonnegative descriptor and has no outstanding
borrower, other close, dup2 replacement, asynchronous I/O or signal handler that
touches it. Output storage and callback context are disjoint from the owner.
Callbacks are synchronous, nonreentrant and do not destroy, retain or mutate the
owner/output. A close callback returns the actual result and captured errno;
success may leave errno stale and must still count as success.

`qc_once` invokes one callback at most. `0` consumes ownership and sets CLOSED/fd-1.
Exactly `-1,EINTR` preserves the identical fd and sets RETRY. The caller decides
when to make another attempt: no unbounded internal loop, backoff or progress
promise is hidden in the wrapper. Any other result sets QUARANTINED and returns
UNKNOWN, preserving the number solely as diagnostic evidence. EBADF signals a
broken ownership premise; it is never converted to success. A malformed callback
pair is quarantined too. A terminal call invokes nothing and preserves output.
Null inputs are INVALID with no publication. Valid admitted calls publish the
state/error observation after the transition; success reports error0.

`qc_handoff` transfers only OWNED/RETRY authority to separate caller storage,
then marks the source MOVED/fd-1. QUARANTINED cannot be retried, handed off or
reinitialized as a live descriptor. Testing whether its number is open is not
identity authority: that number could already name another file. The bounded lab
ends the isolated owning process if unresolved ownership remains; production
recovery would need a separately justified backend/owner protocol.

Cancellation must be disabled before acquiring the descriptor and remain disabled
until it is closed or handed to a live surviving owner. The wrapper cannot safely
re-enable cancellation while returning RETRY: a pending request could kill its
only owner. The host cancellation test deliberately transfers the still-live
descriptor before restoring cancellation and joining the cancelled worker.
Unexpected asynchronous termination is outside this cleanup contract. Calls use
O(1) control storage and one backend operation; syscall latency is not bounded.

## Concrete work and acceptance

Implement `learner.c`. Supply a state table and argument for each transition,
especially `EINTR,EINTR,success`, unknown error, terminal reuse, malformed backend
reply and successful handoff. In `tests.c`, a fault seam retains a real pipe fd on
injected EINTR then calls real close; another seam closes then reports EIO and
reuses the number, proving quarantine must prevent another callback. Neither
sequence changes Linux's kernel contract or counts as QNX execution.

Deliver the implementation, predicted traces, exact compiler/flag/dependency logs,
independent oracle results, six named mutation rejections, and target build/capture
report or exact blocker. `probe.c` exposes state transitions to an independent
Python table oracle. Host compiler configurations are GCC and Clang C17 debug,
optimized and address/undefined sanitizer. A clean host run is necessary but not
sufficient to claim the alternate-OS task actually executed.

Rubric: level0 loses/retries uncertain authority or invents target execution;
level1 reproduces success but cannot explain EINTR/unknown/cancellation; level2
implements every contract branch with the independent table, compiled controls
and exact host limitations; level3 independently adapts to the changed OS after
the original delay, records actual target build and trace and diagnoses a changed
error/trigger contract without assuming all errors share EINTR's rule.

Hints: first separate result from ownership; then separate retained authority from
an integer whose identity is unknown; finally move ownership out before enabling
cancellation. Assessor source and technical analysis stay under
`assessor/alternate_close_depth`. Changed investigations include an interrupted
close followed by cancellation, an error with unspecified retirement, repeated
interruptions with a bounded caller attempt budget, and real target versus seam
trigger comparison. These change authority/progress assumptions, not constants.

## Target and containment boundary

`QNX_TARGET.md` specifies an original resource manager and target client. Only an
authorized QNX SDP8 SDK and disposable private QNX8 guest are permitted. The
manager handles a unique `/dev/cmastery-close-<pid>` name and synthetic state;
no operational service, external system, real credentials or production data is
in scope. Target error injection uses the actual libc/resource-manager path; it
is not evidence of asynchronous signal interruption. A separate original
`/dev/cmastery-signal-close-<pid>` manager retains a received close, signals the
kernel-identified caller thread and replies only after its pending-unblock check;
the corresponding probe records the handler and retained owner before explicit
retry. Its source, build recipe and causal capture checks are supplied separately.
Only that original client is a permitted signal target. Both sources are authored but
not QNX-compiled or executed until the licensed SDK/guest is supplied. Finite
author tests do not establish independent review, learner mastery or all OS paths.
