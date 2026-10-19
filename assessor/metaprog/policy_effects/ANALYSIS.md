# T68.03 assessor argument

sum<=limit initially and after every admitted update. Comparison with limit-sum
avoids overflowing addition; rejecting mode leaves output untouched at the first
excessive prefix. Saturating mode preserves the invariant but does not terminate
the effect protocol. limit0 is the smallest counterexample to skipping callbacks.

Exactly one item is live after acquire success, or a valid partial owner after
failure. Every path releases it once. Project runs only after successful acquire;
its failure still releases before returning. The code reads no owner after release.
Output rollback cannot undo A/P/R effects. A late failure after numeric saturation
therefore returns failure and retains the complete executed effect prefix.

The delayed final-acquisition-after-allocation failure must release the last
partial owner even when the accumulator was saturated at the first element.
Earlier projections cannot be skipped. Dynamic/generated variants must match
status, sentinel, exact ordered trace and resources, not merely successful sum.
The oracle's pure retained reducer comparison intentionally excludes acquisition
failure: it checks the numeric sequence as a separate contract.

Eight controls discriminate cleanup, publication, premature termination, order,
omitted/duplicated effects and both retained numeric faults. Pair every ordinary
semantic rejection with a successful compilation. Compiler specialization may
remove policy selection but leaves function-pointer callback effects; actual
disassembly, symbols and equal-work samples decide the observed cost. The Rust
const-bool control does not establish C resource ownership or this callback ABI.
No author campaign is an independent learner/mastery result.
