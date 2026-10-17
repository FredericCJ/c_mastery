# POSIX lifecycle depth contract, version 1

This required additive laboratory supplies two bounded construction anchors for
PR-T23-01 and PR-T25-01. The AX assessments retain their broader reasoning,
failure, portability and delayed-transfer obligations. Existing accepted units
and historical hours are unchanged. C17 is the language baseline; Linux
pipe2/O_CLOEXEC, fork/execve/waitpid and POSIX pthreads are explicitly hosted
dependencies. No shell, public service, privilege change or hardware operation
belongs in these experiments.

All non-null objects and pointed-to strings must be live for the call; argv is
null-terminated with at least its first element. Outputs are disjoint from all
inputs. These functions cannot validate arbitrary dangling pointers. The caller
has ordinary thread cancellation disabled throughout each call. The target is
an original helper built from tests/helper.c in a new local build directory.
Inputs/environment contain synthetic data only. Deadlock diagnostics terminate
the disposable test process after 15 seconds; a timeout is a failure, never a
semantic pass. Run these functions from the sole process/test owner, with no
external child reaper, SIGCHLD suppression, inherited signal handlers/masks that
interfere with the helper, or concurrent descriptor mutation. Return the
environment to its original state by terminating owned helpers and closing
owned handles. Tests may inspect only their own /proc/self/fd.

## Launch: acquire, replace an image, report and reap

`pl_launch(path, argv, fail_stage, out)` creates one close-on-exec error pipe,
forks, executes the original helper with an empty environment, receives a
native-int error record on exec failure, and waits for the exact child. The
parent closes its write end; the child closes its read end. A complete error
record identifies `PL_EXEC_ERROR` with the saved errno. With no error record,
normal termination produces `PL_EXITED` and its exit status; signal termination
produces `PL_SIGNALED` and signal number. A legitimate exit127 remains an exit,
distinct from exec failure. A result is published only after reaping.

Zero return means a classified child result is available. A positive errno
return means parent-side or error-protocol failure and leaves `out` unchanged.
`fail_stage=1` injects EMFILE before pipe creation; `2` injects EAGAIN instead of
fork after the pipe has been acquired; `0` uses actual calls. Values above2 or
null required pointers return EINVAL before acquisition. Close both pipe ends
on fork failure. Retry interrupted read/write/wait only according to their
operation contracts; never blindly retry close. Reap even after an error-pipe
read failure. A partial error record is EPROTO, not a fabricated errno.

The post-fork child uses only close, execve, write and _exit, plus local
nonallocating computation. No printf, malloc or inherited mutex is used. The
error record is local to processes from one binary, not an external wire format.
EOF alone is not proof that exec occurred: early child death or a failed error
write can also close the pipe. The result records what the protocol and wait
status establish; it is not an authenticated image-execution certificate.
Unbounded helper execution and external cancellation are outside this narrow
API; adding a launch deadline requires a separate terminate/reap protocol.
Storage is O(1); latency includes child execution and is not bounded by C code.

## Pool: partial acquisition and borrower lifetime

`pl_pool(workers, fail_ordinal, out)` requests0..4 joinable threads. Worker i
(one-based) owns an immutable input i and a distinct result cell, and computes
i*i. The caller retains all argument/result cells until every successfully
created thread is joined. Creation stops at the first nonzero pthread return.
The API uses direct pthread error codes, not errno. `fail_ordinal=0` uses actual
creation; an ordinal1..workers injects EAGAIN immediately before that creation.
An ordinal greater than workers or workers>4 returns EINVAL with unchanged
output. Null output is rejected. Zero workers succeeds with all-zero results.

On a valid call, return0 publishes `{created,joined,completed,sum,create_error}`.
All three counts equal the number actually created. Sum includes exactly those
workers. `create_error` is zero on complete acquisition, otherwise the first
creation error. For four workers and failure at3 the result is2,2,2,5,EAGAIN;
successful four-worker acquisition yields4,4,4,30,0. No uninitialized pthread_t
is joined and no argument is freed or leaves scope while borrowed. One caller
owns every join; detached threads are an analysis variant, not an allowed
implementation shortcut. Join failure violates the narrow exclusive-owner /
valid-handle / noncancelled-caller preconditions: the assessor terminates the
teaching process with125 instead of returning stack storage to a live borrower.
This is explicitly not a recoverable generic thread-pool API. Storage and thread
count are bounded by4; the protocol does not promise scheduling fairness.

## Verification and evidence

Build the default learner scaffold first; it must compile and fail a semantic
`PL_CHECK` assertion. Implement only src/posix_lifecycle.c. Include the exact
public header and this contract in the learner workspace. Assessor code is an
explicit build opt-in and never part of learner initialization.

The launch anchor tests exit0, legitimate exit127, ENOENT, original-helper
SIGTERM, both acquisition failures, unchanged failure output, descriptor count
and no unreaped child. The pool anchor exhausts every requested size0..4 and
every applicable creation-failure ordinal, then tests invalid boundaries.
Independent fixture arithmetic computes the expected partial sums. Four
mutants discriminate swallowed exec errors, conflating127 with exec failure,
ignoring injected creation failure, and dropping the final created result.
Mutation success requires the intended assertion, not a compiler error, signal
crash or timeout. Neither suite proves all OS failures or concurrency schedules.

Submit D1 ownership/transition predictions, D2 original implementation, D3 exact
commands/raw results and minimized failures, and D4 the parent AX delayed
changed-condition artifact. A mechanically passing test with an incorrect EOF,
join-lifetime or errno explanation fails the corresponding rubric. Full
multithreaded-fork restrictions, forced short error-channel records, arbitrary
wait failure recovery, cancellation and detached ownership remain separately
assessed obligations, not capabilities implied by these two anchors.

Hints: first draw the two pipe-end owners and the exact point close-on-exec
acts; then distinguish a saved child errno from a wait status. For the pool,
track the successful prefix rather than requested count; place all joins before
the argument array's lifetime ends. Assessor analysis remains under assessor/.
