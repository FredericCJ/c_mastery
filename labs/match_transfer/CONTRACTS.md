# X-T36-003: streaming matches with partial publication and cancellation

This required companion implements the original V1 composition and explicitly
reuses the V2 prefix provider. Preserve X-T36-003 R2/R3, the minimum seven-day
delay and the ordinary two-pass all-match exercise. Families B/C/D/F/G/H/J apply.
The prerequisites are the completed `struct al_stream` adapter and original `struct ad_matcher`,
their ownership/count-on-failure contracts, prefix-function derivation, callback
effect rules and the original local all-match oracle. Reference artifacts are
not learner work. Use C17 with the same byte/offset domains as the retained
`labs/algorithm_lookup/CONTRACTS.md`. A caller owns live, disjoint state, input,
callback context and report objects; calls are serialized and non-reentrant.

Implement `mt_init`, `mt_feed` and `mt_drain` in the separate learner artifact.
The stream owns an actual ALStream,65 pending uint64 offsets, a cursor and state.
Initialization copies a pattern of0..16 bytes through the actual prefix adapter.
Each input chunk is0..64 bytes. The input is borrowed for the feed call only;
the underlying prefix state retains its derived suffix/prefix information,
not a pointer into that input. No target allocation or library matching is used.

Feed first asks the actual adapter to compute all matches into owned pending
storage. Capacity65 includes the empty pattern's initial boundary plus64 byte
boundaries. On successful computation the whole chunk is consumed, even if a
later callback fails. Invoke callbacks in increasing offset order. Result0 means
one accepted effect;1 or any nonzero value except2 means failure with no effect
from that invocation;2 means cancellation with no effect from that invocation.
The report counts attempted and accepted callbacks **in this invocation**.
Earlier accepted effects remain committed. The callback must not retain internal
addresses, modify the borrowed stream/input or reenter it. Context and callback
are not retained after return. ISO C does not enforce these effect/lifetime rules.

On callback failure return `AL_FAILED` and preserve the failed offset plus all
following pending offsets. Only `mt_drain` retries that pending suffix; do not
feed the already consumed chunk again. While pending, a new feed returns
`MT_BUSY`7 without changing stream, report or calling a callback. A successful
drain clears pending state and returns `AL_OK`. Draining an open empty stream
returns OK with zero report. On cancellation return `AL_CANCELLED`, discard the
unpublished suffix and enter CLOSED. Future feed/drain returns `MT_CLOSED`8,
unchanged, until explicit initialization. Cancellation cannot undo committed
effects and is not OS thread cancellation. Reset intentionally discards pending
offsets; they are values with no external destructor ownership.

An invalid argument or prefix-provider syntax/extent/offset-overflow failure
preserves the stream and report and invokes nothing. Caller state must be a
valid initialized object, not adversarially corrupted control metadata. The
output report must be live on every admitted call. Successful initialization
starts a new lifetime; failed initialization preserves its destination. Match
offsets are source-stream absolute offsets. The empty pattern emits boundary0
once and each new byte's ending boundary, never an extra boundary for an empty
later chunk. Distinguish this from treating every chunk as an independent string.

Worked input: pattern`aaa`, chunks`aa` and`aaaaa` yield offsets0,1,2,3,4. If the
second callback fails, offset0 is committed,1..4 remain pending and consumed is7.
Drain retries1, then2..4, exactly once each. If the second callback cancels,
only0 is committed,1..4 are discarded, and the closed stream consumes no later
input. A callback which publishes then reports failure violates this protocol;
blind retry could duplicate effects. A stronger application needs an idempotency
key/receipt protocol, not a claim that C callback types encode transactions.

Required derivations: prefix state across chunk boundaries, monotone pending
cursor, each accepted offset published once, no later callback after cancel,
and why private match staging cannot roll back an external effect. Initialization
cost is the prior prefix-table construction. Feed retains its amortized matcher
bound plus O(matches+attempts); drain is O(attempts). Pending storage is520 bytes
plus actual ALStream/control/layout, regardless of consumed-stream length but
dependent on the declared chunk bound. Repeated failures can make total retries
unbounded; an outer policy supplies deadlines or budgets. Report compiler/target
`sizeof`, stack and actual helper closure; no throughput claim follows from
operation counts or emulator execution.

Submit implementation and actual prerequisite source hashes; a sealed trace for
the example, empty pattern and maximum chunk; counterexamples for replay/drop/
continue-after-cancel defects; independent complete offset lists, failure canaries,
resource accounting and all command/raw outputs. Public tests include every
binary text length0..5/pattern0..3/chunk split (4815 packets), all five failure
ordinals for the overlapping example, new-feed rejection, successful retry,
cancellation and reset, empty chunks, maximum65-offset pressure and UINT64 offset
overflow. The naive window oracle is separate from the reused prefix algorithm.

Compile your `match.c` with the public `tests.c`/`native.c`, your actual lookup
adapter and matching provider, public dependency headers, and strict C17 warnings.
`run.py` supplies this explicit-provider workflow; no assessor file is implicitly
imported. Author tooling is `python3 tools/run_match_transfer.py`. Its three
defined-behavior mutants must compile, then fail the intended assertions, while
the unsolved learner file must also compile and fail. Target builds audit actual
objects and images using original byte helpers and no target runtime archives.

Assess R2 on exact ordinary-versus-stream failure/consumption/publication
conversion, and R3 on overlap/empty/capacity/cancellation falsification and resource
arguments. Level0 violates a contract;1 replays;2 implements, derives and
diagnoses;3 transfers independently after the delay. Every applicable original
criterion needs level2 plus the original ordinary package. Graduated hints:
first separate input consumption from callback commitment; then store the next
unpublished offset; finally write state transitions for failure, cancel and reset.

Security scope is original local synthetic byte arrays and callbacks only. The
attacker controls bytes and a deliberately flawed teaching callback schedule,
not host addresses or external services. A harmless property violation is a
missing, duplicated or post-cancellation offset. It requires the removed ownership
gate; it is not arbitrary code execution. Repair and replay regressions, retaining
the callback no-effect-on-failure and serialized-call limitations. Sanitizer
silence cannot prove these semantic properties.

External review, actual learner integration and seven-day unaided performance
remain separate from author execution. QEMU functionality is not board timing.
This composition's contract is original; source attribution reuses only the
explicitly inspected prefix-provider bodies and contracts registered with T36.
No uninspected published standard or whole textbook is credited.
