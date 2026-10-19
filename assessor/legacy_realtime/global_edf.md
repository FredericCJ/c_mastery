# T52.10 assessor analysis

The original miss depends on sequential ownership: the long job loses its
first service interval and cannot recover it by occupying both CPUs. Keeping
two small positive unit costs while scaling N gives utilization1+2/N and a
one-unit miss; it approaches1 without attaining the zero-delay case. The
registered Linux source gives the corresponding global-versus-partitioned
distinction and sufficient bound. Do not infer that every failing bound is
unschedulable: the two independent jobs9/10 and3/10 give a direct counterexample.

Each explicit job generation remains represented. An overlapping release is
rejected while its predecessor continues; the rejected workload cannot be
claimed fully serviced. The old retained periodic simulator overwrites at a
new release, so only the declared no-second-release horizon is used for its
differential comparison. Later source/queue designs need a separate policy.

DAG topology is validated before output publication. A tick-start ready set
prevents a dependent strand from spending the same interval in which its last
predecessor completes. CPU selection excludes both already-serviced and
overhead-reserved strands. The duplicate-dispatch control intentionally produces
impossible service without relying on undefined behavior; conservation exposes
it. The complete-set test and exact generation labels provide stronger evidence
than an ordinary total checksum.

The dispatch model distinguishes the serialized queue-lock interval, waiting
for that lock, and receiver-CPU migration work. Lock release takes effect at
the next tick, preventing two queue charges in one interval. A reserved strand
cannot be stolen while either charge is in progress. After paying, one service
unit executes before reselecting. This is an explicit bounded nonpreemptible
dispatch policy; it is not a statement that every real scheduler behaves this
way. Greedy CPU order can create migration churn; preserve its cost in traces.

The Python oracle uses node sets and already-reserved event lists rather than
copying the C stage/bitmask representation. Exact row comparison includes
ready/deadline outcomes, every CPU interval and unfinished/rejected generations.
The input corpus contains misses deliberately. A report that counts every
completed model invocation as a schedulable task set fails the assessment.

The staged result and ready-set arithmetic have explicit finite bounds. All
model times/costs are≤256 and at most64 strand bits are shifted within a uint64_t.
The sufficient test uses exact integer service over a divisible horizon and
at most8 terms. Strict Cortex-M4 execution audits the actual compiled helper
closure and stack size, but the model is not a Cortex-M multicore scheduler.
Physical locking/migration costs and a native SMP implementation remain
separate observations, not silently inferred from this finite model.
