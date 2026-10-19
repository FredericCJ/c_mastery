# Staged recovery prompts

Stop at the first level that lets you continue. Record which prompt you used; a guided recovery and an independent assessment are different evidence.

| Mechanism | First prompt | Stronger prompt |
|---|---|---|
| Prefix and size | Which intermediate can fail even when the final answer fits? | Separate all validation/calculation from publication; express multiplication as a division bound before multiplying. |
| Cursor | Which subtraction is defined before you form a pointer? | Validate position ≤ length, then count ≤ length−position. Empty null spans must not perform pointer arithmetic. |
| Owner/view | Which mutation changes a borrow's validity? | Snapshot source bytes before overlapping append; publish length/generation together. A generation check cannot rescue a destroyed owner. |
| Decimal | Is the accumulator allowed to hold the magnitude of INT32_MIN? | Accumulate an unsigned magnitude against a sign-dependent limit and validate the next digit before multiply/add. |
| Sort | What relative order must equal keys retain? | Count only actual key comparisons; shifting strictly greater keys has an independent inversion-pair interpretation. |
| Partial output | Which bytes has the receiver already accepted? | Keep accepted offset monotonic; charge every callback against the call budget, including retries. |
| Cancellation | Does requesting cancellation confirm that a callback can no longer run? | Keep ownership pending until one terminal completion with the current generation; reject stale completion. |
| Rollback | What exactly is owned after the kth successful acquire? | The acquired prefix is a count, and cleanup walks that prefix in reverse. State the assumed behavior of release. |
| Packet | Which validation depends on framing, and which on policy? | Establish exact length and bounded count before decode; stage results until authorization and framing both hold. |

For the admission defect, first predict the final count and the maximum intermediate count. Then use the watchpoint recipe. For memory defects, first draw owner/lifetime edges; a sanitizer report is an observation to explain, not the ownership specification. For timing, first write the measured interval and units; an optimization hypothesis without those has no comparison target.

After repair, change one assumption and wait at least seven days before the delayed attempt. Use a new independently chosen case or withheld variant. These published hints, tests and assessor mutations do not constitute a blind assessment bank.
