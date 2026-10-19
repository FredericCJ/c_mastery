# T25.02 specific assessor analysis

## Partition ownership and the delayed mutation contract

For n>=0 and 1<=w<=4, boundaries floor(n*i/w) form a nondecreasing sequence
from0 to n. Adjacent half-open intervals meet without overlap. Thus every
input contributes once, even when an interval is empty. The selected n<=16
bound gives a sum within [-34359738368,34359738352], well inside int64. Worker
result cells are distinct memory locations. Join orders their final writes
before the controller's merge; merely waiting a guessed duration would not.
The shared variant uses one mutex around each compound addition, not one
lock around the load and another around the store.

The snapshot copy and version are one critical section. Changing the original
after that section cannot change the copied values. The old result therefore
remains10/version7 even when the source has become396/version8. A version
label does not make the copying loop safe without a lock or ownership transfer;
a before/after ordinary version check can itself race. Here no lock-free
version reuse proof is claimed. The concrete hook gives a controlled source
mutation after capture, while the source's own mutex and thread join avoid
inventing an undefined-value oracle.

## Exactly one terminal owner per admitted work item

Admission's successful state change is the transfer point. Full/closed errors
must not leave an orphaned QUEUED state outside the ring. Dequeue is a distinct
transfer to the worker. Close settles only the IDs still in the ring, so it
cannot report worker-owned ID0 as canceled while the worker later reports it
failed. The single mutex serializes these transitions and count/head/ring
updates as one invariant. A sentinel-only policy would need one terminal
signal per consumer, capacity/progress treatment and protection against a
consumer taking more than one; terminal state avoids those concerns in this
chosen nonblocking interface.

The actual failure is an explicit known local failure after dequeue. It
settles FAILED once and does not retry. After an external effect or process
death, UNCERTAIN or a transactional/idempotent protocol may be necessary;
this target has neither and cannot claim exactly-once external effects.
Destruction requires separate quiescence/admission exclusion, not merely an
empty ring. The public busy check is a diagnostic for retained ownership,
not a concurrent reclamation algorithm.

## Equivalent counts and non-equivalent costs

Every mode performs the same admitted exact count function. Private arrays
change the location of writes and add merge work; shards change lock identity;
batching coalesces equal-key increments while preserving their multiplicity.
The scalar oracle has no pthread decomposition, so omitting a worker's merge
is visible. All arrays are validated before output publication and every bin
count is at most262144. Numeric-buffer admission is derived independently
from the number of arrays. Requested thread stacks are a separate fixed
1MiB per four-worker call, not part of that 4MiB buffer budget and not evidence
of resident memory or stack usage.

The large512KiB count array exceeds the reported L1D in the observed guest;
this is a selected cache boundary, not all cache levels. Both data distributions
matter: hot keys can turn batching into fewer shared updates, while distinct
keys add comparisons/flushes. Full-call timings include thread creation and
allocator/zeroing costs. Small samples, unpinned scheduling and other workloads
limit inference; preserve the distribution instead of declaring one mechanism
always faster. Arithmetic correctness is checked outside the timed interval
for every returned count, and warmup has the same complete contract.
