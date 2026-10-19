# T75.07 technical assessment: publication, cleanup and identity

Read the learner HEDGE.md contract first. `hedge.c` composes actual RZHedge,
RzbGate, RzfScope and RZReceiver implementations; none is reimplemented as a
test-local substitute. The request mutex serializes admission, scope/time
publication checks and dedup effects. It does not cover the decoder or disposer.
These callbacks hold counted ownership so metadata can be queried while paused.

For R1, request cancellation and result commitment can happen while another
decoder or disposer is still active. The original completed mask cannot certify
the callback has stopped using storage. In the blocked-disposal test both bits
are set, but callbacks=1 and gate.used[0]=1. Only the second locked finalization
releases the permit and its scope owner. The request owner remains until destroy
or replacement. A copied raw pointer would not satisfy this proof.

For R2, first claim is under the mutex and makes the input descriptor empty.
Exactly one callback owns those bytes. A valid decoded result moves to
winner_blob; losers dispose their local blob. take moves winner_blob to caller;
abandon discards only the untaken winner. The booleans distinguish a taken empty
representation from unavailable result. Repeated abandonment cannot increase
unbounded cleanup ownership. A malformed completion never consumes the input.
Generation, owner and attempt checks precede claim; the stable initialization
identity remains a caller precondition after destruction/address reuse.

For R3, two workers enter decoders and a condition predicate controls which may
return first. Enumerating the four acceptability pairs in both orders detects
the false rule 'first completed response wins'. In particular, a rejected fast
decoder leaves the other eligible. Pausing until clock==deadline makes even a
previously received valid response ineligible at publication. Separate32 rounds
release both real threads at once. The mutex, not those32 examples, supplies
serialization; TSan checks observed accesses but is not an all-schedule proof.

Control71 accepts failed decoding, control72 omits callback/permit quiescence,
control73 ignores request generation. The corresponding tests fail an explicit
RZ_CHECK. They are semantic controls, not intentional undefined behavior needed
to obtain a sanitizer failure. Do not accept a crash or build failure as the
mutant outcome. The learner stubs must compile and fail their anchor assertion.

R4's independent Python model stores abstract completed/cancelled sets and blob
counts, avoiding the C bitmask/gate representation. It predicts7,388 observable
histories. Fixed seeded delay inputs drive16,384 real C request lifecycles. For
perfect correlation, second completion is exactly four ticks later than original,
so it cannot improve the result deadline or latency; it adds nonnegative work.
For the independent sample, comparisons may improve tail latency/timeout count,
but evaluate the observed extra backend duration and attempts. These models have
no queueing or shared server saturation; improved isolated latency is not proof
of improved production throughput. A later resource-coupled model can reverse it.

The received effect uses the original receiver's finite request-retention model:
same identity and delta returns original result, differing delta refuses. Neither
the fence nor the request-generation ticket makes effects durable across process
restart. The required journal/service increments address that distinct claim.

Complexity is O(response bytes) for the supplied decoder and O(1) metadata per
attempt; the retained receiver scans4 entries and gate scans2 tenant slots.
The arbiter itself performs no heap allocation; scope/response allocators are
explicit dependencies. At most two decoder/disposer owners plus one abandonment
disposer exist. Latency can be unbounded if a callback never returns; no API may
turn cancellation into a fake acknowledgement. Measure representation bytes,
stack, code size and repeated wall-time costs separately from virtual latency.

Enforcement: C object/alias/lifetime rules apply to valid objects; POSIX mutexes
supply mutual exclusion and synchronization; runtime tags/counters enforce the
finite state checks; compiler warnings cannot verify ownership; finite oracle,
sanitizer and mutation results falsify selected errors. Callback honesty, joined
destruction, stable initialization identity and clock premises remain explicit
discipline. A Go cancellation function's non-waiting behavior is a useful
comparison, but these C representations are not language-level lifetime safety.

Assessment remains per R1--R4. Require learner derivations and changed assumptions,
including stale worker, different decoder failure boundary and correlation.
Author-provided executable support does not establish independent learner mastery,
physical timing, arbitrary thread correctness or full service completion.
