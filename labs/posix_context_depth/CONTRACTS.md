# T23 process, persistence and clock follow-through

This required additive packet implements the fourteen previously uncovered criteria plus the retained launcher partial listed
below and their exact `-DELAYED` tasks. It supplements PR-T23-01, PR-T23-05 and
PR-T23-06; accepted project identities, workloads and earlier POSIX laboratories
remain intact. Original seven-day independent transfer requirements remain in
force. An author running the changed case today does not satisfy that delay.

## Prerequisites and environment

Read the retained T23.01/.02/.03/.12/.13/.14 teaching and parent project contracts,
the descriptor/ownership work in T18 and the existing launcher error-channel
contract. Bring a descriptor ledger, overflow-checked extent operations, and an
explicit cleanup policy. No completed implementation is silently imported from a
component name: this packet provides `context.h` and unsolved `learner.c`.

Use the active hosted C17 profile A on an unprivileged Linux process. POSIX,
Linux and GNU interfaces below are dependencies, not ISO C guarantees. The
assessed mechanisms are launcher ownership, replacement protocol, clock algebra
and capability decisions; libc/kernel primitives are permitted, but a process
launcher or transaction library implementing those mechanisms is not. Python
oracles and compiler/sanitizer tools are host verification dependencies only.
Record compiler, libc, kernel, filesystem, flags and clock resolution. Do not
infer ISO C17 text from N1570/C11 or from a successful GNU build.

Actual probes use GCC and Clang at O0, O3 and address/undefined sanitizers. Those
instrumented builds can add runtime calls: they diagnose tested paths and do not
certify an arbitrary post-fork child path as async-signal-safe. Read the plain
source call inventory as a separate obligation. This is not a thread sanitizer
or scheduler exhaustiveness claim.

## Problem and operation contracts

### Process and inherited context

Implement `pc_launch(path, argv, env, cwd, output, result)` using an original
fork/exec/wait path. `path` is an absolute path to the original helper; argv and
env are live, terminated arrays of live terminated strings, immutable until
return. Descriptors 0, 1 and 2 are open; cwd and output are distinct live
descriptors >=3, owned by the caller and held until return. cwd is a directory;
output is an open writable stream. Both have close-on-exec set. No other thread
closes/reuses these descriptors or modifies supplied arrays. The caller consumes
bounded output after return; helper output fits the empty pipe capacity. A
general unbounded-output service must concurrently drain it instead.

The function borrows inputs and transfers no descriptors. Create a close-on-exec
error channel before fork. After fork, the child may only close prepared handles,
change to the prepared directory, duplicate output, exec the prepared image,
write an integer error record, and `_exit`. Prepare everything else beforehand.
Never format an error, allocate storage or lock an ordinary mutex in that path.
The parent drains the error record, reaps exactly its child and closes both owned
channel ends on all completed paths. A separate wait failure is a function error;
the result is published only after completed collection. No cancellation or
adversarial signal-handler ownership is promised. EINTR retries preserve progress.

Return 0 with `PCLaunch.kind=1` for ordinary exit and its exit value; kind 2 for
the child's launch error and its errno; kind 3 for a terminating signal and its
number. A parent setup/collection failure returns a positive error and leaves
the result untouched. An exit127 is not itself an exec-failure proof. The error
channel can also close if the child dies before exec: collect the child status;
do not interpret channel EOF alone as successful image replacement.

Implement the changed path `pc_spawn` using initialized file actions, descriptor
duplication and the selected GNU `addfchdir_np` action. It borrows the same inputs.
Distinguish an immediate nonzero spawn return (no child PID published here) from
an eventual exit127. Actual selected glibc reports missing-image ENOENT directly;
this is a versioned observation, not a portable promise for every implementation.
No ambient PATH search or environment inheritance occurs in either path.

The supplied helper accepts an empty argument and `with spaces`, plus exactly
`PC_CONFIG=synthetic=alpha` and `PC_SECOND=beta`. It reports PID, effective IDs,
cwd, resource limits, argument/environment counts and its new-image private
initialization. Missing config returns23 with a bounded marker. Compare parent
cwd/limits afterward. Compare two live parent thread identities and a child:
same virtual address values after fork do not imply the same private contents.
Use a shared anonymous byte only with the stated ordered write/read handshake;
do not turn it into an unsynchronized multiprocess counter.

### Replacement and visible versus durable state

Implement `pc_replace(directory, lock, bytes, n, fail_stage, hook, context, out)`.
The caller owns a private directory and a stable separate `lock` inode, opened
as an unlocked read/write descriptor. Every cooperating writer independently
opens that same lock pathname and follows this protocol; the directory/lock
name is not concurrently replaced. The component borrows both descriptors.
`data` is the only replacement name; `temporary` must initially be absent.
Bytes are synthetic, live and immutable for the call; n is0..4096, NULL only for0.
All output/control storage is distinct. Hooks are synchronous, nonreentrant,
do not mutate inputs or retain borrowed storage, and are permitted to suspend
only the dedicated writer in the interruption experiment.

Acquire the stable lock, exclusively create `temporary`, write all bytes, sync
that file, close it, rename it over `data`, then sync the directory. Report the
last completed stage: 0 none, 1 bytes written, 2 file sync, 3 rename visible,
4 directory sync accepted. `visible=1` means this call completed rename, not
that no later writer can change the name. On failure preserve the first error;
before rename remove only a temporary file this call created. Never unlink a
pre-existing temporary file after EEXIST. After rename do not promise rollback.
Close owned handles and release the borrowed lock on ordinary return. Return
the positive errno and publish the progress/error record. Invalid input and
failure acquiring the lock leave the output untouched. Cleanup failures after
progress retain that progress; no API can promise repair after an arbitrary
filesystem failure. The bounded campaign injects transition failures, not every
possible close/unlock/media error.

`fail_stage` 1..4 injects EIO at write, file-sync, rename or directory-sync.
Stage1 may leave partial temporary bytes before cleanup;0 disables injection.
Observe application buffering separately using a supplied FILE buffer, file
size before/after fflush, and accepted fsync. Replacing the file with a pipe
must yield the actual unsupported synchronization error. Do not classify fflush
as power-failure durability.

Terminate only your own writer after each successful hook. Parent/child initially
share the lock description in this cut harness; the parent must explicitly
unlock its surviving reference after wait. The observed old/new bytes are
process-termination evidence. The original abstract fixture additionally assumes
honored file/device synchronization and a durable old name. It leaves recovery
after rename but before directory sync unspecified. Kernel cache, hypervisor
cache, device firmware and power failure have not been reset by killing a child.

### Clock and capability operations

`pc_deadline(now,budget,out)` checks unsigned nanosecond addition, rejecting
overflow with ERANGE and preserving output. `pc_remaining(deadline,now)` returns
the nonnegative interval length. All operands belong to the same selected clock
domain; C integer types cannot enforce that semantic condition. Derive the
interval relation before implementing subtraction. Recompute against one
absolute deadline after interruption; restarting a full relative budget extends
the contract. CLOCK_REALTIME jumps and Linux suspend differences are supplied
clock-adapter inputs, never changes to the host clock. Record process CPU time
separately from elapsed time.

`pc_choose` owns no resource. A caller-initialized zero PCCapability records one
choice per nonzero epoch. Repeated calls in that epoch return the cached choice.
An epoch change while an operation is in progress returns EBUSY unchanged. A
successful probe selects epoll. ENOSYS/EOPNOTSUPP may select poll only when the
caller has established an equivalent operation contract; all other failures or
weaker alternatives become an explicit error state. The epoch and equivalence
judgment are programmer obligations, not facts established by header visibility.
`pc_operation_status(error, completed)` distinguishes success, failure before
progress and failure after progress; never silently replay a nonidempotent prefix.

`pc_ready` compares a single live pipe read-end, borrowed for the call, at zero
timeout. Both backends are level-sensitive: empty0, data1, repeated observation1,
drained0, EOF1. There is no edge-trigger, regular-file, priority, signal-mask,
multi-handle ordering or performance equivalence claim. Errors return -1/errno;
the epoll handle is local and closed. Changing that contract requires a new
equivalence argument. `pc_request` is the explicit permission/CPU-availability
model: a nonzero requested mask must be a subset of the current available mask;
an injected error wins before publication. On any failure `accepted` is unchanged.
This model does not change host affinity or scheduling policy.

## Necessity, reasoning and original objective bindings

Each row requires its own explanation, implementation/analysis evidence and
changed task; passing a shared binary cannot substitute for those arguments.

| Original objective | Necessary distinction and required evidence | Changed task |
| --- | --- | --- |
| T23.01.a | PID/thread identity, effective credentials, private address space versus shared mapping; ordered before/after contents and addresses | Exec a new original image; compare retained PID/credentials/cwd/limits and replaced image/private mappings |
| T23.01.b | Exact argv/env/cwd/limits, including empty argument and '=' inside value; parent unchanged | Allowlist omits config; exact23 result, no PATH lookup |
| T23.01.c | Separate C declaration, POSIX contract, libc wrapper and Linux mechanism; fork-memory and shared-offset observations | Analysis port in PORTABILITY.md; no non-Linux execution inferred |
| T23.02.c | Missing threads cannot release copied ordinary mutex state; enumerate every child operation | Spawn file actions and failure reporting, source/runtime discrepancy recorded |
| T23.03.a | Descriptor flags versus open-file-description status and position | Independent second open changes offset relationship |
| T23.14.a | Owner/version/optional-support manifest for five interface groups | Single-pipe poll adapter, same level contract |
| T23.14.b | Clean translation-unit declaration probes versus runtime support | New-header/old-kernel model, changed capability epoch |
| T23.14.c | Capability cache, equivalence and operation progress | Capability loss after one nonidempotent effect; partial result without replay |
| T23.12.a | Application/kernel/file/directory layers; injected file-sync error | Pipe synchronization is inapplicable |
| T23.12.b | Stable lock ownership, transactional prefix, post-rename failure | Original uncooperative writer changes only synthetic file; advisory limit |
| T23.12.c | Recovered visible bytes after controlled process termination | Disposable-VM fault design in PERSISTENCE_EXPERIMENT.md, no host-device operation |
| T23.13.a | Monotonic deadline and CPU clock; unchanged absolute deadline after interruption | Active-only versus suspend-aware lease model |
| T23.13.b | Read-only host settings, requested versus accepted state and errors | CPU set changes between inspection/request; reject stale assumptions |
| T23.13.c | Planned/wake/dispatch/completion raw samples, load and instrumentation | Synthetic higher-priority interval invalidates prior empirical maximum |

Require ownership graphs for both launcher channels and storage handles; prove
prefix preservation on partial write/error, identify the rename publication
point, and enumerate possible results after each completed stage. State O(n)
byte movement, O(1) user-space protocol state and fixed handles; kernel work,
filesystem latency and scheduler delay have no constant-time guarantee. Account
for child stack/address space, libc spawn machinery and sanitizer instrumentation
separately from application state. Explain why virtual address equality, an API
return0 or the largest observed latency cannot prove stronger properties.

## Deliverables and verification

D1: completed learner implementation and operation/ownership ledger with exact
source/header/toolchain inventory. D2: predictions and full observed process,
descriptor, replacement and clock traces; retain all failure outputs. D3: an
independent oracle comparison and meaningful mutant rejection, including wrong
environment, suppressed file-sync error, expired-deadline wrap and unsafe replay.
D4: source-inspected interface/portability table and explicit evidence gaps.
D5: each original changed task, delayed on a separate learner occasion; compare
the new assumptions and include the VM experiment design without executing it
against host storage. D6: timing raw data/distribution, filesystem identity and
resource/effect costs; no invented hardware measurements.

Build each tests/*.c driver against learner.c and the public header using C17,
warnings-as-errors and pthreads; helper.c is a separate original executable.
Pass its absolute path to the process driver. Set PC_STORAGE_ROOT to an owned
private test directory on the selected filesystem for storage. Never pass an
operational directory. The author runner `tools/run_posix_context_followthrough.py`
uses private assessor sources, six profiles, independent Python fixtures, clean
feature probes, intended mutants and separately compiled unsolved learner
rejections. Learner exports contain no private reference implementation.

Failure is any incorrect status/progress/content, ownership leak in tested
ordinary cleanup, changed parent settings, missing/ambiguous source scope,
silenced mutant, replay after partial progress, or conflation of model and target
evidence. Named `PC_ASSERT` controls must reject for the intended assertion;
compile failure, timeout or unrelated sanitizer failure is not a passed mutant.
Real OS resource exhaustion remains possible; retain and diagnose that result.

## Rubric and hints

The exact objective-specific criteria and levels remain in the three canonical
legacy assessment JSON files. Apply them individually: level0 lacks a correct
operation contract; level1 reproduces a happy trace but misses the row's boundary;
level2 supplies a correct implementation/analysis, predicted boundary/failure
evidence and honest limits; level3 independently derives and defends the changed
condition without assessor source, including its counterexample. Author test
success establishes neither external review nor learner level3.

Hint1: draw two columns for handle identity and the resource it refers to.
Hint2: mark the first irreversible/publication step and retain the last completed
stage on failure. Hint3: distinguish the authority that promises a property—ISO
C, POSIX, glibc, kernel, filesystem or controlled test model. For the particular
answers and minimized diagnoses use the separate assessor analysis, never ship
it in a learner workspace.

## Security and validation limits

All executable adverse examples target original programs and synthetic files in
new private directories. Attacker capabilities are limited to the supplied
uncooperative writer or controlled failed input/transition; no credential change,
external target or production data is involved. The harmless success condition
for the lock example is a changed synthetic payload while a cooperating lock is
held. It demonstrates a property outside advisory enforcement, not arbitrary
code execution or privilege escalation. The repair is a stable lock protocol for
cooperators plus an explicit external authority boundary; a malicious authorized
writer still invalidates the assumption. Re-run regression with a second open
of the stable lock, replacement data inode and deliberate nonparticipant.

Status: authored/executable; actual execution is only the current source-bound
validation.json. Unrun variants, unavailable sources and failed experiments
remain recorded as such. No independent learner, external review, non-Linux
runtime, actual older kernel, hardware suspend, device power-cut or scheduling
guarantee is supplied by this packet. The finite test set falsifies specific
errors under recorded conditions and does not establish absolute mastery.


## Existing launcher partial: retained reaping authority (T23.02.a)

The earlier launcher package remains unchanged. Its actual ordinary exit0/127,
missing image, pipe/fork/exec and EINTR/descriptor controls are still required.
The additive pc_reap adapter completes its explicitly retained hard-wait failure
obligation. Initialize a unique PCTicket phase1 with the positive PID returned
by your owned launch; all error-channel ends are already accounted for by the
launcher ledger. Do not fabricate tickets from arbitrary PIDs or copy ownership.
No other reaper, SIGCHLD ignore or auto-reap policy is permitted during ordinary
operation. PCWait is a synchronous adapter with waitpid(pid,&status,0) semantics;
it may inject an error before consuming the child, but cannot consume a child
and lie about success. It must not reenter or mutate the ticket.

Retry EINTR. A different recoverable adapter failure preserves phase1/PID and
result storage so the caller can retry or transfer that exact ownership to a
supervisor. No terminal success is invented and no implicit kill occurs. Actual
ECHILD means this API has no collection authority: retire the ticket as phase3,
clear the PID, preserve output and prohibit future wait/signal through this
handle. It cannot reconstruct an externally consumed exit status. The controlled
competing-reaper case diagnoses the broken ownership premise instead of risking
signaling a subsequently reused PID. Successful exact-child collection moves to
phase2, clears the PID and publishes decoded exit/signal state. Reusing a terminal
ticket returns EALREADY without calling the backend. No eventual completion is
promised if an external failure persists forever. EIO/EINVAL in the recovery test
are explicitly injected adapter failures; EINVAL is not asserted possible for a
correct real waitpid call with options0. A real ECHILD observation follows a
controlled competing reap. This is failure containment with an honest loss state.

The changed SIGTERM task has a separately owned original child and exact matching
WIFSIGNALED interpretation. Mutant5 drops the pending ticket after the first hard
error and must fail ownership preservation. Retain its concrete rejection and
compare it with successful retry, duplicate collection refusal and actual ECHILD.
