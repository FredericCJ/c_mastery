# Assessor analysis — replay successor

Do not publish this with learner scaffolds. Read the original X-T76-009 analysis
and retained HAMT checkpoint proof as well; this successor does not erase them.

The abstraction is a finite pair of reducer histories plus a monotonic committed
effect prefix. Unknown remote input is not equivalent to a known equal value:
later information can change the subsequent prediction rule. Rebuild begins at
the corrected snapshot and rederives each unknown prediction from earlier known
inputs. The confirmation scan is independent of value equality. Because every
tick below confirmed is known and conflicts are rejected, a pending effect whose
tick is at or below confirmed cannot change under any admitted correction.

`head-confirmed<=horizon` is preserved by backpressure. An eight-tick array is
not an unbounded rolling protocol; no eviction/epoch-reuse proof is claimed.
Revision does not guard external store concurrency: external serialization and
exclusive store ownership remain premises. Stored canonical comparison detects
stale wrapper copies in the supported single-writer model, not a distributed CAS.

Each new operation copies its control, computes outputs, persists a canonical
snapshot and only then publishes. The store's final flush can fail after complete
new contents are durable. Therefore local-old/disk-new is permitted and poison
cannot be converted to successful rollback. Recovery selects old or new through
the actual store; the caller must not retry from stale local state. The eight-key
transaction's37 fault points are counted after a checkpoint. Recovery tests must
not silently skip store_begin's checkpoint work in a different starting state.

Snapshot digest is checked before parsing; all ranges and reconstructed states
are checked; re-encoding rejects noncanonical inactive bytes. An attacker able to
rewrite the whole snapshot and digest is not authenticated by SHA. Endianness is
explicit at every integer field, and host object padding is never identity data.
The Python model independently builds canonical bytes and folds integer state;
trace digests include all intermediate states and pending/acknowledged effects.

The effect sink's namespace and prefix identity are sufficient only under its
exclusive live-log premise. Persisting an intent and later performing an unrelated
device action cannot atomically commit both; the counterexample increments twice.
The child-exit test instead terminates after the actual inbox commit, reopens both
stores and retries the same identity. It establishes local-process behavior under
successful fdatasync calls, not physical-media survival or exactly-once devices.

Controls1/5 reject missing suffix rebuild/horizon admission;3/6 reject digest or
reserved-byte suppression;4/7 reject early publication/stale store authority;
2/8 reject speculative effects/identity-blind feedback;9/10 reject trailing data
or alternative segmentation. Wrong-endian control11 fails independent PPC64
canonical traces. A hidden-time object must fail the dependency allowlist, while
the linked pure core may use declared memcpy/memset/memcmp and actual reducer/
decision functions. Raw write/exit belongs solely to the target observation harness.
An undefined-symbol audit alone does not detect every hidden global: inspect
definitions/call sites and use the separate hidden random-state counterexample.

Expected state: after correction2=1, ticks3/4 predict1 and remote final position10;
after order2,1,4,3 the confirmed lengths are0,2,2,4. Duplicate2 leaves revision9.
Initial revision1 plus four advances and four distinct corrections gives9.
All effect decisions keep this session revision; identities remain timeline9,
ticks1..4. Values are twice the specified presentation, and acknowledgments leave
the next counter at the last issued tick while pending becomes0.

Sealed decision: ask whether to replace a speculative display immediately or
delay it until confirmation, then require a quantified latency/rollback tradeoff
under the same effect correctness contract. Neither answer may label an external
nontransactional action reversible. For delayed assessment choose horizon2 with
out-of-order corrections and an uncertain sink acknowledgment; require explicit
WAIT/resumption and no duplicate intent. Change the initial random seed and
compare full canonical bytes without consulting worked values. A learner who
passes public anchors but cannot explain confirmed-prefix immutability, disk-new/
local-old uncertainty or a changed-schema identity domain has not met R2/R4.

Cost assessment must count full invariant rebuilds as well as the optimized
suffix: the public defensive API validates the entire history, so total per-call
cost remains O(head), even when the recomputed suffix is short. Native size is
observed per ABI; wire size stays512. No heap allocation occurs in the core.
The sink's actual HAMT update consumes bounded arena cells; durable page traffic
and callback latency are not captured by a pure-core CPU benchmark. Compare
equivalent correct workloads, retain raw samples and explain sampling noise.
