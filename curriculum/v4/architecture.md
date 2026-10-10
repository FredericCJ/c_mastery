# Decisions before implementation, revision after evidence

An architecture decision is required when a component becomes reusable, an
execution/ownership/transport boundary changes, or two plausible designs make
different predictions under the assigned workload. Syntax drills and fixed ABI
facts need predictions and observation, not invented alternatives.

## A compact decision record

Record the requirement and capacity/failure assumptions; two plausible designs;
their predicted differences; the discriminating trace, model or prototype; your
choice; the rejected design; and the condition that would reverse the choice.
Seal this record before the main implementation. Record which experiment is an
exploratory spike so it is not confused with the final implementation.

Use the external behavioral contract to judge both designs. You cannot make an
incompatible design pass by changing a fixed wire format, ownership obligation,
test expectation or timing assumption after observing its failure.

| Progression | What is supplied | What you decide |
|---|---|---|
| Worked comparison, chiefly phases 1–2 | Constraints, alternatives and a comparison method | Complete the predictions, run the comparison, justify a choice |
| Bounded choice, chiefly phases 3–4 | Constraints and plausible alternatives | Design the comparison and explain the trade-off |
| Independent design, chiefly phases 5–7 | Workload, resource ceilings and acceptance boundary | Propose alternatives, interfaces and evidence |
| Architectural revision, chiefly phases 8–9 | A changed or competing requirement and your existing system | Clarify assumptions, revise ownership/boundaries, plan migration and revalidation |

Support fades per mechanism. Your first exception-frame or cache-maintenance
exercise can be guided even after independent design of a hosted service.

## Revisit the decision in code

| Earlier decision | Later pressure | Required reconsideration |
|---|---|---|
| Ring count versus monotonic positions | SPSC publication, non-power-of-two capacity | Which fields each participant owns; wrap bounds and publication order |
| Parser keeps bytes versus caller retains spans | Short transfers and cancellation | Retained lifetime, resumable state and atomic error handling |
| Timer list versus heap | Different task count or cancellation rate | Traversal bounds, measured cycles, code size and WCET assumptions |
| Copied messages versus transferred buffers | DMA completion or peer reset | Who can reuse memory, stale completion identity and recovery authority |
| Reactor versus worker pool | Slow clients and CPU-heavy requests | Fairness, backpressure, bounded queues, cancellation and shutdown |
| Shared store versus one store owner | Client death during commit | Serialization, durability, error propagation and restart state |

At a later handoff, reconstruct the old rationale without the record, predict
whether the new condition reverses it, then consult the original evidence. A
defensible decision to retain the design is valid when the evidence supports it.

Full milestone acceptance retains the six-dimension 0–3 rubric: mechanism,
correctness, memory ownership, resources/dependencies, failure diagnosis, and
communication/transfer. All dimensions must reach two and at least two must reach
three. Do not average away an ownership or correctness failure. Small units use
lightweight evidence until that boundary is ready for review.
