# T25.01 actual source inspection

On 2026-10-17 the author read the complete T25.01 teaching, PR-T25-01
contract, all six current original/changed criterion tasks, the lifecycle
laboratory contract and actual pl_pool implementation, its concrete tests and
the broader hosted_systems pool test. The original synthetic PR-T25-01 closure
fixture was inspected: it is not a pthread lifetime oracle. The new tests
therefore check thread ownership directly, without rewriting that accepted
fixture or its historical identity.

Fresh primary reads used Linux man-pages HTML and WG14 PDFs:

- pthread_create: declaration, successful/failed handle publication, direct
  return status, scheduling and joinable/detached notes.
- pthread_join: joinable/exclusive handle requirement, canceled joiner,
  result and error status, final termination and repeated-join limits.
- pthread_detach: system resource lifetime, repeat-detach caveat, detached
  attributes and absence of a join operation.
- pthread_cancel: request versus completion, deferred/disabled cancellation,
  cleanup order and joined canceled result. The sleep-based example is not
  copied as an enrollment protocol.
- pthread_cleanup_push/pop: cancellation/exit/pop cases, lexical pairing and
  early-leave restrictions. The macros are not modeled as ordinary functions.
- pthread_cond_init family: atomic unlock/wait/relock and cancellation's
  reacquisition. This page includes historical LinuxThreads descriptions;
  those and its broad historical error claims are not assumed to describe
  current glibc. Only the listed wait/cleanup contract is used here.
- N1570 7.26.1 paragraphs1–5, 7.26.5.1 and 7.26.5.6: actual C11 draft
  passages on optionality, signatures, creation status and join; 7.13.2.1p3
  was subsequently read for changed automatic locals across a nonlocal return.
- N3096 6.10.9.3 macro entry, 7.28.1 paragraphs1–5, 7.28.5.1 and
  7.28.5.6: actual April2023 draft cross-check of optionality and C thread
  interfaces. Cover/version was inspected. It is not the final published C23.
- Installed WSL `/usr/include/pthread.h` lines674–710 and actual Clang C17-mode
  preprocessed creator: cleanup uses a saved nonlocal-return environment in
  this C translation. Header SHA256 is
  `5479c95276c6693fdd1baa448f5d457dee57fc02b31277bfa04df63cc0b26a94`.
  The compiler is identified by the campaign, not by the C17 language-mode
  number; this is installed implementation evidence, not a POSIX guarantee
  that every cleanup implementation uses the same machinery.

Open Group Issue8 pthread_cond_wait and pthread_join requests returned403;
the earlier Issue7 condition URL also failed. Exact published C17 and C23
text remains unavailable in this wave. No private book passage was newly
read or inferred from bibliography. The later-edition delayed task remains
partially open for that specific exact-text comparison; actual diagnostics
and draft comparisons still proceed and retain their narrower claims.

Implementation rationale: the first remaining failure was a join error with
stack-backed worker cells, so the successor retains a heap control block and
the consumed-prefix boundary. Detached ownership then needs a separate
monitor whose life is independent of the creator. Cancellation is admitted
only after cleanup is installed. These decisions are original exercise
designs derived from the source contracts, not copied library algorithms.
