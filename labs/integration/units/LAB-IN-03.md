# LAB-IN-03: Incremental codec and malformed-input campaign

Parent: `PR-P3-02`. Budget: 2 hours, charged inside the parent project.

## Contract

Import the cursor/parser/ring rather than duplicating them. Admit fragmented length-delimited records and enter an explicit failed state on malformed input.

## Steps

1. Write the state, ownership and failure contract; predict normal, boundary and fault cases.
2. Implement the selected mechanism against the imported interfaces without reading assessor code.
3. Run the semantic gate and a deliberate negative control; minimize a disagreement.
4. Change one capacity, timing, ownership or failure assumption and record the design consequence.

## Acceptance boundary

Run `integration.p3` with the selected learner implementation. Independent changed-condition implementation, design defense and complete parent-project obligations.

A passing visible check is practice evidence. Record assistance; preserve a sealed independent diagnosis or reconstruction before inspecting assessor material.

## References

- `R-wg14-n1570-spelling`: 6.2.4 and 6.5.
