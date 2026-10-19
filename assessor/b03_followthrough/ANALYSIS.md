# Exact B03 author arguments

The original root/rubric/delayed clauses remain authoritative. This companion
discharges author implementation and selected validation, not independent review,
learner retrieval, published C17 access, actual OS integration or physical timing.

## T23: prefix, fairness and request identity

The base corpus enumerates all callback scripts through length6 over five outcomes,
length0..5, budgets1..3. After EVERY turn an independent state interpreter checks
return, position, terminal, exact callback offsets/count and actual copied bytes.
It also checks terminal idempotence, quota0/65 and oversized positive response.
The test backend's -1 and -2 have synthetic meanings; read EOF, errno, datagram
atomicity, durability and direct-I/O error effects remain separate parent topics.

Round-robin changes next before invoking the bounded pump; therefore invocation
k+1 selects the other writer regardless of k's result. This bounds *turns*, not
wall time: synchronous callback nontermination invalidates the scheduling premise.
The variant corpus tests both starting owners, quota1..64, four hot outcomes and
cold lengths0..5, plus quota0. A ready cold writer is selected at least every second
call and gains min(quota,remaining) bytes. Terminal hot turns do not alter the bound.

Lost reply witness: request[2,3,5] applies2 then times out. Blind replay applies2
twice; new IDs would also duplicate the effect. The service retains ID+full request
and done with each effect, so the same request resumes at3 and5; a lost final reply
returns the saved10 on retry with effects still3. Full payload comparison prevents
same-ID/different-command reuse. Latest-only dedup bounds history and rejects stale
IDs; this is not a general durable exactly-once network service. A crash between
effect and progress would break the argument unless their atomicity were durable.

## T24: quiescence owns reclamation

Base event words through length6, current generation1..3 and event generation0..4
check every prefix against an independent transition interpreter. Starts while
owned, ID exhaustion and stale callbacks after reuse are checked separately.

The changed table adds REQUESTED and QUEUED: timeout requests; rejection returns
ACTIVE; queued ACK retains resources. COMPLETE or final QUIESCENT alone retires.
Example: start1, timeout, queued ACK, backend write, final quiescence. Reclaiming
at ACK would make the backend write invalid; the source mutant is detected before
any invalid lifetime access is executed. Completion then quiescence and the reverse
each report once. Repeated timeouts do not erase QUEUED. All5-event words through
length6 are checked by a separate Python transition table, including stale/future
IDs. Hashes summarize every prefix field; this is a finite corpus, not an unbounded
proof or actual OS cancellation run.

## T25: two different ownership edges

SPSC producer payload write W -> release full; consumer acquire reads that full
modification -> payload read R. This synchronizes W before R. For reuse, R ->
consumer release empty; next producer acquire reads that empty -> next write W'.
Thus R precedes W'. Initial empty is safe from pre-thread initialization; later
empty observations require the reverse handoff. Release/acquire alone without
the matching reads-from relationship is insufficient. Relaxing either necessary
edge can leave conflicting ordinary payload operations unordered.

Two-producer failure in the original representation: P observes empty; Q observes
empty; P writes1; Q writes2; both publish full. The writes conflict without an
ownership edge, and even an imagined serialized execution loses a message. We
do not run this data race natively. GenMC analyzes this bounded original source
under RC11; positive claims remain limited to its exact model/bounds/version.

Claim protocol: only CAS EMPTY->WRITING succeeds. Its read-modify-write establishes
one owner even if two producers observe empty. Publication/reuse edges remain
required after this ownership fix. Consumer claims FULL->READING before its read,
then releases EMPTY; producer acquire observes that release before overwriting.
Failed CAS performs no payload access. A stalled claimant blocks progress;
successful native stress and lock-free atomic reports do not prove fairness or a
lock-free algorithm. The native corpus joins all threads, checks monotonic SPSC
messages and unique per-producer order across two producers. Equal-work SC and
acquire/release profiles preserve the contract; object disassembly and undefined
symbols are retained, without inferring hardware universality or speed from them.

## T28: command bits and observations

For W1C, ack bits are a set difference. Wrong read-modify-write at status3/mask1
writes3 and clears unselected bit1. Correct write1 leaves2. Clear-on-read polling
at status10 returns10 once then0: the first observer consumed the evidence.

For W0C only zero command bits clear. Starting with all8 bits1 and clearing selected
low bits produces command255 XOR mask. For status S and selection M, S AND command
equals S minus M. Upper reserved bits stay1, with no status read. Independent set
enumeration covers all status0..15, mask0..31, pre/post event sets0..15. An unselected
event survives; a selected event before the write may be cleared, and repeated
already-set events coalesce. No C struct layout or host access count proves a
physical bus width, device ordering, read side effect or board behavior.

## T30: representation, coherent snapshot and history

The base corpus exhausts all256 starts,256 elapsed representations,256 delays:
4,194,304 admitted;4,194,304 ambiguous;8,388,608 invalid. Every rejected output
is unchanged. Unwrapped elapsed is the oracle. Samples start250,now4 are consistent
with elapsed10 or266; no predicate on these samples can distinguish those histories.

Change to0.5ms/16-bit ticks:127ms becomes254 ticks; original conservative elapsed
<128ms becomes<256 ticks. Both remain below32768. ceil(ms*den/num) satisfies
ticks*num>=ms*den and, if ticks>0, (ticks-1)*num<ms*den. The independent inequalities
are checked over81,081 rational conversions; product overflow is rejected before
mutation. Physical drift, quantization phase and scheduling latency need separately
stated bounds; ideal rational rounding alone supplies none.

Torn read witness: high0, counter advances255->256, low0 would produce0 using the
old high. High-low-high detects0!=1 and retries. Equality establishes no high change
only under the no-high-wrap premise; the accepted value equals the counter at the
low read. The corpus enumerates all65,536 initial counters and64 schedules of zero
or one increment after each of six read slots (4,194,304 cases), plus perpetual
instability, malformed halves, zero budget and unchanged-output failure. A whole
65536-tick advance can alias high equality. A software counter that stores low0
before high1 can also return an impossible intermediate0: that update protocol
violates our atomic-whole-counter premise and needs masking or a correct sequence
scheme. Even a coherent value cannot establish elapsed<half-range or detect missed
whole wraps. These are model observations, not physical interrupt experiments.
