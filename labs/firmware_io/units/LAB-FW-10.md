# LAB-FW-10: MDIO readiness and TX descriptors

Parent: `PR-T39-07`. Budget: 2 hours, charged inside the parent project.

## Contract

Wait bounded MDIO readiness and populate descriptor length/generation before publishing ownership; reject device-owned reuse.

## Steps

1. Write the state, ownership and failure contract; predict normal, boundary and fault cases.
2. Implement the selected mechanism against the imported interfaces without reading assessor code.
3. Run the semantic gate and a deliberate negative control; minimize a disagreement.
4. Change one capacity, timing, ownership or failure assumption and record the design consequence.

## Acceptance boundary

Run `firmware_io.ethernet` with the selected learner implementation. Independent changed-condition implementation, design defense and complete parent-project obligations. Hosted register models and cross-compilation do not establish actual MMIO addresses, cache/DMA ordering, electrical timing, target reset behavior or physical F439/H745 operation. Supply the parent project's required board captures separately.

A passing visible check is practice evidence. Record assistance; preserve a sealed independent diagnosis or reconstruction before inspecting assessor material.

## References

- `R-ST-RM0090-21`: Ethernet MAC: MII management interface and normal DMA transmit descriptors.
