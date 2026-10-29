# Assessor analysis for the X-T75-005 successor

The rejected source prefix has length `r=2^b mod n`. The remaining interval
length is an exact multiple of n, so each residue occurs equally often.
Accepted outcomes are uniform only under a uniform source sample. The full
32-bit recurrence has odd increment and multiplier congruent to1 modulo4;
the xor-right shifts and odd multiplication are bijections on32-bit words.
This supports a full-cycle finite counting argument, not independence between
successive outputs or clients. Identical seeds produce identical schedules.
The bounded64-draw API makes refusal explicit even if a sequence is
unfavorable. Rejecting a draw advances generator state but never publishes an
output or admitted attempt. A malicious uniform-source claim should lose R2
credit even when the residue test passes.

The actual implementation borrows the immutable cancellation chain through
`struct rzf_borrow`, and separately borrows its transport and limiter. Every send is
charged before calling the sink, including dropped original/retry sends.
Cancellation changes admissible future starts; it cannot release the flight's
scope or reverse a previously observed receiver effect. The16 loss masks make
that distinction visible: any delivery causes total3 once, subsequent
deliveries are duplicates, and all-loss yields total0. A valid peer ACK after
cancellation settles the actual retained flight. The last scope deallocation
then follows finish, not cancellation or creator-reference drop.

The adapter uses the imported transport RTO as a lower bound rather than
pretending application jitter replaces TCP's own timing. For base2, a plan
made at time0 has a candidate0..2 but actual flight deadline1000: all outcomes
collapse to1000. Planning at the RTO preserves the small spread. The actual
deterministic population model records this counterexample for all policies.
Likewise correlated seeds remain synchronized. Different work/latency
observations across full/equal/no jitter depend on the selected outage cutoff;
there is no universal best policy claim.

The delayed case uses the strict inequality `due < deadline`. With now1000,
retry-after9000 and deadline10000, the staged plan is refused and its attempt
counter/output preserved. A consumed random sample is not rolled back. The
flight survives; finish is BUSY until an independently accepted ACK or a
synchronous model abort. A real reactor cancellation adapter would require
its own backend-quiescence acknowledgment, not merely this local reset.

Mutant41 removes the rejection prefix. The8-bit/span3 case immediately exposes
unequal residues and the wrong PENDING result. Mutant42 omits retry admission
to the fractional limiter. A one-token limiter admits only the original;
the positive retains its plan and performs no transport retry, whereas the
mutant sends and violates both the limiter and sink-call assertions. The
positive must pass first; arbitrary process failure is not evidence of either
semantic mutation being detected.

R1: require original-plus-retries accounting, e.g two layers each allowing
three retries can cause16 deepest sends rather than4. Timeout indicates an
unknown effect, not known absence. R2: inspect actual imported source calls,
uncharged/before-deadline starts and all retained owners. R3: compare every
model row and exact receiver/ack trace; do not accept a result histogram alone.
R4: demand the raw virtual-time event sequence and explain both correlated
seed and early-RTO collapse counterexamples before reviewing a delayed
independent transfer. The timing model cannot provide physical latency or
independent learner evidence.

Costs: schedule/random selection is bounded by64 rejection draws and at most
three exponent-doubling steps. Controller state is fixed; one scope reference
is retained and no additional adapter allocation occurs after initialization.
The actual transport has a much larger borrowed fixed parser/packet footprint
and hosted dependencies. Test population arrays are harness storage, not per
operation representation. Any end-to-end cost comparison must count that
transport storage and actual source/link dependencies, and preserve identical
loss/identity/deadline contracts.
