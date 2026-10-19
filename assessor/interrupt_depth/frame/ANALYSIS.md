# Private criterion analysis and transfer packet

Do not distribute this file, reference sources, or oracle/mutation tools with the
learner package. Use the original criterion objects and seven-day minimum; record
ordinary and delayed scores separately. A reference run is not a learner attempt.

## 01.a

The handler's live LR is EXC_RETURN; word5 is interrupted LR. The basic example
has originalSP20000020. An extended frame retains core offsets: r0 at0, PC at24,
xPSR at28; s0 starts32, s15=92, FPSCR=96, reserved=100. Size104, plus optional4.
The padded extended example at20000000 returns originalSP2000006c. The tempting
"skip18words before reading core" repair reverses the actual layout and fails.
The frame can be allocated while FP values remain unwritten. `fp_materialized`
is supplied only after correlating the captured context and lazy-save state.
Do not read unpublished FP words for logging and then claim logging is passive.

Reference `frame.c` stages its result and validates capacity before each accessed
region. Tests enumerate the six legal low-five-bit patterns and reject the other
26; they do not exhaust all32-bit EXC_RETURN values. Short lengths, alignment
padding, address overflow, no-FPU, bad xPSR/PC and lazy state discriminate the
contract. Other valid implementations may use a table or explicit tagged decode.
Require a proof that error exits precede stores, and a separate input stability
premise. Reject claims that a syntactically valid capture guarantees a successful
hardware exception return or that a stacking fault can always be decoded.

## 01.b

The entry shim tests EXC_RETURN bit2 before pushing, reads the selected SP and
live LR, and only then saves r4/LR on MSP. The C prologue cannot be substituted
for this capture point. Hardware owns r0–r3/r12/LR/PC/xPSR, while the shim's eight
bytes and compiler's `.su`/disassembly frames are additional. `xf_trigger` saves
16bytes on MSP before switching Thread mode to its dedicated PSP. A's hardware
frame therefore consumes PSP; a nested B consumes MSP. With an FP-using A callee,
B can have an extended frame even when the original Thread frame was basic.

For every compiler/optimization/callee cell inspect actual instructions and
`stack_ledger.json`, not this narrative as a golden byte count. The ledger reads
all static usage records and the direct-call graph; unknown frames, indirect
calls or recursion fail. Its normal two-level MSP bound is the maximum of the
closed main call tree and `main + trigger16 + shimA8 + captureA + B-hardware +
shimB8 + closed captureB tree`. It deliberately adds per-function maxima even
where outgoing-argument space is no longer live, so it is conservative rather
than a tight watermark. PSP hardware maximum is108bytes. Stack reserve checks
must still pass after changing optimization or the FP callee. Reject counting
only32hardwarebytes, assuming `.su` includes asynchronous nested hardware, or
calling this result a WCET or physical stack measurement.

## 01.c

Group3 with A60/B50/C60 yields A entry, B entry on MSP while A remains active,
return to A, then C tail-chains using the original Thread PSP frame. B's xPSR
IPSR is16; A/C return Thread/PSP. Same addresses alone cannot distinguish a
tail-chain from unstack/restack, so the campaign also checks QEMU's explicit
tail-chain log. It observes24tail-chains across16cases: eight group3 cases have
one each, eight group6 cases have two each. This is selected-emulator evidence,
not a measured silicon cycle saving.

Delayed prompt: give the learner `pend A; begin stacking Thread; pend B; choose
entry; return B; pend C; return A; return C`, under B>A=C urgency and recognition
before A's first instruction. Require context/active/pending rows after every
event. B enters first but the one saved context remains Thread; A has never run
and has no handler frame. B→A→C reuses that frame. Normal nesting instead reaches
two hardware frames, Thread and A. `late_arrival.py` independently checks those
hand-derived landmarks and rejects saving A during the late-arrival window or
pushing another frame on tail-chain. The exact point at which silicon recognizes
the late arrival is implementation-specific. No timed physical claim is made.

## 02.a

Pending while disabled is valid state, not lost work. Enabling under BASEPRI40
does not admit A60 in the declared grouping. Clearing a peripheral source can
leave pending set and permit later delivery. Conversely clearing NVIC pending
while a level source remains asserted can lead to repending. The serialized
model exposes those separate operations but does not model an undocumented
device's level-sampling time. The selector includes all active priorities; a
lowered current handler priority does not remove another still-active handler's
preemption constraint. Verify active+pending does not recurse into itself.
Alternative representations with explicit state-transition functions are valid
if they preserve the same observations and reject malformed declarations.

## 02.b

Use high implemented bits only. For bits4/group5/raw7f: encoded70, logical7,
group1, sub3. An incorrect model that compares raw bytes preempts within one
group. The independent Python oracle partitions bit strings, while the C model
uses arithmetic shifts; all12288config/byte cells and41472selection cells are
checked with exact identities, avoiding missing/duplicate-cell vacuity.
On the selected QEMU profile the observed implemented mask isff, eight bits;
that result must not be copied to an STM32 declaration without readback/manual
support. Group3 A60/B50/C60 nests B; group6 A60/B50/C70 has all group0, so A runs
until exit, B wins pending order, then C. Neither B nor C preempts A there.
The observed order changes from A,B,A-after,C to A,A-after,B,C.

## 02.c

For b bits, urgency0 maps to Arm's largest aligned byte and maximum urgency maps
to zero. The opposite controller maps urgency identically after shifting into
its declared byte field. For four bits urgency15 is Arm00/oppositef0; copying00
would invert the policy. ISO C enforces integer operation semantics only; the
compiler does not enforce valid priorities or legal ISR API calls. Reference
helpers perform runtime admission, selected FreeRTOS configASSERTs diagnose a
specific port convention, and whole-program API/context correctness remains a
separate verification/programmer obligation.

The port requires a nonzero representable max-syscall threshold and a grouping
that avoids subpriority ambiguity; with eight bits PRIGROUP0 inevitably has one
subpriority bit and the threshold low bit must be zero. Candidate priority0 is
above a nonzero threshold and cannot call those restricted APIs. Tests reject
zero threshold, discarded low bits, forbidden grouping and odd eight-bit
threshold. A valid helper result does not validate ISR stack size, kernel object
lifetime, API choice, or scheduling behavior; T30's actual port experiment owns
those claims.

## Distinguishing rejection and review

Six compiled semantic controls shift extended core offsets, drop padding,
consume lazy FP, ignore enable, permit subpriority preemption, or clear pending
on source deassertion. Each must compile successfully and fail its named
semantic check. Three run on the actual M4 target. The offset mutant may pass
structural checks on stale FP words before failing the interrupted-PC range;
that is still the precise wrong-offset mechanism, not a portable output assigned
to undefined behavior. Arrays remain within their declared bounds.

Grade each of the six rows using the public0–3 rubric. Require criterion-specific
analysis and evidence; a partition-oracle pass alone does not cover frames or
FP ownership. For independent assessment withhold these traces and use a new
declared priority-bit/grouping/controller combination and compiler selection.
Record hints, seven-day interval, artifacts, oral counterexample and observed
score. No such learner evidence or non-author review is fabricated here.
