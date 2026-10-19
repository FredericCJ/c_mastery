# Foundation depth exercise contract, version 1

This contract applies only where an exercise explicitly links it. It adds required
depth to the named accepted project; it does not alter that project's accepted
identity, workload or historical evidence. Exercise metadata is canonical. Its
objective coverage describes specified assessment opportunities, not a passed
assessment or a claim of complete mastery.

## Prerequisites

Read the named parent project and its prerequisite module. Bring a source-bound
revision and contract of each named prerequisite exercise/artifact. Record an
adapter when an exercise interface refines an accepted component contract;
never relabel a different wire protocol as the accepted protocol. All slot
callbacks must return normally, must not reenter or mutate the slot, and must not
retain ownership of its storage or access it after reset/failed-initialization
cleanup. Callback context remains live until its final use. No previous
solution is supplied or assumed. Missing prerequisites reopen the corresponding
entry check instead of being hidden by copied assessor code.

## Environment

C17 is the implementation baseline. Use Linux x86-64 or Linux under WSL, GCC
and Clang with explicit `-std=c17 -Wall -Wextra -Wpedantic`, debug and optimized
builds. Record exact compiler, libc, CMake and Python versions. A required tool
that is unavailable leaves its observation open. Header availability and exact
integer widths are checked before building, not assumed from the machine name.

The reusable component has profile C: permitted type/limit headers and original
code, no direct external C-library or OS calls. Explicit allocator callbacks in
the lifecycle exercise are its only storage-acquisition boundary. Hosted tests
have profile A and may use Python arbitrary-precision arithmetic, allocation,
assertions and mature reference functions. An oracle dependency is never a
target dependency. Do not call an existing parser, checked-arithmetic library,
container, `memmove` or serializer to implement the mechanism being assessed.

Unless an individual operation explicitly permits an alias (for example a
subview replacing its parent), outputs are disjoint from input and control
objects. All referenced objects remain live and truthfully sized for the call.
Keep sanitized hosted builds separate from the ordinary dependency audit. For
any strict freestanding target claim, inspect objects, final ELF symbols, map,
link command and generated code. Exclude libc, libgcc, compiler-rt and libatomic;
original helpers need their own ABI/semantic checks. A fully resolved link can
still contain forbidden libraries. Compilation is not execution, and emulator
execution is not physical board validation.

## Reasoning

Before running tests, state the abstraction function, admitted representation,
preconditions, postconditions and transition invariant. Name each live object,
owner, borrower and invalidation event. State operation and temporary-storage
bounds with their assumptions. Distinguish ISO semantics, implementation-defined
choices, compiler extensions, ABI requirements and application contracts. A C
pointer-plus-length pair does not establish the pointer's actual provenance,
storage extent or lifetime. Do not use an invalid pointer merely to test whether
the component can diagnose it.

## Deliverables

In the initialized exercise workspace submit `implementation.c`, independent
tests, `reasoning.md`, `evidence.json`, `dependency-contract.md`, and
`transfer.md`. The supplied family source can be the implementation file if the
workspace names it differently; the evidence manifest records that exact path.
Each brief's D1–D4 labels name sections of these artifacts. Evidence records
source/build/input/environment identities, exact command, exit status, raw output,
oracle premises and the objective/criterion supported. Include a prediction
sealed before execution, a minimized counterexample, and a retained regression.
Do not fabricate source inspection, run results, timing, reviewer independence
or learner performance.

## Verification

Configure and build the copied laboratory with CMake, then run the exercise's
named CTest from metadata. Execute defined code only. Negative translation
sources are compiled separately; undefined snippets are analyzed rather than
given golden runtime outputs. Public tests are guided evidence. Independent
mathematical or state-transition models, generated tests and selected mutants
must expose specific possible errors. A compile failure alone does not count as
a mutant's semantic rejection. Compare old and repaired behavior under the same
admitted inputs and equivalent contracts.

All functional gates and mandatory written criteria must pass. A sanitizer pass,
finite exhaustive model, coverage number or fuzzing campaign proves only its
stated bounded result. Timing requires matched correctness contracts, repeated
samples, workload/target identity and observer costs. No speedup is required.

## Rubric

For every objective-specific criterion use 0 absent/materially wrong; 1 nominal
completion with unexplained gaps; 2 correct contract, boundary argument and
reproducible evidence; 3 independent adaptation and defense under the named
changed condition. Also score all six existing dimensions in
`curriculum/guides/assessment.md`: mechanism, correctness, memory/ownership,
resources/dependencies, failure/diagnosis, communication/transfer. All six need
at least 2 and at least two need 3. Objective criteria need at least 2; performance
cannot offset an ownership, arithmetic, authority or failure-handling defect.

Mark independent assessment pending until a reviewer evaluates a sealed new
attempt before releasing assessor material. Self-review is useful evidence but
is explicitly lower independence. After one to two weeks, attempt the delayed
variant without the earlier implementation or solutions. Record hints received
and reopen only claims invalidated by the result. Finite assessment does not
establish absolute mastery.

## Hints

Each brief has three graduated prompts. Record the highest level consulted.
Assessor solutions, mutation details and withheld challenges remain under
`assessor/`; learner packaging must exclude that tree and its generated copies.

## Security

All input is synthetic and all executable targets are original teaching programs
or learner implementations. The default laboratory is a local process with no
network, production data, credentials or privileged operations. A crash,
undefined execution or sanitizer report establishes neither arbitrary code
execution nor privilege escalation. Security-specific briefs supply their full
threat, containment and harmless success contract.

## Validation

Authored means a specification exists; executable means its build contract is
implemented; executed requires a source-bound receipt. Independent review,
learner assessment and physical validation are separate fields. The brief's
execution field points to a source-bound author campaign; this is distinct from
learner execution and assessment. Append-only receipts record actual outcomes
without rewriting historical acceptance evidence.
Exact C17 text remains an access gap. Inspected N1570 clauses are identified as a
C11 draft and accompanied by bounded C17 issue/correction records where relevant.
Additional effort is uncalibrated and is not charged against historical hours.
