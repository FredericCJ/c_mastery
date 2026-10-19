# T76 cost and enforcement dossier

Use all thirteen named rows from `modern-cost-contract.md` for each original
exercise and its required successor. This is a required dossier, not an optional
optimization appendix. Read native sizes only from the current compiler/ABI
observations. Explicit wire lengths and finite operation counts are model facts;
CPU/elapsed samples are observations with cache, scheduling and instrumentation
uncertainty. Never substitute a faster weaker contract in a comparison.

| Dimension | Domain-specific obligation |
| --- | --- |
| Representation | Distinguish native pointer/arena controls from RPL1/RPA1/RPR1/RPW1 bytes, exact widths and canonical inactive fields. |
| Reachable storage | Count persistent HAMT roots, retained events, retired identities, tombstones, OT contexts, proof siblings, owned partial bytes and rollback snapshots. |
| Lifetime | Name the owner and terminal point for each store/log, local descriptor, callback borrow, pending effect and changed membership epoch. |
| Mutation/aliasing | Keep controls externally serialized; do not relocate escaped HAMT arenas or alias staged output with controls unless the API explicitly permits it. |
| Allocation | Inject actual HAMT allocation failures and page transaction failures; allocation-free joins still have finite capacity and stack/storage costs. |
| Movement/copying | Count staged state copies, canonical encoding, partial-I/O prefixes, projection rebuild and snapshot/epoch compaction. |
| Destruction | Distinguish clearing a copied value from closing a descriptor, recovering a poisoned store or proving no in-flight old-epoch messages remain. |
| Dispatch | Inspect actual direct imported reducer/HAMT/hash calls and indirect page/transport callbacks; host sanitizer helpers are separate from a strict core. |
| Concurrency | State serialization, message/callback lifetime and join premises. Actual pthread projection schedules and TSan are not arbitrary-interleaving proofs. |
| Failure | Charge rejected capacity, ambiguous persistence, retransmission, missing Merkle nodes, malformed wire and rollback backpressure. |
| Compilation | Measure clean, unchanged and copied-header-change builds with actual compiler inputs; keep canonical sources untouched and timing uncertainty explicit. |
| Binary/stack | Inspect code sizes, stack-usage files, link dependencies and implicit memory helpers; constant finite capacities do not mean zero stack cost. |
| Boundary | Specify wire schema, namespace, origin/identity authority, counter exhaustion and changed ABI/endianness. No protocol proof follows from C field types alone. |

Required comparisons and their equal-contract premises:

1. Hash sixteen-word rolling schedule versus sixty-four-word schedule over the
   same message and SHA-256 result. Both retain complete length/padding semantics.
2. Selective Merkle traversal, full node traversal and direct-leaf reconstruction
   against the same trusted domain/count/root. Every mode verifies the resulting
   root before publishing. Compare client hash count and framed requests/bytes;
   report that CPU timing includes the synthetic peer callback.
3. Bulk versus one-byte feeding of the same complete bounded RPW1 stream. Both
   enforce identical depth/field/byte budgets and require the explicit terminator.
4. PN state versus operation history under identical unique-writer and retry
   assumptions. State retains each historical component; an operation log must
   retain enough event identity to suppress retransmission. Do not compare a
   deduplicating state merge to an unchecked sum. Compute storage growth from
   writer/event count and report the fixed-profile native and wire observations.
5. Checkpointed durable commit versus reopening/recovery: count accepted write,
   read and flush calls under the exact page adapter. A pre-checkpointed37-boundary
   transaction does not account for a prior WAL checkpoint. Do not advertise the
   local fdatasync process duration as physical media latency or power-cut proof.
6. Predicted suffix replay versus a full corrected fold under identical complete
   state and effect-commit rules. The defensive public session API additionally
   validates the full history; include this O(head) work when discussing the
   suffix optimization. Speculative replacement cannot skip authoritative
   idempotent-effect bookkeeping to improve timing.

All implementation variants remain finite teaching models. An unmeasured
physical delay, network contention distribution, independent reviewer judgment
or learner's own workload stays explicitly unobserved. That does not excuse
missing executable support for the local experiments. Canonical author results
and raw samples are indexed by `replicated_cost_matrix.json` and the individual
source-bound reports; the learner must reproduce and challenge them independently.
