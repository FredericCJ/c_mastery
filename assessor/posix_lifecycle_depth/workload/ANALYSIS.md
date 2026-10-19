# Private equivalent-workload analysis

The ordinary record function validates logical extent before conversion, copies
through a64-byte staging object and advances only by the actual short-read count.
Mutant1 permits64 bytes beyond logical EOF, returning ENODATA where the required
contract rejects ERANGE before I/O. Mutant2 treats one short positive return as
complete and copies zero-initialized missing bytes. Mutant3 publishes its first
three bytes before EIO. None requires undefined behavior to reveal the mistake.

The mapped control uses PM_MUTANT1 from the existing library. A32-byte record at
logical-end-minus8 remains within the mapped last page and therefore looks
accessible, but violates logical extent. The test rejects that weakened contract
before any timing comparison. Timing all8192 validated copies and the same digest
avoids comparing a borrowed pointer with a copied record. The explicit reader's
staging cost is part of its unchanged-output-on-error promise.

ABBA order and eight paired repetitions expose variability rather than proving
cache equality. Prevalidation warms both paths, and mapping setup is separate.
Sparse random access changes the address sequence over the same logically sparse
file. Wrapped calls count library dispatches, not hardware/kernel events. Prior
isolated truncation/protection helpers remain the actual process-fault evidence;
the new pread truncation test reports ENODATA and unchanged output. An unstable
backing file invalidates the timing contract, not just its average performance.

Require the learner's original delayed changed-condition argument and actual
implementation evidence. The author's reference samples and finite fault cases
do not prove every cache state, filesystem behavior, interleaving or mastery.
