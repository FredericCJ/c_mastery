# Private T23 assessor analysis

Do not copy this file or context.c/models.py into learner workspaces. Assess each
original criterion separately; these author explanations are not evidence of
independent review or a learner's seven-day transfer. Public inputs and controls
are in labs/posix_context_depth; source scopes are in the dedicated research gate.

## T23.01.a

The two parent threads share PID and address space, but have distinct observed
Linux thread IDs. pthread_t is not promised to equal that Linux number. After
fork the child has a different PID and the same observed effective IDs; both
private addresses can have the same numeric value while parent7/child9 differ.
The shared byte changes1→5 and is inspected only after the control/reap sequence.
Reject "different addresses prove isolation" and "same addresses prove shared
objects". In the changed exec case the result PID must match the helper PID;
effective IDs, prepared cwd and resource limits match, while the helper's new
image starts its own initialized private value. The old anonymous mapping is
not carried over by exec; do not probe an expired pointer to demonstrate this.
The source semantics establish that boundary, the fresh helper observes its new
image. No set-ID image or credential transition is exercised.

## T23.01.b

An environment entry is one string; the first equals sign separates name/value.
The helper must observe `synthetic=alpha` as the entire value, count exactly two
entries, retain the empty argv entry and report no PATH. Parent cwd and limits
stay unchanged because fchdir is child-side. Missing config produces the helper's
23 result, unlike missing image ENOENT. Mutant1 discards the explicit environment
and must fail the intended process assertion. Level3 explains how an inherited
ambient environment or implicit search would defeat the reproducible contract.

## T23.01.c

The public authority table has four distinct layers. fork observations cannot
prove a specific libc syscall path. The inspected glibc spawn source is an
implementation explanation, not a guarantee about all kernels or vendors.
pipe2 is outside the declared2008 port baseline but recorded by the current
manual as standardized in2024. `/proc/self/fd` has no ISO/POSIX declaration;
replace its inspection role with an application-owned ledger. The analysis port
must reject a naïve pipe+fcntl sequence in an arbitrary threaded process: it
changes the atomic close-on-exec creation argument. No non-Linux execution claim
is accepted without a separate environment receipt.

## T23.02.c

At the fork point a normal mutex is locked and two workers exist in the parent.
They cannot run in the child to unlock its copied mutex. The original child uses
only prepared close/fchdir/dup2/execve/write/_exit operations; errno is captured
before cleanup can overwrite it. Keep the error channel distinct from helper
stdout. Inspect child source, not only passing sanitizer output. The spawn
adapter initializes/destroys file actions, orders fchdir before output duplication
and calls a GNU action explicitly. Packaged2.43 shares args.err with its child
and returns that error after collection; this explains selected observed
ENOENT. Other implementations can return success followed by127. Distinguish
that historical/manual contract from the actual bounded backend result.

## T23.03.a

After duplicate creation the common offset is0. Reads yield a→1, b→2; pread at4
yields e and leaves2; the next ordinary read yields c→3. The independent second
open reads a and owns offset1 while the duplicate still reports3. Closing the
first handle does not destroy the open description retained by the duplicate.
FD_CLOEXEC belongs to a descriptor and dup clears it on the new handle; setting
O_NONBLOCK through the duplicate changes the shared description. Reject a
repair that saves/restores the offset around a sequential read as an equivalent
positioned operation under concurrency.

## T23.14.a

The required interface inventory covers openat, pipe2, epoll, pthread_create and
mmap, plus launcher dependencies. Mark optional facilities and versioned
declarations rather than labeling everything "C/Linux". Actual poll/epoll controls
check exactly empty/data/repeated/drained/EOF for a single pipe. A regular file,
edge trigger, atomic signal-mask replacement or multi-handle ordering changes
the contract; an unexamined fallback is not accepted. Complexity/resource
comparisons must include creating/closing an epoll handle on each teaching call.

## T23.14.b

Hidden-declaration probes intentionally fail in clean strict C17 translation
units; the matching feature-selected probes compile. That is frontend evidence.
Runtime ENOSYS/EOPNOTSUPP is explicitly modeled, not induced by lying about the
actual running kernel. Epoch1 success cannot justify a retained choice in the
new modeled environment epoch2. Keep generated probe source, diagnostics,
header/libc/kernel identity and model results separate. Changing a feature macro
after headers have already been included is not a repair.

## T23.14.c

Selection uses one result per epoch. EIO/EPERM/EINVAL are preserved as failures,
not silently converted to "unsupported". An equivalent poll fallback is admitted
only for ENOSYS/EOPNOTSUPP before an operation starts. An in-progress epoch change
returns EBUSY unchanged. After nonzero completed work, an error is partial status2
and the caller must reconcile the already visible effect. Mutant4 returns the
before-progress classification and is rejected. Level3 supplies an explicit
nonidempotent counterexample: increment5→6, lose capability, then replay would
incorrectly produce7. Retry policy needs either no progress, an idempotency token
or a compensating application contract; none is inferred here.

## T23.12.a

The FILE buffer first owns a byte without increasing file size; fflush makes
it kernel-visible; accepted fsync is a different stage. Injected file-sync EIO
after a successful write returns error5 with stage1 and old named data. A pipe
returns the selected actual EINVAL, because this regular-file synchronization
operation is not applicable. Do not describe tmpfs or process-kill observations
as physical durability. The final runner chooses and reports a private ext
filesystem directory rather than relying on /tmp, which was observed as tmpfs
in the preliminary experiment. Even ext-backed guest storage is not a power-cut
measurement.

## T23.12.b

Failures at stages1/2/3 leave old data and a cleaned owned temporary file;
stage4 follows successful rename, therefore reports visible new data and EIO.
The result must preserve progress instead of returning a Boolean rollback claim.
An EEXIST temporary file belongs to another operation and is not unlinked.
The old data lock and newly opened replacement lock can both succeed because
they protect different inodes. A second independently opened stable lock must
conflict. A synthetic uncooperative writer can still change the data while the
stable advisory lock is held. That demonstrates the protocol's participation
precondition, not failed kernel memory protection. Mutant2 suppresses file-sync
failure and is rejected before a false success can be accepted.

## T23.12.c

The cut driver waits for a successful stage marker, sends SIGKILL only to its
owned child and reaps it. Parent's duplicate lock reference survives, so explicit
unlock is required. Cuts1/2 recover visible old, cuts3/4 visible new in the actual
run. The abstract oracle says durable outcome at3 is unspecified; it does not
predict loss merely because directory-sync has not happened. Require the exact
VM/block experiment design, independent reader, restored initial image and
flush/model intervention in the delayed analysis. It is honest author analysis
completion; no guest/power/device execution is inferred. The current logical
Python matrix is not the proposed block fault experiment.

## T23.13.a

Remaining time is the cardinality of the nonnegative interval [now,deadline).
Expiry returns0, not an unsigned wrap. Exact addition overflow rejects unchanged.
The adapter changes wall values while monotonically advancing elapsed values;
wall jumps cannot change the same absolute monotonic deadline. Actual one-shot
signal interruption retains that deadline. CPU time measures consumed execution,
not elapsed waiting. Under a15-active/100-suspended model the active-only lease
has5 ticks left while the suspend-aware lease has expired. Neither is universally
correct: choose whether an external lease expires while the machine is asleep.
Mutant3 wraps after expiry and fails before timing measurements.

## T23.13.b

Read settings without changing the host policy. A child lowers only its soft
descriptor limit and reports it; parent verifies both unchanged limits after
wait. The permission adapter preserves the old accepted mask on EPERM/EINVAL.
A request for CPUs0/1 after CPU1 disappears cannot be accepted as if the earlier
snapshot still applied. This is a deterministic model, not a claim that physical
CPUs were offlined. Distinguish kernel-intersected masks from the stricter modeled
request contract; no privilege or isolation setting changes are performed.

## T23.13.c

Retain all12 raw planned/wake/dispatch/done samples per profile and compute
median/observed maximum from them. The first userspace timestamp after sleep is
not a kernel scheduler tracepoint. Instrumentation, compiler and competing shared
host load affect the distribution. A synthetic higher-priority interval [20,120)
delays work released at20 by100; no previous finite maximum excludes that changed
environment. Level3 must identify the missing bound on interfering execution,
not propose raising priority as an unsupported proof.

## Scope and assessment separation

The five mechanism drivers share controls but do not collapse fifteen rubric
judgments. Keep each ordinary and exact delayed task in the binding table.
The reference functions have documented preconditions and bounded failure
campaigns; arbitrary signal cancellation, hostile namespace mutation, network
filesystem locks, all possible I/O errors and actual power loss require additional
evidence. Tests, source audit, model and proposed experiment have different
claims. No finite collection of these results proves absolute mastery.


## T23.02.a — closure of the prior partial

The old source-bound launcher campaign still supplies creation/exec/error-channel
and ordinary/signal tests. The new adapter preserves an owned positive child PID
on EIO/EINVAL injected before wait and later actually reaps that exact child.
Retiring the ticket early would lose the only collection obligation; mutant5
exposes this. ECHILD is different: after a controlled competing real reap, no
valid exit result remains for this ticket. Clear authority, preserve output and
deny further backend calls. Killing the remembered numeric PID after ECHILD is
not a recovery protocol. It can target a different process after reuse. Do not
promise recovery from permanently unavailable OS services; preserve or report
the ownership/loss state. The original delayed SIGTERM task is actually repeated
with a dedicated child. This successor closes the enumerated author implementation
item while independent learner demonstration remains required.
