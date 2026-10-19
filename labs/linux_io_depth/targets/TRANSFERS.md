# Required T62 progress, scheduling and submission transfers

This continuation retains PR-T62-01/02/03/05/06/08 and its exact a/b/c objectives.
The shared14-field contract is `curriculum/v5/contracts/b06.md`; families
A/B/C/D/F/G/H/I/J apply. Start with the learner vector-progress cursor,
readiness loop, raw-ring implementation and ownership ledger. Supplied
`transfer.h`, `transfer.c` and `transfer_test.c` are public; the assessor solution
and analysis are separate. C17 hosted, Linux, POSIX threads and the explicitly
named UAPI are dependencies. Neither liburing nor a queue library may implement
the assigned mechanisms. Pthread synchronization itself is permitted.

## IO5: partial progress survives interruptions and failure

Implement `it_progress` for0..8 live ordered iovecs, total<=SSIZE_MAX, a byte
cursor initially0 and the callback in the header. Return0 complete,1 suspended,
or negative errno. Build a bounded temporary vector of the remaining suffix;
skip zero spans without null-pointer arithmetic. Positive callback results
advance exactly that prefix. EINTR retries without advancement; EAGAIN suspends;
other negative errors terminate this invocation while preserving the accepted
prefix. Zero progress on a nonempty suffix, or more than requested, returns EIO.
At most16 callback attempts per invocation bound interruption spinning, with
return1 if incomplete. Invalid count/extent/cursor rejects unchanged before any
callback. Valid source spans remain live and immutable across resumptions;
the callback returns valid negative errno or an actual accepted count, cannot
retain temporary iovecs or reenter the operation. Control objects are disjoint.

D1: code, prefix invariant and failure ledger. The script2,EINTR,EAGAIN,1,EPIPE
over ABC/empty/DEFG must stop at prefix2 then3, with bytes ABC exactly once;
resumption outputs DEFG. Explain why restarting at vector0 corrupts the stream
even when the final count looks plausible. This callback test injects results;
the adjacent COMPLETIONS exercise supplies actual partial writev/splice evidence.
Cost is O(attempts×8) and fixed stack, with no allocation.

## IO6: fair edge-triggered readiness and two deadline policies

Implement `ITLoop`: two ready bits, alternating selection, monotonic unsigned
nanoseconds, start/last/now and positive idle/total limits. Valid initial state
has start<=last<=now, closed0, next0/1. Earlier time rejects EINVAL unchanged.
At or beyond either limit close before accepting new progress; positive progress
refreshes last but never start. Empty wakeups refresh neither. Terminal state
stays closed. A ready bit remains set after a budget-limited visit and is removed
only on actual EAGAIN/end-of-stream/error handling. Selection costs O(2).

D2: actual epoll integration with two nonblocking synthetic pipes, budget4bytes,
an application ready set, own eventfd, blocked SIGUSR1/signalfd and timerfd.
PeerA supplies32bytes while peerB supplies1; B must receive service within two
visits. Consume the eventfd count, signalfd record and timer expiration. Restore
the process signal mask after consuming the owned signal. Retain the observed B
latency as raw nanoseconds, not a hard real-time guarantee. Independently test
idle5,total12 with progress at0/4/8: total still closes at12, and progress exactly
at an expired deadline cannot resurrect the operation. Show how treating a
budget boundary as EAGAIN loses an edge; continuously draining A can starve B.

## IO7: multiple producers, a non-power-of-two queue, one ring issuer

Implement a capacity3 queue of copied uint64 values using one mutex and two
condition variables. Head/count are bounded indices; modulo3 is not bitmasking.
Push waits while full, pop while empty, with predicates rechecked after wakeup.
Close is terminal and wakes all waiters; pushes then reject, pops drain existing
values before returning0. Initialization unwinds a failed condition-variable
construction. Destroy requires all users joined and no waiter; thread cancellation
and owner death are outside this contract. Mutex/condition calls operate on
valid initialized objects and are assumed to succeed after initialization.

D3: four actual producers submit100 unique values each. One consumer owns the
raw-ring issuer and sends one NOP at a time, proving a bijection among queue
values, submission IDs and400 terminal CQEs. Test wrap, full state, closed-drain
and refusal. Queue storage owns copied values; future payload-bearing operations
must retain their distinct payload ownership until terminal CQE, not pop or SQ
consumption. Explain O(1) queue work, fixed storage, synchronization blocking and
why this is a serialized multiple-producer adapter rather than a lock-free ring.

## IO8: linked timeout of a multishot operation

D4: extend `completion_probe.c` with a multishot POLL_ADD linked to a100ms timeout.
Observe one ready CQE with MORE, drain its byte, then require the target terminal
ECANCELED and timeout ETIME in either order. The operation and timeout storage
stay live after MORE and until terminal consumption. Preserve unique NOP CQEs
across repeated SQ wraps; range membership alone cannot detect duplication.
Different kernels/opcodes may reject this combination: record that result and
retain the obligation rather than silently treating EINVAL as success.

## Verification, rubric, hints and transfer

Run `python3 tools/run_b06_io_transfer.py` and
`python3 tools/run_b06_io_completion.py`. Require actual reference success in
GCC debug/release and Clang address/undefined sanitizers, compiled scaffold
rejection, and the four transfer mutants: repeated prefix, power-of-two masking
on capacity3, resetting total time and fixed-priority starvation. Each must fail
its semantic assertion, not just compilation or timeout. Existing ring mutants
still test over-admission and discarded MORE. Include commands, compiler/kernel
identity, raw timing and source hashes; finite schedules are not exhaustive.

| Criterion | Objectives | Level2 required evidence | Level3 independent transfer |
|---|---|---|---|
| IO5 | T62.01.a/b/c | D1 exact bytes, prefix/failure proof, mutant | terminal error after a different vector boundary without losing progress |
| IO6 | T62.02.a/b/c,T62.06.a/b/c | D2 fair ready queue, two deadline proofs, actual control fds | three peers, bounded task costs and cancellation during readiness processing |
| IO7 | T62.03.a/b/c | D3 ownership/bijection, queue unwind and close proof | batching without submission duplication after partial enter progress |
| IO8 | T62.05.a/b/c | D4 MORE/terminal identities and timeout lifetime | distinguish timeout expiry, cancellation acknowledgment and racing successful terminal result |

Level0 violates lifetime, bounds or terminal admission; level1 runs a happy path
without explaining mechanism; level2 meets the row's complete evidence; level3
requires independent changed-condition reasoning and implementation after at
least seven days, not an author run. Hints: advance a byte prefix before rebuilding
vectors; keep kernel readiness separate from your scheduling budget; draw the
queue producer/issuer/CQE owners; list every operation that can still use storage.
Assessor analysis: `assessor/linux_io_depth/targets/TRANSFER_ANALYSIS.md`.

Only original programs, own descriptors, signals and synthetic bytes are in scope.
No external network target, real credentials or host setting changes. Actual
results are in source-bound `io_transfer_validation.json` and
`io_completion_validation.json`; external review and learner assessment remain
unperformed. Direct-I/O filesystem experiments and cross-directory power-loss
recovery are separate required mechanisms, not established by these tests.
Inspected primary manuals: followup source records WRITEV, EPOLL, EVENTFD,
SIGNALFD and LINK-TIMEOUT; exact locators are in the B06 followup register.
