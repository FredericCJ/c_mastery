# Private dispatch and removal analysis

10.a requires caller and callee inspection. All versions import the exact existing
stream provider. There is a common SFWrite dispatch and, depending on adapter,
a switch or operation-table call inside it. LTO may expose the provider and adapter;
a separately compiled shared backend excludes its body from that link-time view.
Inspect the actual executable and shared object, not a count of `->` in source.
Require the ELF dependency as evidence of actual loading. This variant loads at
process startup; hot reload/unload safety is outside the contract and cannot be
inferred. The retained binary's branch/call evidence supports only its tool flags.

10.b has an independent arithmetic oracle: t=n+2, k=ceil(t/q), offered=k*t minus
q*k*(k-1)/2. The two/three-backend traces repeat known schedules over3000 frames;
derive totals from those schedules without running the implementation. Independently
reconstruct checksum and all frame bytes. The224 cells each preserve nine measured
samples after one warmup. Require actual representation bytes and copies/allocations
ledger. A timing advantage unsupported by these equal correctness checks fails.
Do not force a conclusion that payload64 must dominate: one-byte backend still
performs66 dispatches, whereas all-byte backend performs one. This interaction is
the changed-condition analysis, not an inconvenient result to exclude.

10.c reference decision: retain runtime dispatch for the comprehensive multi-backend
component because backend variation and load-time linkage are explicit requirements.
Permit the fixed-one-byte specialization as a separate build choice only where
backend-one is fixed by the consumer contract and the measured evidence justifies
maintenance cost. The specialization shares validation, owns no additional resource
and removes its kind/table storage. It still uses the stream callback. The supplied
conformance run demonstrates its narrower behavior, not equivalence to three-backend
selection. Adding backend-all changes the switch case and operation registry;
call sites using the stable erased signature need no new ownership protocol. A
hard-coded fixed call site must change or reject the new requirement. A decision
that names these concrete sites and remaining lifetime obligations earns more than
one claiming that tables are always faster or always more maintainable.

Semantic mutants corrupt bytes and silently choose quota-one instead of the
requested table backend. Both must compile and fail intended conformance checks.
Malformed table tests are runtime admission, distinct from C function-type
diagnostics. Assess code annotations, causal resource ledger and a changed backend
proposal independently from test outputs. Keep original seven-day delay and withhold
this analysis. No author experiment establishes reviewer independence or learner mastery.
