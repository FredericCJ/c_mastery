# Systems investigation contracts, revision1

These nine original C17 components deepen remaining B04/B05 domains and connect
to accepted architecture, communication, debugging, OS and execution artifacts.
They do not replace those artifacts. Linux x86-64 GCC/Clang host execution uses
profile C mechanism code (type/limit headers, caller storage, no mechanism
libraries) and profile A verification tools. Exact uint32_t/uint16_t and 8-bit
octets are required. No target execution or strict freestanding link is claimed.

Native objects are live, correctly typed, sufficiently sized, initialized and
mutually disjoint. No concurrent mutation/reentry. Numeric validity does not
establish native pointer provenance/lifetime. Inputs and output/control storage
do not overlap. Nonzero status preserves controls and outputs; callback effects
are outside rollback. S_PENDING is an unchanged observation. Modeled I2C loss or
NACK is a successful observation that enters a terminal state, not an API error.
Names and status values in systems.h are canonical. Compiling UNIMPLEMENTED
stubs intentionally fail semantic checks. ISO C supplies arithmetic/object
semantics; runtime checks and caller discipline enforce model contracts.
Changing to C23 does not supply native device ordering or lifetime protection.

## Pipeline schedule

0..16 records are an already-selected RETIRED instruction stream, not raw code
with wrong-path instructions. Opcode ALU/LOAD/BRANCH, registers0..7, register0
constant, sources left/right; branch destination must0 and taken is0/1. Nonbranch
taken must0. Initial registers ready. Five stages IF,ID,EX,MEM,WB are one cycle
each. issue[i] denotes IF cycle, beginning1. Issue is in-order at most one/cycle;
stalls are inserted before IF for this equivalent scheduling model. Full
forwarding permits ALU dependency distance1 and load-use distance2. Without
forwarding, WB-write-before-ID-read permits distance3. Taken branch adds two
flush bubbles before next retired instruction. No structural/cache/exception
hazards; no multi-issue/out-of-order claim. cycles is final WB=lastIF+4, or0 empty.
Unfilled issue slots remain unchanged. O(n), eight readiness entries plus16
transactional output entries. Derive the stage diagram, not merely the recurrence.

## Sampled I2C transmitter byte

Initialize SI2c {byte0..255,bit0,DATA}. drive returns whether to release SDA
(one), never actively drive high; data is MSB first. Each sample represents
one clock observation, with scl/sda in0..1. Low SCL yields PENDING unchanged,
so stretching cannot advance a bit. High SCL with release1 and observed0 during
DATA loses arbitration, enters LOST and releases SDA henceforth. Drive-low yet
observe-high is inconsistent and rejected. After eight data bits enter ACK,
release SDA and sample the ninth clock: low=>DONE, high=>NACK. Terminal samples
are rejected; drive in terminal states releases SDA. These are O(1) steps.

This primitive assumes valid START/transaction context, stable samples and a
controller-transmitter role. It does not implement addresses, START/STOP,
repeated START, read termination, analog timing, retries or bus clear. Preserve
the earlier transaction state machine when composing. A losing controller has
no right to inject STOP merely because its own byte stopped. Logic samples do
not establish pull-up/rise-time/electrical compliance. The inspected UM10204
Standard/Fast-mode passages, not a register datasheet, support these distinctions.

## Streaming one's-complement checksum

Zero-init SChecksum. add consumes borrowed octets, arbitrary count including0,
null input allowed only for0. sum is a folded16-bit sum; pending records a
high-order octet awaiting its low-order partner. Never pad at a chunk boundary.
finish is nonmutating, pads only a final odd byte with low zero and returns the
16-bit complement. Repeated finish is stable and later add remains possible.
Empty checksum is0xffff; all-ones sum complements to0. This is a numeric checksum
value; wire serialization remains explicit. O(n) add, O(1) finish/storage. It is
error detection, not authentication, and does not parse IPv4/TCP fields itself.
Compare scatter/gather chunks against the accepted contiguous packet checksum.

## Paired sample summary

Input arrays contain1..32 paired uint32 durations in declared identical units.
Compute signed64 difference B-A before sorting; output lower median at sorted
index floor((n-1)/2), and number of pairs where B<A (ties are not wins). Inputs
remain unchanged. No average, ratio-of-medians or median-of-unpaired samples may
substitute. O(n^2) elementary sorting and O(n) storage suffice. Sampling order,
warmup, consumed output and clock contracts are caller experiment obligations.
Synthetic samples test arithmetic; actual observations must retain raw values
and environment. Neither median nor sampled maximum is WCET or causal evidence
that code alone caused a timing change.

## Failure-preserving token reduction

Input0..32 tokens, output32 bytes, length/calls outputs and deterministic callback
classify(ctx,bytes,count) returning PASS, same-failure FAIL or UNRESOLVED.
Callback borrows storage only for its call and must not mutate/reenter/retain it.
It may log its own observations; these are not rolled back. Initial input must
be FAIL, otherwise CONFLICT. Budget bounds actual callback calls; exhaustion is
RANGE with all caller outputs unchanged. Invalid callback outcomes after initial
classification yield INVALID. Preserve order, remove complement chunks, refine
partition until no single deletion preserves FAIL, including the final empty
trial for one token. UNRESOLVED never authorizes reduction. On success output
the failing subsequence, final length and exact calls used. Bytes after final
length are unchanged. No guarantee of globally shortest or unique result;
1-minimality is relative to deterministic replay and this deletion grammar.
If a final single-deletion trial is UNRESOLVED, return PENDING unchanged rather
than certifying 1-minimality from an inconclusive observation.
Worst-case O(n^2) callback trials and O(n^3) copied token work under n<=32; derive
a tighter bound for the selected strategy. Fixed local arrays, no allocation.

## Replacement policy comparison

0..32 page IDs are octets (values, not addresses);1..8 initially empty frames.
Implement FIFO, LRU and offline OPTIMAL. Fill lowest empty slot; hits never move
FIFO hand; LRU updates recency on every access; OPTIMAL evicts a page whose next
use is farthest in the future, with never-used-again distance n and lowest-slot
tie break. Output fault count and evicted[i] (-1 on hit/empty-frame fill), zero
unused result entries. FIFO hand advances after misses; OPTIMAL need not expose
its hand. O(n*frames) FIFO/LRU, O(n^2*frames) straightforward lookahead, bounded
storage. Compare to accepted CLOCK through an explicit trace adapter. Offline
optimality uses future input, unavailable to a real online OS; it is an oracle,
not a deployable paging prediction claim.

## Atomic wait enrollment model

Zero-init SWait. Actor1 consumer,2 producer. LOCK requires no owner; UNLOCK
requires actor ownership. WAIT requires consumer ownership and no existing
enrollment. If predicate true, mark ready and retain lock; otherwise atomically
enroll waiting, clear ready and release lock in one modeled transition. Producer
PUBLISH sets predicate under its lock. SIGNAL under producer lock consumes one
enrollment if present and marks ready; it may occur with false predicate
(spurious wake). Ready is eligibility, never permission to skip predicate recheck.
O(1) fixed storage. Only one waiter and one producer, serialized transitions;
real scheduler/interrupt/atomic implementation remains the accepted CMX adapter.

## TLB revocation acknowledgement model

Zero-init SShootdown. begin requires inactive and nonexhausted generation,
increments generation, snapshots targets bitmask CPUs0..7, marks active and
pending=targets. The caller has already unpublished the mapping and prevented
untracked new users. ACK requires exact active generation and a currently pending
CPU bit, removes it, and represents trusted completed local invalidation. Reclaim
requires active and pending0; otherwise PENDING unchanged. Completion marks
reclaimed and inactive. Empty target set is legal. Counter exhaustion fails,
never wraps. O(1), fixed storage. Offline CPUs cannot be silently removed from a
pending mask; an explicit quiescence protocol is required. Native ISA shootdown,
IPI delivery, memory ordering and page-table locks are separate obligations.

## Peer restart and uncertain outcome

Initialize SPeer {epoch1,sequence0,outstanding0,uncertain0,ready1}. One outstanding
request. submit requires ready/no outstanding and nonexhausted sequence, increments
sequence and publishes distinct epoch/sequence outputs. ACK requires exact current
epoch+sequence and outstanding, then clears outstanding. restart with no trusted
quiescence returns PENDING unchanged. With trusted quiescence and nonexhausted
epoch it captures uncertain=outstanding, drops retained work only after that
boundary, clears ready/outstanding, resets sequence and increments epoch. ready
requires exact epoch and currently not ready. An old ACK after sequence reuse
must fail. The uncertain flag records the most recent restart outcome and is
not cleared by ACK; an application must inspect/report it before the next restart.
O(1), fixed storage. Trusted quiescence/boot identity comes from the supervisor,
not from an untrusted peer claiming reset. This model issues no hardware reset.

## Assessment and integration

Per-exercise briefs require arguments, mutations, independent expected traces,
changed-condition variants and delayed assessment. The same-author oracle is
methodologically separate, not independent human review. Borrowed inputs and
control representation are explicit, but ISO C does not enforce linear usage,
atomic transitions or device facts. Publish only learner scaffolds, never the
separate assessor implementation. Preserve accepted baseline hours/identities.
