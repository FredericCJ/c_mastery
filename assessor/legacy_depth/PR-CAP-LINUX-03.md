# PR-CAP-LINUX-03 capstone composition assessor analysis

Withheld technical analysis. The learner receives the shared capstone contract, project supplement and public fixtures. Assess the specific integration claims below; passing a retained component gate is not complete capstone acceptance.

<a id="t3207b"></a>
## T32.07.b

The publication point distinguishes construction rollback from running-service recovery. File close/registration removal may have operation-specific failure semantics; do not blindly retry close on a reused descriptor. Require a monotonic ownership ledger, not merely a final allocation count. Reject an implementation that registers cleanup only after a later failing operation or keeps callbacks pointing to expired locals. The bounded scope capacity and conditional cleanup modes are part of the integration contract.

**Independent expected relation:** Every failed prefix releases exactly its acquired resources once in legal dependency order; no partially initialized service is published and no accepted external output is falsely rolled back. Successful destruction releases all resources once. The cleanup.scope variant has identical effects and release identities; its runtime protocol is not automatic language unwinding.

**Reject superficial evidence:** Cleanup that passes the happy path proves every acquisition prefix is safe. The learner must show the causal boundary and an independent discriminator; an unrelated compiler/runtime failure is not a semantic negative control.

**Delayed transfer review:** Registration cancellation becomes asynchronous: cleanup must wait for terminal callback quiescence before releasing its context. Select a new compatible event history and require predictions before execution. Check updated preconditions, resource accounting and retained successful behavior, not recall of the supplied values.

<a id="t3506b"></a>
## T35.06.b

Queue occupancy excludes the active output and uncommitted input candidates. A fixed queue does not bound allocation before admission. An overwrite policy changes semantics and cannot preserve the original acknowledged-delivery contract without an explicit exception.

**Independent expected relation:** Accepted IDs1,2,3,5 and rejected4,6 match the sequential fixture; no queue exceeds3. Whole-system backing includes all six possible frame buffers, and continued overload yields finite rejection without hidden unbounded retention.

**Reject superficial evidence:** Backpressure at one queue bounds the entire system, including hidden active and input buffers. The learner must show the causal boundary and an independent discriminator; an unrelated compiler/runtime failure is not a semantic negative control.

**Delayed transfer review:** Change reject-newest to overwrite-oldest only for explicitly replaceable telemetry. Specify ownership release and which acknowledged records may be lost; commands must not inherit the policy accidentally. Select a new compatible event history and require predictions before execution. Check updated preconditions, resource accounting and retained successful behavior, not recall of the supplied values.

<a id="t2104b"></a>
## T21.04.b

A logical terminal flag alone does not synchronize memory reclamation. The event-loop or thread contract must establish quiescence. The conservation law detects silent loss and double disposition; identity-based release tracking detects offsetting double-release/leak errors.

**Independent expected relation:** Final delivered[1,2], discarded[3,5], queued0, active0 and four exactly-once releases. Repeated shutdown/completion does not alter counters; no buffer is released while adapter access remains possible.

**Reject superficial evidence:** Cancellation requested means the output buffer can be freed immediately. The learner must show the causal boundary and an independent discriminator; an unrelated compiler/runtime failure is not a semantic negative control.

**Delayed transfer review:** The final completion callback may synchronously trigger destruction of its owning service. Specify a callback API that performs no later access to the destroyed object. Select a new compatible event history and require predictions before execution. Check updated preconditions, resource accounting and retained successful behavior, not recall of the supplied values.

