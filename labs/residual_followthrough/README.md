# Following an original obligation through its successors

The original project task and its changed-condition variant remain the assessment
contract. A newer component may supply a missing mechanism, but a component name,
passing anchor, or completed child count is not enough to establish that contract.

For each selected parent, keep a dossier with four entries:

1. Copy the exact original requirement and changed assumptions. Identify what is
   executable, what requires a derivation, and what requires an observation.
2. Name the producer interface and its lifetime, capacity, ordering and failure
   premises. Link the public contract of the successor used by your implementation.
3. Predict the boundary case and the broken implementation that should fail it.
   Record the actual compiler, arguments, result and output. A failed compilation
   does not establish that a behavioral control detected the intended defect.
4. State the evidence limit. A finite model, native execution, emulated CPU,
   audited target image, physical measurement and independent learner attempt are
   different observations. Keep each in its own column.

Use the original project export and its unsolved sources. Existing assessor
reference executions are available to the assessor separately and do not count as
your implementation, delayed attempt or review. The original minimum delay is
unchanged. There is no new replacement task that reduces an original requirement.

For T18/T21, draw ownership and generation transitions before running the failure
paths. For T23, distinguish operation-specific interruption and retry rules: a
copier's retained suffix says nothing about whether repeating another operation
is safe. The alternative-OS interrupted-close task still needs a real execution
environment with the changed contract; a Linux fault seam is a different test.

For T36, keep validation costs separate from kernel work and derive the complete
invariant, including ancestor relationships. For T38 through T42, follow a byte
through every acceptance boundary: register/DMA, frame channel, transport, parser,
application and physical wire. For T43 through T50, use the specific mechanism's
cost, fault, recovery or machine-transition observation, rather than importing
an unrelated benchmark or model pass.

T52's stopped-clock correction is a useful audit exercise: write a timeline with
raw time and logical time in separate columns, identify which operation owns the
elapsed interval, and predict how an incorrectly admitted stopped operation can
count it twice. Compare your diagnosis with the original failing port only during
assessment. Current guard results are author evidence; physical timing is separate.

The exact origin and constituent-task index is retained in
`research/v5/expansion/residual_followthrough/evidence_map.json`. Its references
preserve the historical records; they do not rewrite those records as if their
original observations had already passed.
