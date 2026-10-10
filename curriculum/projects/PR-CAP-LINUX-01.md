```json
{
  "id": "PR-CAP-LINUX-01",
  "title": "Linux architecture and threat model",
  "module": "INTEGRATION",
  "topics": [
    "T20.01",
    "T22.01",
    "T35.01"
  ],
  "status": "core",
  "hours": 8,
  "profile": "A",
  "fixtures": [
    "fixtures/integration/wire.json"
  ]
}
```

## Contract

Specify a bounded telemetry service that reuses P3 and P5, with input source, message processing, output sink and operational limits. Select architecture from at least two alternatives. This is an integration milestone: link the prior component dossier and record only the new work. Apply the profile boundary in {ref}`guide-profiles` and the common review in {ref}`guide-assessment`.

## Inputs and expected results

Use `fixtures/integration/wire.json`. It contains an explicit input and independently recomputed expected model result. Predict the result before inspecting the answer. Synthetic data validates the stated model only; observations from actual binaries, concurrent runs or hardware must be recorded separately. Include one new normal case, one boundary and one failure case specific to this contract.

## Steps

Draw boundaries and ownership transitions, enumerate malformed input, overload, external failure and shutdown threats. Freeze memory, message and descriptor ceilings. Map reusable evidence to the contract and identify the remaining integration tests.

## Failure cases

Reject out-of-contract sizes before access. Preserve ownership and invariants when a backend fails, an operation cannot progress or storage is exhausted. Select applicable failures from the contract above and demonstrate each response. Retain the smallest reproducer and a regression check; unexplained termination or a fabricated successful observation fails the gate.

## Acceptance and evidence

Submit architecture decision record, threat/failure model, dependency profile and traceability plan. This milestone adds design integration; existing parser and queue implementation hours are not charged again. Link evidence to the covered topic objectives and state what each artifact establishes. Project hours include implementation and verification; review and later retrieval have separate budgets.

## Hints and recovery

Start from the smallest deterministic fixture and write a state/ownership table. If actual behavior disagrees, separate the model, adapter and environment before editing the core. Consult the linked module's bounded readings and reduce the failure to one boundary transition. Do not resolve an oracle disagreement by changing the expected result without a contract argument.

## Transfer challenge

Change one declared capacity, backend or environment assumption. Predict the surviving invariants and regenerate the affected evidence. Explain the cost of the change and preserve the previous regression. No complete C solution is supplied: the implementation and its defense are learner work.
