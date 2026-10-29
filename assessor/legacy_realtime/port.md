# Server port technical analysis

The finite adapter stages only `struct sr_port`/`struct ss_budget`. It preflights all local
counter limits; the retained `cmx_sleep` either refuses without mutation or
publishes the exact sleep and increments its global wait sequence. Once that
call succeeds, assignment of the staged adapter cannot fail. Thus no failed
admission consumes a right or produces an untracked sleep. `sr_close` owns only
the exact wait-sequence/deadline pair and does not equate close with task exit.
The actual task trampoline calls `cmx_exit`; detachment waits until a different
task is running and only then permits raw-slot reaping. The stale binding
fails even after the raw CMX slot becomes READY again.

At each normal timer boundary update due rights, inspect the actual RUNNING
task, and charge one unit. Exhaustion suspends the server until its oldest
right returns. The same retained SysTick then wakes due sleepers and pends
PendSV; the assembly saves/restores the disjoint PSP frames. A READY task is
not running; a WAITING old-current task before PendSV is trace category3.
The unit-right oracle supplies expected budgets independently of the imported
C replenishment representation. Actual guest logs are retained in full and
checked against the same consumption timestamps and exact close boundary.

The initial cross-compiler run exposed a harness boundary error: Clang O3
inlined the PSP sample into a leaf background task without a stack prologue.
Its restored empty descending-stack pointer legitimately equaled the one-past
stack end. The sample now accepts that pointer without dereferencing it;
the low canary boundary remains excluded and the retained PendSV check still
validates the saved frame below the top. Keep the failed attempt as evidence
of the changed-layout diagnosis, rather than counting it as a learner defect.

The Q-in-P check applies to the **charged discrete quanta**. The controlled
server starts possibly partway through a quantum but never voluntarily
blocks during payload, so rounding up is conservative under the short-mask,
timely-handler premises. It does not establish a bound for arbitrary code.
The deliberate long-mask scenario watches five timer downcounter wraps while
the retained runtime clock is unchanged, then sees exactly one pending
delivery/charge. That concrete witness rejects the common mistaken inference
that periodic interrupt hardware queues every elapsed tick. A physical
enforcement claim would require an independent elapsed-service clock and
measured/proved dispatch, masking and handler bounds. No such measurement is
fabricated here; QEMU instruction-count timing is not hardware WCET.

Mutant1 increments accounting count but omits the credit debit; the normal
quota/window and budget checks reject it. Mutant2 debits without suspending,
so a subsequent observed dispatch exhausts unavailable rights and is contained.
Mutant3 ignores generation and accepts a closed old binding after real reap
and stopped control-slot reuse. Mutant4 sleeps only until the current boundary
and is immediately made ready, violating the intended budget wake ordering.
Compiler failure and ordinary faults are not accepted controls. Public stubs
and independent producer stubs compile but fail specific semantic assertions.

Assess PORT-A/B/C separately. The original polling/cohort/ledger/overload
criteria remain supported by their predecessor packages. This port adds real
dispatch enforcement for a stated finite environment; it is not an alternative
whole scheduling course or evidence of learner independence.
