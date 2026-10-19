# Source-bound T75 cost interpretation

The final `resilience_service_cost_validation.json` contains641 observed command
or captured-artifact records, four GCC/Clang O0/O3 configurations,40 workload
observations and280 measured host samples after40 warmups. Both counted and
counter-free binaries pass the9096-history composition oracle and121554-history
owned-pressure oracle. The latter also executes the native pressure harness.
Every one of the11 successor objects has a size/disassembly/undefined-symbol
record; compiler dependency closures and source pins bind these observations.

The four configurations agree on this host layout: scope64 bytes/alignment8,
fractional bucket48, timer object848, retry104, retained NetStack13904, owned
arbiter608, gate224, journal224, outbox272, MsgHub224, service328, owned queue136
and producer pump616. Allocator metadata, borrowed device storage and OS objects
behind POSIX interfaces are additional. The large retained NetStacks dominate
the controller's automatic state; tiny token-bucket fields do not imply a tiny
composed service. GCC O3 reports the controller frame as31920 bytes, journal
open480, retry plan144, hedge completion112, producer begin96, credit release48
and scope release16. These are individual compiler frame observations, not a
whole-thread stack bound.

For long-tail responses, unhedged work dispatches4 and returns at virtual tick8;
the hedged configuration dispatches3 and returns at3, while both become quiet
at8. Here the hedge avoids later retries but allocates one extra18-byte response:
successful requested heap totals/peaks change from82 to100 bytes. This does not
generalize to correlated loss: all-loss unhedged uses4 dispatches, hedged8, both
have no winner and become quiet at17. No response allocation occurs in those
all-loss histories. Slow-consumer restart yields two deliveries of one event and
one durable ACK. Cancellation can leave one committed effect and notification
while producing no acceptable reply. These are actual C counts under the
declared model, not inferred from the exercise heading.

The current GCC O3 medians for100 complete controller runs are223882ns unhedged
tail and247942ns hedged tail; correlated loss is206255ns versus263957ns. These
host totals include fixture setup, tracking, allocator and synchronization work.
They are neither network tail latency nor proof that one strategy is faster on
another platform. Raw distributions remain in the report: the Retry-After
sample maximum417921ns exceeds its150113ns median, illustrating noise that must
not be hidden by a single favorable sample. No hard worst-case bound follows.

The inspected GCC O3 scope object uses `lock cmpxchg` for checked references and
indirect allocation/disposal calls. Fractional credit uses unsigned division and
has no undefined external symbols in this build. The pressure object calls the
actual MsgHub and pthread synchronization routines and has indirect disposers.
The hedge calls mutex primitives, clock/decoder/disposer function pointers and
actual gate/scope operations. The service object calls `hs_read_until`, `poll`,
`send`, `clock_gettime` and checked policy operations. The journal calls device
callbacks; the outbox has an implicit `memmove` call in this optimized build.
The timer and several other objects depend on `__stack_chk_fail`, and the
controller uses `memset`. These hosted dependencies are allowed and recorded;
the result cannot be reused as evidence of a helper-free freestanding image.

The13-dimension worked contract is `labs/resilience/followthrough/COST.md`;
the readiness producer emits a mechanism-specific matrix with measured object
records and explicit bounds. The native six-frame queued burst is a separate
arrival workload from the timing model's burst-loss workload. The two should
never be conflated to close an obligation. Prior cost attempts are retained in
the initial/pre-object-audit reports, with their narrower source identities.
No historical accepted receipt, learner seal or independent assessment changed.
