# Private retry criterion analysis

The public counter14 trace is the smallest lost-reply counterexample. In the
actual receipt adapter, all publication keys share one store commit. The
reapply mutant skips both duplicate return and durable high-water rejection;
recovery after a persisted uncertain operation applies7 again. The split-receipt
mutant first commits effect7, then begins a separate receipt transaction. A cut
between those commits leaves no durable identity, so retry returns14. These
are logical duplicate effects, not memory-safety faults.

The payload-binding mutant returns an old success for changed delta8. The
public harness rejects it at the conflict assertion. Three-entry expiry is
explicit: keep the durable high-water mark after forgetting an old result and
reject that ID as stale. This sacrifices replay of every old result, not safety
by silently reapplying it. Reopening is necessary after an indeterminate store
commit; a live in-memory cache cannot replace recovery inspection.

Read the linked service source/receipts only for their stated scope. The public
74-cut two-model campaign is original learner-facing support; the five-mode,
sector and QEMU observations are separately reused evidence. Host/model/QEMU
results do not show physical power-loss behavior. Retain the original delayed
learner gate and do not label author execution independent learner mastery.
