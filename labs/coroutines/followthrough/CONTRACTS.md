# Required T72 changed-condition mechanisms

This contract extends X-T72-001 through X-T72-009, preserving their objective,
rubric and delayed-variant identifiers. It does not apply to X-T72-010's
separately maintained higher-order laboratory. The original exercise, the
original coroutine contract, and the modern cost contract remain required.
Each numbered section below is a concrete additional implementation and
assessment obligation, not an optional discussion prompt.

## Environment and common ownership

Use C17, hosted Linux, GCC and Clang. The portable algorithms use ISO C
representations and injected allocation; pthreads, mmap, and the existing
x86-64 System V fiber backend are separately identified platform dependencies.
No implementation may substitute a coroutine, parser, query, or effect library
for the mechanism being built. Host Python oracles and sanitizers are
verification dependencies, not part of the learner component.

Headers and unsolved C files in this directory are learner artifacts. The
public test programs are supplied observations. Assessor implementations and
analysis are withheld. Inputs, output objects and control objects are distinct
unless an operation explicitly transfers ownership. Borrowed arrays and callback
contexts remain live, correctly aligned and stable through their last use.
Do not copy a live owning frame or mutex. Except for CAFrame's documented
thread-safe operations, calls on one frame are serialized and nonreentrant.
Callbacks do not perform nonlocal jumps. Allocators return fresh suitably
aligned storage or null; release is nonfailing and does not retain the pointer.
The code cannot authenticate a lying allocator or validate arbitrary addresses.

For every section, submit a representation/state diagram, implementation,
pre/postcondition and invariant argument, allocation/cleanup trace, rejected
counterexample, independent expected results, changed-condition demonstration,
and all thirteen dimensions of the modern cost review. Preserve caller outputs
on failure unless a section explicitly permits diagnostic or partial-progress
output. Separate data consumed, state committed, output published and storage
reclaimed. Record exact commands, tool versions, source hashes and limitations.

## 1. Fallible owning map and unequal zip

Implement owned_stream.h's CFSource, CFItem, cf_map_next and cf_zip_next for an
immutable finite uint32 array with an exact length. A mapper consumes its input
before invocation. It may acquire a buffer and return an error; it must publish
every partial owned buffer in the supplied initially empty CFItem. The adapter
releases that partial item and leaves caller output unchanged. A successful
item has a valid allocator and a buffer of its declared length. A failing
mapper that hides an allocation violates its prerequisite contract.

Zip first checks both exact extents, then consumes one value from each source.
It stages both mapped items and publishes both only if both succeed. A failure
on the second item releases the first and the second partial item. Both inputs
remain consumed. If either source is exhausted, neither advances, including
repeated DONE calls. Distinct output owners must initially be empty.

Use inputs [1,2,3,4,5], fail each callback in turn, and all unequal lengths
0..5. Count live allocations after every return. Explain why this stronger
no-extra-pull rule requires exact finite extents: Rust Iterator::zip may attempt
another first-iterator pull after exhaustion. It is not a universal zip law.
Map costs one callback and bounded item cleanup; zip costs at most two. Callback
work and buffer size are explicit parameters, not hidden O(1) assumptions.

X-T72-001-DELAYED uses the unequal zip operation with a second callback failure.
Hints: distinguish pulling from publishing; stage owners separately; enumerate
the failure after the first allocation but before the second publication.

## 2. Unfused grouping, asynchronous batches and ordering

Implement CFVolcano using separate scan, selection-mark and contiguous grouping
stages. Mark negative-valued rows instead of deleting their group boundaries.
Compare with the actual earlier co_query component for all short streams.
The scan may inspect the next key but must not consume that next group's index.
Overflow preserves caller output and committed index. Close is idempotent.

CFBatch borrows capacity0..64 rows. Calls publish a monotonically growing
immutable prefix via available; EOF is terminal and forbids later growth. Keep
one pending group across empty batches and across a blocked sink. sink_ready=0
must not publish or discard a completed group. Cancellation drops the borrow
and makes future polling reject. Separate internal scan progress from the
externally committed source position. Overflow preserves this call's state;
work retained from earlier calls is still retained. This is serialized prefix
publication, not concurrent memory synchronization.

Use all streams of length0..6 over two keys and values -1,+1, every batch split,
negative-only separators, UINT32 sum overflow, and output canaries. Require
equivalent complete results, not identical function-call counts. Instrument
scan and predicate calls to compare fused and unfused work.

X-T72-002-DELAYED implements cf_order: wait for EOF, require capacity at least n,
reject n>64, then stably sort into a distinct output array. Before EOF return
PENDING without changing output. Insufficient storage returns RANGE unchanged.
Insertion sorting is O(n squared) comparisons and O(n) admitted storage; it is
not an unbounded streaming sort. Hints: retain a complete group while blocked;
EOF differs from an empty batch; an ordering barrier changes memory admission.

## 3. Generator borrow, transfer, clone and fixed address

Implement CFGenerator for count0..47 Fibonacci values F0=0,F1=1. Each next
allocates a four-byte little-endian buffer, stages it, and then replaces the
prior current item. Allocation failure preserves index and current item.
Borrow lasts only until next, take or destroy. Take transfers the current item
and clears the frame's owner; clone allocates independent bytes and leaves the
original owner unchanged, including on failure. Destroy releases current item
and frame and clears the owner's pointer; repeated destroy through null succeeds.

The frame records its final address. A struct copy fails the self-address
admission check but is not a valid second owner. This runtime check does not
make reading a freed frame valid. Test every count and each allocation failure,
retain transferred values across suspension, and interleave two generators.
Compare bytes against an independent integer recurrence, not the same recurrence
implemented with the same word width. Peak allocation includes frame, old/new
staged item and any caller-owned clones.

X-T72-003-DELAYED requires the stable self-reference and a counterexample to
ordinary struct assignment as a move. Compare with C++ coroutine frame
allocation/destruction rules and the explicit state in LLVM lowering; neither
source makes arbitrary C aliases safe. Hints: decide who releases each item;
stage before changing the old owner; a cleared owner differs from a stale alias.

## 4. Nested guarded fibers and cooperative cleanup

Use the existing actual x86-64 System V COFiber backend. CFFiber owns an mmap
region containing an admitted 64KiB..16MiB usable stack rounded to pages, with
one PROT_NONE guard page on each side. Initialize the underlying fiber only at
its final stable address. The output must be fresh and own no live mapping;
keep the wrapper, callback argument and mapping live. A valid creation attempt
clears the output before allocation. If protection or backend initialization
fails, cleanup is attempted; if unmap also fails, the output retains its mapping
with a dead backend and the caller must retry destruction. Do not retry creation
over that retained obligation. Argument/admission rejection leaves output alone.
No MAP_FIXED mapping or host security-setting change is permitted.

An outer fiber creates/resumes an inner fiber; both retain ordinary automatic
variables and own allocated resources across yield. Cancellation only records
intent. cf_fiber_suspend reports it at a cooperative boundary; callbacks unwind
their own resources and return. Suspended destruction rejects. Completed/fresh
destruction releases the mapping only after the backend accepts destruction.
System-call failure is an explicit failure result; it is not proof of cleanup.
If unmap fails after backend destruction, retain the mapping and retry only
unmap; do not destroy detector state twice or resume that dead backend. The
host test injects one retained-unmap error and then performs the real retry.

Inspect architectural RSP in both supplied stacks and preserve independent
caller/fiber rounding modes. Run the guard fault in a disposable child process
against its original anonymous mapping; expected SIGSEGV shows inaccessible
memory, not a recovery mechanism. Compare sanitizer annotations with the actual
switch and state what schedules or stack accesses were observed.

X-T72-004-DELAYED derives an AAPCS64 layout on paper: x19..x29, SP, return
address, the required preserved SIMD portions, alignment and platform x18 rule.
Identify additional obligations for pointer authentication, scalable vector
state, signals, unwinding, thread migration and shadow stacks before code.
No AArch64 backend execution is claimed. Hints: distinguish OS stack allocation
from ABI state; return through cleanup; a guard page cannot resume an overflowed
C execution safely by itself.

## 5. Owned expression continuations and external suspension

Implement CEFrame for a forward acyclic graph of1..16 literal/add/multiply
nodes. Each binary continuation owns a temporary left operand while its right
operand is evaluated. Use explicit tags/phases and a bounded continuation stack,
not C recursion. Each budget unit executes one transition; zero budget preserves
state and output. Checked int64 intermediates admit only int32 results.

Allocation or arithmetic failure is terminal and releases every temporary.
Cancellation at every reachable phase does the same and is idempotent. A final
external CEDeliver callback may return PENDING repeatedly; the frame retains
the completed value until acknowledgement, then publishes the caller result.
The callback receives a value, never a borrowed temporary owner, and does not
reenter the evaluator. Other callback errors terminate with IO.

Enumerate small expressions and budget cuts against mathematical evaluation,
allocation failure at each continuation, overflow, and cancellation before and
after either operand. Prove depth bounded by graph length even when DAG sharing
re-evaluates a node. Time depends on expanded traversal, not merely node count.
X-T72-005-DELAYED is the externally suspending delivery case. Hints: tag the
return destination; cancel owns the same unwind obligations as failure; final
value computation and external acknowledgement are different transitions.

## 6. Explicit cut and segmented-input diagnostics

Implement CPNode's forward graph of at most16 nodes and inputs of at most64
bytes, with literal, sequence, ordered choice, empty and cut. A successful cut
marks the entire current root attempt committed; a later mismatch propagates
CP_COMMITTED through enclosing choices. This deliberately specified global
scope is narrower than a general parser framework's nearest-choice cut policy.
Without cut, ordered choice retries only NO_MATCH. Exhausted budget is RANGE.

At the current available end, eof=0 yields PENDING; eof=1 yields NO_MATCH for a
required literal. PENDING must not select a fallback alternative. Resume by
re-evaluating the same immutable prefix with more bytes and an explicit budget;
this version does not retain a hidden parser stack between calls. Output end
changes only on success. Diagnostic output records furthest failure offset,
the four lowest distinct expected byte values, a truncation flag, steps and
commit information. Diagnostics may change on failure.

Use (cut('a') 'b') / ('a' 'c') with input ac, its no-cut counterpart, every
prefix and EOF choice, five expected symbols, zero budget and malformed edges.
X-T72-006-DELAYED is the segmented input/PENDING distinction. Derive all short
expected parses separately. Hints: carry commit separately from match status;
insufficient data is not a failed alternative; bounded diagnostic capacity
requires a stated selection and truncation rule.

## 7. Cache keys and bounded direct left recursion

Implement CEMemo for the fixed root E <- E '+' digit / digit at position0.
Include input address/length, explicit input revision, environment revision and
ten-bit allowed-digit mask in the cache key. The caller increments revision
when bytes or predicate meaning change. The code cannot detect a lying revision.
Cache both failure and success with exact endpoint and value.

Use a failing seed for an in-progress call, then grow the result while its
endpoint strictly advances. A seed alone only finds the base case. Limit input
to64 bytes and bound evaluations by budget; budget exhaustion invalidates the
cache and preserves output. Evaluate all short strings over 0,1,+,x against a
direct shape/sum oracle, mutate bytes at the same address with a new revision,
change the predicate, and retry after budget exhaustion.

X-T72-007-DELAYED is this bounded seed-growth policy. Explain why one root cell
does not handle arbitrary mutual left recursion or establish universal linear
packrat complexity. Derive this grammar's growth bound separately. Hints: a
cached endpoint belongs to its predicate/input; failure is a cacheable result;
strict growth is both a semantic and termination condition.

## 8. Threaded notification tickets and asynchronous quiescence

CAFrame owns a mutex, an eight-byte buffer and up to four outstanding CATickets.
Create wants1..8 bytes with nonzero generation. Reserve only while open;
serials retire rather than wrap. A ticket binds live owner, generation, serial
and slot. Notification consumes it once, even after cancellation; a copied
ticket is not another borrow. A valid notification copies bytes while holding
the mutex. Excess data closes with error. Zero bytes consume the ticket without
completing. Stale tickets are rejected only while their owner remains live.

The first terminal transition wins: completion before cancel stays complete;
cancel before completion suppresses later data. Closing forbids new reservations.
Requesting cancellation is not reclamation. ca_ack requires no outstanding
tickets AND the backend's external promise that all workers have joined and
no future entry through a raw frame pointer is possible. The counter cannot
prove that promise. Before acknowledgement poll is PENDING and output unchanged.
After acknowledgement complete data publishes once; cancellation/error preserves
output. Destroy requires acknowledgement, zero tickets and exclusive ownership.
Default non-robust pthread mutex operations must satisfy their valid-object and
same-owner unlock contracts; thread termination while holding the lock is outside
this recovery model. There is no lock-free or real-time progress guarantee.

Test all six orders of two deliveries and cancellation, actual concurrent
notifiers, stale/duplicate tickets, full ticket capacity, retirement, oversized
data and output canaries. Keep the owner live until join and acknowledgement.
This is a new asynchronous-disarm contract, not an assertion that the original
COAsync synchronous disarm may return early.

X-T72-008-DELAYED composes actual MsgHub copied messages through ca_channel_deliver.
An empty channel retains the ticket; a received byte is copied before the
message's storage can change. Closed/cancelled channel consumes the ticket and
requests cancellation; values above255 terminate with range error. Retain the
hub until all users join and destroy it quiescently. Hints: cancellation intent,
callback completion and quiescence are three events; ticket identity does not
extend object lifetime; publish only after the external acknowledgement.

## 9. Handler scope, owned suspension and explicit cloning

CHEffect composes actual COEffect with one owned four-byte resource. A borrowed
acyclic handler chain of at most four nodes has question masks and callbacks.
Lexical mode captures the initial chain; dynamic mode uses the chain passed
at each step. The first matching handler answers question1 or2. PENDING does
not consume the continuation; callback error preserves it for a stated retry.
Overflow also preserves the live continuation. Completion releases the owned
resource and publishes the sum; abort releases once without publishing.

Clone an active effect by allocating a new frame and deep-copying its resource,
repairing the token's owner to the new core. Failure leaves the source unchanged
and releases staging allocations. Chains remain shared immutable borrows with
explicit lifetime. Two clones may then receive different second answers and
be aborted independently. Copying the owning struct is prohibited.

Use lexical/dynamic nested policies producing different answers, a pending
handler, overflow followed by valid retry, abort at each question, allocation
failure1/2, and a clone with independently changed resource bytes.
X-T72-009-DELAYED is this explicit multi-shot policy. It copies a bounded
defunctionalized continuation; it does not capture arbitrary C stacks or inherit
Koka's type/effect guarantees. Hints: identify which state is owned versus
borrowed; clone each owned resource; update self-referential continuation owners.

## Assessment, containment and evidence limits

Required retained-construction alternatives are supplied in alternatives.h:
CRUnfused composes separate selection and mapping stages over actual co_next;
its source-pull counts and outputs equal co_flow_next on the same admitted
inputs. Predicate and mapping counters disclose the additional work. A
CRCallerGenerator holds the same Fibonacci sequence in caller-owned storage,
requires no allocator and has no heap-destruction obligation. Its storage must
remain live until discarded; equal values do not make its ownership API equal
to the allocated generator.

cr_push stages a copy of CFBatch, offers one complete group to a borrowed sink,
and commits that group only after sink OK. Sink PENDING must have no committed
external effect and leaves the group available for retry; other errors return
IO without committing that group. Prior accepted groups remain delivered.
Budget limits offered groups; zero budget invokes no sink. emitted reports this
call's accepted groups, including on failure. Thus this push API preserves
the same order, negative-group boundaries and arithmetic rejection as pull,
under an explicit sink-admission contract. A sink that can accept and report an
error needs a richer progress result. Test each batch split, blocked sink and
retry, then compare against complete pull output. Intentional mutants consume
a blocked group or misreport progress.

tools/run_t71_t72_alternatives.py records exact count comparisons before raw
fused/unfused timing samples. Timing uses identical range0..1000/take200 and
1000 repetitions with retained result checksum. Record sizeof values, code
disassembly, callback/scan counts and raw samples; never infer a universal
speed ranking or allocation equivalence from those observations. The separate
retained-baseline runner checks every reachable factorial state with all budget
pairs0..28. Explain the deterministic-transition composition induction needed
to extend those observations to arbitrary finite budget partitions; the finite
checks alone do not quantify over all partitions.

The actual five-import final adapter, its blocking and failure boundaries, and
the new concrete PR-T19-04 stream producer are specified in ADAPTERS.md and
labs/api_depth/stream/CONTRACTS.md. This supplements the earlier synthetic port
and the separately implemented nonblocking modern_composition adapter.

For each original rubric R1 require the representation/invariant and enforcement
argument; R2 the working implementation with allocation/cancellation boundaries;
R3 an independent oracle or trace plus detection of both supplied semantic
mutants; R4 the exact named delayed variant after the existing minimum delay,
including cost/assumption changes and a counterexample to a stronger guarantee.
Level0 is absent/incorrect evidence, level1 a happy path or copied explanation,
level2 all stated obligations with reproducible observations, level3 independent
changed-condition derivation and explanation of remaining limits. Passing a
finite author suite is not a level3 learner result or external assessment.

Defect demonstrations target only these original local programs, synthetic
bytes and disposable child processes. Attacker control is bounded input, graph,
event order, callback failure or stale live-owner token. Observable success is
an erroneous result, leaked owner, duplicate publication or accepted forbidden
transition; guard SIGSEGV only establishes the access violation. Do not infer
arbitrary code execution from a crash or sanitizer report. Repair the invariant,
rerun its negative control and explain residual caller/platform assumptions.

Reports under research/v5/modern distinguish retained baseline checks, new
mechanism checks and actual local Linux composition. Hardware timing,
published exact C17 access, independent learner work and external review remain
separate. No source/library comparison grants ISO C enforcement that it lacks.
