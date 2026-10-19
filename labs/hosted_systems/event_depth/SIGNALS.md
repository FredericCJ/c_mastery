# Signals as notification, ownership and an atomic wait boundary

Required criteria: **T24.01.a–c, T24.02.a–c** in PR-T24-01. Prerequisites are
the original signal-roundtrip component, C object lifetime, descriptor ownership,
T21 cleanup/error reasoning and the T51 mutex/condition enrollment contract for
the changed threaded receiver. Submit your own prerequisite implementations;
author reference executions are not learner artifacts.

## Representation and derivation

Separate generation, the kernel's pending state, acceptance/delivery and the
application predicate. For a blocked ordinary signal, generating the same signal
twice does not encode two units of work. Implement `EsCounted`: two slots, a head
index and an occupied count. `push` returns 1 on admission, 0 when full and -1 for
an invalid representation; `pop` returns 0 without changing its output when empty.
Prove `head<2`, `count<=2`, FIFO order across wrap, and no publication on rejection.
Both operations use constant space and time, require serialized ordinary code,
and are not a handler-safe queue.

For the handler port, one single-threaded process owns an `EsSignals` object,
two nonblocking close-on-exec pipe descriptors, saved dispositions and a saved
mask. Initialize the object to zero. The managed set is SIGUSR1 (stop), SIGUSR2
(a distinct wake condition), and SIGCHLD (child state changed). A second live port
is rejected. Existing pending managed signals cause admission failure rather than
being silently consumed as new port events. Open blocks the set before publishing
the handler state and leaves it blocked in ordinary code. Every acquisition
failure restores the mask and releases the acquired descriptors/dispositions.

The handler assigns distinct `volatile sig_atomic_t` predicates, attempts one
nonblocking byte write, and restores `errno`. The descriptor is read from a
lock-free atomic object; admission checks this property. `volatile` alone does
not make an arbitrary shared descriptor or a read-modify-write protocol safe.
The byte is a wake hint, not the event count or authoritative predicate. A full
pipe may reject the byte while the predicate remains true. Mask all managed
signals during each handler to prevent their nested modification. With both
ends owned and alive, teardown cannot introduce a handler-side broken pipe.

`es_wait` atomically replaces the mask during `ppoll` and restores it on return.
It reports a predicate or pipe hint; the caller must inspect/consume predicates.
`es_take(selected)` clears only selected predicates while signals are blocked,
and drains notification bytes. It must preserve other predicates even though
their bytes are drained. Its pipe-drain cost is proportional to queued bytes;
the handler and predicate operations are bounded constant work. Do not describe
a pipe's finite capacity as an unlimited event queue.

Close requires producer quiescence, including reaping owned children. It abandons
remaining owned notifications, restores dispositions, closes both descriptors
once and restores the original mask. It is not concurrent destruction; a copied
public struct is not a second owner. A failed drain keeps ownership intact. The
chosen Linux close semantics do not license blind retries of a reused descriptor.

`es_remaining` subtracts normalized, nonnegative `timespec` values after comparing
them, borrowing one second only when necessary. `es_wait_until` obtains a fresh
monotonic time after every interruption and timeout; it never restarts the total
budget. Its injected operations make the clock/wait contract independently
testable. `es_wait_fd` supplies actual CLOCK_MONOTONIC/ppoll adapters. Readiness is
permission to try an operation, not ownership of bytes. No observed timeout is
a worst-case scheduling guarantee.

`es_reap` scans at most64 known positive child PIDs, uses WNOHANG, retains partial
progress, writes zero only for a successfully reaped slot and records that child's
status. It must not reap an unrelated child. A SIGCHLD notification means inspect
all owned children; it is not one token per exit. Return the count newly reaped,
or -1 with prior successful slots still committed.

## Required experiments and outputs

| Criterion | Ordinary construction or diagnosis | Changed condition, withheld assessment |
|---|---|---|
|01.a|In a fresh child, block SIGUSR1, generate twice, inspect pending membership, accept once, then require empty. Compare two admitted queue records and rejection of a third.|Queue four same-number real-time signals with payloads7,9,8,9 and recover their order. In that child only, save RLIMIT_SIGPENDING, set its soft limit to0, require sigqueue EAGAIN, restore it. Report that this is controlled admission failure, not measurement of the host's total queue capacity.|
|01.b|With sigaction SA_RESTART off/on, interrupt an owned child blocked in a pipe read. Record return value, errno only on failure, handler observation and transferred byte. Independently record an8-byte request returning2 bytes and advance its cursor.|At a1-second absolute deadline, injected times0,.4,.8,1.0 must produce wait budgets1,.6,.2 seconds. Also interrupt actual ppoll three times and observe expiry. An errno value after success is not an error result.|
|01.c|Inspect the flawed handler below. Replace it with the saved-errno predicate/self-pipe protocol. Fill the pipe before delivery, verify the durable predicate and errno, then test two distinct predicates.|Use a synchronous signal-accepting thread with all workers masked. Allocate a diagnostic in ordinary receiver context, transfer ownership under a mutex, join every participant and free once. Explain why these services are legal there.|
|02.a|Arrange check-false, handler-set, then pause in an original child. Observe the child blocked despite its true predicate, then rescue only that child with a second owned signal. Implement atomic-mask sigsuspend; compare self-pipe and signalfd.|Consume stop while retaining another wake predicate. For signalfd read both records, then require EAGAIN; keep the selected signals blocked. A read consumes a signal, not a duplicate subscription.|
|02.b|Block the managed set before creating two workers; each verifies its inherited mask before enrollment. A single dedicated receiver accepts a process-directed signal.|Add a plugin-created worker. Require masked creation/entry validation. In an isolated defective variant, the only unmasked worker receives the process-directed signal before the receiver starts accepting; prove why a plugin can invalidate routing.|
|02.c|Create three owned children, observe each exit without reaping, then reap all with one sweep while SIGCHLD is blocked. Combine this with a shutdown predicate and repeat the sweep without duplicate cleanup.|Hold the last child at a command gate. After the first sweep, let it exit before the atomic sleep transition. The pending SIGCHLD must still lead to its reaping. Preserve exit statuses20,21,22 and all owned slots reaching zero.|

Original defective specimen for **analysis only**:

```c
/* POSIX handler called while ordinary code may hold lock/stdio/allocator state. */
void bad_handler(int sig) {
    pthread_mutex_lock(&lock);
    char *text = malloc(32);
    printf("signal %d at %p\n", sig, (void *)text);
    free(text);
    pthread_mutex_unlock(&lock);
}
```

Give a concrete interrupted critical section and the resulting circular wait or
reentrancy violation. Merely moving malloc outside the handler leaves the mutex
and stdio defects. The harmless success condition is a demonstrated notification
or routing property violation in an owned process, not exploitation of another
program. Do not assign a portable output to this unsafe specimen.

## Verification, enforcement and limits

The public test uses160,801 millisecond deadline pairs and9,841 queue histories
of length0–8 against a shifting-array oracle. It checks full-pipe delivery,
independent predicate consumption, mask restoration and an already-pending signal
at the atomic wait. These bounds do not cover every signal schedule. Add cleanup
traces for each acquired descriptor and a prediction of each negative specimen.
For blocking cases, use the owned child's `/proc/PID/syscall` identity and arguments
or an equivalently justified supported enrollment observer; a sleep before kill
does not establish that the intended syscall was interrupted. If access is denied,
retain that failure as missing execution evidence. No host security changes.

Inspect each generated port object and its imports. A hidden atomic helper defeats
the selected lock-free-handler claim; do not silently add libatomic. Separate
the ordinary hosted imports from the handler's actual write/errno path. A passing
sanitizer run does not prove async-signal-safety or absence of lost notifications.
For the new threaded receiver, prove the mutex/condition happens-before edges and
join-before-destruction independently of a particular scheduler result.

ISO C17 alone does not define POSIX masks, sigaction, self-pipes, pthread routing,
procfs or signalfd. N1570 is an inspected **C11 draft**, whose7.14.1.1p5 restricts
asynchronous handler object access and library calls; it is not a newly inspected
published C17 text. POSIX adds its own async-safe operations and errno contract.
signalfd and ppoll here are selected Linux interfaces. Source passages and exact
capture identities are in `research/v5/expansion/legacy_events/resources.json`;
unavailable published-standard passages remain source gaps.

Hints: first identify where the authoritative predicate lives; next draw the
mask state at each check, handler and wait; finally force the one-byte write to
fail while leaving the predicate true. For reaping, separate observing an exit
from consuming its wait status. Private solutions and delayed rubrics are withheld.

Generated-code qualification: this teaching port is compiled with
`-fno-stack-protector` in GCC/Clang. The first actual object audit found an
implicit `__stack_chk_fail` call in the handler under the compiler default; its
async-signal-safety was not established, so that build was not accepted. The
selected flag removes that dependency from the audited handler. This is a
translation policy for original lab objects, not a host setting or a general
recommendation to disable mitigations. Sanitizer builds remain separate
instrumented observations and do not establish a production handler closure.
