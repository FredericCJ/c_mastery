# Private T52.05 technical analysis

The adapter's copy is essential: the actual retained update commits `last`
and due records before charge validates `n`. With Q3 fully consumed at0,
raw charge(10,4) returns FULL after restoring3 and advancing last10; the adapter
returns FULL with its original last0/remaining0/credit3@10 intact. A later
valid call can still process the credit. The unchanged contract is neither
suppression of due work nor a claim that the producer was already atomic.

The inductive ledger proof starts with remainingQ and no credits. Update moves
amounts from future credits into remaining. Charge moves n back into exactly
one future credit. Validation keeps every intermediate sum<=Q, so unsigned
arithmetic cannot overflow despite finite counters. Coalescing at equal time
does not change eligibility. When storage is full, move the latest credit to
the new deadline and add n; this delays old capacity and does not create any.
Compare representation, not just final remaining: an earlier merged deadline
can appear to conserve Q while violating interference.

In a half-open P-tick window, each unit of capacity can be consumed at most
once because its per-dispatch return is at least P later. Initial capacity is
also Q, so this gives a bound for this exact unit-service model. Grouped
credits invalidate that per-unit argument: service at6 can return14 if its
active interval began4. The inspected book and original report present a
periodic-task replacement argument; the later inspected RTAS2010 paper disputes
the original replenishment formulation with cohort/overrun counterexamples.
Do not relabel that historically claimed theorem as a verified guarantee.
Never transplant the simpler strict-window
claim to that algorithm. `ss_compare` explicitly rejects a segment of lengthP
or more; it does not purport to implement every classical active interval.

Polling request@2 misses the empty first dispatch@1. It waits through the
supervisor@10 and receives service11,12, finishing13. Background first runs
after ordinary service@1,@2 and finishes5. Per-dispatch runs2,3 and finishes4.
Those observations compare identical input, but overload may admit different
later requests because completion frees slots at different times. Report the
admitted sets before comparing latency; comparing only completed averages
would introduce selection bias.

The diagnostic ordinary task has explicit drop-old state at its next release;
there is no hidden overwrite. Each released two-unit job contributes to
executed+dropped+currently pending. Every server request similarly conserves
its admitted demand. Supervisor service is nonnegotiable under all three
policies; the corresponding mutant demonstrates why an average server rate
does not by itself enforce this ordering.

Seven semantic controls each compile successfully and must reach the public
assertion marker. The independent token-list oracle models modular refusals
and complete traces; it is separately written but is still author evidence,
not independent human assessment. The real retained advanced-source stub and
the public successor stub are separately tested. Target runs audit actual
closure and execute the finite model; they are not task-stack/IRQ server ports
or physical latency measurements.
