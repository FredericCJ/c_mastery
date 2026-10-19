# Version and predicate comparison inspection

On2026-10-17 the author read the actual PostgreSQL18 transaction-isolation page,
including sections13.2.2 and13.2.3. The first describes a stable transaction snapshot,
retries on concurrent writes, and possible serialization anomalies. The second
explains dependency monitoring and predicate information, with a two-transaction
example whose results cannot arise in a serial order. It also requires retrying
aborted transactions and treating their observed results accordingly.

The course uses that distinction to motivate its original two-key write-skew
exercise. Its complete predicate universe is exactly two keys, including keys
whose value is zero; an observation mask can therefore record every relevant
input. PostgreSQL has dynamic rows, index/predicate state and a different conflict
detection implementation. The course neither reproduces that database nor claims
that two-key tests establish database serializability. The source record pins the
retrieved HTML; inspection, code execution and learner assessment are different
kinds of evidence. Published C17 remains an access gap.
