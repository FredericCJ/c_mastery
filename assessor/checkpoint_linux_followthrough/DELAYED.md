# Withheld P5 changed-condition package

Administer each retained V1 no earlier than its seven-day interval after the learner's
ordinary attempt. Preserve original variants; the rows below define executable
inputs and assessor discriminators. Record attempt times, provider hashes, allowed
documentation, reasoning before execution, failed predictions and revised arguments.
Do not award learner evidence from the author reference run. All targets are local
synthetic teaching processes; no host security changes or external endpoints.

| Leaf/V1 | Changed condition and private fixture | Required distinction |
|---|---|---|
| T24.08.b | LT to ET, three full CRC frames into queue2, kernel drained, parser-held third; take one, request no new bytes. Retained linux.c plus fd_reuse.c. | Explicit resume delivers third exactly once; token7/g1 cannot touch reused7/g2. |
| T25.02.b | Replace reject-new by blocking admission with two workers and64 messages; ownership.c. Close a separately full queue while producer sleeps. | No consumer-needed lock held while waiting, all64 complete in normal case; closed producer returns CLOSED, not FULL/success. |
| T25.02.c | Two copied workers and an independently scheduled producer reset input after every offer. | Worker-local message/decoder state, exactly-once sequence histogram; result lock distinct from queue wait. |
| T18.04.c | Payload128 instead of inline32; fail first allocation, then overwrite original and callback on another thread. | Unchanged failure output, stable owned copy, no release until matching terminal callback. |
| T24.11.b | Acknowledge gate held past an already-expired monotonic deadline; retry creation failure versus abandon; injected late EIO and EINTR after descriptor reuse. | Retain context/queue on timeout, separate observe/join; close exactly once under Linux contract. |
| T25.04.c | Competing consumer steals after wake before predicate; waiter.c seam; repeat closure as abort with one owned and two queued. | Rewait on false predicate; abort discards queued2 while owned1 survives until finish. |
| T21.06.b | Real poll/read worker replaces trusted Boolean; gate releases callback under mutex. | Demonstrated synchronization edge before passing true to unchanged in_shutdown; no stale ticket or deadline shortcut. |
| T21.06.c | Reporter calls begin/cancel/complete/destroy recursively and returns17; report_trace.c. | All reentry refused while REPORTING, ARFARF, error17 retained, no duplicate terminal transition or generation wrap. |

Give only the changed premises and public interfaces, not these private C files.
Permit an alternative representation with an equivalent observable contract, but
require a fresh lifetime/lock graph and failure-order derivation. Assess each leaf
with the four-level rubric in ANALYSIS.md. Diagnostic success without a repaired
invariant is level1 at most. Retain failed mutant outputs as evidence of test
sensitivity; a compiler rejection is not a successful runtime mutation test.
