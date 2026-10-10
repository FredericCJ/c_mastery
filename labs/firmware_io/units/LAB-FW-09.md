# LAB-FW-09: DMA generation and cancellation

Parent: `PR-T29-04`. Budget: 2 hours, charged inside the parent project.

## Contract

Model CPU, device and cancelling ownership; require matching generation and observed quiescence before reuse.

## Steps

1. Write the state, ownership and failure contract; predict normal, boundary and fault cases.
2. Implement the selected mechanism against the imported interfaces without reading assessor code.
3. Run the semantic gate and a deliberate negative control; minimize a disagreement.
4. Change one capacity, timing, ownership or failure assumption and record the design consequence.

## Acceptance boundary

Run `firmware_io.dma` with the selected learner implementation. Independent changed-condition implementation, design defense and complete parent-project obligations. Hosted register models and cross-compilation do not establish actual MMIO addresses, cache/DMA ordering, electrical timing, target reset behavior or physical F439/H745 operation. Supply the parent project's required board captures separately.

A passing visible check is practice evidence. Record assistance; preserve a sealed independent diagnosis or reconstruction before inspecting assessor material.

## References

- `R-ST-RM0090-21`: DMA controller: stream enable/disable and DMA_SxCR EN.
- `R-OS3-ST-RM0399-R4`: DMA and memory architecture: domain reachability and cache maintenance obligations.
