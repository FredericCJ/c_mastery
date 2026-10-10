# LAB-EX-08: Active ARP queue and bounded retries

Parent: `PR-T41-04`. Budget: 2 hours, charged inside the parent project.

## Contract

Retain one immutable payload, match the resolved peer, bound retry count, and separate address resolution from payload transfer.

## Steps

1. Write the state, ownership and failure contract; predict normal, boundary and fault cases.
2. Implement the selected mechanism against the imported interfaces without reading assessor code.
3. Run the semantic gate and a deliberate negative control; minimize a disagreement.
4. Change one capacity, timing, ownership or failure assumption and record the design consequence.

## Acceptance boundary

Run `communications.v4_arp` with the selected learner implementation. Independent changed-condition implementation, design defense and complete parent-project obligations.

A passing visible check is practice evidence. Record assistance; preserve a sealed independent diagnosis or reconstruction before inspecting assessor material.

## References

- `R-NET-RFC9293`: TCP retransmission and state processing.
- `R-NET-RFC826`: ARP packet reception.
