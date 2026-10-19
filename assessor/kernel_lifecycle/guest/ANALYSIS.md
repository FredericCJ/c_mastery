# T57 guest assessor analysis

Reference ownership is deliberately redundant: a registry reference survives
until unpublication and a grace period; each open acquires a nonzero reference
while the pointer is RCU-protected; each accepted queueing owns one reference.
The running worker's reference keeps storage alive while it may acquire the next
one. The mutex makes closed and enqueue decisions one serialized transition.
cancel_work_sync returning true means a pending instance was removed and its
reference must be dropped by teardown. False does not justify dropping a running
worker's reference a second time. rcu_barrier covers deferred module callbacks.

The user-copy transaction does all potentially faulting copy before locking and
publishing. The observer proves preservation of write count and first byte for
this fault, not all user mappings. Authorization is the kernel capability check,
never an input flag. Existing fds retain a revoked object until their final close.

The IRQ continuation originally exposed a real integration error: irq_alloc_descs
assigns THIS_MODULE as descriptor owner, and request_threaded_irq pins that owner.
Trying to rely only on module exit to free the request makes ordinary unload
unavailable. The one-shot reference waits for the threaded callback, cancels the
timer and free_irq synchronizes remaining execution before init returns. Device
managed cleanup then runs on both failure and normal unregister. Do not repair
this by forced unload or by ignoring the error. If the variant needs a persistent
IRQ, add an explicit close path and assess its ownership graph.

The bounded negative uses might_sleep in the primary handler under the debug
kernel and requires the actual invalid-context diagnostic. Corrected task and
threaded handlers use sleepable resources, primary uses only its bounded update.
This demonstrates execution-context restrictions without pretending that a
simulated IRQ validates a real controller, DMA interaction or board latency.

Withheld transfer KG3: publish replacement B while readers hold A; an epoch wait
protects A's lookup storage but is not permission to free A while an fd holds it.
Require an independent interleaving argument and counterexample to conflating
grace periods with reference ownership. KG4: move timer startup after device
publication and schedule immediate close; diagnose why cancellation can precede
the late arm. The repair moves arm inside the publication lifetime or serializes
arming with close. Passing the public finite schedule is insufficient alone.

KG5 reference: the registry mutex serializes entry replacement; both lookup
paths acquire a kref before their protection ends. Replacement closes A and
publishes B before waiting, so B lookups continue during A retirement. The
retiring registry reference keeps A alive until the grace period and callback
drain; an old fd then keeps A alive beyond that wait. A worker can enqueue only
under A's lock while its captured generation matches A's open generation.
Close increments the generation under the same lock and closes admission, then
waits without either worker lock. Thus no enqueue linearizes after closure,
although previously admitted work may still execute before cancellation returns.
The finite probe verifies old work has stopped when replacement returns, not
that cancellation prevented every already-started callback from running.

The epoch model has current2, retired1, reader(old)=1, reader(new)=2 after
replacement. Clearing only old permits reclaim while new remains2. Waiting for
all slots is an unnecessary liveness restriction: a chain of overlapping
new-epoch readers can keep the all-readers predicate false indefinitely. The
64-turn public trace is a finite witness to that predicate, not an infinite
execution proof. For the delayed multiple-retirement variant, keep ordered
retired epochs, explicit per-object ownership, bounded queue admission and
nonwrapping identities; a minimum active epoch optimization requires a proof
that no older reader can enter after publication has advanced.
