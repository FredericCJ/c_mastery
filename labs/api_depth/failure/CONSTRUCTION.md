# T21.03–04: failure-complete construction

Use `construction.h`, the deliberately unfinished `construction.c`,
`construction_tests.c`, and the actual T18 `struct ac_ledger` provider. The original
criterion IDs and delayed variants in PR-T21-01 remain the assessment identities.
Read TAXONOMY.md first. Implement the resource ledger prerequisite yourself;
the author campaign explicitly imports the assessor T18 implementation instead.

Resource 0 is memory, 1 a channel, 2 a registration and 3 a second registration.
Both registrations depend on **both** base resources. The two base acquisitions
may exchange order. A constructor acquires three or four resources, then publishes
exactly once. Failure points 1..n precede each acquisition; n+1 precedes publication;
0 means no injected failure. Output handles remain unchanged on failure.
An initialized control block has stable storage until all handles and callbacks
have ceased using it. Its generation admits 1, 2 and 3, then retires without wrap.

Represent NEW, PARTIAL, ACTIVE, FAILED and STOPPED explicitly. The ownership mask
is an obligation, not an acquisition counter. A failed close may either retain or
consume its resource according to `struct ac_ledger.consume_on_error`. Inspect that
postcondition before retrying. Traverse reverse acquisition order, but never close
a base while either dependent remains held, including by an external transferee.
For example, if closing resource 2 retains it, cleanup leaves 0,1,2 held. A later
successful close of 2 permits 1 then 0. A consumed-on-error close of 2 instead
permits both parents immediately while its error remains recorded.

Implement nested, labeled and loop-based acquisition paths. Implement three cleanup structures as well: nested unwinding, descending cleanup
labels and a reverse loop. All may share one close-at-position primitive, but each
must perform its own traversal. Show where conventional unconditional fall-through
becomes a conditional dependency check when close can retain ownership. Compare
complete event histories, not just totals. Three control structures sharing a
release primitive are not three independent correctness oracles.

Transfer records contain resource and generation. Transferring registration 2
removes it from the constructor's mask but not from the ledger; cleanup must wait
for the external owner to release it. A numeric slot without generation is not an
ownership identity. Reject reinitialization of an ACTIVE object. Repeated cleanup
after success has no new release events. Reserve event-log headroom before acting.

`ae_after_publish` models an external committed effect. A failed notification
returns AMBIGUOUS and leaves ownership and publication intact. Ordinary cleanup
must refuse while this effect requires compensation. `ae_compensate` accepts an
explicit environment assertion that the effect is absent; it is a **model input**,
not an implementation of a remote compensation service. Explain what evidence a
real adapter would require and why a lost compensation acknowledgment is another
ambiguity, not permission to erase the flag.

Deliver the implementation, ownership-state diagram, pre/postcondition table,
full acquire/close/transfer traces, first/secondary-cause analysis and resource
bound. Work is bounded by four acquisitions and four close attempts per sweep;
the ledger is fixed capacity 64. Check all 16 close-error and 16 consumption masks,
both acquisition orders, all prefixes, three styles and both resource counts.
The supplied independent graph oracle checks 16,896 combinations. Add an
independent fourth-resource scenario before revealing the assessor analysis.
Run debug, NDEBUG, optimization and sanitizer configurations. Hosted harnesses
are verification dependencies, not part of the component runtime closure.

Hints: (1) write the owner after each transition; (2) derive a dependency graph
before the cleanup order; (3) record the primary error before running cleanup.
The rubric awards nominal trace reproduction only level 1; level 2 requires
correct retained/consumed ownership and failure preservation; level 3 requires
the changed dependency, generation and postpublication arguments plus independent
counterexamples. Actual delayed learner performance remains unassessed. No finite
grid proves the implementation or an arbitrary external backend correct.
