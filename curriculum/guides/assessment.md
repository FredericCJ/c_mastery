# Mastery gates and the component dossier

Every linked objective needs evidence that could expose an incorrect implementation or explanation. The original objective definitions are retained, and separately versioned extensions append new claims. Use the complete objective map for the current counts. These are separate evidence claims rather than independent assignments: one well-designed experiment may support several objectives, provided its record identifies each claim and its limits.

## Common rubric

Score each dimension 0–3: **0** absent or materially wrong; **1** succeeds on the nominal case with unexplained gaps; **2** satisfies the declared contract and explains boundary behavior; **3** transfers the reasoning to a changed condition and defends the tradeoff.

| Dimension | Evidence for level 2 | Transfer for level 3 |
|---|---|---|
| Mechanism | Predict state transitions and explain generated or observed behavior | Explain a contrasting representation or target |
| Correctness | Contract, invariant, termination and independent checks agree | Adapt the argument after a contract perturbation |
| Memory and ownership | Lifetimes, aliasing, bounds and cleanup are accounted for | Change capacity or ownership policy without losing guarantees |
| Resources and dependencies | Bounds and permitted dependency closure are recorded | Remove a dependency or tighten a resource limit |
| Failure and diagnosis | Invalid input/exhaustion follows an explicit policy; failure is minimized | Recover after a new injected failure without corrupting state |
| Communication and transfer | Another reader can rebuild and assess the result | Defend architecture alternatives and reproduce it after a delay |

Core mastery requires all six dimensions at least 2 and at least two at 3. Correctness, memory/ownership, declared dependencies and failure handling are mandatory: a higher performance or documentation score cannot compensate for a failure there. Awareness projects need accurate explanation, one inspected example and a clear limit; they do not claim implementation mastery. Electives have their own declared gates.

## Growing demands

For T01–T06, evidence may be a compact prediction table, small C program, negative compilation test and explanation. For reusable algorithms and abstractions, require a full component dossier. Systems and firmware gates add binary inspection, environment-sensitive behavior and failure injection. Integration and capstone gates require cross-component traceability, architecture alternatives and a perturbation that changes a real boundary.

Submit a dossier containing:

1. Problem, inputs, outputs, capacities and failure policy.
2. Public API, representation and invariants.
3. Ownership, lifetime and execution-context model.
4. Correctness argument, termination and complexity/resource bounds.
5. Test strategy, supplied and independent cases, oracle assumptions and failure minimization.
6. Build commands, tool versions, binary/dependency evidence and measurements.
7. Alternatives considered, selected tradeoffs and limitations.
8. Objective IDs supported by specific evidence files.

Code comments explain local decisions. The dossier explains why the architecture and contract fit the problem. Do not substitute a function listing for the argument.

## Remediation and delayed retrieval

On failure, name the violated contract and smallest reproducer before editing. Classify the cause: misunderstood semantics, wrong representation, incorrect algorithm, incomplete failure policy, unsuitable tool/environment or invalid oracle. Repair the model, add the reproducer to regression tests, then retest on at least one new case.

Delay retrieval by roughly one to two weeks. Without the old implementation, reproduce a key invariant or rebuild a small variant. Change one parameter: capacity, signedness, allocator, backend, ABI, concurrency strategy or dependency policy. Compare evidence and record which conclusions survive. A delayed failure reopens that gate; it does not erase evidence for unrelated objectives.

## Capstone review

Review the Linux and firmware capstones independently, then compare their shared portable core. Demonstrate malformed input, overload, shutdown/reset and one backend failure. Explain resource ceilings and what happens at each ceiling. Show a complete provenance chain from contract through source and build to artifact and observed result. Record a review defense and answer a counterproposal with evidence.

Accept a capstone only when all mandatory dimensions pass and every required objective has a credible artifact. A sanitizer pass, bounded proof, coverage number, stress run or benchmark is a particular kind of evidence, with particular assumptions. None is a universal substitute for the others.

## Debugging practice and feedback

For {ref}`T44`, submit the contract, pre-run predictions, matching source/image/input/environment identities, actual observations, earliest supported invalid transition, competing hypotheses and discriminating test, reduced trigger, invariant-based repair, regression matrix and delayed variant. Mark a supplied finite history as a model. Mark an actual debugger/tool record as an observation of its particular build and execution. A stopped state or smaller input does not by itself establish the complete causal history.

Begin with a guided, defined-state defect and an explicit invariant. Then receive only a symptom and input. The independent gate withholds the planted cause, solution and oracle until the learner seals and submits a dated diagnosis dossier; the learner chooses observations before editing. A visible public fixture or self-authored fault is guided or lower-independence practice. After a repair, challenge it with an adjacent boundary or a competing cause. One to two weeks later, change the capacity, build configuration, failure point or completion order and repeat the diagnosis without the old solution.

Feedback names the missing inference or violated contract before providing a command or solution. A diagnostic-string match earns no causal credit. A clean run retains its executed-path and tool limits. If a required observation is unavailable, record the narrower evidence and leave that claim open. The reduction reading {ref}`R-PD-DDMIN-ISSTA2000` and debugger contract {ref}`R-PD-GDB-20261009` support this practice; the project supplies the exact bounded scope.
