# Stable timer alternatives and a suspended consumer

This is required successor support for X-T75-003 R1-R4 and
X-T75-003-DELAYED. Implement `src/timers_follow.c` against `followthrough/timers.h`.
The original hashed wheel remains a prerequisite. The list, heap and two-level
wheel are three actual implementations behind the same bounded API, not three
names for one scan. The private reference is separated from learner publication.
The host harness imports the real accepted `execution` workspace's `cmx.h/cmx.c`
for a deliberately narrower scheduler comparison. Supply that workspace through
`CMASTERY_EXECUTION_ROOT`; an assessor source requires its separate explicit
`CMASTERY_EXECUTION_ASSESSOR_ROOT`. No ready-made timer container is allowed.

## One comparison contract

Use hosted C17, exact uint64 timestamps and uint32 generations, a caller-owned
stable-address object and sixteen fixed slots. No allocation or callbacks occur.
All operations on a timer object are externally serialized. Initialization is
allowed only before identities escape or after their owners retire the entire
object; reinitializing at the same address is not an identity renewal protocol.
Slots start at generation zero; each admission increments a non-maximal generation.
A slot that has used `UINT32_MAX` retires after its last active timer. Owner address,
slot and generation all participate in cancellation. Serial numbers order ties;
admission refuses before uint64 sequence wrap. Live maximal-generation timers still
expire/cancel normally. No call executes or retains a user callback.

Schedule accepts an observed admission time no earlier than the previous admitted
observation or processed time, and delay1..63. Zero delay returns `RZ_EXPIRED` and
does not enqueue. Invalid time/horizon or checked-addition overflow returns
`RZ_RANGE`; no free nonretired slot or serial exhaustion returns `RZ_FULL`.
All admission failures preserve the object and output identity. Cancellation of
an old/foreign/inactive identity returns `RZ_STALE`, without changing a current
timer. A schedule output must not overlap the timer object. All output objects must
be live writable storage distinct from live timer state. Representation corruption,
callbacks that mutate containers and unsynchronized callers are outside the API.

Advance processes an inclusive absolute target no earlier than processed `now`.
Each call may span at most1024 ticks; larger requests fail transactionally and the
caller catches up in bounded chunks. It returns at most sixteen records, ordered
by original due time and then admission sequence. Each record preserves owner,
slot, generation, due time, sequence, latest observed clock and lateness. Returning
k expiries requires at least k record writes. Cancellation before the serialized
advance wins; cancellation after its expiry record exists returns stale. An empty
advance at the same timestamp returns an empty record set. Failed advance preserves
both timer state and the output record array.

## Three representations

The sorted intrusive list keeps nodes in `(due,sequence)` order. Search on insertion
is O(N), unlink by checked identity is O(1), and expiry removes the due prefix.
The binary heap keeps a position in each node; insert and arbitrary checked removal
repair upward or downward in O(log N), and expiry removes roots in stable key order.
The two-level wheel has eight fine buckets and eight coarse buckets, each with
intrusive bidirectional links. A delay within the current fine interval goes into
`due mod8`; other timers enter `(due/8) mod8`. Every eighth processed tick revisits
the corresponding coarse bucket and cascades only nodes within its fine interval.
Full absolute due times are retained: a bucket coincidence is not an expiry proof.

The wheel admits timers while its consumer is behind the observed clock. Such a
timer may be more than one coarse revolution ahead of the processed clock, even
though its admission delay is at most63. An early coarse-bucket visit leaves it
linked until its actual interval. The exact tick checks plus absolute identities
prevent early firing. Fine-bucket expiries are ordered by admission sequence before
publication. This adds O(k²) insertion ordering in the bounded implementation;
it is not a constant-time output claim. The cancellation path remains O(1).
Tick catch-up costs the number of missed ticks plus visited/cascaded/output work.

All three runtime-selected alternatives use `struct rzt_timers`, which reserves fields
for all representations. Therefore their actual `sizeof` is identical. Report
that measured fact; do not claim a smaller specialized list object was measured.
The diagnostic work build counts explicit ordering comparisons, index/link field
writes, processed wheel ticks, coarse visits and largest returned expiry set.
It excludes scalar metadata, output writes, initialization and loop comparisons
that do not compare timer keys. `RZT_NO_WORK` removes these counters for timing.

## Suspended processing and the retained scheduler

In the native delayed test, a producer admits eight timers at observed100 while
tick consumption remains at0, alongside eight original timers due1..8. A mutex
serializes admission and catch-up. Advance to100 returns original deadlines1..8
with lateness99..92; advance108 returns the new deadlines101..108. No deadline is
silently rebased to the resume time. An unrelated thread's wall clock is not read
without a stated epoch/rate contract. This is a local virtual-clock experiment,
not an interrupt latency or board-timer measurement.

The inspected PR-T50-05 project requires a stable queue and list-to-heap transfer;
the retained `cmx_tick` implementation actually scans sixteen task slots. Preserve
that accepted code and its historical evidence. The actual import comparison
checks matching due/cancelled task sets at every tick, including a uint32 wrap
within the documented half-range model. CMX exposes ready task states, not this
adapter's stable ordered expiry records or generation-checked timer handles.
Those stronger properties are compared only between the three new alternatives.
Reopen the design question in a new dated decision record; do not rewrite the
original sealed learner decision or claim an absent comparator already existed.

## Deliverables, expected results and assessment

Submit D1 all list/heap/link/generation invariants and a scope comparison with CMX;
D2 the three implementations, caller synchronization and explicit import; D3 raw
oracle/control results; and D4 the suspended-clock variant and 13-dimension cost
dossier. At time0, due9 must not fire at1, fires at9, and an old generation cannot
cancel a replacement occupying the same slot. Sixteen equal-deadline records
must be returned in admission order even after odd slots are cancelled/reused.

The sorted Python model checks 58,821 action histories over three implementations,
including paused admission, stale cancel and one/100-tick advance. It separately
checks 13,056 pairs: all due1..16, all cancellation points0..16 and all three
representations. The public C tests cover every full-bucket unlink position,
maximum generation/serial, timestamp overflow, catch-up refusal preservation,
concentrated expiry, native admission during suspension and the real CMX import.
Control31 omits coarse cascade; control32 admits an old generation. Both must fail
at the intended `RZ_CHECK:` assertion, not merely fail compilation or time out.

Before timing, record the expected insertion/cancellation/expiry tradeoff in a
new author/learner decision. Compare empty ticks, sixteen equal expiries, scattered
expiries and half-cancelled timers using the same sixteen-slot capacity, stable
ordering, identity and horizon. Verify equal checksums first. Separate counter
runs from unsanitized timing. Record raw repeated timings and per-advance elapsed
samples, generated code, static stack usage and whole-program size. Per-call timer
observations include clock-call overhead and host scheduling noise; they are not
physical interrupt jitter or a worst-case execution-time proof. The largest observed
sample is a sample maximum, not a hard bound.

Level1 only sorts nominal timers. Level2 must preserve stable identity/order across
all mutations, explain full-time checks and late admission, and meet exact traces
and intended mutant rejection. Level3 independently handles the hundred-tick
suspension and justifies a changed horizon/workload choice from measured evidence.
Hints: retain full due times; next make removal repair every neighbor/index;
finally distinguish admission time from processed time. Author execution is not
an independent delayed learner attempt, a physical measurement or a proof of every
possible schedule. All demonstrations remain within original local teaching code.
