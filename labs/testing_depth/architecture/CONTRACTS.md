# T32.01–03: production boundary, independent tests and real adapters

This required successor implements all nine original PR-T32-01 criteria and
their nine named changed conditions. The original project, fixture, units,
historical hours and acceptance records remain unchanged. C17 is the active
edition; the exact published C17 text remains unavailable. No C23 feature,
compiler extension, POSIX operation or host verification library is an ISO C
guarantee. The supplied `architecture.c` is deliberately unfinished.

## Inputs, representation and ownership

Read `architecture.h`. Implement the operations from permitted C primitives;
do not use an existing queue or collection library. A `TaQueue` borrows a live,
properly aligned `TaStorage` until its last use. Both are fixed-address typed
objects. Do not copy an active queue as an independent owner, use expired
storage, call concurrently, or mutate representation fields outside the
diagnostic experiment. Controls, backing storage, input and output ranges are
disjoint. Null control/storage is admitted only by `ta_init`; other operations
require a successfully initialized live control. C types do not enforce these
premises. Host heap allocation is permitted in the harness, never the core.

Capacity is1–8. `TA_RING` uses capacity+1 physical bytes and one spare slot;
`TA_COUNTED` uses exactly capacity bytes plus an explicit count;
`TA_COMPACT` shifts a dense prefix on removal; `TA_LINKED` uses exactly capacity
caller-owned nodes, a free chain and a FIFO chain. `extent` must equal the
required physical slot count. Invalid initialization leaves both objects
unchanged. Successful initialization requires fresh or quiescent storage;
unused byte/node values have no defined abstract meaning.

Push copies one byte or returns FULL without modifying either object. Pop and
peek require a live disjoint byte output; null output returns INVALID. EMPTY
preserves output and state. Peek never removes. Pop removes exactly the first
accepted byte. `ta_count` is the abstract count. Invalid raw pointer extents
cannot be detected by this C API and must not be passed to manufacture tests.
For `ta_batch`, `accepted` is a live disjoint output, set to0 on admission;
null input is permitted only at length0. FULL commits exactly the accepted
prefix and reports its length. Do not apply single-operation unchanged-state
claims to this explicitly partial operation. All sizes are bounded by actual
caller-provided extents, not by an unchecked pointer-plus-count assertion.

The separate `ta_invariant` checks the active representation. Linked chains
partition all nodes exactly once and agree with head/tail/count. Counted/ring
positions agree modulo their physical extent; compact layout retains a dense
prefix. This diagnostic is not the public FIFO oracle and must never be used
as the sole expected-result generator. Ring/counted/linked push/pop are O(1),
compact pop O(capacity), diagnostics O(capacity), batch O(accepted prefix).
Every implementation uses O(capacity) caller storage and no allocation.

## Concrete workflows and failure semantics

The original input pushes7,8,9 into capacity2: accept7,8, reject9, then pop7,8;
empty pop preserves a123 sentinel. Test all four layouts under this same
capacity, including the all-slots-usable variant. Surround live storage with
canaries, snapshot protected bytes before refusals, and enumerate every
length-seven word over PUSH7/PUSH8/POP/PEEK against a Python list. Conservation
is accepted=removed+remaining; successful peeks change neither side. Include
longer repeated wrap/drain sequences and capacities1 and8 separately.

`ta_decode` accepts exactly the original three-byte packet `{2,a,b}` and
publishes two output bytes only after complete validation. `ta_begin` admits a
fresh `TaFlow`, a valid packet, one of the four layouts and positive callback
budget. It copies packet values through the actual queue into owned staging;
no packet borrow survives. Invalid begin leaves the entire flow unchanged.
The flow may be reused only after terminal state and with no callback active.

`ta_step` calls its synchronous, nonreentrant writer at most once. The callback
may borrow the remaining bytes only until return. OK admits0..remaining bytes;
WAIT requires0; ERROR admits a bounded prefix and then terminates with IO.
Accepted bytes advance exactly once. An invalid status/count terminates INVALID
with `uncertain=1`: external effects cannot be reconstructed from a lying
callback and must not be blindly replayed. A null writer returns INVALID
without changing the active flow. Otherwise each attempt consumes one budget;
completion takes precedence over exhaustion, and no-progress exhaustion is
TIMEOUT. Later step/cancel calls preserve a terminal result and perform no I/O.
Cancel of an active flow is terminal CANCELLED and does not undo prior effects.
There is no clock or asynchronous cancellation claim in this attempt budget.

Script a one-byte prefix followed by the second byte, zero progress followed by
timeout, immediate error, oversized count, cancellation, and error after a
nonzero accepted prefix. Check callback counts, exact bytes, retained progress,
no retained input, and all terminal paths. Diagnosis must identify the failed
transition, rather than treating any nonzero process exit as a killed mutant.

## Actual host adapter and changed assumptions

`socket.c` creates and closes an original local AF_UNIX socket pair. Its first
adapter caps each real send request to one byte, thereby producing partial
progress relative to the component's two-byte request. This is controlled
adapter chunking, not a claim that the kernel shortened a one-byte syscall.
It verifies received bytes, then closes the consumer: `MSG_NOSIGNAL` contains
SIGPIPE while the actual EPIPE result terminates the producer. It separately
fills a small nonblocking send buffer with65536-byte requests until actual
positive short progress and EAGAIN/EWOULDBLOCK are observed, then drains and
checks every accepted byte. This Linux-specific observation is not a portable
promise about the exact socket-buffer capacity or transport delivery.
Successful `send` means locally accepted bytes, not remote durable effect.
No external network, credentials, deployed target or host security change is
part of the task. A platform without these socket semantics requires a new
adapter and recollected evidence, not a renamed mock.

## Runtime audit and deliverables

Run `python build.py --source YOUR_SOURCE --output PROGRAM` and the replay and
socket harnesses with `--test`. Provide the production source and exact build
commands; independent predictions/list oracle; all semantic counterexamples;
source/tool/flags/target identities; and a dependency/evidence matrix. Compare
plain, ASan+UBSan, GCC coverage and supported trap-only undefined checks using
separate object symbol lists and link maps. The hosted test contains malloc,
stdio and memory comparisons; the strict core must not acquire them. Coverage
and sanitizer instrumentation deliberately introduce host-only dependencies.
Audit the actual generated code: `-ffreestanding` alone does not ban implicit
helpers. The supplied campaign checks native x86-64 objects and an explicit
no-library link, not every ISA or a board image. Trap-only behavior may be
compiler/target-specific; absence of undefined symbols is not proof that all
undefined behavior is checked. Do not time instrumented builds as production.

## Per-objective assessment and changed-condition readiness

| Original objective | Required evidence and separately assessed changed condition |
|---|---|
| T32.01.a | Component/harness/socket ownership and undefined-symbol audit; replace stack-backed harness storage with malloc/free without changing core behavior. |
| T32.01.b | Independent list predictions7,8/FULL/EMPTY; rerun the same oracle against linked caller nodes. |
| T32.01.c | Four actual dependency maps; trap-only helper audit and its detection limits. |
| T32.02.a | Every intermediate FIFO/conservation observation plus separate invariant hook; peek adds no removals. |
| T32.02.b | Extrema/refusal/canary snapshots; partial batch commits a precisely reported prefix. |
| T32.02.c | Same behavioral suite on spare-ring and compact array; counted ring uses all slots while preserving abstract capacity. |
| T32.03.a | Real decode/queue/writer composition, exact once-only progress/cleanup; zero-progress budget terminates without an extra callback. |
| T32.03.b | Actual socket acquisition/send/receive/close and separate scripted backend; early consumer close yields contained EPIPE. |
| T32.03.c | Explicit logical/OS/emulator/physical matrix; unsupported emulator peripheral remains unobserved rather than silently reassigned to host tests. |

For each row retain the original criterion ID and its `-DELAYED` ID and7-day
minimum. Author reference execution only makes the assessment ready; it is not
the independent learner's attempt. Level0: wrong/missing contract or evidence.
Level1: fixture reproduced but representation, ownership or environment
confused. Level2: all row obligations, independent falsification, diagnosis and
bounded explanation. Level3: delayed unfamiliar variant implemented and
explained unaided, including changed assumptions and residual limits.
Hints: first list abstract observations; then enumerate ownership transfers;
finally compare each result against the promised publication point. Separate
assessor analysis is deliberately excluded from learner exports.

The supplied `evidence_matrix.json` is an explicit expected authority example,
not a fabricated measurement receipt. Extend it with your actual observations.
The property matrix must leave UART physical baud and ADC analog accuracy
open. Grammar/FIFO are logical properties exercised by finite inputs; socket
results are actual selected OS observations. An emulator without the chosen
UART/ADC cannot establish even that peripheral model's behavior. No physical,
independent-human review, learner-mastery or exhaustive-C proof is supplied.
