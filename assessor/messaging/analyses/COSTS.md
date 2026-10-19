# Assessor: messaging cost and progress

Assess each T73.01..10 `.d` objective separately. The public comparison checks
conservation and coherent pairs; it deliberately has no performance pass
threshold. Reject a claimed improvement achieved by dropping accepted messages,
forgetting close/drain, relaxing ownership or hiding an unbounded retry. Inspect
the learner's generated code and current source pins before considering timing.

The registry alternative uses channel-before-registry lock order. A stack node
remains linked only while its select call is active. Clearing its sticky flag
before scanning creates the relevant proof split: an earlier publication is
visible to a subsequent protected scan; a later publication either supplies a
value to that scan or marks the registered node before sleep. A condition wait
atomically releases the registry; the notifier cannot slip between release and
registration. Notifications do not transfer ownership of a value, and a waking
selector can still lose to another selector. A single notification boolean
needs no wrapping sequence counter. The wrong control removes the sticky flag;
the controlled hook publishes after the empty scan and its extra wait count is
the failed assertion. Closed accepted values remain drainable; the second
control rejects that invariant.

The algorithm has O(2) scanning but O(W) notification and a stack node for each
active call. Its extra locks and scans can be more expensive than one common
mutex. It is a correct comparison, not evidence that lock splitting necessarily
helps. No registry pointer may survive unregister. POSIX pthread cancellation
is outside its contract: condition-wait cancellation would require a cleanup
handler to unlink the node and release the reacquired mutex. Cooperative
`mfr_cancel` does neither asynchronous stack unwinding nor arbitrary preemption.

For sequence comparison, count both BUSY and successful reads. A variant that
retries until success cannot be compared as a one-attempt reader. Atomic payload
fields keep failed sequence samples within the C abstract machine; an ordinary
non-atomic payload would not become race-free merely because the reader rejects
an inconsistent sample. The separate RC11 campaign, not the timing run, assesses
the selected weakened orders for finite clients.

For transactions, every commit must increase both values together. Timestamp
reservation may advance the clock without a commit, so timestamp count is not
the success count. The mutex alternative serializes a larger region and has
different progress properties while meeting the same paired-value contract.
For snapshots, a two-field copy removes pin lifetime from subsequent reads but
charges the copy and caller storage; large allocated immutable graphs change
that tradeoff. The one-pin timing workload retains two live versions, not the
maximum possible footprint of every version workload.

Per-item and batch16 demand both deliver N values through the same executor,
capacity and callback. Expected request counts are N and ceil(N/16). A request
does not imply immediate scheduling and an end-to-end timing includes the
executor wakeup path. Keep the context until join and the last handle release.
For operation/actor cycles, synthetic descriptors are resource tokens with
counted drop callbacks; measured constant-time drops do not bound arbitrary
destructors. Supervisor replacement performs two closes and two opens per
iteration, plus16 initial opens and16 final closes. Actual failure cleanup is
assessed in the distinct fault campaign, not inferred from the normal timing.

The nursery query reports attributes of actual running threads through a GNU
API. It does not touch every stack page, find maximum frame depth or measure
resident memory. `.su` files are compiler observations per function, and the
link map/symbol report are observations for the chosen ABI and flags. Thread
library stacks and callback costs require separate lifetime/call arguments.

The adversarial steal schedule lets both clients execute yet one lose every
round. Its extension is bounded by finite index retirement on one instance;
repeated initialized instances remove any supposed fixed per-client success
bound across work episodes. This does not show an individual `steal` call fails
to return. POSIX wake order and arbitrary callback duration independently defeat
unsupported deterministic latency claims. Require learners to distinguish
successful acquisition, method completion, global progress and scheduler
fairness, and to say which assumptions were added for any conditional bound.

Expected evidence: independently checked counts, raw repeated timings and
observer samples, per-thread retries, exact object/layout sizes and lock-free
queries, current source hashes, maps/disassembly/stack observations, all13 cost
dimensions, and a changed-assumption implementation. Author-team inspection and
passing tests establish support for these bounded experiments, not independent
learner performance or an exhaustive concurrency proof.
