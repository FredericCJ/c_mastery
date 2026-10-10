```json
{
  "id": "PR-CAP-LINUX-04",
  "title": "Linux defense and transfer",
  "module": "INTEGRATION",
  "topics": [
    "T20.10",
    "T33.10",
    "T35.12"
  ],
  "status": "core",
  "hours": 8,
  "profile": "A",
  "fixtures": [
    "fixtures/integration/wire.json"
  ],
  "edition": 2,
  "evidence_template": "component"
}
```

## Working card

**Build:** Linux defense and transfer. **Evidence:** component dossier. **Start:** predict the supplied case before reading its expected result.

**Engineering question:** Declare the newly exercised boundary and reference existing component evidence.

**Changed-condition challenge:** A changed adapter completion or ownership assumption invalidating an otherwise correct component.

Use `python -m cmastery init PR-CAP-LINUX-04 <empty-workspace>` to obtain the brief, fixtures and any executable family scaffold. A `not-run` or `unavailable` result leaves the corresponding claim open. Apply the [shared contract cards](../guides/component-contracts.md), then the project-specific obligations below.

## Contract

Defend the completed service against the common mastery gate, then change one substantial boundary without rewriting the portable core. This is an integration milestone: link the prior component dossier and record only the new work. Apply the profile boundary in {ref}`guide-profiles` and the common review in {ref}`guide-assessment`.

## Inputs and expected results

Use `fixtures/integration/wire.json`. It contains an explicit input and independently recomputed expected model result. Predict the result before inspecting the answer. Synthetic data validates the stated model only; observations from actual binaries, concurrent runs or hardware must be recorded separately. Include one new normal case, one boundary and one failure case specific to this contract.

## Steps

Choose a changed allocator, tighter capacity, different backend or concurrency strategy. Predict affected invariants and evidence before modifying code. Repeat relevant gates and perform delayed retrieval of a critical reasoning task.

## Failure cases

Reject out-of-contract sizes before access. Preserve ownership and invariants when a backend fails, an operation cannot progress or storage is exhausted. Select applicable failures from the contract above and demonstrate each response. Retain the smallest reproducer and a regression check; unexplained termination or a fabricated successful observation fails the gate.

## Acceptance and evidence

Submit complete dossier, recorded defense, perturbation comparison and unresolved limitations. Acceptance requires every mandatory dimension at level two and transfer evidence at level three; performance cannot offset correctness failure. Link evidence to the covered topic objectives and state what each artifact establishes. Project hours include implementation and verification; review and later retrieval have separate budgets.

## Hints and recovery

Start from the smallest deterministic fixture and write a state/ownership table. If actual behavior disagrees, separate the model, adapter and environment before editing the core. Consult the linked module's bounded readings and reduce the failure to one boundary transition. Do not resolve an oracle disagreement by changing the expected result without a contract argument.

## Transfer challenge

Change one declared capacity, backend or environment assumption. Predict the surviving invariants and regenerate the affected evidence. Explain the cost of the change and preserve the previous regression. No complete C solution is supplied: the implementation and its defense are learner work.

