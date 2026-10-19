# T37.06 pipeline depth audit

Inspected the actual PR-T37-04 brief, full fixture, three criterion/V1 tasks,
X-T37-001 rubric/variants, T37.06 teaching, retained pipeline contract and exact
s_pipeline body. The retained implementation schedules an already-selected
retired stream and inserts equivalent pre-IF delays. It does not represent
held ID/IF instances or actual branch squash. Its unchanged body is reused
only for compatible four-instruction totals, with explicit extraction hashes.

The new scheduler tracks every fetched instance, real stage occupancy,
producer availability, configurable memory duration, branch resolution,
register-file order and shared-port access. A separate renamed out-of-order
model implements the original changed task with explicit different assumptions.
These are original declared pedagogical machines; no commercial pipeline
claims require an uninspected textbook. The exact C data-race definition was
freshly read from N1570 printed20,§5.1.2.4p25. N1570 is a C11 draft, not final
C17/C23. No new Hennessy/Patterson book inspection is claimed.

Initial standalone compilation found one new misleading loop indentation and
an unrelated old s_reduce warning. The new layout was clarified; the exact
selected predecessor function is extracted instead of altering the frozen
family or weakening diagnostics. The first matrix pilot found an invalid test
assumption: a port conflict masked by an existing IF stall adds no separate
delay. An isolated independent stream now supplies the necessary positive
port-conflict witness. The first recorded campaign stopped at29commands on
learner array-parameter spelling diagnostics; the matching public-array spelling
was restored and the failed receipt retained as attempt_stub_array_parameters.json.

The next campaign stopped at116 commands because the cost oracle incorrectly
required the qualifier `static`. Actual GCC output reports4352 bytes with
`dynamic,bounded` for ap_schedule and464 static bytes for ap_ooo. The inspected
GCC Developer Options `-fstack-usage` section distinguishes bounded adjustments
within a function from an unbounded dynamic allocation. Actual disassembly
contains a fixed frame plus call-site stack adjustments. The corrected oracle
accepts static or explicitly bounded usage below8192 bytes, and still rejects
unbounded dynamic usage. This is a function estimate, not a whole-call-stack
proof. The failed receipt is retained as attempt_stack_cost.json.

All original identities, tasks, delays, historical receipts and descriptive
hours are preserved. Independent learner/reviewer and physical evidence remain
separate; analysis-only race examples are never reported as concurrent runs.
