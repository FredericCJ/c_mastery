# LAB-EX-04: Unsigned stable radix passes

Parent: `PR-T15-03`. Budget: 2 hours, charged inside the parent project.

## Contract

Implement four byte passes with caller-owned scratch, explicit capacity, and unsigned ordering over the complete uint32 domain.

## Steps

1. Write the state, ownership and failure contract; predict normal, boundary and fault cases.
2. Implement the selected mechanism against the imported interfaces without reading assessor code.
3. Run the semantic gate and a deliberate negative control; minimize a disagreement.
4. Change one capacity, timing, ownership or failure assumption and record the design consequence.

## Acceptance boundary

Run `algorithms.v4_radix` with the selected learner implementation. Independent changed-condition implementation, design defense and complete parent-project obligations.

A passing visible check is practice evidence. Record assistance; preserve a sealed independent diagnosis or reconstruction before inspecting assessor material.

## References

- `R-clrs-4`: Data structures, dynamic programming, amortized analysis and sorting.
