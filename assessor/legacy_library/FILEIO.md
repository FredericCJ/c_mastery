# Withheld FILE and adapter analysis

The buffer offset is the sole authority for replay. Each successful writer
prefix advances both offset and cumulative committed count; an input read can
advance consumed count earlier. A read result containing data plus ERROR/EOF
must be remembered until those bytes drain. Refuse zero-progress MORE and invalid
count/status combinations. Check callback budgets including retries. A state
with unwritten data cannot admit another read. The replay mutant is rejected by
the bounded trace, not by invoking undefined behavior.

The original f_write_all treats zero as terminal despite describing no progress;
it has no resumable zero status. Preserve that actual contract and map EAGAIN to
-2. A status-plus-count writer is a different representation: see the real T19
SVWriteResult successor. Do not claim the FWriter adapter can preserve an
arbitrary simultaneous error at a full acceptance boundary. A student's richer
adapter is acceptable when its semantics and consumer integration are explicit.

The close callback consumes an owned FILE even on error; mark ownership absent
before invoking it and never inspect the old pointer afterward. A borrowed
stdout remains usable. A synthetic close callback makes the retained-handle
mutant fail safely without double-closing libc storage. Actual fopencookie then
separately verifies the libc boundary, including GNU's nonnegative writer return.
These are original teaching cookies, not faults injected into third-party data.

ISO's input fflush restriction differs from POSIX's seekable active-handle rule.
The actual harness labels that extension, never executes a prohibited mixed
read, and uses ftello/fflush/fseeko to transfer activity between the two FILEs.
The d/e result alone is insufficient: require the learner to derive why each
transition is permitted and why a distinct buffer does not imply a distinct
open-file description. A POSIX no-data-loss writing guarantee does not imply
that every input sees data exactly once under all implementations.

The pipe experiment fills a new owned descriptor until EAGAIN, records no
committed user bytes, removes the known filler and resumes. It must not drain
user bytes accidentally or use seek as hidden recovery. A terminal write at
prefix1 reports1; no generic retry-from-start is valid. A real blocking callback
could exceed a wall-clock deadline despite a small callback budget.

All ordinary and changed tests are separate from independent learner attempts.
For delayed transfer supply public contracts, the original source fixtures and
a fresh permitted pipe/handle arrangement, withholding this analysis and the
reference adapters. Preserve the original seven-day IDs and full rubric. A
nominal abc copy earns only superficial credit; the causal ownership/progress
and layer arguments distinguish understanding from reproduced output.
