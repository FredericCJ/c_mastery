# Assessor-only timer analysis and delayed packets

Assess the three original criteria independently. Read public CONTRACTS/TIMERS
for premises, source attribution and environment; do not export this file or
the reference implementations. The current author receipt will identify exact
compiler/source versions, commands and observations. It cannot certify an
unaided learner, physical clock accuracy or every possible execution.

T24.09.a: ordinary eventfd combines2+3 into5; the four-byte read rejects and
does not consume it. The full read drains it. Saturation rejects a further1
without wrapping, whereas semaphore mode produces three1s. The 64-bit value
is host representation, not a big-endian protocol. The rearm proof is an
invariant: the one-shot represents the minimum of all ARMED deadlines after
every mutation. Recompute after insertion, cancel, begin/finish or replacement;
empty means disarm. Timer notification is merely a reason to recompute due
work against the monotonic clock. A cached notification is not itself a new
callback identity. A heap with stable handles is an acceptable alternate
representation if it preserves tie, deletion, exhaustion and ownership rules.
Reject a polling implementation that ignores earlier insertion, a Boolean
counter, or a native result obtained with the wrong provider stub.

T24.09.b: for now>=d and p>0, count=floor((now-d)/p)+1. Since d>=1 the numerator
is at most UINT64_MAX-1, so +1 cannot overflow. Guard count>(MAX-d)/p before
forming next=d+count*p. The count/effects remain meaningful even if no next
instant exists. For d10,p3,now20, catch-up effects4, skip effects1, both next22.
`now+p` gives23 and is a drift mutant. The calendar oracle uses repeated
addition and a list of due instants for14,112 small parameter cells; it does
not call the production formula. Separate explicit full-width tests cover
d1/p1/nowMAX, dMAX/p1/nowMAX and d1/pMAX/now1. In the native periodic trace the
read's actual linearization falls between surrounding clocks: require the
reported count between the corresponding bounds, not an arbitrary exact6.
Effect count is a decision, not evidence that callbacks executed at deadlines.

T24.09.c: successful arm consumes a token, and begin checks that exact token
before touching the new operation. Exhaustion must preserve an already armed
registration on rejected rearm; only inactive storage retires. Cancel of an
active callback enters CANCELLING; finish alone clears the borrow. Cond-wait
releases the mutex while retaining the logical callback/context. Calling a
blocking cancel from that callback would wait for itself. Returning2 avoids
that contract, but moves quiescence responsibility to the caller. A different
API may wait from a separate owner provided it rejects self-wait and proves
the waiter's own storage lifetime. Neither API can make a dangling C pointer
safe after the programmer frees storage early. The forced running/cancelled
tests establish both branches; the200 racing cases may all choose one order.

Deliberately compiled negative controls: lost periodic count, phase drift,
ignored generation, reused epoch and early release. The early-release source
is also run against actual thread borrowing, not merely the core state test.
The known-race TSan control is the original reactor teaching counter and is
an expected sanitizer diagnostic under this environment, never a portable UB
golden value. NDEBUG builds keep the harness's CHECK assertions active.

## Withheld changed-condition packets (at least seven days)

For .a, give three registrations with deadline ties, remove the minimum,
insert an earlier timer while a prior notification is cached, and exhaust
the epoch. Require both selected identity and actual one-shot reprogramming;
request a failure after acquiring the first descriptor and inspect cleanup.
The packet uses the public API but withholds its operation order until the
attempt. Supply the sequence in the attempt record, do not change constants
alone: remove, replace, cache and retire are different transitions.

For .b, ask for an application that can aggregate counters and one whose
effects must run individually. Switch catch-up to skip, move the dispatch
across several periods, and force next-instant exhaustion. Require the
learner to explain whether skipped effects are acceptable for each application,
derive resource bounds and preserve phase. A faster runtime cannot excuse a
different effect contract. The finite oracle is an independent arithmetic
check, not a proof about wall-clock scheduling.

For .c, begin a callback that blocks on an owned predicate. Deliver cancel
before, during and after it, then attempt rearm and repeated finish. Ask the
learner to redesign cancellation to wait from a separate controller and to
construct the self-cancel deadlock counterexample. The implemented variant
chooses explicit still-running status; a waiting design is an assessable
alternative, not falsely labeled the reference implementation. Require a
small interleaving argument in addition to native traces. Preserve actual
attempt identity, date and delay; no author campaign supplies that evidence.
