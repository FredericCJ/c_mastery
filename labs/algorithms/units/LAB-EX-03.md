# LAB-EX-03: Byte-offset free-list coalescing

Parent: `PR-T16-04`. Budget: 2 hours, charged inside the parent project.

## Contract

Split first-fit spans, reject overlapping returns, coalesce both neighbors and preserve state when capacity is unavailable.

## Steps

1. Write the state, ownership and failure contract; predict normal, boundary and fault cases.
2. Implement the selected mechanism against the imported interfaces without reading assessor code.
3. Run the semantic gate and a deliberate negative control; minimize a disagreement.
4. Change one capacity, timing, ownership or failure assumption and record the design consequence.

## Acceptance boundary

Run `algorithms.v4_freelist` with the selected learner implementation. Independent changed-condition implementation, design defense and complete parent-project obligations.

A passing visible check is practice evidence. Record assistance; preserve a sealed independent diagnosis or reconstruction before inspecting assessor material.

## References

- `R-clrs-4`: Data structures, dynamic programming, amortized analysis and sorting.
