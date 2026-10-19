# P3 completion analysis

The encoder validates total output capacity before touching destination or length.
Wire fields are accessed as bytes, so neither input alignment nor host byte order
enters the protocol representation. The CRC primitive checks the independent known
vector; the encoder's extent is separately tested so a mutually wrong codec pair
cannot satisfy the exact fixture or Python zlib oracle.

The parser separates retained candidate bytes from a pending copied message. A bad
candidate removes only its sync, permitting another valid frame inside its already
received bytes to become the next candidate. Removing the whole claimed length
would lose sequence7 in the nested fixture. The fixed42byte buffer always contains
either an incomplete candidate or bytes about to be rejected/consumed before append.
No payload length controls an append until its six-byte header has been validated.

Queue refusal is not parser consumption. The queue owns a copied message before
`ready` is cleared. Clearing on CP_FULL loses a frame; failing to clear on success
duplicates it. The design is serialized and has no fallible publication callback;
neither property may be transferred silently to a concurrent or durable adapter.

Four original mutations omit version from CRC, swap sequence encoding, skip a whole
bad candidate, and erase pending state on refusal. Tests require CP_CHECK semantic
failure rather than a signal, assertion-free exit or compilation failure. The public
stream driver bounds input at256 bytes and enforces a finite operation budget.
Python's full-buffer scanner and zlib produce expected frames without calling C.

The original AX rubrics remain binding. Test success does not independently defend
the caller's object lifetime, source/destination disjointness, transfer architecture,
research analysis or delayed attempt. Current C17/C23 source availability and physical
P4/capstone validation remain separately recorded external obligations.
# P1 and P6 assessor extensions

P1 separates byte reservation from typed-object ownership. The arena never
manufactures a typed object by casting declared character storage. The pool's
actual declared records carry generations and pins; checked lookup must validate
before producing a pointer. Release/reset while a pin remains is a lifetime
violation even if the FIFO has already removed the handle. The small generation
limit makes retirement, rather than eventual wraparound, observable. Atomic
batch staging copies all determinate fields and incurs a fixed stack cost; prefix
admission instead records precisely the committed ownership transfer. Neither
policy can claim to roll back an external callback by copying local memory.

P6 exact accumulation first widens every int32 term. The numerator bound is four
times the input extrema, inside int64. A denominator4 representation postpones
rounding rather than silently implementing integer division. Rounding uses signed
quotient/remainder and tests even quotient parity, including negative ties; never
take abs(INT64_MIN). Checked int32 conversion follows rounding. Half-pole IIR
state is an independent mechanism: its impulse coefficients form a convergent
geometric sequence. An unstable pole such as1.25 grows even at zero input once
state is nonzero. This counterexample refutes transferring the FIR's finite-memory
reasoning to all recursive filters.

For binary64, 2^53+1 rounds to2^53; subtraction then yields0, while cancellation
first yields1. The FIR sequence[-4*2^53,2*2^53,4] reproduces this distinction in
its third output. A bitwise reproducible audit contract must retain the chosen
addition order and prohibit FMA/reassociation; an approximate telemetry contract
needs a separately justified error bound, especially near zero where relative
error alone is unusable. Passing dyadic fixtures does not justify all real inputs.

Transactional numerical failure has two different expected outcomes. Before
sample k publishes, prefix mode owns exactly k-1 outputs and corresponding
history; retrying from zero would double-process accepted input. Atomic mode
owns none. The two injected mutants either publish a failed atomic prefix or
report one uncommitted sample as accepted. Whole-input preflight is separate:
invalid later data must preserve all state even in prefix mode under this API.

The vector wrapper stages input into disjoint local storage before any output,
supporting overlap in either direction. This is a deliberate public-contract
change from the portable scalar overlap-refusing API. Internal restrict applies
only to the separate staged input and output; no false alignment promise is used.
Four-wide AVX operations and scalar tails evaluate the same operation tree.
All lengths0..256 and offsets across32 bytes are tested, including cancellation
and chunk histories. Synthetic CPUID/XCR0 probes independently remove required
bits and count XGETBV calls. The host probe must never issue XGETBV when OSXSAVE
is absent. Compiling AVX successfully says nothing about OS state support.

Measurement compares the same staged API and includes its checks/copies. Actual
disassembly must retain scalar, auto-vector and AVX arithmetic and work-consuming
calls under LTO. The first Clang experiment showed packed SLP operations in a
purported scalar baseline; retaining that failure and disabling the separate SLP
transformation is stronger evidence than relabeling it scalar. Raw paired samples
and empty-harness overhead reveal noise and workload costs; do not infer a speedup
from one best sample. Cache perturbation is an operational procedure, not measured
proof of cache residency. The separate report now records actual QEMU Nehalem
user-emulation with AVX unavailable; it does not establish physical no-AVX host
behavior. Delayed learner work remains unperformed.

## Linux composition and cancellation assessment

The exact nonblocking admission trace accepts1,2,3,5 and rejects4,6 because
admission is evaluated against current queue occupancy, not the total slot count.
An item removed by a worker still consumes a pool slot. Count queued, worker-owned,
completed and discarded items separately; their conservation relation is meaningful
only for successful admission, not every attempted offer. The work copy belongs
to the worker, while its generation handle identifies the reservation to release.
Reusing a slot changes the generation before a new completion can be accepted.

A notification does not represent an owned item. Signal-before-wait succeeds
because the predicate is already true; a spurious wakeup returns to the loop.
Two consumers may wake for one item, but only the mutex owner observing nonempty
state may remove it. Closing wakes a blocked producer, which returns CLOSED even
if an abort simultaneously frees space. The distinction is admission permission,
not merely free capacity. A deadline provides a decision point; it cannot prove
worker quiescence or authorize destroying a live mutex/context.

The real epoll trace reads three frames into the adapter, admits two and retains
the third. At that point the kernel pipe is empty, so neither LT nor ET owes a
new readiness event when downstream capacity later opens. Explicit rescheduling
of retained work is necessary. This is more precise than the rule of thumb to
read until EAGAIN: the state includes kernel bytes, input-buffer suffix, parser
candidate and complete pending message. Each has a different owner. On descriptor
reuse, reject the old token before calling read; querying whether an fd number is
currently valid cannot establish that it still names the old connection.

The64-frame two-worker trace permits completion reordering but rejects duplication
or loss with an independent seen-array. Its joins establish test-thread quiescence.
This finite trace and TSan results do not prove all interleavings, fairness or
unbounded liveness. Reporter failure does not requeue already processed work.

The asynchronous large-payload variant retains allocation and context after
cancel and after quiescent=false. Matching completion changes ACTIVE/CANCELLING
to REPORTING under the mutex before any callback. Concurrent duplicates therefore
see a non-completable state. The callback executes outside the mutex, so attempted
reentry returns BUSY/STALE rather than deadlocking. It cannot start a new generation
until release and callback return are complete. A failed reporter still releases
exactly once; the report failure remains available for diagnosis. An implementation
that frees on cancel or accepts an earlier generation fails explicit semantic
checks without needing to manufacture a use-after-free crash.

## Acquisition, image and timing assessment

For the N=4 ring, head/tail distinguish empty from full by reserving one slot.
The payload-before-head and read-before-tail edges protect ordinary sample storage;
the index atomics alone are insufficient if these edges are removed. Under two
producers, both can read head0/tail0, write slot0 and store head1. One sample is
lost even with sequentially consistent index operations. This is a serialized
counterexample to the wrong argument, not an invitation to execute an ISO C data
race and infer a portable outcome. A correct multi-producer variant must change
the reservation/publication protocol and its progress argument.

The16-bit calculation is (6-65530) mod65536 =12, but a full-wrap interval would
have the same result. Rejecting differences>=32768 resolves only the declared
bounded-ordering convention. It does not discover missing wraps. For a64-bit
snapshot on a32-bit core, the supplied cut exposes new low0 with old high0 during
rollover from0xffffffff to0x100000000. The odd/even sequence rejects this torn
combination. All words are atomic and sequentially consistent in this deliberately
conservative implementation. A reader preempting an odd writer must return
uncertain after its bounded attempts; spinning forever in a higher-priority ISR
would prevent the writer from finishing. Sequence exhaustion is explicit refusal.

TXE only admits another data-register write. DMA transfer completion only ends
the DMA memory transaction. Neither implies that the last stop bit has left the
UART. The last polling write must not consume a stale TC read taken before it;
the WIRE state requires a later observation. Timeout/disable requested while
DMA owns the buffer keeps the buffer live. EN observed low and UART disabled
permit failed-operation reclamation; failed acknowledgement does not. A late
old-generation event cannot publish after reinitialization. The bounded fault
record preserves the first cause and the retry ceiling prevents an infinite
recovery loop. Half-duplex physical turnaround remains a separate board claim.

The actual target runner poisons both data VMA and BSS before reset. Omitting
either initialization step is rejected by a semantic guest failure. A linker
map alone cannot show that reset performed a copy. Both good RAM placements
execute; moving the F439 DMA object into CCM is rejected from the resulting ELF.
The explicit byte-clear helper is required by an observed Clang build, not
assumed absent because the source never spells memset. No final undefined
symbols or resolved forbidden runtime archives are accepted. These tests support
original reset/PendSV behavior on MPS2, not STM32 USART registers, bus timing,
physical acquisition or fault injection. Reference physical measurements remain
unavailable until a dedicated board campaign is actually performed.

For analytical timing, distinguish admitted-work response time from loss. Under
a declared uniprocessor fixed-priority model, calculate each accepted job's own
service and blocking plus interfering releases within the candidate response
window; iterate until a fixed point or deadline violation, stating release jitter
and priority assumptions. A measured maximum is not that bound. Lowering capacity
can decrease measured latency by dropping precisely the most delayed jobs, so
report offered, admitted, completed and dropped counts with the distribution.
No synthetic tick fixture in this family is a measured nanosecond sample.

## Resource measurement scope

The arena's two successful requests use14 payload bytes, six header bytes and
five alignment bytes, leaving seven tail bytes that cannot satisfy the next
header/alignment/payload request. The report derives these values from actual
reserve operations. The typed pool has no variable-size external fragmentation,
but fixed slot size, metadata and retirement consume capacity. Heap-backing the
pool changes its owner and deallocator requirements without replacing the typed
record declaration. It does not test arbitrary effective-type construction from
raw storage; that named transfer remains separate.

The host measurement holds2000 complete initialize/admit/drain cycles per sample,
five discarded warmups and31 recorded repetitions. Instrumented and uninstrumented
modes preserve admission/output checks; only additional live-slot scans differ.
The failure workload injects ordinal3 and owns exactly two records. malloc/free
backing allocation is outside the timed interval and is separately declared as
a hosted dependency. Medians and spread describe this process/environment, not
a hard latency bound, allocator-wide fragmentation or a universal performance
ranking. Revisit the model if record sizes, capacity, helper generation or
callback ownership changes.

