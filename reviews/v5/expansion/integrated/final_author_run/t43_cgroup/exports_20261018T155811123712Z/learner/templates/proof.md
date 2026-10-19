# Bounded proof and analysis record

Claim and project/objective IDs:

Implementation revision, standard, integer widths and target:

Input relation, capacity and valid-storage/lifetime assumptions:

Invariant at entry, initialization proof, preservation proof and exit implication:

Termination measure and why each accepted transition decreases it:

Checked arithmetic before evaluation; unchanged-state condition on rejection:

Frame condition: every object that may change, objects that must not change:

Resource bound with simultaneously live storage and callback/interrupt assumptions:

Tool, version, exact command and actual result (not-run/unavailable/pass/fail):

For CBMC, state input bound, unwind bound, unwinding-assertion outcome and property list. The supplied `models/prefix_cbmc.c` is a harness, not a completed proof. Example command, only after the tool is available:

```sh
cbmc models/prefix_cbmc.c src/foundation.c -I include --function main \
  --bounds-check --pointer-check --signed-overflow-check --unwind 17 --unwinding-assertions
```

For WP/ACSL, add explicit valid-span and separation requirements, success/rejection postconditions and `assigns` clauses to the actual chosen implementation. Record every unproved obligation, assumption and model of callees. For Eva, state initialization/context assumptions and unresolved alarms. Neither a harness file nor a finite C test is a proof receipt. If the tool is absent, preserve the handwritten proof and keep the automated claim open.

Counterexample or changed input that would refute an overbroad claim:

Reviewer explanation and delayed transfer date:
