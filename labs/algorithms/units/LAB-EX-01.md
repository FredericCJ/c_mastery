# LAB-EX-01: Failure-preserving dynamic-array reserve

Parent: `PR-T14-02`. Budget: 2 hours, charged inside the parent project.

## Contract

Use explicit typed allocation callbacks, check byte-size overflow, preserve storage on failure and invalidate old borrows only after success.

## Steps

1. Write the state, ownership and failure contract; predict normal, boundary and fault cases.
2. Implement the selected mechanism against the imported interfaces without reading assessor code.
3. Run the semantic gate and a deliberate negative control; minimize a disagreement.
4. Change one capacity, timing, ownership or failure assumption and record the design consequence.

## Acceptance boundary

Run `algorithms.v4_array` with the selected learner implementation. Independent changed-condition implementation, design defense and complete parent-project obligations.

A passing visible check is practice evidence. Record assistance; preserve a sealed independent diagnosis or reconstruction before inspecting assessor material.

## References

- `R-clrs-4`: Data structures, dynamic programming, amortized analysis and sorting.
