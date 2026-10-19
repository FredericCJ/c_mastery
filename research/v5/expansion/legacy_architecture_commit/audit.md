# T37.01/T37.05 execution and transfer audit

Read the actual PR-T37-03 brief, fixture, all six legacy criteria/V1 tasks,
X-T37-002 including its six rubrics and V1/V2, complete clock contract, public
SC types and actual sc_clock/sc_run implementations. The retained producer
already implements the ordinary one-clock state machine. Reimplementing it
would obscure the real gaps: external-input precedence, pending load lifetime,
cancellation, speculative ownership and actual ISA/evidence comparisons.

The additive core directly compiles the unchanged producer. It also directly
links the unchanged native/Thumb aa_isa_compare implementation, after reading
its actual interface and assembly. Fresh primary page reads are DUI0553B3-19
and Intel Vol1 revision0933-16 for the zero-status relation. The toy extension
updates its bit only on SUB; no false equivalence to all real instructions is
claimed. ISA register/byte distinctions use the preceding frozen PR01/PR02
evidence and remain distinct from the toy instruction-index PC.

The ordinary eleven transitions, budget4+7 continuation, invalid-after-two
partial progress and alias/rollback cases are rerun. New load issue/stall/ready/
cancel and speculation predict/resolve/retire/squash have explicit owned
scratch states and counterexamples. Actual native and emulated comparisons
cover all65536 byte pairs; source/output pins are bound to finite checks.

The original request to investigate a contradictory physical trace is supplied
as a clearly labeled original synthetic assessment scenario. It never becomes
a claimed captured board measurement. The learner must examine the proposed
instrumentation, clocks, interrupts and memory latency while retaining valid
bounded-model conclusions. Actual physical observation remains not-run.

No original criterion, variant identity, historical receipt or hour is changed.
Exact final C17/C23 source access, external review and independent seven-day
learner assessment remain separate. No other frozen producer is modified.

The first recorded campaign stopped at command53: the ALIAS mutant correctly
failed an earlier loop assertion, but not the specifically required alias
marker. The focused alias/invalid-clock controls were moved before the full
trace, retaining all checks. The rejected receipt remains in
attempt_mutant_marker.json; a semantic failure at the wrong assertion is not
credited as the intended mutation evidence.
