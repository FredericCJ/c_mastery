# LAB-IN-05: Queue shutdown after asynchronous quiescence

Parent: `PR-P5-02`. Budget: 2 hours, charged inside the parent project.

## Contract

Import the asynchronous lifecycle and ring; cancellation is a request. Drain only after completion, and preserve exactly-once release on repeated shutdown.

## Steps

1. Write the state, ownership and failure contract; predict normal, boundary and fault cases.
2. Implement the selected mechanism against the imported interfaces without reading assessor code.
3. Run the semantic gate and a deliberate negative control; minimize a disagreement.
4. Change one capacity, timing, ownership or failure assumption and record the design consequence.

## Acceptance boundary

Run `integration.p5` with the selected learner implementation. Independent changed-condition implementation, design defense and complete parent-project obligations.

A passing visible check is practice evidence. Record assistance; preserve a sealed independent diagnosis or reconstruction before inspecting assessor material.

## References

- `R-wg14-n1570-spelling`: 6.2.4 and 6.5.
