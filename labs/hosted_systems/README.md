# Hosted Systems labs

Twelve bounded implementation units within existing project budgets. Start with learner src; the default build compiles but semantic gates reject unfinished functions. Assessor code is selected only through an absolute CMASTERY_ASSESSOR_ROOT. Passing these visible tests is finite mechanism evidence, never whole-project mastery.

Supported execution: Linux x86-64 (WSL Ubuntu included), C17, GCC or Clang, CMake and Ninja. Machine-image gates additionally require GNU binutils and the x86-64 SysV/Linux ABI. Hosted synchronization uses POSIX/Linux APIs. No physical MCU claim follows from these runs.

```sh
cmake -S labs/hosted_systems -B build/hosted_systems -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/hosted_systems
ctest --test-dir build/hosted_systems --output-on-failure
```

Select one CTest name with `ctest -R "^family.case$"`; use the actual names below. Work in a copied learner workspace when editing. Do not consult assessor implementations before an independent attempt.

## LAB-HS-01: Owned descriptor from a borrowed handle
Parent: `PR-T23-01`. New work: 2 hours within its existing budget. Tests: `hosted_systems.descriptor`.

Duplicate a real file descriptor while preserving the borrowed handle and shared offset.

1. Define borrowed and owned descriptor roles.
2. Implement hs_dup_owned with atomic close-on-exec duplication.
3. Close the owned descriptor and observe the original offset.

Rejecting boundaries: FD_CLOEXEC set; different descriptor shares offset; invalid descriptor rejects.

Still required: Exec inheritance, descriptor census and failed-redirection rollback remain.

Assigned source passages:
- `R-SYS-LINUX-MAN-6-19` — fcntl(2) — F_DUPFD_CLOEXEC; dup(2) — shared open file description and offsets

## LAB-HS-02: Bounded partial file transfer
Parent: `PR-T23-02`. New work: 2 hours within its existing budget. Tests: `hosted_systems.copy`.

Copy an actual fragmented pipe stream to a real file with a caller-set byte ceiling.

1. Implement hs_copy using bounded storage.
2. Track bytes actually written when failure interrupts a chunk.
3. Reuse the transfer contract on EOF and /dev/full.

Rejecting boundaries: 600 bytes from many writes; seven-byte bound; zero work; bad input and ENOSPC preserve accepted count.

Still required: Forced EINTR/short output scheduling, pathname races, positioned offsets and durability remain.

Assigned source passages:
- `R-SYS-LINUX-MAN-6-19` — read(2) and write(2) — short transfers, EINTR, EOF and error return contracts

## LAB-HS-03: Monotonic deadline and cancellation
Parent: `PR-T24-05`. New work: 2 hours within its existing budget. Tests: `hosted_systems.deadline`.

Poll real input/cancellation descriptors against an absolute monotonic deadline.

1. State the single-reader ownership rule.
2. Implement hs_read_until with cancellation winning simultaneous readiness.
3. Recompute remaining duration after interruptions.

Rejecting boundaries: Empty input times out; ready input reads; cancel leaves simultaneously ready data untouched; bad descriptor rejects.

Still required: timerfd/eventfd accumulation, rescheduling, callback execution latency and independent races remain.

Assigned source passages:
- `R-SYS-LINUX-MAN-6-19` — poll(2) — readiness, return values and EINTR; clock_gettime(2) — CLOCK_MONOTONIC

## LAB-HS-04: Child exit and complete reap
Parent: `PR-T23-01`. New work: 2 hours within its existing budget. Tests: `hosted_systems.process`.

Create and reap real child processes, retaining full parent-side ownership of their status.

1. Implement hs_spawn_exit.
2. Distinguish raw wait status from decoded exit code.
3. Verify that no child remains waitable after success.

Rejecting boundaries: Exit codes 0 through 255; reject 256 before fork; leave output unchanged on rejected input.

Still required: exec error channels, multithreaded fork restrictions, abnormal termination and pipeline cleanup remain.

Assigned source passages:
- `R-SYS-LINUX-MAN-6-19` — fork(2) — descriptor inheritance; waitpid(2) — WIFEXITED and WEXITSTATUS; _exit(2) — status

## LAB-HS-05: Shared mapping with offset records
Parent: `PR-T23-03`. New work: 2 hours within its existing budget. Tests: `hosted_systems.mapping`.

A real child publishes a record into MAP_SHARED storage using fixed-width offsets.

1. Validate offset plus length without overflow.
2. Write fixed-width metadata and payload in the child.
3. Use successful wait as completion before the parent reads.

Rejecting boundaries: Shared bytes visible after reap; adjacent canaries intact; header overlap and overrun reject; zero-length tail record allowed.

Still required: Cross-endian format, unrelated process attachment, resizing, concurrent publication and crash ownership remain.

Assigned source passages:
- `R-SYS-TLPI-2010` — §49.2 Creating a Mapping: mmap(), pp. 1020–1023
- `R-SYS-LINUX-MAN-6-19` — mmap(2) — MAP_SHARED and anonymous mapping; waitpid(2) — child completion

## LAB-HS-06: Framed pipeline reader
Parent: `PR-T23-04`. New work: 2 hours within its existing budget. Tests: `hosted_systems.frame`.

Read a big-endian length-prefixed binary message from actual pipe writes and detect truncated EOF.

1. Implement hs_read_frame by reusing a short-read loop.
2. Keep output length unchanged on a rejected frame.
3. Declare close-on-protocol-error as the caller policy.

Rejecting boundaries: Binary zero bytes; one-byte writes; zero-length frame; oversized and truncated body/header reject.

Still required: Terminal restoration, writer backpressure, broken-pipe policy and full multi-process pipeline remain.

Assigned source passages:
- `R-SYS-LINUX-MAN-6-19` — pipe(7) — byte stream, capacity and EOF; read(2) — short reads

## LAB-HS-07: Masked signal handoff
Parent: `PR-T24-01`. New work: 2 hours within its existing budget. Tests: `hosted_systems.signal`.

Generate and synchronously consume real SIGUSR1/SIGUSR2 while restoring the original mask.

1. State exclusive ownership of the chosen signal and absence of a preexisting pending instance.
2. Block before generating and waiting.
3. Restore the mask only after consuming the notification.

Rejecting boundaries: Both allowed signals arrive; SIGKILL rejects; mask unchanged; no notification remains pending.

Still required: Thread-wide mask coordination, handler safety and self-pipe/signalfd tradeoff defense remain.

Assigned source passages:
- `R-SYS-LINUX-MAN-6-19` — signal(7) — pending signals and masks; pthread_sigmask(3) — mask preservation; sigwaitinfo(2)/sigtimedwait(2) — synchronous consumption

## LAB-HS-08: Local message ownership
Parent: `PR-T24-02`. New work: 2 hours within its existing budget. Tests: `hosted_systems.ipc`.

A real AF_UNIX SOCK_SEQPACKET child transforms a bounded binary request and replies.

1. Implement socketpair ownership across fork.
2. Reverse the payload without changing message boundaries.
3. Close unused endpoints and reap on every parent completion path.

Rejecting boundaries: 64-byte binary request; zero message; capacity and oversize reject; exact response bytes.

Still required: Addressed server lifecycle, peer credential policy, interrupted calls and unexpected peer failure remain.

Assigned source passages:
- `R-SYS-LINUX-MAN-6-19` — unix(7) — SOCK_SEQPACKET message boundaries; socketpair(2), send(2) and recv(2) — return values

## LAB-HS-09: Small readiness reactor
Parent: `PR-T24-04`. New work: 2 hours within its existing budget. Tests: `hosted_systems.reactor`.

One bounded turn per borrowed pipe collects one byte from each source without closing it.

1. Track completion separately from descriptor identity.
2. Service readiness before EOF when both are reported.
3. Publish the sum only when every source has completed.

Rejecting boundaries: Different ready order; EOF without byte rejects; borrowed descriptors remain open; zero source count rejects.

Still required: Generation-tagged registrations, stale epoll events, fairness, partial requests and load shedding remain.

Assigned source passages:
- `R-SYS-LINUX-MAN-6-19` — poll(2) — POLLIN/POLLHUP and readiness handling; read(2) — EOF

## LAB-HS-10: Bounded worker ownership
Parent: `PR-T25-01`. New work: 2 hours within its existing budget. Tests: `hosted_systems.pool`.

Join at most four real workers to reduce at most 256 immutable values into private partial sums.

1. Seal shared-index versus static-partition decision before implementation.
2. Implement hs_pool_sum with cleanup for a partial create failure.
3. Compare uneven input with one through four workers.

Rejecting boundaries: Every worker count; zero items; negative values; invalid capacities leave output unchanged.

Still required: Creation-failure injection, speedup/locality measurement and portable C thread comparison remain.

Assigned source passages:
- `R-SYS-POSIX-THREAD-LIFECYCLE-2024` — pthread_create — DESCRIPTION and error returns; pthread_join — completion and resource lifetime
- `R-SYS-POSIX-MEMORY-2024` — §4.15.2 Memory Synchronization — thread creation, mutexes and joining

## LAB-HS-11: Condition queue drain and shutdown
Parent: `PR-T25-02`. New work: 2 hours within its existing budget. Tests: `hosted_systems.queue`.

A four-slot real pthread queue uses predicates and wakes blocked producers/consumers on close.

1. Implement queue operations under the shared invariant.
2. Define close as reject-new-work then drain-existing-work.
3. Broadcast shutdown and join users before destroy.

Rejecting boundaries: 1000 item stream; spurious broadcast; close empty blocked reader; close full blocked writer; FIFO drain; repeated close.

Still required: Timeout semantics, multiple-producer fairness, race detector and independent ordering argument remain.

Assigned source passages:
- `R-SYS-POSIX-COND-2024` — RATIONALE — Condition Wait Semantics: predicates, spurious wakeups and associated mutex
- `R-SYS-TLPI-2010` — §30.2.3 Testing a Condition Variable’s Predicate

## LAB-HS-12: Clock domains and child resource limit
Parent: `PR-T23-06`. New work: 2 hours within its existing budget. Tests: `hosted_systems.clocks`.

Observe actual wall/process clocks and enforce RLIMIT_FSIZE only in a child.

1. Measure nanosleep using both clock domains.
2. Inspect the real descriptor soft limit.
3. Install a child-only file-size ceiling and capture EFBIG without changing the parent.

Rejecting boundaries: Clock resolutions positive; wall delay observable; actual file stops at eight bytes; parent resource limits unchanged.

Still required: Affinity/scheduler privilege campaigns, cgroups and real-time guarantees remain separate evidence.

Assigned source passages:
- `R-SYS-LINUX-MAN-6-19` — clock_gettime(2)/clock_getres(2) — CLOCK_MONOTONIC and CLOCK_PROCESS_CPUTIME_ID; nanosleep(2) — interruption; getrlimit(2) — RLIMIT_NOFILE, RLIMIT_FSIZE and SIGXFSZ

## Component exports

Each reusable contract has a separate static library target and declared public symbols in family.json. Observation/ELF reports are claims, not library exports. Artifact imports are intentionally empty for these introduction units: the later integration consumers must explicitly link the resulting libraries and check the required producer evidence. Do not treat pedagogical ordering as a code dependency.
