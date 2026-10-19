# Timer identities, counter notifications and callback ownership

This additive C17 hosted contract implements PR-T24-05's three ordinary tasks
and supplies the corresponding seven-day transfer assessment. It changes no
historical acceptance record. Linux interfaces and POSIX synchronization are
dependencies of the native adapter, not features guaranteed by ISO C. C23 does
not turn descriptor readiness into lifetime safety; no published C23 equivalence
is asserted. Use GCC/Clang, Linux with timerfd/eventfd/epoll, libc and pthreads.
No timer/event-loop library may implement the policy. Python is a host checker.

`TdTimer` is caller-owned, address-stable storage initialized by `td_init` with
a generation limit in 1..65535. Its public fields enable inspection, not arbitrary
mutation. All operations on one timer and scans of a timer array must share one
serialization discipline. The reference race adapter uses one POSIX mutex.
The plain C core provides no synchronization, automatic owner checking, or
safe reclamation of a freed object. Acquire the mutex before inspecting state.

`td_arm(timer, deadline, period, skip, &generation)` accepts an absolute positive
uint64 instant and either a zero one-shot period or a positive period. `skip` is
0 or 1. Success returns1 and publishes a fresh generation, replacing an ARMED
registration. Invalid arguments return-1. RUNNING/CANCELLING or generation
exhaustion return0 with the output unchanged. An exhausted IDLE object becomes
RETIRED; a failed exhausted rearm preserves an existing ARMED registration.
No generation is reused, including after cancellation. Do not reinitialize
storage while any old token or callback borrower can exist. This teaching
epoch retires rather than asserting that a larger counter cannot wrap.

`td_begin(timer, generation, now, &dispatch)` returns1 only for a current ARMED
generation whose deadline is due. It transitions to RUNNING and provides the
original scheduled instant, expiration count, effect count, next instant and
arithmetic-exhaustion flag. It returns0 for stale, early, inactive or reentrant
dispatch and leaves the output untouched; null arguments return-1. Begin does
not call a callback. A successful caller owns a callback borrow until finish.

For periodic timers, count every deadline in the closed interval from the
registration's deadline through `now`. Catch-up requests that many logical
effects; skip requests one. Both preserve the same phase and first future
deadline. Compute the count without wrap and check the next-instant product
before evaluating it. If no future instant is representable, deliver the
current aggregate and become IDLE at finish, reporting `exhausted=1,next=0`.
A one-shot always produces one effect and no next instant. Returning a large
count is O(1); actually executing that many effects is a different, potentially
unbounded callback workload. A real application must budget, aggregate or reject
such work explicitly. This core does not secretly skip required effects.

`td_cancel` returns1 for a stopped ARMED registration,0 for IDLE/RETIRED and2
for RUNNING/CANCELLING. Result2 marks CANCELLING and means **still borrowed**.
It is not permission to free callback state. Repeated cancellation preserves
this result until `td_finish` returns the borrow. Finish accepts only the current
RUNNING/CANCELLING generation, returns1 once, and either rearms the computed
next period or becomes IDLE. A stale/repeated finish returns0; null returns-1.
There is no asynchronous pthread cancellation and no blocking cancel operation.
Join actual callback threads before destroying their mutex/condition objects.

`td_earliest` returns1 with the lowest-index earliest ARMED timer,0 for an empty
eligible set (outputs unchanged), or-1 for invalid pointers. Count0 permits a
null array. Scan cost is O(N), storage O(1); the native exercise uses three
timers. Replace this with a heap only after preserving token, tie, failure and
ownership contracts. `sizeof(TdTimer)` and target padding are measured facts,
not portable wire sizes. Core imports must contain no allocation service.

The Linux adapter owns each descriptor until close, reads native eight-byte
counters into `uint64_t`, distinguishes EAGAIN/EINTR/errors, and never interprets
notification count as callback identity. Rearm/disarm cannot erase an event
already copied to user memory. Its absolute time conversion is explicitly
limited to the selected signed time_t of at least64 bits. No suspend, realtime
clock changes, alarm capabilities, physical clock accuracy or real-time latency
bound is tested. No global host settings may be changed.

Public `timer.c` and `native.c` are intentionally unfinished. `tests.c`,
`race_tests.c` and `run.py` are supplied scaffolds. The inherited deadline
provider's learner source is also unfinished; integrate a learner-owned
`hs_read_until` with its original cancellation-wins-ties contract. Author-side
runs identify their separate reference provider. Keep assessor code outside a
learner workspace. Acceptance is criterion-specific in TIMERS.md, never merely
the existence of a compiled executable.
