# Assessor: grace, epochs and retained callback references

Do not export this analysis or implementations. Keep the original T26.07/.08 criterion and delayed IDs, with author execution separate from independent review and learner performance.

## T26.07.a

The baseline tracker advances at most one epoch beyond a stalled reader's announcement. Therefore active4/current5 is reachable, but active1/current5 is not. Eligibility for retired2 is true with only active4; retired5 is false. Adding the artificial active1 makes both false as a conservative ledger diagnosis, not an executable legal history. The bounded2-participant/depth2/epoch5 model explores339states/979edges and verifies that distinction. Nesting keeps the outermost announcement unchanged; clearing on inner leave creates premature eligibility. State/section/registration saturation never wraps identities. Remaining active sections prevent ordinary departure.

## T26.07.b

Reclamation requires the conjunction of removed publication, completed declared read-side protocol, no retained references, and exclusive reuse ownership. The publication event supplies visibility, not those lifetime facts. `struct ag_borrow` checks section/registration/version identity; calling copy after outer exit fails while preserving output. A raw C pointer lacks even that check. Capturing a callback obtains the version reference while the borrow is valid; add failure rolls it back. Its owned reference persists across read exit and pending removal. Drop ends that reference, after which collection still checks epoch eligibility. No native invalid pointer is executed.

## T26.07.c

With the original reader at0, first publication advances the current epoch to1 and further advance attempts remain pending. Sixteen retirements are still possible at stamps0/1 and all remain retained. The next update fails FULL with17active slots, and legitimate reader exit permits16collections. On the selected native layout AGObject is52bytes, so retained object storage is832bytes; the512-byte payload figure excludes metadata. The normalized hazard/epoch comparison uses equal32-byte logical objects: one protected object retains32 payload bytes; an old epoch retains512 after16 retirements. It is a ledger, not a claim that a particular hazard implementation has the same struct layout. Joined departure transfers cleanup authority; a merely paused participant may resume and must remain protected.

## T26.08.a

The AG old version is retired at0; the new reader enters at1. When the old reader exits, the newer announcement does not block eligibility for retirement0. The retained CMX coarse quiescent test waits for every active slot, so it is more conservative and can be delayed by new readers. This is not a defect in its explicit contract. AGWriter flags identify the updater within one serialized domain. Two real writer threads use the same mutex around acquire/publish/collect/release. Without that mutex the flags and ordinary metadata race; no atomicity is inferred from the word 'writer'. The selected run performs64updates, not a scheduling guarantee.

## T26.08.b

The local model explicitly tracks up to4readers and requires serialized metadata. Selected Linux7.0.14-cmastery-b06 supplies its own per-task/per-CPU infrastructure: nested read sections, permitted kernel preemption, grace detection and deferred callbacks. In PREEMPT_RCU explicit blocking within a read section remains forbidden for the selected non-RT build. New readers need not extend an already-started grace period. `rcu_assign_pointer`/`rcu_dereference` manage publication; `call_rcu` schedules post-grace cleanup. `synchronize_rcu` waits for a grace period, not necessarily queued callback completion; stop producers and use `rcu_barrier` before unloading callback code.

The original guest module pins a HARD hrtimer toCPU0 and runs the writer onCPU1. The timer takes read lock, obtains version1, signals readiness and boundedly spins while the writer publishes2. The writer's new read returns2; its deferred release must remain pending during the held old reader. After release the old reader rechecks1 before unlocking, then the callback frees version1. Successful initialization checks all observations. Cancel/join the timer/thread sources, remove current publication, synchronize readers and drain callbacks before module code disappears. `in_hardirq()` is an observed nonzero context mask, not necessarily the integer1. The harmless negative changes the recorded grace condition and rejects initialization; it does not perform early free. The guest is diskless/networkless and does not alter the host.

## T26.08.c

Two successful captures create refs2. A removes itself and B during dispatch; the actual provider retains both snapshot invocations until the dispatch frame completes. A's requests must leave drops0 and collection blocked; B then sees the old immutable value through its own reference. The provider drops both contexts after invocation, refs reach0 and collection frees the retired slot. Destruction inside A instead suppresses B and still drops both after A returns. Fifth/closed adds retain no reference and preserve handles. The registry is not inherently refcounted: this adapter deliberately gives each capture a counted version reference; its own control storage still follows the producer lifetime rule. Claiming that ordinary callback registration alone supplies ownership fails this leaf.

## Evidence limits

The native core is externally serialized; the actual writer mutex adapter is not Linux RCU. Bounded state exploration, a few guest grace periods and sanitizers cannot prove all executions or universal progress. The pinned kernel's KASAN/lockdep/atomic-sleep configuration is recorded and must not be inferred for another kernel. No independent review, learner delayed assessment, physical IRQ/controller or target board result is asserted. The separate T26.09/.10 module/context/LKMM obligations remain required.
