# Private T24.01–02 analysis and delayed assessment

Do not include this directory in learner exports. The original six AX criteria
and their six minimum-seven-day variants remain authoritative. `signals.c` is an
author reference, `signal_probes.c` supplies owned-process/thread experiments, and
`signal_failures.c` injects four acquisition failures with the GNU linker wrapper
mechanism. Do not mistake any of these for learner-built artifacts.

## Criterion-specific discrimination

**T24.01.a.** Before delivery, the ordinary pending bit is present, but a second
zero-time synchronous acceptance yields EAGAIN after the first consumed it. The
two-cell application queue instead returns7 then9 and refuses a third record
without overwriting. Award2 only if the learner distinguishes kernel pending
membership from counted application admission and restores the original mask.
The delayed real-time variant must recover four ordered SI_QUEUE payloads, then
show controlled EAGAIN at the owned child's zero soft RLIMIT_SIGPENDING. That
limit is enforced against real-UID queued signals; it does not measure a private
kernel queue capacity. Existing UID contention can prevent the positive four
admissions and must be recorded as unavailable execution, not skipped. Award3
only with the ownership, finite resource and ordering argument. A standard signal
sent repeatedly after each consumption is a different experiment.

**T24.01.b.** The observer enrolls the original child in `read` using its syscall
number and descriptor, then sends SIGUSR1. Without SA_RESTART expect -1/EINTR;
with it, the handler acknowledgement precedes a parent write of X and return1.
The short-read probe obtains2 of8 available/requested bytes. It does not claim
that a later signal retroactively caused the short read; the point is that
positive progress is committed independently of a subsequent retry. A solution
which discards a positive return and restarts at buffer offset0 fails. The delayed
budget schedule has independent hand-calculated intervals1,.6,.2; resetting to1
is rejected even if a quick real run happens to finish close to its deadline.
Accept monotonic absolute `clock_nanosleep`/other justified APIs with equivalent
contracts, not realtime-clock subtraction vulnerable to clock adjustment.

**T24.01.c.** The defective handler can interrupt a thread while that same thread
holds `lock`; a nonrecursive mutex acquisition cannot make progress until its
interrupted owner resumes, which requires the handler to return. Changing to a
recursive mutex does not make malloc/stdio reentrant or async-safe. This is an
analysis counterexample under explicit POSIX premises, not a portable runtime
output or exploit chain. The reference handler writes independent predicates
before notification and restores errno even when write fails with EAGAIN. Its
descriptor load is an actual lock-free atomic access; no blanket guarantee is
made for arbitrary atomic types or platforms. Award2 requires this reasoning
plus full-pipe and saved-errno evidence. In the delayed receiver, sigtimedwait
returns in ordinary thread context, then allocation and mutex transfer are legal
under their ordinary contracts. Joining establishes the end of accesses before
free/destroy. Award3 requires explaining both context and ownership, not just
moving text to a second function invoked by the handler.

**T24.02.a.** The broken child reports its check-false point, its handler writes
an acknowledgement, then it reports predicate-true before pause. The parent
observes SYS_pause and rescues that same child with SIGUSR2. This is a bounded
property violation, not an uncontrolled hang. The repair blocks SIGUSR1 before
checking; a pending instance is delivered during atomic sigsuspend unmasking.
An always-unblocked self-pipe protocol is another valid repair if its durable
predicates, drain ordering and full-pipe argument are complete; do not insist on
one implementation. signalfd uses blocked signals and exact record reads. Grade
the delayed two-condition task separately: clear-all loses OTHER after STOP and
is a semantic mutant. Neither a single boolean nor draining bytes can represent
two independently consumed predicates without additional state.

**T24.02.b.** The two workers and plugin verify inherited masks before announcing
readiness under the mutex. The receiver's accepted process signal cannot be
delivered to a worker with that signal blocked. In the bad-plugin control, only
the plugin unblocks SIGUSR1 and no synchronous waiter is active; its handler
acknowledges the process-directed signal, leaving no pending signal for the later
receiver. This demonstrates an interface violation without asserting which of
several eligible threads an implementation must choose. Award2 for the inherited
mask and ownership proof;3 for a plugin creation/entry contract that handles
foreign threads and rejects a broken integration. Merely masking the receiver
or relying on the process creator's mask forever is insufficient.

**T24.02.c.** WNOWAIT observes all three exits while retaining their waitable
statuses. A full owned-PID WNOHANG sweep obtains20,21,22 and clears all three
slots; a second sweep obtains0. SIGCHLD may have coalesced to one notification,
and shutdown is an independent predicate. The delayed final-child gate is
released after the first sweep, with its exit observed before waiting; the
blocked pending signal bridges that interval into ppoll. The one-child mutant
returns1 when three are already known waitable and must fail. Award3 only if
the learner derives why a newly exiting child cannot be lost between a sweep
and the atomic wait, and excludes unrelated child ownership. ECHILD is not
silently converted to successful cleanup of a supposedly live owned PID.

## Reference invariants, failures and source limits

Open installs handlers only while their signals are blocked; its one owner
maintains both pipe endpoints until quiescent close. Ordinary accesses to the
three signal predicates occur with the managed mask blocked. Handler assignments
are individually permitted storage operations; their conjunction is not an
atomic multi-field snapshot without that mask. The compiled handler must import
only the selected write/errno path, with no libatomic, malloc, stdio or mutex.
The object-wide import list contains ordinary setup and wait operations too;
do not confuse whole-object dependencies with handler calls.

The four acquisition cuts are pipe creation and each of three sigaction installs.
Every failure must restore mask/dispositions, balance descriptors and permit a
fresh successful open/close. Fault injection changes only this original program's
link resolution; it does not alter libc or the host. Broader OS resource failures,
signal delivery scheduling and destruction under external nonquiescent senders
are outside this one-owner contract and must not be labeled tested.

The public queue oracle shifts a two-element array; the implementation wraps its
head. The deadline oracle uses small integer milliseconds, not the same timespec
borrow algorithm. There are160,801 deadline pairs and9,841 queue histories, not
an unbounded proof. Source access is exact and versioned in resources.json;
N1570's storage/call rules are a C11 draft comparison. Published C17 access is
still a distinct gap. Parallel authoring is not external review.

Run the private campaign across GCC/Clang O0, O2/NDEBUG and ASan/UBSan. Require
all13 original process/thread modes in every profile and each public test. The
five compiled mutants must fail their intended assertion with exit1; a crash,
timeout or compile error is not their semantic success. Compile the learner stub
and demonstrate its behavioral rejection. For the ordinary receiver, run TSan
separately and retain any environment failure honestly. The guard must reject
missing variants, missing modes, stale artifacts, skipped lock-free inspection
and a substituted crash result. Actual delayed learner attempts remain pending.

The first object audit rejected compiler-inserted `__stack_chk_fail`; retain that
failed receipt. The selected reference translation explicitly uses
`-fno-stack-protector` and reaudits the handler, without claiming that arbitrary
compiler mitigation runtimes are async-safe. This does not change host policy.
