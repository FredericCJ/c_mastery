# Composition completion source inspection

On 2026-10-17 the author opened and read the named sections of Linux man-pages
6.19 close(2), poll(2) and pthread_join(3), recorded in completion_resources.json.
The close discussion distinguishes Linux descriptor release from later error
reporting and explains the danger of retry after descriptor reuse. Its POSIX
edition discussion does not make Linux EINTR behavior universal. The poll
description separates readiness from completion and keeps buffered data distinct
from hangup. The join description and notes establish single-joiner termination
before reclaiming thread-associated storage. Only these passages were inspected;
two read(2) retrievals timed out and the POSIX read page returned HTTP403.

The authored adapter uses actual local nonblocking socket reads and a polling
worker, joined before cleanup. It does not claim syscall cancellation or a
general concurrent coroutine API. The close experiment opens and closes local
/dev/null descriptors; pre-release retention and post-release diagnostics are
injected outcomes, not observed kernel failure or durable filesystem behavior.
Actual source-bound compiler/sanitizer observations live in the new completion
campaign, separate from prior historical receipts and learner assessment.

The first seven-profile run passed all correct implementations and all six
ordinary profiles. Its TSan mutant7 failed the intended quiescence assertion,
but an unjoined intentional worker produced an exit-time thread-leak diagnostic
instead of the required exit1. The harness now records the failed invariant,
joins the stopped worker solely for containment, then fails the same assertion.
The final seven-profile campaign passed. This correction did not relax mutant
acceptance or suppress sanitizer checks.
