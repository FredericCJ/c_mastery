# T23–T29 depth audit and expansion decisions

This is an author passage review, not independent human review or a new acceptance
of the historical edition. The accepted modules, project briefs, unit identities,
hours and receipts remain unchanged. The expansion has no learner-time ceiling.

The review read the seven modules' assigned teaching passages and all 40 projects'
contracts, steps, failure cases, acceptance requirements and transfer work. It
checked the complete 40 synthetic fixtures, the matching oracle branches, unit
objective/test links and the first substantive unit steps. It also read the
hosted-system, firmware-I/O and execution laboratory contracts and their relevant
tests. Selected assessor implementations were inspected to establish what those
tests actually implement: hosted process, pool, queue and mapping, and execution
SPSC, hazard queue, collection and epoch transitions. This was not a fresh review
of every historical implementation or learner portfolio.

## What existed, and what it establishes

The teaching is substantive and the projects generally contain concrete
construction and failure requirements. The principal gap was not absent titles:
topic checklists and general transfer prompts did not provide separate leaf-level
rubrics, counterexamples, expected arguments and delayed changed-condition
assessments. Forty required supplements now supply 258 project-specific criteria
over 240 unique objectives. The 18 repeated T26 objectives have distinct
awareness and construction parents; their additional construction criteria demand
source, controlled schedules or finite models, and generated dependency evidence.
They are not counted as 18 new topics.

The retained hosted laboratory provides useful but bounded anchors. Its process
helper forks and exits, without replacing the image or distinguishing exec failure
from a legitimate status 127. Its pool handles a creation failure in source but
the visible test does not force every partial acquisition position. Its mapping
test uses a fork-inherited shared mapping and a native-endian header after wait;
it does not demonstrate unrelated-process attachment or concurrent publication.
The bounded queue tests closure and wakeups, but a sleep before a broadcast is not
proof that the target waiter reached a particular enrollment point. Generic
closure fixtures used for several OS projects do not establish those OS behaviors.

The execution family has real SPSC and MPMC mechanisms, not merely headings. The
inspected SPSC implements both publication and reuse release/acquire edges. The
hazard queue has protection validation, bounded storage, retirement incarnations
and permanent retirement at tag exhaustion. Its four-thread exact-once stress is
finite evidence, not a linearizability proof for every history. Epoch metadata is
externally serialized; the simplified RCU model is not Linux RCU. These existing
contracts are reused explicitly, while their restrictions remain assessed.

The firmware-I/O family has valuable side-effect models, including separate SPI
TXE/RXNE/BSY milestones, I2C recovery and DMA stop ownership. Host models do not
measure the electrical interface, prove a selected board's reset path or validate
cache/device visibility. The new criteria require exact target identity, link
closure, observation boundaries and target-specific failure arguments instead of
promoting model tests to hardware coverage.

## New executable work

`labs/posix_lifecycle_depth` supplies two complete, inspectable original teaching
targets with learner stubs, public contracts, independent fixtures, semantic
tests and separated assessor implementations. No accepted unit is replaced.

The launcher implements a close-on-exec error pipe, exact-child reap and distinct
normal exit, signal termination and reported exec failure outcomes. Its controls
reject ignoring the error channel and conflating status 127 with exec failure.
Tests include acquisition failure before pipe creation and after pipe acquisition,
descriptor-count restoration and no remaining child. The contract explicitly
limits what EOF proves if the pre-exec protocol itself is interrupted or reporting
fails. Forced EINTR, partial error-record publication and a general child deadline
are still separate required investigations.

The thread anchor creates at most four actual POSIX threads, injects failure at
every creation position, joins exactly the successful prefix and checks each
worker result. Its controls reject ignoring the requested failure and omitting a
joined worker's result. Storage remains live through join; detached workers and
recovery after a join failure require different ownership contracts. A join
failure under the teaching preconditions terminates that teaching process; it
is not presented as a reusable production recovery policy.

The source-pinned campaign records eight profiles: GCC and Clang debug, release,
ASan/UBSan and TSan. Each has two ordinary anchors and four discriminating compiled
mutants. The generic producer also verifies that both learner stubs fail and the
independent six-case prefix-sum oracle agrees. Only the recorded observations are
claimed; the complete 258 learner assessments remain unattempted.

Both actual project exports were initialized outside the repository, with full
learner criteria and their laboratory contract. Their unsolved sources compiled
and reached semantic assertions, and no assessor source was copied. Nine export
controls rejected direct assessor paths, traversal, absolute paths, source and
destination symlink escapes, directories, missing files and noncanonical path
separators. These are packaging checks, not completed learner exercises.

## Evidence and integration boundaries

Fourteen newly inspected primary resource records identify actual passages,
inspection limits and PDF hashes where applicable. Previously inspected B06 and
legacy-systems passages are reused by reference, not duplicated as new resources.
N1570 is a C11 draft; it is not described as the published C17 or C23 standard.
RM0090 Rev21 is historical. Uninspected current datasheets, board schematics,
errata, vendor startup pairs, broader hazard/epoch/RCU proofs and exact selected
kernel-model passages remain in the gap ledger.

The leaf-specific task, expected argument, two hints, four-level rubric and
seven-day changed-condition variant reside in each canonical project supplement.
Assessor arguments remain under `assessor/legacy_depth/`. Source/topic-to-objective,
artifact and rubric links are emitted as `traceability.json`. Per-domain audits
and remaining-work records preserve the distinction between authored opportunity,
executed assessor anchor and actual learner evidence.

No missing source, physical experiment, guest execution or delayed assessment is
made optional to declare completion. The T26 construction, T57 kernel and T62
advanced-I/O links are required expansion work even where historical text used
optional language. The accepted historical metadata remains an unchanged record.
