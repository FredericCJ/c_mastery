# T75 owned deadlines and fractional admission: assessor analysis

This extends X-T75-001 and X-T75-004; do not publish it into learner workspaces.
The original rubric IDs remain unchanged. Source passages are the existing
gRPC/Go and token-bucket registry entries; no new reading is implied by this code.

For X-T75-001 R1-R2, construction first uses the retained deadline constructor to
validate min/capped addition, then takes the parent owner before allocation.
Failure releases that owner. The parent core pointer remains valid as long as the
child exists. A final child release also decrements its parent edge; at most eight
cells can be deallocated in one release. Refcount updates acquire no new owner
from a raw possibly dead pointer. Cancellation atomics publish only the request;
they do not publish every application payload or reclaim arbitrary contexts.

R3 requires actual two-thread join before ticket acknowledgement. Inspect the
pre-release count assertion: mutant12 omits retain and must fail here, before a
dangling dereference. The 1,364 word model treats acknowledgement as one-shot and
models cancellation precedence over expiry while any borrower remains. A creator
owner and a borrower owner are different resources; count only successful retain
operations. The four construction cuts cover parent allocation and all three child
allocations, including a failure after two completed children.

For R4, export at time14 leaves six ticks; subtracting B2 and P1 leaves three.
Receiver1003 therefore expires1006. For actual delay d<=B, the remaining end-to-end
budget cannot exceed R. The imported local parent can shorten it further. No
bound follows if B is merely an estimate that the actual network exceeds.
Cancellation after export is not automatically transmitted to the remote scope;
the envelope propagates a deadline budget, not a cross-host cancellation channel.
The student must name this limitation. A remote acknowledgement requires its own
protocol; the native join demonstration establishes local backend quiescence only.

X-T75-001-DELAYED is implemented by the cell tree: release the root creator owner
and continue checking a retained child, then cancel through a still-owned parent
edge. Do not let a student claim that atomic cancellation makes a stack address
stable. The public header exposes fields for education; C provides no static
guarantee against illicit structure copies or early ticket acknowledgement.

For X-T75-004 R1-R2, let missing numerator N=(C-T)q-r. At most
(2^32-1)(2^16-1) is needed. If e>=ceil(N/p), saturate before multiplying. Otherwise
e*p<N and adding r remains bounded by (C-T)q. Dividing once yields exact integer
tokens and remainder. When full, N=0 and saturation immediately discards excess.
This proves the large-time path without relying on unsigned wraparound followed
by clamping. Refusal for impossible cost/backward time precedes refill; an admitted
attempt that lacks credit retains refill. These are different failure classes.

For R3 the oracle's rational representation and repeated history generation are
independent of this bounded-numerator algorithm. Mutant21 rounds every update to
whole tokens and is exposed by the three one-tick calls at rate1/3. Splitting an
idle interval must not change credit, including when saturation occurs midway.

For R4, stage two valid refills and then commit either two debits or no debits.
Refill on denied spend is legitimate, so "rollback everything" is an incorrect
repair. Mutant22 charges average while peak denies. Comparison policies use equal
nominal rate and quota but explicitly different boundary envelopes; do not accept
a performance conclusion from the fixed-window boundary counterexample.

The delayed migration transfers a rational state, rebases local time, and requires
single admission authority. Export is nonmutating to permit serialization failure
without altering the source, so activation of both snapshots is a caller protocol
violation. A durable fenced transfer is not implemented by this arithmetic example.
Cost arguments: O(1) bucket work/storage with bounded divisions; zero allocation;
scope checks/releases O(depth<=8), one allocation per scope, reference-count cache
traffic and a thread-safe allocator requirement. Atomics need not be lock-free on
every target. Actual generated code, elapsed time, stack and code-size measurements
must be read from separately pinned campaigns; this analysis invents none.
