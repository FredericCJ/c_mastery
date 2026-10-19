# P2 exact transfer composition

This is the current implementation contract for all four original criteria in
`curriculum/v5/expansion/legacy_assessments/PR-P2-02.json` and their exact
`-DELAYED` variants. The original fourteen-field assessment contract, learning
objectives, prerequisite dossiers, graduated rubric and seven-day independent
delay remain required. This package adds executable support; it changes no
historical criterion, hour, baseline or receipt.

## Environment and prerequisites

Use C17 and the completed `algo_ring`, `InTransfer`, `f_write_all`, `CPWTransfer`
and their actual public contracts. `writer.h` deliberately imports
`labs/integration/include/integration.h`; an unrelated channel implementation
does not satisfy this composition. Strict Profile D has no default C runtime,
allocator or operating-system calls inside the portable core. A separate hosted
Linux x86-64 harness may use libc, pipes, `poll`, and the documented raw syscall
adapter. GCC/Clang inline assembly and Linux error numbers are extensions and
OS/ABI contracts, not ISO C. Do not link the hosted harness into the strict image.

The later product comparison additionally requires the completed `PR-T65-03`
`er_transfer` and `adapters.h` progress adapter. It is a staged transfer task,
not a prerequisite for the original P2 construction. Import exact producer
source hashes and contracts in the learner dossier. Copies of public headers
are scaffolds; they do not establish prior learner acceptance.

Controls, queues, callbacks and output objects are live, aligned and disjoint.
Access is serialized. A callback does not reenter, retain its temporary pointer,
destroy the owner, or exit nonlocally. Initialize controls to zero and queues
with `algo_ring_init`; queue values are bytes. Callback result and extent obey
the producer contract. Bound a session before unsigned diagnostic counters or
cumulative accepted counts can overflow. No numeric check validates an arbitrary
pointer's lifetime or provenance.

## Problem and required reasoning

Implement every function in `writer.h` using the supplied unfinished
`learner.c`. Preserve the original queue and transfer APIs. For a byte transfer,
the queue owns bytes not yet removed; `InTransfer` owns the one active byte;
the sink owns accepted bytes. The required trace is 65 retry, 65 accepted,
66 terminal error, explicit recovery, 66 accepted. A terminal wrapper blocks
further callbacks until recovery. Recovery keeps the active byte and first
cause. `budget` bounds the actual producer's callback work; it is not a promise
that the callback itself cannot block. Report three separate state columns.

`CWFSpan` owns a copied span of at most16 bytes. `begin` refuses a busy owner or
invalid extent. `take` first validates and stages exactly the requested prefix
from a copy of the ring, then commits queue removal and the active span together.
Refusal preserves the original queue. The ring representation contains values
and indices, with no internal self-pointer; explain why this specific copy is
valid rather than assuming every container is copyable. Once active, accepted
bytes never return to the queue. Failure retains the same suffix and first
cause; explicit recovery resumes at that offset. `step` makes at most one
callback. Completion releases the active state. A zero-length admitted span is
already complete and is retired by the next `step` without invoking the writer.
Keep an active `CWFSpan` at a stable address: its nested transfer borrows its
own byte array, so copying or relocating the complete control would not rebase
that pointer. Caller-owned source bytes may change after successful copying.

For nine bytes `ABCDEFGHI`, derive remaining lengths9,6,6,4 and offsets0,3,3,5
under results3,EINTR,2,4. A zero result consumes one turn without moving the
cursor. A failure after3 retains offset3; recovery followed by2,4 delivers nine
bytes exactly once. Bound ordinary work by the callback budget plus at most16
copied/staged bytes. No heap allocation occurs. Account for the copied ring,
temporary span, active storage and metadata separately.

Normalize three interfaces deliberately: raw Linux negative errors, libc
`-1` with immediately captured `errno`, and the teaching callback's `-2` retry.
The same integer `-2` means different things at these boundaries. `CWFError`
contains category, operation, accepted count, first cause and separate first
cleanup cause. Extracting it performs no logging or I/O. An optional caller
reporter may copy the record; core failure propagation must remain correct
when that reporter is compiled out. Do not send diagnostics through the failed
writer or report that unsuccessful diagnostics were delivered.

## Verification and deliverables

Deliver D1 sealed transition/ownership predictions, D2 implementations plus
explicit producer dossier, D3 exact commands and expected/actual state traces,
and D4 the original changed tasks after their independent delay. Run `tests.c`
for the byte and span cases. `progress.c` checks normal, zero and terminal
progress, separate numeric conventions, first-cause retention, and actual
nonblocking raw/libc pipes. It alternates full/not-ready/EAGAIN with drained/
ready/progress three times, taking exactly one backend callback per step.
Its scripted EINTR input tests the adapter transition; it does not measure
asynchronous signal delivery. Its raw adapter is explicitly Linux x86-64.

The harmless requeue mutant must emit the duplicate sink65,66,65. The suffix,
short-count, retry-advance, cleanup-overwrite and product-cause controls must
compile and fail their intended value assertions. Compiler errors, crashes,
timeouts or a different sanitizer diagnostic are not substitutes. Run GCC and
Clang at O0/O3 and ASan/UBSan. A separate strict reference image composes the
actual queue/transfer providers with explicit entry and byte helpers, executes
the same byte ownership trace, and audits the complete symbol/map/disassembly
closure. Unused functions from the broader integration provider are removed
with explicit function-section garbage collection; disclose that narrower
linked surface.

For the modern comparison, apply the same3,EINTR,2,EIO trace to `CPWTransfer`
and actual `er_transfer` through `CPXProgress`. Compare offset5, first cause5,
cleanup9 and four callbacks. `ER_OK` denotes an invocation's progress, not
whole-transfer completion. Measure actual result/control sizes and generated
static branch sites, and count callbacks and adapter payload copies. These
are different cost observations; static sites are not runtime branch counts
or mispredictions. The adapter makes zero additional payload copies in this
comparison, while the copied-span adapter deliberately does copy. No language-
level automatic cleanup, exhaustive matching or shared `Result` layout follows
from a C tag and accessor discipline.

## Rubric, hints and variants

The original objective-specific rubrics govern acceptance. Level1 only shows a
successful write. Level2 additionally derives each ownership transition,
preserves exact accepted prefixes and causes, detects every applicable mutant,
and audits the actual dependency profile. Level3 requires the named independent
changed task after seven days; this author's campaign does not satisfy it.

Hint1: tabulate queue, active suffix, sink and first cause independently.
Hint2: count callback attempts, including zero/retry, rather than successful
bytes when proving a turn bound. Hint3: separate transfer acceptance from error
record publication; do not restart a completed prefix to simplify recovery.
Assessor implementations and analysis are withheld under
`assessor/checkpoint_followthrough/writer/` and `writer_analysis.md`.

The four delayed variants remain, without substitution: repeated EAGAIN and
readiness; one-byte to copied partial-span ownership; raw to libc backend with
separate dependency scope; and logger removal plus the actual PR-T65-03 product
comparison under equivalent error/progress semantics.

## Security and evidence limits

All inputs are synthetic; the only OS resources are this child process's local
pipes. It contacts no service and changes no host policy. Faults are original
teaching mutants whose success condition is a duplicate byte or wrong retained
state, never arbitrary execution. Repair and rerun the same regressions.
Author execution, external review, independent learner performance and physical
observations are separate dimensions. The current source-bound report and
guard determine which commands actually passed. Finite checks do not prove
absolute mastery, universal syscall behavior, liveness of arbitrary callbacks,
durability or signal timing.
