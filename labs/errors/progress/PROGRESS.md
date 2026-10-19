# Required T65.03 progress, cancellation and durable-epoch investigation

Complete original X-T65-003 R1-R4 and X-T65-003-DELAYED. Preserve the original
er_transfer contract. Import an actually completed PR-T19-04/U-T19-04-04
io.stream workspace, public stream.h and its exact contract. Implement progress.c;
do not substitute a local imitation of sf_start, sf_pump or sf_cancel. The author
adapter calls all three actual producer operations and actual er_transfer.

Use hosted ISO C17, GCC and Clang. The portable state core uses fixed caller
storage and no allocator. The additional Linux harness uses owned pipes and an
unlinked temporary file; operating-system calls are host experiments, not core
dependencies. The original stream producer retains its documented formatting-only
warning exception; the new core remains under all strict warning/error flags.
Published C17/C23 texts remain inaccessible. Linux write/fsync documentation is
a platform contract, not an ISO C guarantee or observed physical durability.

## Representation, framing and borrowed lifetimes

EptStream embeds the actual SFStream copied frame plus its actual SFHandle,
confirmed durable position, epoch, active/state and uncertainty fields. Payload
length is 0..64. The producer copies `[length,payload,modulo256 sum]`, so offsets
count framed bytes, not payload bytes or application-level effects. The frame is
2..66 bytes. The checksum detects some accidental differences; it authenticates
nothing. EptHandle binds the live control address and nonzero producer generation.

Initialize only fresh nonowning storage after all previous handles are discarded.
The live control stays at the same address and is not copied or modified behind
the API. All input, control, output and callback contexts are disjoint. Callbacks
are synchronous, nonreentrant, do not retain spans, mutate/free controls, jump
across the caller or start another operation. Calls are externally serialized.
A generation check cannot validate a freed pointer or enforce thread exclusion.

Start requires no active request, valid arguments and a representable next
generation. It copies through actual sf_start and publishes the handle after
success. An active request returns BUSY; oversized payload or exhausted generation
returns RANGE; admission failure preserves the old control and output handle.
Mutating or destroying the original payload after successful start is legal.

## One-call progress and terminal outcomes

Step offers the remaining copied suffix at most once. Its bridge calls actual
er_transfer and converts the admitted relative count for sf_pump. Preserve the
backend diagnostic separately from the positive count. IO with the final bytes
remains IO, not DONE. AGAIN with the final bytes remains AGAIN for this activation;
the next activation observes the producer's DONE without another callback.
OK with a short positive count returns AGAIN because bytes remain. Zero OK on
nonempty input, excessive count or unsupported status is a contract violation.

The observation reports accepted, durable, epoch, state and uncertainty. Accepted
bytes are irreversible external progress. A contract-violating backend may have
performed unreported effects: accepted then denotes only the known prefix, not
proof that no further bytes reached the transport. Mark uncertainty kind2, retain
the copied owner and refuse further writes. This state cannot be repaired by the
flush-reconciliation API. The isolated target may be abandoned after quiescence;
the adapter cannot safely infer a replay position.

Ordinary IO, DONE or CANCELLED states are stable and make no new write callback.
Cancellation between calls cancels the actual producer if still active, retains
accepted bytes and copied storage, and prevents further writes. This is not
asynchronous syscall cancellation. It makes no claim about a callback that is
still running or retains an illegal borrow. An empty payload still has a two-byte
frame; the original er_transfer empty-byte case remains a separate zero-callback
test. Null required outputs/callbacks are INVALID, and inactive/stale handles are
STALE; these admission errors preserve supplied outputs.

Example: payload [1,2,3] becomes [3,1,2,3,6]. A two-byte AGAIN result accepts
[3,1]. A subsequent three-byte IO accepts the remaining [2,3,6] and terminates IO.
Neither result establishes durability. Cancellation after the first call leaves
exactly that two-byte prefix externally visible, without replay or rollback.

## Separate durable epochs and uncertain acknowledgement

Checkpoint calls a separate trusted backend with accepted, prior durable and
prior epoch. If no accepted bytes await confirmation it succeeds without a
callback. At epoch UINT32_MAX it refuses an advancing checkpoint before invoking
the backend. The result contains a confirmed absolute prefix, epoch and outcome:

| Outcome | Admission and state effect |
| --- | --- |
| FLUSH_OK | Positive progress required; prefix lies between prior durable and accepted; advancing prefix uses exactly prior epoch+1. |
| FLUSH_IO | The same prefix/epoch rules apply, but unchanged prefix and unchanged epoch are also valid; retain confirmed progress despite the diagnostic. |
| FLUSH_UNKNOWN | Ignore reported prefix/epoch, keep the last confirmed pair and mark uncertainty kind1. |

An invalid result similarly marks kind1 without publishing an untrusted pair.
No further write or checkpoint runs while uncertain. The accepted count alone
must never update durable. Each epoch confirms an ordered prefix and cannot
wrap, skip or identify an unrelated request. A rejected report does not undo
external synchronization already performed by the backend.

Reconcile is allowed only for kind1 uncertainty. It requires an explicit trusted
backend verification assertion and a prefix within the old durable..accepted
interval. An unchanged prefix has the same epoch; an advancing prefix uses exactly
the next nonwrapping epoch. Numeric invalidity or missing verification preserves
all state/output. Reconciliation does not derive truth from readable cached bytes,
authenticate a peer or implement power-loss recovery. A false caller assertion
violates the contract. Kind2 write uncertainty cannot use this narrower repair.

Release requires a nonpending terminal write state and explicit transport
quiescence. All synchronous borrows must have returned; the external transport
must no longer access this frame. If accepted exceeds durable or uncertainty
remains, release additionally requires explicit abandonment of the incomplete
external result. This releases local ownership, not the already visible bytes.
Without abandonment it returns BUSY and retains the owner. A new start after
release changes generation; an old handle is STALE. The caller must choose a new
or reconciled external request/transport, not append a fresh frame to an abandoned
truncated one and call that recovery.

## Actual host experiment and fault boundaries

The native harness fills an owned nonblocking pipe until actual EAGAIN, observes
zero accepted bytes, drains it, then limits the writer to two bytes and cancels.
Closing the owned writer produces exactly [3,1] then EOF at the reader. The quota
is a deliberate bounded offer; it is not mislabeled a kernel-generated short
write. No closed-reader SIGPIPE experiment or host signal-setting change occurs.

An owned unlinked file receives two then three framed bytes. The first actual
fsync succeeds and the teaching backend returns confirmed prefix2/epoch1 with an
injected diagnostic. A second checkpoint is refused before fsync. A third actually
syncs prefix5/epoch2 but deliberately loses its acknowledgement. The controller
retains the actual successful backend result separately and supplies it to trusted
reconciliation. Final bytes are independently checked. No reboot, power cut,
directory durability or recovery solely from file readability is claimed. Actual
fsync failure is conservatively UNKNOWN; the injected pre-call refusal is labeled
separately from a syscall failure. This is a bounded original teaching backend.

## Required evidence, rubric and hints

D1 derives the product of count/status and accepted/durable/uncertain states.
D2 implements the explicit bridge, copied lifetime and release policy with actual
producer imports. D3 enumerates original lengths 0..5, positions 0..6, counts 0..6
and every status plus an invalid tag; compare callback counts, exact offered spans
and unchanged position on violation. Add composed framing and epoch admission
tables, finite counters, stale handles and invalid reports. D4 executes cancellation
after progress and separate durable epochs, preserving the unchanged original
rubric and delayed variant ID. Submit exact commands, source identities, raw
observations, independently predicted results and all thirteen cost dimensions.

Core, cancellation, epochs, uncertainty, retained baseline and native anchors
assess different obligations. Ten compiled mutants include status erasure, false
durability, ignored cancellation, discarded durable progress on error, premature
release, unverified reconciliation and forgotten uncertainty plus actual retained
ER_MUTANT5/6. A named semantic assertion is required; crashes, sanitizer aborts or
compile failures are not substitute evidence. Learner stubs compile and reject.

Level1 reproduces nominal output only. Level2 requires all original criterion
mechanisms, invariants and independent expected results. Level3 derives the changed
authority/lifetime argument without a worked solution, including a counterexample
to replay or a durability inference. Passing author tests does not grant learner
mastery. Hints progress from drawing two independent progress axes, through
checking the relative count before conversion, to identifying which external
authority can resolve an uncertain acknowledgement. Private solutions and sealed
traces remain under assessor/, outside learner export.

## Cost and enforcement table

| Dimension | Required evidence |
| --- | --- |
| Representation | Actual EptStream/SFStream sizes, alignment and explicit state product. |
| Reachable storage | At most66 copied frame bytes plus fixed controls; caller-owned backend context. |
| Lifetime | Copied source independence; synchronous suffix borrow; retained owner until release. |
| Mutation/aliasing | Serialized controls and disjoint outputs; generation validates identity only. |
| Allocation | No explicit core allocator calls; host file/pipe costs are separate. |
| Movement/copying | Start copies payload through actual producer; no copy per retry. |
| Destruction | Explicit terminal/quiescent release and abandonment policy; no implicit C destructor. |
| Dispatch | One borrowed write/flush callback per admitted activation; actual imported producer calls. |
| Concurrency | No concurrent control access or asynchronous callback cancellation. |
| Failure | Known progress, write-report uncertainty and flush uncertainty have distinct authority. |
| Compilation | Clean/unchanged builds and current actual dependency closure. |
| Binary/stack | Six-profile object imports, code sizes and stack observations. |
| Boundary | Linux API assumptions, trusted reconciliation and physical evidence limitations. |

Start is O(payload length); each step/checkpoint has bounded core work plus one
external callback, whose latency is not bounded by this contract. Seven10000-run
samples measure the same five-byte frame, one write, one known epoch and release.
Include validation overhead, raw CPU resolution/noise and zero separate warmup.
No tail-latency or universal abstraction-overhead claim follows. Targets contain
only synthetic bytes in original local processes; a state-integrity defect is not
arbitrary code execution. Physical storage validation, independent human review
and unaided delayed assessment remain separate.
