# Actual single-core dispatch accounting for PR-T52-05

## 1. Identity and exercise families

`LRS-RTOS-SERVER-PORT` is the required runtime continuation of
`AX-PR-T52-05-T52.05.a/b/c`. Families B, C, D, F, G, H, I and J apply:
construct and integrate the adapter, diagnose wrong accounting, preserve
refusal transactions, inspect generated code, and change the execution model.
The three original criteria and their delayed variants remain required;
this continuation does not replace their polling, cohort or ledger contracts.

## 2. Objectives and inspected references

Explain exactly when the retained `cmx` scheduler selects a PSP context;
derive consumption-linked eligibility from the completed `struct ss_budget`; and
separate finite tick accounting from a physical CPU execution bound. Inspect
the functions recorded by `R-LRPORT-CMX` and `R-LRPORT-CORTEX` in
`research/v5/expansion/legacy_realtime_port/resources.json`.
The predecessor server/cohort references support the policy comparison, not
the correctness of this new runtime adapter. The small adapter is original
course construction, with a separate assessor implementation.

## 3. Prerequisites and learner artifacts

Supply completed `PR-T50-01` task lifecycle, `PR-T50-02` Cortex context-switch
artifacts, and the `PR-T52-05` server ledger. Revisit `T50` task states,
generation retirement, saved interrupt masks and exception return. Import
actual completed `cmx.c`, `advanced.c`, `server.c` and their public headers;
do not replace these mechanisms with a simulated counter or copy the assessor
solution into a learner workspace. The predecessor cohort comparison remains
an independent logical experiment and is not dispatched by this adapter.

## 4. Environment, edition and dependency profile

C17 is the implementation baseline. Hosted GCC/Clang tests may use libc for
the harness. The strict freestanding image uses the retained course startup,
context assembly, Cortex port and explicit byte helpers, with no libc,
libgcc, libatomic, allocation or implicit division helpers. The target is an
owned QEMU `mps2-an386` Cortex-M4 guest, three disjoint 512-word PSP stacks,
fixed priorities and a private 128 KiB RAM linker map with 8 KiB reserved for
the handler stack. The runner audits linked imports and actual SVC/PendSV/
SysTick/PSP instructions. GNU attributes, inline assembly, exception entry,
MMIO and interrupt serialization are target/compiler conventions, not ISO C
threads or ISO C guarantees. QEMU instruction-count timing is repeatable
emulator timing; it is not a calibrated hardware cycle model.

## 5. Problem, inputs and outputs

Implement the four operations in `port.h` using the unsolved `port.c`.
`sr_init` binds one existing READY/RUNNING task and its full generation to
quota `1..16`, period `Q..2^31-1`, the current kernel time and a 16-record
`struct ss_budget`. Storage is fresh and live; kernel, adapter and outputs do not
overlap. Reinitializing a live binding or mutating its fields is forbidden.

`sr_tick(s,boundary)` runs exactly once before `cmx_tick(k,boundary)` with
`boundary=k->now+1=s->last+1`. Clocks do not wrap in this experiment. Advance
due credits; charge one unit only if the bound task is actually RUNNING and
is the kernel's current task. On exhaustion, suspend it through actual
`cmx_sleep` until the earliest credit. Record the exact sleep sequence and
deadline. The real SysTick handler then advances time and PendSV selects a
context. A READY task is not an observed running quantum. A former current
task awaiting PendSV may already be WAITING; trace it as `ran=3`, not payload.

`sr_close` is idempotent and cancels only this binding's exact outstanding
sleep. It admits a short cleanup path but forbids further payload. The task
returns through the retained real exit trampoline. `sr_detach` requires
closed, EXITED and not current; only then may the owner reap. Reused raw task
slots must not make old bindings valid. The guest's final raw-slot reuse is a
stopped control-state test: it does not install or execute a new PSP context.

Inputs are the host cases `basic`, `guards`, `generation`, `stream`; 285
independent workloads `(Q=1..6,P=Q..12,higher_priority_pattern=0..4)`; and guest
scenarios 0: supervisor C1/T5, 1: supervisor C3/T5, 2: close an actually
throttled task at time20, 3: deliberate five-wrap interrupt-mask violation.
Normal guests use Q3/P10, a saturated server at middle priority, and a lower
priority worker. Keep all 72/72/22 normal tick rows and the mask witness.

## 6. Mechanism and necessary counterexample

The old logical budget model does not itself stop a running PSP task.
Conversely, observing a tick counter does not establish measured CPU use.
For this adapter's normal image the server never voluntarily blocks or
yields during payload; preemption and throttling occur at timer boundaries.
Starting partway through a quantum is charged conservatively at its end.
Every mask and handler interval must be shorter than a timer period, and
dispatch must settle before another payload interval. These are explicit
environment premises; a finite trace does not prove them for arbitrary code.

Scenario3 deliberately polls the timer downcounter through five wraps while
interrupts are masked. On release the retained pending interrupt yields only
one observed tick and one charge. Its harmless success condition is exactly
`wraps=5;observed_ticks=1;charges=1`. This falsifies a universal safety claim
for sample-only accounting. Do not repair this intentionally flawed
experiment or describe five wraps as five delivered interrupts. The remedy
for a broader runtime is an independently justified elapsed-service clock
and accounting at every block/yield/dispatch boundary, plus an enforcement
latency bound; that broader adapter is not supplied or claimed here.

## 7. Reasoning obligations

Prove conservation of the imported Q rights, explain why the earliest credit
is the only permitted wake deadline, and identify the publication point for
budget plus kernel suspension. Every non-OK operation preserves both objects;
in particular exhausted CMX wait-sequence capacity must not consume budget
and leave a task running. A non-OK timer callback requires immediate owned
image containment, as in the supplied harness; continuing dispatch is not an
allowed recovery. No forged success or silent budget reset is permitted.

Explain the binding generation, exact wait identity, close versus exit and
reap, and why cancellation of an unrelated wait is a defect. The kernel and
adapter require one external serialization domain; there is no SMP claim.
No external caller may cancel/reassign the bound task's sleep or exit it
behind the adapter. Bound clock/counter arithmetic and refuse before wrap;
counter exhaustion must not prevent already-admitted close/detach cleanup.

The budget operation scans at most16 credits, uses no dynamic allocation,
and stages bounded adapter storage. The kernel sleep itself is constant work;
the retained timer scans up to16 tasks and the scheduler scans up to16 tasks.
Count these separately. Code size includes the harness/imports; stack usage,
logical quanta and physical worst-case execution time are different claims.

## 8. Implementation and experimental deliverables

Submit your adapter source, an ownership/state diagram, actual imported
artifact identities, unchanged-failure snapshots, full host oracle outputs,
three-profile target build/link/disassembly evidence, every guest tick trace,
and the deliberately violated-premise trace. Keep a dependency ledger showing
explicit byte helpers and no hidden runtime libraries. Explain each of the
four semantic mutants: omitted charge, omitted suspension, ignored generation,
and early wake. Keep the public stubs unsolved in distributable course sources.

## 9. Verification and acceptance

Use `run.py` with explicit completed imports for hosted checks. `oracle.py`
replays 22,800 transitions using independent unit rights, not the C ledger.
Use `target_run.py` with explicit runtime/include/source/linker paths to build
and execute each scenario. A compile failure, signal, sanitizer finding or
generic guest fault does not count as an intended mutant rejection: require
the specific `SR_CHECK:` or `SR_TASK_CHECK:` and expected exit1 after successful
builds. Positive runs require the full trace, oracle agreement and
`SR_TASK_PASS:`. Test a stub for this adapter and separate budget/kernel
producer stubs so a synthetic replacement cannot masquerade as integration.
The retained unsolved kernel source has unused placeholder parameters: only
that imported negative-control translation unit disables that diagnostic; its
build must still succeed and its semantic assertion must fail. Do not change
the placeholder bodies or relax positive-reference compiler diagnostics.

In normal guests, independently count charged running quanta in each half-open
P window, check supervisor releases, observe lower-priority progress and
distinct live PSP ranges, and require cleanup before detach/reap. A finite
passing run supports these exact traces and contracts, not universal mastery,
the classical server theorem, SMP correctness or physical timing guarantees.

## 10. Assessor rubric

`PORT-A` (T52.05.a): level0 confuses logical budget with dispatch; level1 adds
a timer counter; level2 derives the exact current-state/credit/wake chain and
passes imported reference/stub checks; level3 explains partial-quanta and
the five-wrap counterexample without overstating cohort equivalence.

`PORT-B` (T52.05.b): level0 corrupts capacity on refusal; level1 handles only
ordinary sleep; level2 preserves both objects under clock/counter/sequence
refusal and binds the exact generation/wait; level3 independently derives
close/exit/detach/reap ordering and diagnoses the stale-slot control.

`PORT-C` (T52.05.c): level0 presents a pass label alone; level1 counts average
service; level2 retains all tick/PSP/import evidence and independent window
checks under both preemption patterns and throttled close; level3 transfers
the accounting argument to a changed timing premise and states the necessary
new clock/dispatch observations. Physical WCET is never inferred from QEMU.

## 11. Graduated hints and separate technical analysis

Hint1: inspect the weak tick hook's order relative to `cmx_tick` and PendSV.
Hint2: stage the budget, then ask which actual `cmx_sleep` refusal paths mutate
the kernel; reserve counter capacity before admission. Hint3: a task ID is
insufficient, and a WAITING state alone does not prove ownership of a sleep.
The assessor implementation and derivation live separately under
`assessor/legacy_realtime/port.c` and `port.md`; they are not learner downloads.

## 12. Changed assumptions and delayed transfer

Preserve the original seven-day delayed variants. Add these concrete transfer
inputs: supervisor C3/T5 with the same Q3/P10; close at time20 while the server
is throttled; full wait-sequence and charge counters; a reused slot after exit;
and the actual five-wrap mask witness. Reference support for these inputs is
executed separately from any unaided learner attempt. For an independently
timed or voluntarily blocking runtime, derive the extra hook/clock contract
before implementation; the current evidence cannot be relabeled as that port.

## 13. Security and isolation

Only original owned course programs and a disposable emulator image are used.
The adversary in scenario3 controls the teaching server's mask duration; the
defect is loss of elapsed-service information in one pending bit. Five wraps
with one charge is a bounded accounting-property violation, not privilege
escalation. Stop the guest after the witness. Repair the overstated guarantee
by enforcing the declared premises or constructing the broader accounting
contract; retain the witness as a regression against universal claims. No
physical flashing, host interrupt-policy changes, deployed firmware or real
credentials are involved. Emulator isolation and scoped timing limits remain.

## 14. Validation status and limits

Authored public contract and separate reference implementation; status and
exact commands are recorded by `tools/run_server_port.py` and
`tools/check_server_port.py` in `reviews/v5/expansion/legacy_realtime_port`.
Do not infer an executed profile from this prose. Source-bound reports must
pass before recording author validation. External review, independent learner
assessment and physical board measurements remain separate, unperformed
dimensions. The explicit pending-bit witness is successful falsification of
an invalid premise, not successful budget enforcement under that premise.
