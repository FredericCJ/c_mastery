```json
{
  "id": "PR-CAP-LINUX-02",
  "title": "Service assembly and packaging",
  "module": "INTEGRATION",
  "topics": [
    "T22.05",
    "T23.04",
    "T24.08"
  ],
  "status": "core",
  "hours": 8,
  "profile": "A",
  "fixtures": [
    "fixtures/integration/pressure.json"
  ]
}
```

## Contract

Assemble the service from reviewed components with adapters, configuration and a reproducible build. Keep portable headers free of Linux-specific dependencies. This is an integration milestone: link the prior component dossier and record only the new work. Apply the profile boundary in {ref}`guide-profiles` and the common review in {ref}`guide-assessment`.

## Inputs and expected results

Use `fixtures/integration/pressure.json`. It contains an explicit input and independently recomputed expected model result. Predict the result before inspecting the answer. Synthetic data validates the stated model only; observations from actual binaries, concurrent runs or hardware must be recorded separately. Include one new normal case, one boundary and one failure case specific to this contract.

## Steps

Run fixture streams through real input and output adapters. Build the portable core separately for firmware. Package public headers and consume the library from a second small build tree. Record compiler/libc versions and final dependency closure.

## Failure cases

Reject out-of-contract sizes before access. Preserve ownership and invariants when a backend fails, an operation cannot progress or storage is exhausted. Select applicable failures from the contract above and demonstrate each response. Retain the smallest reproducer and a regression check; unexplained termination or a fabricated successful observation fails the gate.

## Acceptance and evidence

Submit executable, package/consumer evidence, end-to-end results and resource ledger. Input/output failures follow the stated policy, not an assertion or silent data loss. Account for every accepted message. Link evidence to the covered topic objectives and state what each artifact establishes. Project hours include implementation and verification; review and later retrieval have separate budgets.

## Hints and recovery

Start from the smallest deterministic fixture and write a state/ownership table. If actual behavior disagrees, separate the model, adapter and environment before editing the core. Consult the linked module's bounded readings and reduce the failure to one boundary transition. Do not resolve an oracle disagreement by changing the expected result without a contract argument.

## Transfer challenge

Change one declared capacity, backend or environment assumption. Predict the surviving invariants and regenerate the affected evidence. Explain the cost of the change and preserve the previous regression. No complete C solution is supplied: the implementation and its defense are learner work.
