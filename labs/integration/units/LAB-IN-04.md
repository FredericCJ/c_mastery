# LAB-IN-04: DMA completion to foreground queue

Parent: `PR-P4-02`. Budget: 2 hours, charged inside the parent project.

## Contract

Import the DMA generation protocol and ring. Do not enqueue completion before observed quiescence or consume ownership when the queue is full.

## Steps

1. Write the state, ownership and failure contract; predict normal, boundary and fault cases.
2. Implement the selected mechanism against the imported interfaces without reading assessor code.
3. Run the semantic gate and a deliberate negative control; minimize a disagreement.
4. Change one capacity, timing, ownership or failure assumption and record the design consequence.

## Acceptance boundary

Run `integration.p4` with the selected learner implementation. Independent changed-condition implementation, design defense and complete parent-project obligations. Hosted register models and cross-compilation do not establish actual MMIO addresses, cache/DMA ordering, electrical timing, target reset behavior or physical F439/H745 operation. Supply the parent project's required board captures separately.

A passing visible check is practice evidence. Record assistance; preserve a sealed independent diagnosis or reconstruction before inspecting assessor material.

## References

- `R-wg14-n1570-spelling`: 6.2.4 and 6.5.
