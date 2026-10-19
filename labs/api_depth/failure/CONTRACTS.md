# Shared T21 executable successor contract

This required extension implements the existing thirty T21.01–10 leaf criteria
and their thirty changed-condition variants. It preserves PR-T21-01..04, original
AX identities, minimum-seven-day assessments, historical hours and acceptance
records. Each original criterion supplies its stable ID, family, purpose,
prerequisites, D1–D4 deliverables, rubric and delayed task; the named packet below
supplies concrete executable inputs, representations, failure semantics and bounds.

| Existing criterion range | Learner teaching and code | Private assessor package |
|---|---|---|
|T21.01–02|TAXONOMY.md; taxonomy/public_error/error_abi|taxonomy.c/.py, error_abi.c, TAXONOMY.md|
|T21.03–04|CONSTRUCTION.md; construction|construction.c/.py, CONSTRUCTION.md|
|T21.05|TRANSACTION.md; transaction/publication|transaction.c/.py, publication.c, TRANSACTION.md|
|T21.06|CANCELLATION.md; cancellation|cancellation.c/.py, cancellation_threads.c, CANCELLATION.md|
|T21.07–09|OBSERVABILITY.md; observability/diagnostics|observability.c/.py, diagnostics.c, process/fault probes, OBSERVABILITY.md|
|T21.10|CRASH.md; crash|crash.c/.py, crash_site.c, CRASH.md|

The packet is C17. N1570 and N3096 citations retain their actual draft editions;
only the explicit assertion comparison uses the latter. Newlib headers for the
Arm cross-translation are compile-time declarations/macros, not permission to link
newlib. The strict core audit retains every named public core function in actual
Cortex-M4 O0/O2 and x86-64 ClangO2 images. Original T11 byte helpers and narrow
original Arm EABI glue satisfy compiler-generated copies/clears. Inspect object
imports, all final symbols, the map, disassembly and stack files. Any remaining
libc/libgcc/compiler-rt/libatomic import fails the strict claim. No target image
execution or board measurement follows merely from a successful audit.

Hosted changes are explicit profile extensions: publication uses pthreads and
heap allocation; fault policies use isolated original processes; symbolization
uses selected GNU/Linux ELF, dladdr and binutils; Python is an author-side oracle.
None belongs to the strict core runtime. Record actual tool versions and inputs.
Use octets and unsigned types with at least17 value bits for the selected clock
implementation. All fixed-capacity handles and buffers have stable live storage;
public C structs are not unforgeable capabilities. Bounds are runtime checks,
ownership/serialization remains an API obligation, and type diagnostics cannot
prove these contracts. No library may implement the taught mechanism on behalf
of a first-principles submission.

Start with `cmastery init PR-T21-01 <workspace>` (likewise02–04). This exports
public packets and intentional compiling stubs without assessor implementations.
Reuse your separately validated prerequisite provider artifacts; do not mistake
fresh prerequisite stubs for implementations. A simple independent unit command
from the exported root is:

```sh
cc -std=c17 -Wall -Wextra -Werror -O2 -I labs/api_depth/failure \
  labs/api_depth/failure/transaction_tests.c \
  labs/api_depth/failure/transaction.c -o transaction-test
./transaction-test
```

A fresh stub must fail. Other packet tests add the named provider implementations
and include paths. Hosted publication adds `-pthread`. The private campaign is
`python3 tools/run_failure_followthrough.py --module taxonomy --module construction
--module transaction --module cancellation --module observability --module crash
--module target --report reviews/v5/expansion/legacy_failure/current_validation.json`.
It uses identified assessor providers and cannot confer learner credit. The
source-bound guard validates exact positive profiles, semantic negatives,
observations, source/output identity and required changed-condition experiments.
An older generic project smoke check is not evidence for these new packets.

For every criterion submit a prediction before execution, contracts and invariants,
implementation, failure/ownership trace, independent counterexample or oracle,
exact command/source/tool identities and a limitations statement. The assessor
grades each original0–3 rubric separately: superficial nominal reproduction is1;
correct explanation plus relevant evidence is2; changed-condition independent
transfer is3. Hints progress from representation/owner identification, to the
commit or last-dereference boundary, to a small discriminating example. Withhold
private reference bodies and delayed technical analyses. Do not administer a
delayed attempt until the original minimum interval has elapsed.

All fixtures target original curriculum code, local processes and synthetic data.
The deliberate assertion defect, missing pin and corrupted records are assessment
specimens; do not repair them out of the negative campaign. No external target,
host security change, deployed device or actual hardware fault is authorized by
these exercises. A crash, sanitizer report or modeled invariant violation is not
an arbitrary-code-execution claim.

Status and evidence: scoped and full results live under
`reviews/v5/expansion/legacy_failure`; active exact bindings are in
`research/v5/expansion/legacy_failure/support.json`. Source access limits remain in
`source_access.json`. The six-dimensional disposition records distinguish teaching,
implementation, author execution, external review, independent/delayed learning
and physical observations. Finite test matrices do not prove absolute mastery.
