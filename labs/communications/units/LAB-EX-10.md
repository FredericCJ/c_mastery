# LAB-EX-10: CAN FD configuration and loopback

Parent: `PR-T38-11`. Budget: 2 hours, charged inside the parent project.

## Contract

Respect INIT/CCE configuration order, bounded acknowledgement and supported FD lengths; keep H745 clock/cache/message-RAM/cable obligations open.

## Steps

1. Write the state, ownership and failure contract; predict normal, boundary and fault cases.
2. Implement the selected mechanism against the imported interfaces without reading assessor code.
3. Run the semantic gate and a deliberate negative control; minimize a disagreement.
4. Change one capacity, timing, ownership or failure assumption and record the design consequence.

## Acceptance boundary

Run `communications.v4_canfd` with the selected learner implementation. Independent changed-condition implementation, design defense and complete parent-project obligations. Hosted register models and cross-compilation do not establish actual MMIO addresses, cache/DMA ordering, electrical timing, target reset behavior or physical F439/H745 operation. Supply the parent project's required board captures separately.

A passing visible check is practice evidence. Record assistance; preserve a sealed independent diagnosis or reconstruction before inspecting assessor material.

## References

- `R-NET-RFC9293`: TCP retransmission and state processing.
- `R-NET-RFC826`: ARP packet reception.
- `R-OS3-ST-RM0399-R4`: FDCAN controller: configuration, message RAM and internal loopback.
