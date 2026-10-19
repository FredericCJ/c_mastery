# Checkpoint composition completion contract, version 1

## P4 acquisition, target startup and completion boundaries

`CPFRing` has four declared sample objects and a selected physical size2..4;
one slot remains unused, so usable capacity is size-1. Exactly one producer
owns head and exactly one consumer owns tail. The producer writes a complete
sample before its release store; the consumer acquires head before reading and
releases tail only after copying. The producer acquires tail before reuse.
Initialization requires lock-free unsigned atomics and occurs before interrupts
or threads can access the ring. No ISR allocates, blocks or calls host facilities.
Drop-newest leaves existing FIFO items unchanged; the producer-only drop count
saturates at UINT_MAX. Read that diagnostic after producer quiescence. Two
producers violate the reservation argument even if each index access is atomic:
both can observe the same head and overwrite one sample. This API does not claim
multi-producer safety. A second producer needs serialized admission or a separately
proved reservation/publication design.

Samples100..104 at capacity3 yield100,101,102 and two drops. Repeat at capacities1
and2 (physical sizes2 and3) and across100 index wraps. Each sample carries a32-bit
sequence,16-bit value and16-bit acquisition timestamp. `cpf_frame` uses the low
16 sequence bits in the P3 header and all32 bits in its eight-byte payload,
followed by little-endian value and timestamp. Timestamp acquisition belongs in
the producer; a DMA-completion timestamp answers a different question.

`CPFTx` owns its encoded wire array at a stable address. Begin copies a message
only from IDLE. Polling accepts one byte per TXE observation. The last write
enters WIRE and cannot consume an earlier TC observation from that same call.
DMA completion requires a disabled stream and zero remaining count; it releases
the memory transaction but this API deliberately retains the array until USART TC.
TC is required before a half-duplex direction change. No function toggles a real
transceiver pin or claims its electrical turnaround has been measured.

Each completion identifies the current generation. A stale completion cannot
publish or release. A timeout records the first generation/cause and enters
ABORT; requesting disable is not acknowledgement. Recovery retains the buffer
while either the DMA stream or UART remains enabled. Only confirmed quiescence
permits release and a new generation. Failed acknowledgement leaves the operation
pending and retained, including after a deadline. Generations retire at the
declared limit; recovery also retires at the retry limit. A recovery discards a
partial frame and reports failure; it never silently replays accepted bytes.

`CPFClock` supplies a deliberately conservative multiword snapshot: one writer
uses sequentially consistent32-bit atomics for sequence, low and high words.
Odd sequence means an update is in progress; a reader accepts only equal even
sequence values bracketing the parts. Attempts are bounded and leave output
unchanged on uncertainty, including when the reader preempts the writer. The
writer refuses before sequence wrap, preventing an ABA assumption. This is a
specific ISO C atomic protocol; unprotected ordinary concurrent hi/lo accesses
would create a data race. `cpf_elapsed16` accepts modular differences below32768;
the caller must additionally establish that the real interval is less than the
half range. The returned12 for65530→6 does not prove that no complete wrap elapsed.

The target producer `tools/run_checkpoint_firmware.py` uses original assembly
reset/vector code, audits data/BSS/VMA/LMA/stack and explicit runtime providers,
and poisons RAM before actual MPS2-AN386 Cortex-M4 execution. PendSV is explicitly
assigned priority255; it is not assumed low at reset. Tests execute in Handler
mode and publish samples through the same ring/frame core. Clang's observed
`__aeabi_memclr4` dependency has a separately pinned original byte-loop provider
with length0..31 guard tests. GCC/Clang, two RAM placements and omitted-copy/
omitted-zeroing mutants prevent initial emulator contents from standing in for
startup work. No libc, libatomic, libgcc or hidden default startup is linked.

The F439 adapter is separately cross-built at two AHB-SRAM placements. It configures
HSI/APB clocks, PA2 AF7 USART2, polling then DMA1 stream6/channel4, and explicit
TC/quiescence handling. A CCM relocation is rejected. This is inspected source
and image evidence, not executed STM32 peripheral evidence. The detailed board
boundary is in `targets/README.md`. Physical acquisition, latency distributions,
bus contention, transceiver turnaround and failure injection remain required
external observations; QEMU wall time must not be reported as board latency.

## P5 copied workers, Linux readiness and asynchronous lifetime

`CPAQueue` owns four inline message slots, a FIFO of two indices and a mutex/
monotonic condition variable. Each slot is free, queued, worker-owned or retired.
Offer copies the message; take copies it again and returns a slot/generation
handle. A worker may use its copy independently, but must finish exactly once
to release the reserved slot. A stale or duplicate finish changes nothing.
Generation exhaustion retires that slot. Blocking calls wait in predicate loops
under the same mutex; no waiting producer holds the consumer's mutex across
sleep. All control objects remain at stable addresses. Pthread cancellation and
robust mutexes are outside this adapter; shutdown is cooperative.

Close transitions OPEN→CLOSING→DRAINING and refuses later offers. Abort discards
queued slots exactly once but retains worker-owned slots. STOPPED requires zero
queued and zero owned slots. Deadline expiry returns PENDING without inventing
quiescence. Destruction additionally requires no waiters and an external guarantee
that no thread can enter again. Join is an owner obligation, not implied by a
counter reaching zero. Diagnostic counters are exact only while their total
increments remain representable in size_t; the finite exercises establish that
bound. This is a bounded reference contract, not an indefinitely accumulating
production monitoring system.

`CPLReader` is a separately serialized Linux adapter owning one nonblocking fd
after successful epoll registration. Its epoll token carries slot and generation,
never a dereferenced heap pointer. Token validation precedes read, even after the
same fd number is reused. Registration failure leaves fd ownership with the
caller. Close invalidates the token, deregisters and closes once; it preserves
separate deregistration and close error fields and never retries close on a
possibly reused integer. The reader owns a64-byte input buffer and a P3 parser;
queue refusal preserves the parser-held frame and unread input cursor. After a
worker frees space, the application explicitly resumes this reader even without
another edge. A turn has at most32 parser/read actions; PROGRESS requires prompt
rescheduling, BLOCKED requires capacity notification, DRAINED permits waiting for
readiness. EOF with a partial candidate returns TRUNCATED. Explicit close may
abandon a pending/incomplete frame; the service owner must account for that loss.

Tests use anonymous local pipes, actual epoll LT/ET, copied64-frame delivery to
two pthread workers, descriptor close/reuse, partial EOF and generation retirement.
Worker completion order may differ from input order, but each accepted sequence
appears exactly once. `dup2` atomically installs the test's owned descriptor;
close-plus-dup is not a concurrent replacement algorithm. No network listener,
third-party service or external data is involved.

`CPOperation` is the changed large-payload/asynchronous-completion adapter. It owns
a copied1..1024-byte allocation, generation and callback context. Allocation
failure leaves the caller's generation output and operation unchanged. Cancel
requests completion but does not release. A completion with quiescent=false keeps
the bytes/context alive; only matching quiescent=true enters REPORTING, calls the
reporter once and releases once. Reporter failure is retained separately from
lifetime success. Callbacks execute outside the mutex, while REPORTING rejects
reentrant begin/cancel/complete. Allocator/release callbacks must be nonreentrant;
none may retain the borrowed report bytes. A new operation starts only after the
report/release phase ends; stale earlier generations cannot complete it. Generation
exhaustion retires the operation. Destruction requires external caller quiescence.
This explicit protocol supplies checks that ISO C does not enforce through types.

## P2 transfer ownership and actual custom-entry images

`CPWTransfer` borrows a live immutable byte extent until completion or an explicit
owner decision after failure. It stores length, accepted prefix and a bounded
error record. Every `cpw_step` invokes at most one writer callback, with precisely
the unaccepted suffix. Positive normalized progress advances by that count only;
zero, EINTR and EAGAIN return pending without advancing. A terminal cause stops
future callbacks until explicit recovery. Recovery retains accepted bytes and
the first failure cause; it never restarts at offset0. Impossible callback counts
or inconsistent tagged results fail with protocol cause4096. The callback is
nonreentrant and does not retain the borrowed extent after returning. Transfer
controls, error records and callback context do not overlap data. No function
reports errors by recursively invoking the writer.

The normalized result is a C tag/count/cause product. Raw Linux negative values,
libc -1 plus captured errno and the scripted -2 retry convention have separate
adapters. Their numbers are not interchangeable. The declared Linux retry values
are EINTR4/EAGAIN11; another OS must supply another mapping. Error records retain
operation ID, accepted count, first primary cause and first cleanup cause. Later
cleanup errors do not replace either original cause. A C enum does not enforce
exhaustive matches, valid variant fields, cleanup or language-specific Result
layout. Callback and branch costs remain visible.

`CPWChannel` actually composes the P1 typed pool and FIFO. Values0..255 are copied
into pool records. Removing a handle from the queue transfers it to the active
writer state; the live record is released only after its byte is accepted. A
retry or terminal error retains the same active byte while queued records remain
separate. The channel stays at a stable address because its transfer borrows the
channel's own byte. Tests observe65/retry/65accepted/66/error/recover/66accepted,
then independently count the sink, active record and queue. The duplicate mutant
requeues an already accepted byte. A separate nine-byte transfer exercises exact
prefix ownership under3,EINTR,2,4, zero progress, EAGAIN and a permanent failure
after3. One step is one bounded event-loop turn, not a busy wait for readiness.

The target adapter is an original profile-D x86-64 Linux image. Its entry assembly
captures the initial stack, establishes C-call alignment and calls the portable
writer; it terminates through syscall60, with no incoming C return address.
The diagnostic and deliberately invalid-descriptor probe use syscall1. The
process-entry layout, call alignment and syscall register conventions are ABI/OS
contracts, not ISO C. Volatile BSS/data sentinels verify zero-filled versus
file-backed initialization. Actual fixed and PIE images are built with GCC and
Clang at O0/O2 without startup/default libraries. Every final ELF is checked for
entry containment, PT_LOAD extents/permissions, interpreter absence, relocation
requirements and all defined/undefined symbols; maps and disassembly are retained.

Clang O0 actually emitted memcpy/memset for C aggregate operations. The declared
providers in `targets/byte_helpers.c` use original bounded byte loops, with
volatile accesses preventing pattern replacement into recursive helpers. They
are tested in the real image for lengths0..32 and offsets0..3, including return
values and untouched bytes. No libc, compiler runtime or allocator is linked.
A GCC O2 variant removes these optional providers and re-audits the actual
closure; that result does not authorize removing them from Clang O0. A large
aggregate-copy object independently exposes the compiler's memcpy dependency.

The PIE variant deliberately contains no load-time relocations and runs under
the kernel's chosen load bias. A separate original PIE with an absolute data
pointer creates a relocation requirement and is rejected before execution; this
is not a general self-relocating runtime. Other negative images have an entry
outside executable segments or a resolved forbidden malloc symbol despite an
empty undefined-symbol list. They are inspected, not executed as successful
images. `tools/run_checkpoint_entry.py` captures actual write/EBADF/exit traces
of the teaching child using an explicit strace executable. Trace tooling is
host-only; no tracing library enters the target. The target has finite retry
budget16 for its diagnostic, reports an error if exhausted, and never changes
host security settings. A different ABI remains a separately named adapter task.

## P6 numerical state, ISA dispatch and measurement

The portable C17 core supplies two FIR representations of coefficients[1,2,1]/4.
`CPNExact` accepts int32 samples and emits an int64 numerator with an implicit
denominator4. Each widened sum lies within [-8589934592,8589934588], so no signed
overflow occurs. Initial history is zero. `CPNDouble` requires radix2 binary64,
round-to-nearest mode, preserved subnormals and no floating-point contraction or
reassociation. All input/history values must be finite and within +/-1e100.
The result evaluates `(0.25*x + 0.5*previous) + 0.25*older`; cancellation makes
this evaluation order observable. C17 alone does not require binary64 or this
compiler/FP-environment model. The build checks representation constants and
uses `-ffp-contract=off -fno-fast-math`; callers retain the FP-environment
precondition. Zero history, chunk history and owner lifetime are explicit state.

Each call accepts0..256 samples and a live output extent with capacity>=n.
Null input/output is permitted only at n=0. State and plan are live, initialized
and disjoint from data. Exact integer input/output extents are disjoint. The
portable double API permits exact in-place operation and refuses partial overlap
using pointer equality between actual elements, not ordering unrelated pointers.
That bounded overlap check costs O(n^2). Input-domain and capacity failures leave
state and all output elements unchanged. Exact in-place processing saves each
input before writing; successful history stores the final two input values,
including the prior-history case n=1. No allocation, implicit output growth or
I/O occurs in the portable core. Integer numerator outputs are not rounded samples.

The fixed-point variant `cpn_round_even` rounds a numerator/4 to int32 using
nearest/ties-to-even, including negative ties. Out-of-range rounded results return
CPN_CAPACITY and preserve output. It never negates INT64_MIN. The one-pole variant
`cpn_iir_half` uses y[n]=0.5*x[n]+0.5*y[n-1], zero initial state for its impulse
test and the same finite domain. The geometric impulse sum is finite because
abs(0.5)<1; this specific proof does not establish stability of an arbitrary IIR.
It stages the block, accepts arbitrary data overlap and preserves state/output
on bad input/capacity. Its finite implementation uses O(n) time and256 doubles.

`cpn_transaction` exposes both accepted-prefix and atomic failure semantics.
It preflights the entire input/capacity and stages computed outputs. Failure
ordinal1..n means refusal before publishing that sample; zero or >n succeeds.
On failure prefix mode commits exactly ordinal-1 samples/history and returns
that count; atomic mode commits zero. The caller retries only the unaccepted
suffix with its returned state. The committed-count output is disjoint from all
other objects. Preflight refusal preserves it, while a simulated operational
failure writes the accurate count. Staging permits arbitrary data overlap but
requires O(n) temporary storage. All five failure cuts, successful continuation,
ties/extrema and the half-pole impulse across every split are executable checks.

`checkpoint_vector.h` deliberately strengthens the public overlap contract:
both directions of arbitrary overlap are supported through a 258-double local
history buffer. All inputs are copied and checked before output or state changes.
This creates a truthful disjoint `restrict` contract for internal kernels; it
does not validate arbitrary pointers or grant alignment. Every load/store uses
a valid live double extent, including unaligned AVX loads and scalar tails.
Time is O(n), extra stack is 258*sizeof(double) plus ordinary call state; the
fixed staging cost is included in every backend comparison. The scalar,
compiler-vectorized and explicit AVX paths have identical numerical and failure
contracts. A slower vector path is a valid result.

The x86-64 adapter uses GCC/Clang extensions separately from the ISO C core.
AVX requires CPUID leaf1 OSXSAVE and AVX plus XCR0 enabling XMM/YMM state.
`cpv_detect` calls the injected XGETBV reader only after both CPUID bits pass.
The host probe performs the actual instruction; synthetic probes test every
missing bit and call ordering without executing unsupported instructions.
`cpv_plan` chooses scalar fallback when unavailable; forced AVX returns
CPN_UNSUPPORTED without changing the plan. Initialized plans are immutable and
valid only in the same execution environment; a fabricated AVX plan violates
the precondition. Programs are compiled for baseline x86-64 without global AVX;
only the isolated target-attributed function may contain AVX instructions.
There is no AVX2 requirement: these four-double operations need AVX only.
The dispatch assumes a consistent supported ISA across CPUs on which the OS
schedules the process; CPU hotplug/migration into a different feature contract
requires revalidation. No privileged instruction or host setting change is used.

Run `python tools/run_checkpoint_numeric.py --report
reviews/v5/expansion/legacy_completion/checkpoint_numeric.json` on Linux x86-64.
It compares GCC O3, GCC O3+LTO and Clang O3, checks actual packed arithmetic,
retained kernel calls and absent FMA, and verifies all outputs outside timing.
Clang disables SLP while its scalar loop separately disables loop vectorization;
the independent compiler kernel retains loop vectorization. This avoids labeling
packed basic-block arithmetic as a scalar baseline. Raw samples cover six sizes
or working sets, thirteen paired repetitions and two declared cache procedures.
128MiB perturbation is not proof that every cache is cold. Allocation, input
generation and output checks are excluded; staging, validation and state reset
remain included. Empty-loop/clock overhead is retained rather than silently
subtracted. Backend order rotates. Median/min/max, actual CPU/flags/affinity,
failures, output consumption and raw samples remain reviewable. No minimum
speedup or reproducibility on another host is asserted.

The criterion-specific changes are: overlapping-input staging for
AX-PR-P6-02-T34.06.b; no-AVX fallback for T34.07.b; compiler/LTO retention checks
for T34.09.a; and working-set/cache-procedure comparisons for T34.09.c. Running
the check-only executable on a machine lacking AVX remains separately recorded
when that environment is available. Pure probe simulation is not that execution.
These are assessor reference attempts, not seven-day delayed learner evidence.
Inspected dispatch/compiler/clock passages are registered in
research/v5/expansion/legacy_completion/resources.json.

## P1 storage and ownership composition

`checkpoint_memory.h` adds the original P1 byte-arena/typed-pool/queue
composition without converting a declared byte array into typed allocated storage.
CPMArena owns a view of caller-declared unsigned-character storage; its reservation
returns only offsets/bytes. CPMPool contains four actual CPMRecord objects. Its
checked lookup returns a pointer to those real typed objects, never a cast of the
arena bytes. The tests copy an integer representation into and out of a separately
declared integer object. This is distinct from assigning effective type to an
aligned declared byte array. Genuine malloc-backed typed allocation remains an
explicit changed-storage variant, not this representation's hidden implementation.

All P1 operations are externally serialized on initialized live objects. Queue,
pool, arena controls, source values and outputs are disjoint. Pool/arena pointers
remain stable; handles are owner-relative and cannot be transferred to a different
owner merely because slot/generation happen to match. Caller-provided alignment
is a truthful guarantee for the actual byte backing storage, not a number that
makes an unaligned address aligned. The initializer checks power-of-two supplied
and required alignments, required<=256, and supplied>=required, then starts empty.
Refusal leaves the previous control object unchanged.

Each reservation consumes a three-byte encoded length header before alignment
padding. Payload lengths0..65535 are supported; header bytes are length LE16 and
reserved zero. Checked remaining-capacity arithmetic precedes every addition.
For capacity32/alignment8, requests5,9,1 produce starts8,16 and refusal at needed
start32/end33. Refusal preserves offset25, header bytes and output span. Zero-size
reservation still consumes header/padding. The arena exports no arbitrary typed
lvalue. A view is produced only after generation and extent checks; callers use
only spans issued by that owner, not forged interiors or records from another arena.

Pins represent outstanding queue/callback borrows. Each successful pin requires
exactly one unpin; this count is not an independently identifiable borrow token.
Reset is refused while any pin remains. Queue removal or cancellation request does
not complete an outstanding callback. Reset increments generation and invalidates
all old logical spans even though the backing byte array stays alive. Generation
is deliberately limited to3; reset refuses permanently at saturation. This makes
the no-wrap policy executable. Initialization over an existing live owner is not
a permitted way to bypass retirement or outstanding borrows.

The typed pool initializes every object and metadata field. Allocation chooses
the first free slot and returns(slot,generation); exhaustion leaves the handle
output unchanged. Each allocation attempt increments the bounded diagnostic
counter, including injected failure. `cpm_fail_at` resets that counter and selects
one failing acquisition ordinal; zero disables injection. Counter saturation
refuses rather than wraps. Release requires a current live generation and zero
pins, then increments generation or retires the slot at3. Retired slots are never
reused. Lookup validates live owner metadata before producing a pointer. A raw
pointer is only valid while that object remains live and a retained borrow must
hold a pin. No promise is made to validate an arbitrary dangling raw pointer.

The four-entry FIFO owns allocated handles until pop transfers ownership to the
caller. The caller eventually releases each popped object. Prefix batch admission
publishes each successful acquisition and returns the exact count, including on
failure. Retrying starts at that count. Atomic batch admission first checks FIFO
capacity, then stages a complete declared pool/ring copy; it commits both only
after all acquisitions succeed. Failed atomic admission preserves live values,
generations, pins, attempt counter and queue. Its finite stack cost is
sizeof(CPMPool)+sizeof(CPMRing), which must be included in P1's resource ledger.
This whole-state staging technique is not automatically suitable for larger pools,
external effects, callbacks or concurrent raw-pointer readers.

Allocation/release and queue operations are O(4) or O(1); a batch of n requests is
O(n*4). No mechanism grows backing capacity, invokes a user callback, logs or uses
a general-purpose allocator. The exact sequence11,22,33,44/refused55/take/take/66,77
removes11,22 then33,44,66,77. A three-record offer with only two slots commits
exactly two under prefix semantics; the atomic version commits none on failure.
All four failing acquisition ordinals are tested for both policies against an
independent live-object and FIFO ledger.

This supports AX-PR-P1-01-T14.01.c, T14.05.c, T16.02.a, T18.04.b/c and the
allocation/rollback/lifetime facets of AX-PR-P1-02-T16.14.b/c and T21.04.b.
AX-PR-P1-02-T13.01.c still requires the separate generated-code/effects audit;
full foreign callbacks, LTO/stack-protection variants and learner dossiers remain
separate evidence. Compiled controls mutate failed-reservation progress, publish a
failed transaction, reset while pinned, and wrap a retired generation. All failures
remain harmless local state/output violations in original teaching code.

This additive laboratory implements the exact shared P3 framing and copied queue
boundary from `curriculum/guides/integrations.md`. It supports the original nine
criteria of PR-P3-01/02; it does not replace the historical `integration.p3`
decimal-record exercise. The learner source remains intentionally unsolved.

## Representation and dependencies

The target core is ISO C17 with eight-bit bytes and available exact uint16_t and
uint32_t types. It uses caller-owned actual CPParser, CPQueue and CPMessage
objects, unsigned-character buffers, and byte/bit operations only. No allocation,
library codec, structure overlay, external I/O, thread or platform primitive is
part of the core. C11/C17 use the same object, alignment and unsigned arithmetic
rules relevant here; no C23-only mechanism or extension is required. Host CTest,
Python zlib, printing, sanitizers and subprocesses are verification dependencies.

CPParser has at most 42 candidate bytes and one copied completed message. The
two-entry CPQueue owns its message copies. Payload length is at most 32. The
parser and queue remain at stable addresses until all calls return. Calls on
each pair are externally serialized, including producer/consumer operations.
Public structure fields permit diagnostic inspection; arbitrary modification,
uninitialized objects, out-of-range counters and concurrent mutation violate
the precondition. This model does not implement a concurrent queue.

All input/output/control objects are disjoint unless an operation explicitly
documents otherwise. Every non-null pointer denotes the full live extent needed
for that call. These are caller obligations, not claims of pointer validation.
Null data with length zero is allowed for CRC and feed. Message payload bytes
beyond length are not meaningful and are never compared as evidence.

## Exact wire and admission

The frame is A5, version01, sequence uint16 little-endian, length uint16
little-endian, payload, and four little-endian CRC bytes. Total is 10+length.
CRC covers version through the last payload byte, excluding A5 and the stored
CRC. The reflected polynomial is EDB88320 with initial and final XOR FFFFFFFF.
Sequence wrap, duplicates and gaps belong to the application; the parser does
not reject a second valid frame with the same sequence.

`cp_encode` first rejects missing required pointers or length above32 with
CP_INVALID, then insufficient output capacity with CP_FULL. Both leave output
bytes and `written` unchanged. On CP_OK it writes exactly 10+length bytes and
sets `written`. Offset byte buffers are legal because accesses are bytewise.
Encoding is disjoint, not an undocumented in-place algorithm. Empty payload is
a valid ten-byte frame. `cp_crc32(NULL,0)` is zero.

## Streaming and recovery

`cp_parser_init` resets used/ready and invalidates any prior logical message.
It requires a valid non-null parser. No byte clearing is needed for correctness.
`cp_feed` requires an initialized parser and a live `used` output. Invalid null
arguments return CP_INVALID before changing state or the output. For valid
arguments it initializes `used` to zero, then reports exactly the bytes accepted
from this call. Callers advance input only by that count.

* CP_NEED_MORE: all provided bytes were consumed, no message is published, and
  bounded partial state survives. Incomplete header/payload/CRC is not rejection.
* CP_REJECT: one unsupported-version, oversized-length or bad-CRC candidate was
  rejected. Restart searching exactly one byte after its starting sync, keeping
  its remaining candidate bytes. Retry with the unconsumed input; a retry with
  zero bytes may reveal a valid embedded frame.
* CP_FRAME: one copied message is ready. Until delivery succeeds, every feed
  returns CP_FRAME and consumes zero bytes. Later bytes must remain upstream.

Garbage before sync is discarded. Unsupported version is rejected after two
bytes, oversized length after six; payload is never stored under an unchecked
length. A CRC mismatch publishes no message. `cp_peek` copies the ready message
and returns CP_FRAME without consuming it; otherwise it returns CP_NEED_MORE
and leaves output unchanged. Reset is an explicit discard, not an acknowledgement.

The pending message plus candidate buffer is a fixed resource ceiling, independent
of upstream input length. For maximum candidate size B, the simple shifting and
CRC rescans cost O(NB) over N accepted input bytes, plus O(B) buffered follow-up
work per candidate. B=42 here. This is deliberately auditable, not a claim of
constant work per feed or the best possible resynchronization algorithm.

## Publication and failure

`cp_queue_init` creates an empty queue. `cp_deliver` requires live initialized
objects. With no pending message it returns CP_NEED_MORE; with two queued items
it returns CP_FULL and changes neither parser nor queue. Otherwise it copies the
complete message to the tail, increments count, then clears parser readiness.
There is no fallible callback between publication and consumption. The queue
owns its copy before the parser can accept more input. This defines the single
commit point for the serialized operation, not crash durability or lock freedom.

`cp_take` returns CP_NEED_MORE without changing output when empty. On CP_OK it
copies the oldest message and releases that slot. Public handles, durable sinks,
asynchronous cancellation and application sequence receipts require distinct
adapters. A future callback that can fail after publishing cannot simply reuse
this commit order without an idempotency or acknowledgement protocol.

## Required artifacts and assessment mapping

Use the original complete learner contracts and separate assessor analyses for
PR-P3-01/02 in `curriculum/v5/expansion/legacy_assessments/` and
`assessor/legacy_depth/`. Their prerequisite dossiers, explanation obligations,
rubric levels, graduated hints, seven-day delayed variants and evidence limits
remain binding. New executable support is narrowly mapped as follows:

| Original criterion | New observable support |
| --- | --- |
| AX-PR-P3-01-T04.06.b | Offset buffer, exact sequence258 fixture, explicit LE bytes |
| AX-PR-P3-01-T15.12.b | Empty/max lengths, version and length rejection, truncated CRC |
| AX-PR-P3-01-T15.12.c | 123456789 vector, zlib differential check, altered/recomputed payload |
| AX-PR-P3-01-T18.01.b | Capacity/output preservation and incomplete-output sentinel |
| AX-PR-P3-02-T15.13.b | All4096 partitions of13bytes and held message under queue refusal |
| AX-PR-P3-02-T15.13.c | Nested valid frame recovered after bad42byte candidate, all55 chunk sizes |
| AX-PR-P3-02-T21.05.b | Full queue, unchanged pending message, exactly-once retry publication |
| AX-PR-P3-02-T32.06.a | Independent whole-buffer oracle; deterministic217stream corpus, three chunk sizes |
| AX-PR-P3-02-T32.06.c | Compiled skip-candidate and erase-on-refusal faults with semantic rejection |

Passing an anchor supports its stated mechanism; it does not satisfy every
reasoning clause or independently assess the learner. Preserve the original
stream and the reduced failing stream when diagnosing a mutation. The nested
fixture is an explicit small counterexample to skipping the entire bad candidate.
Research-level follow-up must minimize other failures rather than copy that answer.

## Security and changed conditions

Only this original decoder and synthetic byte streams are targets. The attacker
controls every input byte, chunk boundary and modeled consumer delay; execution
is a local process with no network access or real data. The properties are bounded
storage, correct resynchronization, unchanged rejected outputs and no lost/duplicate
queue publication. A harmless success condition for a faulty version is a missing
synthetic sequence7 or a lost queued synthetic message, never arbitrary code execution.
The repaired version, four compiled mutations, capacity checks, sanitizer profiles
and independent oracle are the regression controls. CRC permits deliberate payload
changes when an attacker recomputes it; no authenticity, secrecy or replay resistance
is claimed. Timeout, physical UART/DMA and concurrent-service evidence remain separate.

Required transfer variants remain the original named variants, including version2,
durable acknowledgement, consumer-refusal fuzz input and changed queue capacity.
This batch does not fabricate a delayed learner attempt, independent review or
execution of those new architectures. Its readiness report states what was actually run.
