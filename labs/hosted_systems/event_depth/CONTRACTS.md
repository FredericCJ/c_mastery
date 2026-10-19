# Required T24 event-system successor

This package extends the existing PR-T24 criteria rather than replacing their
identifiers, family selections, prerequisite records, rubrics or seven-day delayed
assessments. The exact ordinary and changed tasks remain in
`curriculum/v5/expansion/legacy_assessments/PR-T24-*.json`. An observation covers
only the criteria listed in its current source-bound support record. Historical
elective wording for T24.12 does not exclude advanced I/O from comprehensive scope.

The active implementation language is C17. This is hosted profile A with explicit
POSIX/Linux ports, libc and pthreads. It is not a strict freestanding submission.
Original queue, framing, ownership and scheduling mechanisms must be implemented
from their representations; calling a library that supplies the taught mechanism
does not satisfy construction. Kernel interfaces used as transport are declared
dependencies. Python test orchestration, procfs enrollment observation and object
inspection are author-side tools, not target components.

The initial signals packet is `SIGNALS.md`, `signals.h`, the compiling learner
stub `signals.c`, and `signals_tests.c`. Its six leaves are T24.01.a–c and
T24.02.a–c. Further T24 packets retain separate scope until implemented. The
private reference bodies, adversarial probes, delayed analysis and campaign
driver live under `assessor/hosted_systems/event_depth`; do not export them to
learners. A fresh stub is expected to compile and fail its behavioral test.

From an exported project, compile your implementation with:

```sh
cc -D_GNU_SOURCE -std=c17 -Wall -Wextra -Werror -fno-stack-protector -O2 -pthread \
  -I labs/hosted_systems/event_depth \
  labs/hosted_systems/event_depth/signals_tests.c \
  labs/hosted_systems/event_depth/signals.c -o signals-tests
./signals-tests
```

Each criterion requires a sealed prediction, representation and ownership
argument, implementation or explicit premise-based analysis, exact evidence with
tool/source identity, a relevant negative control, and its separately administered
changed-condition task. Grade the original 0–3 rubric per leaf: a reproduced
nominal trace is at most superficial completion; independent explanation,
falsification and changed-condition transfer are assessed separately. Finite
campaigns do not prove arbitrary schedules, all platforms or absolute mastery.

Isolation: use only original processes created by the exercise, process-local
descriptors and synthetic inputs. Send signals only to the exercise process or
its known live children. Do not enumerate or signal unrelated processes. Per-child
resource exhaustion and watchdogs are permitted within that child; no host policy,
privilege, kernel configuration or external target changes are part of the lab.
Record permission failures as unavailable evidence, never substitute timing
guesses for a mandatory enrollment check. An unsafe handler analysis is not an
arbitrary-code-execution claim. Do not execute its undefined operations as if they
had portable expected output.

Actual author observations are versioned under
`reviews/v5/expansion/legacy_events`. Teaching, code, author runs, external review,
independent/delayed learner attempts and physical observations are separate states
in the active support and repository work inventory. No author run fills the last
three dimensions automatically.
