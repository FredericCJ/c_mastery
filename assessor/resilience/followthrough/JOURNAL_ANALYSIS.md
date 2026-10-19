# T75.08 technical assessment: effect and acknowledgement are different facts

The journal's durable state is one ordered prefix, not a separately updated
counter plus lookup table. Each request record carries enough input to replay
actual rz_receive and verify its original response/duplicate decision. Recovery
therefore detects many semantic corruptions even if a superficial checksum is
recomputed. It does not authenticate a malicious writer with access to the device.

The body flush establishes that a subsequently durable commit marker references
an already durable body. The final flush establishes the acknowledgement premise.
Failure before a reply is ambiguous: the record may or may not be durable. The
live control deliberately remains unchanged and poisoned, so it cannot issue a
different identity based on an unjustified rollback assumption. Recovery resolves
the durable prefix. A retry then either replays the original response or performs
the still-absent effect. No double application occurs within retained identity
scope. Once retention expires, the same numerical identifier may represent a new
admitted effect; the code does not claim eternal exactly-once processing.

Control81 removes final flush: a successful call can lose its commit region under
the no-new-persistence reset, violating acknowledgement-implies-durable. Control82
removes body flush: commit-region-first persistence produces a full record with
an unflushed body, and recovery cannot accept the expected old/new valid prefix.
Control83 lowers the staged resource fence to the supplied stale token and allows
the synthetic forbidden delta. All controls compile and must fail RZ_CHECK; a
build error, unexplained crash or sanitizer report is not the intended evidence.

Do not estimate cut counts from prose: the actual loops are two operation
kinds times three persistence modes times68 cut positions =408 append scenarios;
six recovery-interruption scenarios; and three modes times70 positions =210 mirror
scenarios. The independent Python model additionally includes cut0/no-cut and
therefore414 append cases. It constructs canonical wire bytes with Python integer
arithmetic/packing and a map of retained request/results; it never calls C to
derive expected bytes. The request cross-product has2,916 cases. Native file tests
use a real child exit after successful fdatasync and recover a missing reply; no
power-loss or filesystem-wide durability claim follows from those observations.

For the delayed variant, two independent replicas each enforce only their locally
known high-water fence. Fence5 at primary cannot retroactively notify delayed
replica that still sees4. The correct original example is authority serialization
with readonly follower snapshots, not a client-side token comparison. Mirror
refuses divergent prefixes, preserving evidence of the flaw. Availability during
authority loss is explicitly sacrificed. Deriving replicated failover requires
additional agreement/recovery assumptions from T76; do not label this single-owner
repair consensus or claim a fencing token alone supplies replication.

Memory/control costs are fixed; replay time is O(records) and storage64 bytes per
accepted operation including duplicates and issued tokens. Two durable flushes
per new record are visible costs, not hidden in a constant-time claim. Partial
transfer loops can take arbitrarily long if device callbacks block; zero progress
refuses instead of spinning. Existing request lookup scans4 records. Mirror
preflights the entire existing prefix before writes, then appends each missing
record with its own barriers, trading throughput for a small recovery argument.
The log's finite capacity is a deliberate refusal boundary, not an unimplemented
compactor silently credited as present. The service's durable notification
extension must preserve its own capacity-before-effect and acknowledgement rules.

Enforcement boundaries: ISO C object/representation rules underlie the code;
static assertions bound its 32-bit-int wire profile; runtime validation checks
syntax, sequence, time, fence and result; POSIX/device contracts supply persistence;
tests model selected failures; trusted exclusive device ownership and clock epoch
remain discipline. Source checksums are not authentication. Arbitrary drive errors,
independent learner performance, a new physical clock epoch and replicated
authority failover are not established by this campaign.
