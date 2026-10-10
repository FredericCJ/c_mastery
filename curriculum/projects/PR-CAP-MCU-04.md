```json
{
  "id": "PR-CAP-MCU-04",
  "title": "Firmware defense and transfer",
  "module": "INTEGRATION",
  "topics": [
    "T30.10",
    "T33.10",
    "T35.12"
  ],
  "status": "core",
  "hours": 8,
  "profile": "E",
  "fixtures": [
    "fixtures/integration/irq.json"
  ],
  "edition": 2,
  "evidence_template": "component"
}
```

## Working card

**Build:** Firmware defense and transfer. **Evidence:** component dossier. **Start:** predict the supplied case before reading its expected result.

**Engineering question:** Declare the newly exercised boundary and reference existing component evidence.

**Changed-condition challenge:** A changed adapter completion or ownership assumption invalidating an otherwise correct component.

Use `python -m cmastery init PR-CAP-MCU-04 <empty-workspace>` to obtain the brief, fixtures and any executable family scaffold. A `not-run` or `unavailable` result leaves the corresponding claim open. Apply the [shared contract cards](../guides/component-contracts.md), then the project-specific obligations below.

## Contract

Defend the full firmware dossier and shared-core portability, then perturb capacity, backend, clock configuration or a permitted runtime dependency. This is an integration milestone: link the prior component dossier and record only the new work. Apply the profile boundary in {ref}`guide-profiles` and the common review in {ref}`guide-assessment`.

## Inputs and expected results

Use `fixtures/integration/irq.json`. It contains an explicit input and independently recomputed expected model result. Predict the result before inspecting the answer. Synthetic data validates the stated model only; observations from actual binaries, concurrent runs or hardware must be recorded separately. Include one new normal case, one boundary and one failure case specific to this contract.

## Steps

Predict which timing/resource arguments change before modification. Repeat map/symbol, functional, race/ownership and physical timing evidence as applicable. Rebuild a small critical component after a delay without its old source.

## Failure cases

Reject out-of-contract sizes before access. Preserve ownership and invariants when a backend fails, an operation cannot progress or storage is exhausted. Select applicable failures from the contract above and demonstrate each response. Retain the smallest reproducer and a regression check; unexplained termination or a fabricated successful observation fails the gate.

## Acceptance and evidence

Submit defense, perturbation comparison, shared-core build evidence and explicit pending limits. Accept only when mandatory memory/correctness/failure/dependency gates pass and the required physical evidence exists. Link evidence to the covered topic objectives and state what each artifact establishes. Project hours include implementation and verification; review and later retrieval have separate budgets.

## Hints and recovery

Start from the smallest deterministic fixture and write a state/ownership table. If actual behavior disagrees, separate the model, adapter and environment before editing the core. Consult the linked module's bounded readings and reduce the failure to one boundary transition. Do not resolve an oracle disagreement by changing the expected result without a contract argument.

## Transfer challenge

Change one declared capacity, backend or environment assumption. Predict the surviving invariants and regenerate the affected evidence. Explain the cost of the change and preserve the previous regression. No complete C solution is supplied: the implementation and its defense are learner work.

