# Actual copied-message and lifetime CMX ports

## 1. Identity and assessed families

Required successor for `PR-T51-06` and `PR-T51-14`: retain the six original
`AX-PR-T51-06-T51.06.{a,b,c}` / `AX-PR-T51-14-T51.14.{a,b,c}` identities and
their named delayed variants. This contract adds construction, boundary,
diagnosis, failure/recovery, verification and composition work. It does not
replace the existing owned-buffer, fragmentation, acknowledgement, robust
owner-death or supervisor-restart exercises in the parent lifecycle contract.

## 2. Objectives and inspected evidence

Map copied snapshots, bounds and acceptance to T51.06.a/b; actual blocking
three-producer/two-consumer delivery, timeout and abort to T51.06.c; admission,
references and checked generation reuse to T51.14.a; contrast planned CMX exit
refusal with the separately implemented robust recovery protocol in T51.14.b;
condition reacquisition, callback quiescence and ordered destruction to
T51.14.c. The passage register is
`research/v5/expansion/legacy_concurrency_queue_port/resources.json`:
`R-LCQP-COURSE`, `R-LCQP-NVIC`, `R-LCQP-NVIC-SOURCE`, `R-LCQP-REGISTERS`.
Only its recorded passages were inspected. CMSIS register/exception behavior
is an architecture/interface contract, not an ISO C guarantee.

## 3. Prerequisites and actual imports

Complete the original copied-message/lifetime library (`cl.h` and learner
`core.c`), CMX scheduler/latch/mutex/condition producer (`cmx.h` and learner
`cmx.c`), saved-mask IRQ port (`wait/irq.h` and the learner IRQ implementation),
and retained Cortex runtime startup/context/port/board/support sources. Keep
each completed artifact's identity, source receipt and dependency profile.
The task runner requires explicit paths for all implementations; it never
selects assessor sources. Do not implement a replacement integer queue or
simulate a blocked task by calling its entry again.

## 4. Environment and dependency profile

Active ISO C17, original freestanding profile E target. Host GCC/Clang builds
exercise serialized state transitions; address/undefined sanitizers run only
on the host. QEMU `mps2-an386`, Cortex-M4, is a dedicated teaching image with
no disk or network target. The actual task image imports the retained
SVC/PendSV/SysTick implementation, six 768-word aligned PSP stacks and an
8192-byte reserved handler-stack allowance. Those are workload allocations,
not proved worst-case stack bounds. C ABI/Arm assembly, privileged MMIO,
exception masks and QEMU behavior are separate from ISO semantics.

No target allocation, libc, libgcc, libatomic, compiler-rt or third-party
kernel is permitted. Audit the map, undefined symbols and generated code.
Explicit original `support.c` byte helpers may satisfy compiler-generated
copies/clears; report which helpers linked. Host Python, compilers,
disassemblers and QEMU are verification tools, not target dependencies.

## 5. Concrete problem and supplied inputs

Implement the three supplied unsolved adapters `cl_cmx.c`, `cl_cmx_irq.c`,
`cl_cmx_task.c` against their public headers. A `struct cc_queue` embeds two original
`struct cl_message` slots (0–4 bytes each), a lifetime registry of eight leases, and
sixteen per-task operation records. `struct cc_handle` binds a live queue, actual CMX
task slot/generation and object/lease generations. `struct cc_ticket` additionally
binds a nonzero operation serial. `struct cc_port` owns an implicit callback lease.
Caller-owned queue/kernel/storage remain live and stable even when checking
a stale handle. Invalid raw pointers are not made safe by a generation tag.

Use `tests.c` inputs: copied `{3,0,9,7}` and `{5,6,7,8}`, zero-length messages,
two occupied slots, a third blocked sender, short output, selected timeout,
abort and condition reacquire. The logical dispatch case sends sequences
0–19 from each of three producers to two consumers. Implement the actual
six-stack `task_workload.c` cases: 60 copied messages; full queue with a timed
send then abort; and a timed-out condition waiter held in mutex reacquisition
across close. Outputs are checked results, original payloads, accounting
ledgers and explicit PASS/failure markers, not just successful linking.

## 6. Necessary mechanisms and misconceptions

A copied queue accepts ownership of a snapshot only when the original
`cl_send` commits; enrollment is not acceptance. A blocked sender's buffer is
borrowed and must remain unchanged/live until its result is terminal.
Timeout/cancel selected by CMX must be settled before a later free slot can
cause a copy. A condition timeout is a selected outcome, not permission to
return without the mutex. Disabling an IRQ, clearing pending, and observing
inactive are distinct operations. Waking tasks does not release their object
references. Reusing a task stack before actual exit/reap is invalid even when
the logical operation looks finished.

## 7. Required invariants, operations and failure rules

All core entry and actual CMX progress share one exclusion domain. The C core
is externally serialized; it is neither lock-free nor independently thread
safe. On the UP port this is the exact saved PRIMASK, with NMI/HardFault and
unrelated peripheral access excluded from the participating set.

* Register only a READY/RUNNING actual task, once per queue. Admission holds a
  lease. Operation identity combines owner, task generation, lease/object
  generation and serial. Serial/CMX wait-sequence exhaustion refuses new
  enrollment with `CC_RETIRED`; it must not prevent release/close/destruction.
  Object or lease generation exhaustion refuses reuse rather than wrapping.
* Immediate send copies once; a full queue enrolls one actual CMX latch wait.
  Immediate receive commits only when output capacity suffices. Too-small
  output does not dequeue. Zero-length payload needs no input/output bytes.
  Failed admission leaves ticket/output and ownership unchanged; private
  scratch fields are not part of a byte-for-byte control-object promise.
* `CC_BLOCKED` means collect the eventual result using the same ticket, never
  reenter the original operation. Successful receive publishes its stored
  `struct cl_message` on collection only. Old/consumed tickets return `CL_STALE`.
  A failed or closed receive leaves the caller's output untouched.
* Progress first caches selected timeout/cancel outcomes, then services the
  oldest eligible receiver/sender using retained nonwrapping wait order.
  Queue conservation is `accepted = delivered + discarded + count`.
  Delivered does not mean application-acknowledged. Borrow pointers clear
  on terminalization; the synchronous veneer returns only after suspension
  and actual resumption, then collection under exclusion.
* Ordinary close stops admission and drains accepted messages. Abort discards
  queued messages and returns pending operations appropriately. Neither action
  silently drops registrations, owned mutexes or callback references.
* Condition wait releases its actual owned mutex only after successful wait
  enrollment. At timeout/signal/cancel, reacquisition may remain blocked beyond
  the deadline. Preserve that already-selected result across close or later
  cancellation. A collected condition result enters `CC_REACQUIRED`; explicit
  `cc_condition_unlock` is required before release. Admission rejection keeps
  the mutex owned. Closed condition is `CL_CLOSED`; nonownership is rejected.
* Close, settle operations, collect results, unlock reacquired mutexes, release
  task references and join callback activity before destroy. Drain the queue
  or explicitly abort. The core's trusted
  `cc_port_release_after_quiescence` cannot establish external quiescence.
  The concrete IRQ0 join runs in privileged thread mode, saves exclusion,
  disables that owned software-only source, clears pending, and checks enable,
  pending and active all zero before dropping its lease. No later actor may
  repend that source after join. This is not a DMA/peripheral-stop protocol.
* Synchronous task calls require the matching current RUNNING task and
  IPSR/PRIMASK/BASEPRI/FAULTMASK all zero. Handler/masked calls are refused.
  The IRQ publish entry is nonblocking. Actual return/exit and subsequent
  coordinator reap precede stack reuse; the retained runtime still refuses
  planned exit while synchronization is owned. Unexpected owner death is the
  separate robust record experiment, not a new property of CMX.

Storage is O(N + two payload slots), N=16. Each scan is O(N); progress can
terminalize O(N) operations and repeatedly scan CMX/adapter tables, so a
whole progress call is O(N²), plus at most four copied bytes per accepted
message. Derive the additional mutex priority-propagation work from the actual
producer. Do not turn these finite operation counts into an IRQ latency/WCET
bound without target measurements and an instruction/cache/interference model.

## 8. Implementation and experiment deliverables

Submit completed adapters, imported producer receipts, state/ownership
diagrams, exact event-order traces, ledger outputs, generated image/map/symbol
audits and commands. Preserve failed attempts with root-cause analysis.
Explain each return value and which failure preserves the input/output,
operation identity, borrowed lifetime and mutex ownership. Supply a separate
host/serialized, actual IRQ, and actual PSP-runtime observation table.

`run.py` builds a selected host case using explicit source/include arguments.
`task_run.py` builds a selected real runtime scenario using explicit runtime,
execution, lifecycle, adapter and task sources. Its `--build` directory is
caller-selected. Both exported runners work without an assessor directory.
`irq_run.py` builds `m4.c` using explicit execution/lifecycle/adapter/IRQ/mask
and support sources, include directories and the supplied lifecycle
`targets/m4.ld`. Its map/symbol/generated-code checks refuse undeclared
runtime imports. All three runners expose exact required paths through
`--help`; no implicit completed implementation is provided.

## 9. Verification and pass/fail evidence

Every normal, boundary and shutdown case must reach its designated marker
with the expected status. Test exactly the opposite event orders for timeout
versus space and condition signal/timeout versus close. Require unchanged
failed outputs and exactly one terminal result. Distinct negative controls:
ignore ticket serial, omit selected-result settlement, drop a pending-reacquire
lease, leave byte admission open after lifetime close, corrupt copied bytes,
omit IRQ disable or pending clear, lose the caller's mask, omit blocking
PendSV, accept masked entry, and substitute each unsolved producer/adapter.
A compiler error, arbitrary crash, generic failure or timeout is not an
accepted semantic rejection.

Independently expected message sets are the Cartesian product
`{0,1,2} × {0,...,19}` with payload identity, plus explicit count conservation.
The logical scheduler test checks per-producer dequeue order. The real runtime
checks set uniqueness rather than imposing dequeue order on later consumer
logging, which can itself be preempted. In its throughput scenario SysTick is
disabled after actual handler rejection is observed; explicit blocking PendSV
must work without a tick accidentally rescuing it. The other two scenarios
use actual SysTick deadlines. Require distinct in-bounds PSP samples, actual
exit/reap, generation refusal and no undeclared linked helpers.

## 10. Criterion-specific rubric

Level 0: wrong ownership/result, unsafe release, missing actual imports or
unsupported execution claim. Level 1: ordinary queue works but selected
timeout, condition ownership or teardown is unexplained. Level 2: satisfy the
original objective and its concrete corresponding sections here, reject its
named control, and account for all bytes/references/results. For T51.06.a/b
assess copy-at-acceptance and failure preservation separately; for .06.c
assess actual blocking and delivered/discarded/not-accepted categories; for
T51.14.a assess lifetime plus stale identity; for .14.b assess the CMX/robust
policy counterexample; for .14.c assess reacquisition and joined callbacks.
Level 3: after the original recorded delay, independently implement and defend
that criterion's existing changed-condition variant and show exactly which
adapter obligations change. A passing author campaign is not learner credit.

## 11. Graduated hints and separate analysis

First draw the moment a byte snapshot becomes accepted and the lifetime of
the blocked caller's stack. Then inspect the actual CMX wait result and
condition pending-result/reacquisition fields. Finally separate three facts:
result selected, task runnable, and owned reference released. For the IRQ case,
list enable/pending/active separately before writing the join. The technical
solution and control analysis are separately retained under assessor material;
they are not supplied by learner initialization.

## 12. Changed assumptions and delayed transfer

Retain all six original delayed tasks: owned-buffer transfer; fragmented
variable backing; application acknowledgements; implicit timer reference;
repairer death; supervisor restart with exhausted generation. Their existing
implemented mechanisms and host evidence remain separate imports. Explain how
to adapt each to actual blocking stacks without weakening ownership rules.
Additionally move the IRQ source from software-only to a simulated peripheral
that can assert again: derive the required device-stop/ack/drain contract,
and reject the claim that the IRQ0 join alone supplies it. Physical peripheral
validation remains required if making a physical claim.

## 13. Containment

Use only these original teaching programs, synthetic bytes and a disposable
QEMU guest. No flashing, host security setting, external network, production
firmware or other person's data is involved. Intentionally faulty controls
may violate the teaching queue's property; success is a confined assertion
showing the wrong byte/result/ownership transition. No exploit or privilege
claim follows from a crash or sanitizer report.

## 14. Status and limits

Authored learner scaffolds and separate complete assessor adapters. Current
source-bound reports distinguish host cases, IRQ observations and actual PSP
task execution. Finite observations, sanitizers and code review do not prove
all schedules, bounded real-time progress or absolute mastery. No physical
board, independent human review, unaided learner attempt, asynchronous task
termination recovery or general SMP port is asserted. The original accepted
unit identities, hours and historical receipts are unchanged.
