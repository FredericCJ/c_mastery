# PR-T24-05: notifications are not callbacks

Prerequisites: T18 lifetime contracts, T23 descriptor ownership and monotonic
waits, T24.07/.08 nonblocking readiness and generation tokens, and T25 mutex and
condition predicates. Bring your `hs_read_until` implementation and a written
callback ownership diagram. Read CONTRACTS.md before inspecting a result.
The stable criteria are AX-PR-T24-05-T24.09.a/b/c; the original assessment JSON
retains their family, delayed variant and independent mastery rubric.

## T24.09.a — full counters and an earliest-deadline loop

A readiness bit says a read may make progress. It is not the value returned
by that read. Linux timerfd and ordinary eventfd return accumulated native-width
counts; semaphore eventfd has a different consumption policy. Consult the
inspected [timerfd description](https://man7.org/linux/man-pages/man2/timerfd_create.2.html)
and [eventfd description](https://man7.org/linux/man-pages/man2/eventfd.2.html).
These are Linux6.19 interface sources, not ISO semantics. Exact source identities
and bounded inspected passages are in `research/v5/expansion/legacy_timers/resources.json`.

Implement the core API and the native `event` mode. Write2 and3 to an initially
empty ordinary eventfd, attempt a four-byte read, then read the full value.
Require the short read to reject without consuming the count. Check empty
nonblocking read, saturation at UINT64_MAX-1, rejection of a further1, and
rejection of UINT64_MAX. Repeat with semaphore initial3 and explain each read.
Use synthetic notifications only; close every acquired descriptor on every path.

For the changed-condition `rearm` mode, replace a periodic wake with one absolute
one-shot timer representing the earliest of three application timers. Start
with a deadline5s away, insert one5ms away, recompute and rearm, then dispatch
that timer and restore the original absolute late instant. Cancel the last
timer and disarm. Integrate your actual `hs_read_until` using a distinct owned
eventfd for cancellation. Show that a pending cancellation is returned as-3.
Do not count an old synthetic deadline fixture as this native integration.
The observed dispatch may be late; no arbitrary latency upper bound is a pass
condition. The harness timeout is only a bounded test failure, not an OS promise.

Required evidence: exact flags and widths, syscall results and errno, three
selection transitions, unchanged output on an empty scan, generated imports,
and cleanup on a failed create/arm/wait. Explain why rearming only on the next
old periodic tick delays an inserted earlier deadline. Graduated hints: first
separate notification from count; then identify who owns each descriptor; then
compare the old and new minimum after each registration mutation.

Implement a `failure` mode with controlled acquisition cuts before timerfd,
before eventfd and before epoll creation, followed by an actual invalid timer
nanosecond setting and one successful setup. Release the successfully acquired
prefix in reverse order in every case; require9 total closes and check each
owned descriptor is invalid immediately after close in this single-threaded
test. This injects local adapter failures, not system-wide resource exhaustion.

## T24.09.b — derive the phase before measuring dispatch

Let the first periodic deadline be10 and the period3. Dispatch at20 encounters
deadlines10,13,16,19. Derive the effect count and next scheduled instant for both
catch-up and skip. The next instant follows the registration's phase, not the
time when the handler happened to run. A next deadline computed as `now+period`
accumulates drift. Derive the maximum unsigned case without evaluating an
overflowing multiplication; distinguish defined unsigned wrap from a correct
deadline algorithm. A finite timestamp domain needs an exhaustion contract.

Implement `periodic`: use CLOCK_MONOTONIC, first expiration2ms ahead, period2ms,
and deliberately hold only this teaching loop until first+5.5 periods. Record
the absolute first deadline, period, clock immediately before the counter read,
returned count, and actual dispatch clock immediately after. Derive lower and
upper possible counts from those clock samples; reject a fabricated exact
kernel timestamp. Disarm before cleanup. Evaluate catch-up and skip over the
same returned interval and explain why many expirations do not prove timely
execution of many handlers. Separately distinguish CLOCK_BOOTTIME's suspend
policy; do not suspend the host or change its clock for this exercise.

Required artifacts: arithmetic derivation; a complete native trace; a calendar
oracle that enumerates small absolute instants independently of the production
division formula; extrema, zero-period and exhausted-next tests; and a cost
argument separating constant-time count computation from effect execution.
Hints: write the actual due instants first; locate the first *strictly future*
instant; finally guard the multiplication before computing the next deadline.

## T24.09.c — copied events and an active callback borrow

Implement `cached`: arm generation1, collect an actual epoll event into user
memory, cancel and disarm, then arm generation2 and update the registration's
event data. Deliver the saved event after the new timer exists. It must not
begin generation2, even when the supplied clock is far past its deadline.
Collect and execute the new event once. Repeated cancellation/finish must not
invent another effect. Exercise a three-generation epoch and refuse reuse.

The changed task runs an actual callback thread. Begin takes a borrow under the
mutex, then the callback waits on a condition predicate, atomically releasing
the mutex while it remains active. Another thread cancels, observes result2,
and attempts a forbidden rearm. Only the callback's eventual finish returns
the borrow. Also force cancel-before-begin and run200 barrier-start competing
orders. Record either permitted outcome, not a demanded scheduler distribution.
Use enrollment predicates rather than a guessed sleep. Destroy no shared state
before join. Explain the deadlock risk of a blocking cancel invoked by its own
callback and why this API chooses an explicit status instead.

Run the public core and thread tests in both unoptimized and optimized C17,
then sanitizers. A sanitizer-clean run is bounded evidence, not a lifetime proof.
Provide a known-race control when relying on TSan instrumentation. Replacing
an old-generation comparison with `fd` equality and treating result2 as released
must each be rejected for the intended semantic reason after successful compile.
Hints: separate descriptor, registration token and callback borrow; write the
RUNNING→CANCELLING→IDLE transitions; retain the context through finish, not merely
through disarm.

## Submission and assessment

From an initialized learner workspace, for example:

```sh
python3 labs/hosted_systems/timer_depth/run.py --mode core
python3 labs/hosted_systems/timer_depth/run.py --mode races
python3 labs/hosted_systems/timer_depth/run.py --mode rearm --deadline-source path/to/your/deadline.c
```

Submit source, ownership/state diagrams, counter/dispatch traces, independent
oracle, generated-object import/size output, and a minimized discrepancy from
one incorrect implementation. Supply cleanup/failure evidence, not only normal
output. The reference implementation is withheld. Level0 violates the contract
or lacks evidence; level1 reproduces an ordinary result without the causal
argument; level2 explains and falsifies each named misconception with its own
artifact; level3 independently derives and validates the changed task after
the original minimum seven-day delay. Author reference execution earns no
learner level. A result can pass .a while failing .b's drift derivation or .c's
borrow lifetime. External review and actual delayed performance remain separate.
