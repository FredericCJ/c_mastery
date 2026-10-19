# Launcher failure schedules and descriptor transfer, version 1

This required successor supplies executable support for LRUN-03 and the original
criteria AX-PR-T23-01-T23.02.a, T23.02.b, T23.03.b and T23.03.c (each latter leaf
uses the same `AX-PR-T23-01-` prefix). It inherits the parent legacy assessment's
14-field specification and seven-day independent attempt policy. This document
adds exact executable inputs, contracts, controls and implementation deliverables;
it does not replace the original rubric or claim that passing tests proves mastery.
Families A, B, C, D, F, G, I and J apply to these failure and transfer mechanisms.

## Prerequisites, sources and environment

Retain the completed PR-T23-01 descriptor ledger, fork/wait implementation and
T23.02/T23.03 teaching. Bring a prediction for every result below. The bounded
primary source passages are registered under `R-LPOSIX-*` in
`research/v5/expansion/legacy_posix_followthrough/resources.json`. The inspected
Linux man-pages contracts distinguish ISO C17 objects and byte arithmetic from
Linux/POSIX fork, descriptor, signal and image replacement semantics. No new claim
about published C23 or an uninspected POSIX text follows from these Linux tests.

Use a disposable, unprivileged Linux process with GCC or Clang, libc and owned
temporary files/pipes. There is one child reaper, no concurrent descriptor
mutation, no caller cancellation, and no inherited SIGCHLD suppression or
interfering signal disposition/mask. Strings and arrays remain live for each call;
outputs do not alias inputs. The launcher is bounded in storage, not in child
execution time. The test process has a 15-second diagnostic deadline; timeout,
compiler failure, signal termination and sanitizer findings never pass a mutant.
No target service, credential, privilege, host policy or deployed equipment is used.

## Supplied inputs and required implementation

Implement `launch_stub.c` against `launch.h`. Keep `tests.c`, `original_tests.c`,
the two injection sources and `../tests/helper.c` as verification artifacts.
`original_tests.c` also tests the learner's retained `src/posix_lifecycle.c`;
it does not replace that implementation with a completed source. The helper
accepts `zero`, `127` and `signal`; the missing image is a checked nonexistent
sibling pathname. Synthetic file contents are exactly `ABC` and `xyz`.

`plx_launch(path, argv, options, out)` creates one close-on-exec pipe, forks one
child, closes the opposite ends, attempts the original helper with an empty
environment, reads the error channel and reaps that exact child before publishing.
`options.mode` is one of the six header constants, and `chunk` is 0 (uncapped)
or 1..8 bytes per read/write attempt. Other values return EINVAL before acquisition.
The deliberately faulty modes belong only to this teaching target:

| Mode | Transition | Required observation |
|---|---|---|
| NORMAL | exec the requested original image | exit0/127 or reported ENOENT |
| SETUP_FAILURE | report stage1 and EACCES | PLX_FAILED, EACCES, stage1 |
| SHORT_RECORD | write three bytes, then exit127 | EPROTO; output unchanged |
| UNKNOWN_STAGE | report stage99 | EPROTO; output unchanged |
| EARLY_EXIT | exit126 before exec/report | PLX_EXITED,126; no image proof |
| EARLY_SIGNAL | self-SIGTERM before exec/report | PLX_SIGNALED,SIGTERM |

The new stage record is exactly eight bytes: unsigned stage32 and errno32,
little-endian, decoded bytewise; valid stages are 1 setup and 2 exec. The local
Linux errno range is 1..4095. A full valid record also requires normal exit127;
empty channel plus an ordinary wait status reports only that wait status.
Unknown stage, invalid code or any nonempty extent other than eight is EPROTO.
The original `pl_launch` retains its native-int same-binary record. Its followthrough
test forces one-byte reads/writes, first-operation EINTR, an incomplete record,
and early death without changing the historical reference implementation.

Only the positive accepted prefix advances. Retry read/write/wait on EINTR;
short successful transfer is progress, not completion. Do not retry close.
Close every acquired end and reap after a protocol/read failure. No external
reaping or SIGCHLD policy change is permitted; arbitrary hard wait failure and
general launch cancellation require a distinct handle-retaining recovery design.
No output is published on launcher error. Child code may use only local byte
operations and the inspected async-signal-safe calls in its path; no stdio,
allocation or ordinary mutex calls occur between fork and exec/_exit.

`plx_read_all` and `plx_write_all` transfer 0..8 bytes and expose `used` and the
number of observed EINTR returns. Zero length succeeds without a syscall;
nonzero buffers and result cells are valid/disjoint. EOF before a complete read
is EPROTO, zero-progress write is EIO, other errors retain the accepted prefix.
`plx_wait` waits only for a positive child PID, retries EINTR, and does not claim
ownership of a child reaped by another participant.

## Redirection with aliases and partial failure

`plx_redirect(sources[2], destinations[2], fail)` borrows two live descriptors;
the two nonnegative destination numbers must differ and be below INT_MAX.
The source numbers may equal their destinations or cross them. First preserve
both original source open-file descriptions in close-on-exec temporary handles
above both destinations. Then atomically duplicate those snapshots onto the
destinations. Success clears FD_CLOEXEC on each resulting destination, including
the source-equals-destination case. Temporary handles are closed on every path.
Original caller descriptors are never blindly closed as cleanup.

`fail`0 uses real calls; 1/2 inject EIO before each snapshot acquisition, 3/4
before each destination replacement. Failures before replacement preserve both
destinations. Failure after the first replacement may leave that destination
changed: this is a child setup procedure, not an atomic two-descriptor transaction.
Its caller must report the setup error and _exit, not continue with mixed streams.
Do not implement atomic replacement as close-then-dup. Close-error evidence for
an overwritten destination needs its own retained handle and policy.

## Grandchild writer ownership

`plx_tree(writer, release, ready, repair)` is called only in the original forked
helper, owns these three distinct valid descriptors, forks exactly one grandchild,
closes its own copies, and waits for that exact child. Negative descriptors or a
repair value outside0/1 return EINVAL before transfer. The grandchild closes its
writer before publishing `R` when repaired; in the flawed experiment it holds
the writer until the parent's `G` release token. Both modes then close/reap fully.

The outer parent consumes `R` and EOF on the ready channel, establishing that
both helper levels closed that channel and the intermediate helper's writer.
Its nonblocking data read is EAGAIN in the flawed mode and EOF in the repaired
mode. After release and exact reap, both modes produce EOF and restore the
initial descriptor count. No sleep or PID ordering is used to infer enrollment.

## Verification, deliverables and assessment

The author producer `tools/run_posix_followthrough.py --report
reviews/v5/expansion/legacy_posix_followthrough/launch_validation.json` explicitly
selects separated assessor sources. Learners compile their own source instead:

```sh
cc -std=c17 -Wall -Wextra -Wpedantic -Werror ../tests/helper.c -o helper
cc -std=c17 -Wall -Wextra -Wpedantic -Werror -I. tests.c launch_stub.c -o tests
./tests channel ./helper
./tests stages ./helper
./tests redirect ./helper
./tests inheritance ./helper
./tests interrupts ./helper
```

Replace `launch_stub.c` with the learner's implemented file. The unchanged stub
must compile but fail the intended `PLX_CHECK`. The author producer also builds
the original API and deterministic linker wrappers; inspect its exact commands
to reproduce that variant. Optimized glibc may emit `__read_chk`; the wrapper
preserves its destination extent check and logs that injection actually ran.
Injected EINTR is reported separately from real SIGALRM interruptions.

The actual-signal test installs a process-local handler without SA_RESTART,
starts a 1ms interval timer, and holds a child operation for100ms. It requires
positive observed EINTR counts for blocked read, full-pipe write and exact-child
wait. It stops the timer, restores the prior handler, closes handles and reaps.
Under an adverse host schedule no observed interruption means test failure,
not permission to invent an interruption or quietly skip the case.

D1 is the descriptor/open-file-description and child-state ownership graph plus
predicted table. D2 is the original implementation and an async-safe call audit.
D3 includes exact compiler/link commands, raw ordinary/injected/real-signal
results, descriptor counts, every failure ordinal and a minimized failing control.
D4 is the original delayed criterion variant, attempted independently after the
required delay: stage-tagged short records, crossed sources, and a grandchild
writer are distinct transfers, not substitutions of constants.

Level0: missing/unsafe ownership or false execution claims. Level1: ordinary
launch works but short records, aliases or EOF are misclassified. Level2: each
criterion's specific boundary and negative control passes with the causal
argument, including the partial-failure guarantee. Level3: independent delayed
variant and changed-platform contract analysis agree with evidence. Record local
self-assessment separately from externally reviewed evidence; no reviewer is
manufactured by the runner.

First hint: distinguish what pipe closure proves from what wait status proves.
Second hint: draw edges from descriptor numbers to open-file descriptions before
changing any destination, and retain a handle for every edge still needed.
Assessor implementations and technical analysis are under the restricted
`assessor/posix_lifecycle_depth/followthrough` tree, never learner exports.

Validation is author reference evidence only. These bounded cases do not establish
all signals, schedulers, cancellation, malicious images, launch deadlines, general
multithreaded descriptor mutation or the learner's independent mastery. The
historical campaign remains a dated narrower observation; current evidence has
separate source pins and reports.
