```json
{
  "id": "PR-CAP-MCU-03",
  "title": "Fault containment and update policy",
  "module": "INTEGRATION",
  "topics": [
    "T31.09",
    "T31.10",
    "T35.08"
  ],
  "status": "core",
  "hours": 8,
  "profile": "E",
  "fixtures": [
    "fixtures/integration/recovery.json"
  ]
}
```

## Contract

Add documented fault records, watchdog/progress policy and a defensible update/boot handoff design. Implement the safe fault/recovery slice within the existing application; a production bootloader is not required. This is an integration milestone: link the prior component dossier and record only the new work. Apply the profile boundary in {ref}`guide-profiles` and the common review in {ref}`guide-assessment`.

## Inputs and expected results

Use `fixtures/integration/recovery.json`. It contains an explicit input and independently recomputed expected model result. Predict the result before inspecting the answer. Synthetic data validates the stated model only; observations from actual binaries, concurrent runs or hardware must be recorded separately. Include one new normal case, one boundary and one failure case specific to this contract.

## Steps

Use the synthetic slot fixture to explain invalid-image rejection. Inject a selected recoverable peripheral failure and a controlled software fault on the board. Record persistent-state validity, reset reason, ownership cleanup and recovery limits. Discuss authenticity separately from CRC validity.

## Failure cases

Reject out-of-contract sizes before access. Preserve ownership and invariants when a backend fails, an operation cannot progress or storage is exhausted. Select applicable failures from the contract above and demonstrate each response. Retain the smallest reproducer and a regression check; unexplained termination or a fabricated successful observation fails the gate.

## Acceptance and evidence

Submit fault decoding, reset/recovery traces, watchdog rationale and update threat model. Power-loss and authenticity claims need evidence beyond the toy slot policy. Never describe selecting a valid record as secure-update certification. Link evidence to the covered topic objectives and state what each artifact establishes. Project hours include implementation and verification; review and later retrieval have separate budgets.

## Hints and recovery

Start from the smallest deterministic fixture and write a state/ownership table. If actual behavior disagrees, separate the model, adapter and environment before editing the core. Consult the linked module's bounded readings and reduce the failure to one boundary transition. Do not resolve an oracle disagreement by changing the expected result without a contract argument.

## Transfer challenge

Change one declared capacity, backend or environment assumption. Predict the surviving invariants and regenerate the affected evidence. Explain the cost of the change and preserve the previous regression. No complete C solution is supplied: the implementation and its defense are learner work.
