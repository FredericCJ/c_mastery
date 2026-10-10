# LAB-FW-02: Stack and reset-vector validation

Parent: `PR-T29-01`. Budget: 2 hours, charged inside the parent project.

## Contract

Reject unaligned stacks, out-of-RAM stack tops, non-Thumb reset addresses and out-of-flash entry points.

## Steps

1. Write the state, ownership and failure contract; predict normal, boundary and fault cases.
2. Implement the selected mechanism against the imported interfaces without reading assessor code.
3. Run the semantic gate and a deliberate negative control; minimize a disagreement.
4. Change one capacity, timing, ownership or failure assumption and record the design consequence.

## Acceptance boundary

Run `firmware_io.vectors` with the selected learner implementation. Independent changed-condition implementation, design defense and complete parent-project obligations. Hosted register models and cross-compilation do not establish actual MMIO addresses, cache/DMA ordering, electrical timing, target reset behavior or physical F439/H745 operation. Supply the parent project's required board captures separately.

A passing visible check is practice evidence. Record assistance; preserve a sealed independent diagnosis or reconstruction before inspecting assessor material.

## References

- `R-ENH-ARMV7M-DDI0403EE`: Exception handling: vector table, initial stack pointer and Thumb execution state.
