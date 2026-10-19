# B03 original exercise followthrough

Complete the public stubs for X-T23-002, X-T24-002, X-T25-002, X-T28-002 and
X-T30-002 after studying their original briefs and versioned contracts. The
original B03 scaffold remains a required provider; implement it separately.

Run `python run.py --build <empty-directory>` with GCC or `--cc clang`.
The default learner implementations compile and fail semantically. Author
references are explicitly supplied through `--source`, `--atomic-source` and
`--base-source`; they are never copied into learner exports.

For each original rubric produce a separate argument, original-case evidence,
changed-condition implementation, minimal counterexample and explicit premises.
The delayed task remains a learner event at least seven days after study. An
author's immediate execution is not that event. Work through these questions:

1. Trace every confirmed short-write prefix and callback quota. How can two
   connections make progress when one backend is perpetually interrupted? Why
   would byte replay be wrong after an application effect with a lost reply?
   Use request1=[2,3,5], one visible step, then a lost final reply and exact retry.
2. Compare final cancellation acknowledgment with queued acknowledgment. Identify
   the resource owner after rejection, queue acceptance, timeout and final
   quiescence. Demonstrate both completion/quiescence orderings and stale IDs.
3. Draw publication AND reuse happens-before edges. Show how two producers can
   both observe empty in the original SPSC representation, then implement a claim.
   Compare acquire/release with seq_cst under identical ownership and workload.
   Report actual atomic size/alignment/support and helper symbols separately.
4. Enumerate selected/unselected event bits around W1C and W0C commands. Contrast
   repeated debugger reads of a normal register and a clear-on-read register.
   Derive reserved-bit encoding before using any actual peripheral documentation.
5. Derive a rational physical-time conversion, retained horizon and changed width.
   Show an incoherent split read, a valid coherent protocol and a history that
   violates its premise. Do not infer sampling validity from a coherent value.

Public traces are examples, not the assessor corpus. Invalid concurrent payload
access is analyzed by event interleavings or a bounded memory-model tool; do not
execute a known C data race in a native process. Host runs give no physical or
independent learner/reviewer evidence.
