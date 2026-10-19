# T24.08: notifications, identities and scheduling composition

## T24.08.a — carry unfinished edge-triggered work

Register a nonblocking pipe with five queued bytes and read at most two per
operation. Compare actual LT and ET registrations. After consuming the initial
notification and only two bytes, test for another event without blocking forever.
Then account for the remaining three bytes. Show the stalled ET design and two
repairs: drain to EAGAIN, or retain a runnable item when a quota expires and resume
it without requiring a fresh edge. The explicit runnable item must not duplicate
the connection in a ready queue.

The changed-condition task adds EPOLLONESHOT. Drain to EAGAIN, insert a byte while
the registration is disabled, verify no notification yet, then rearm with MOD and
receive that byte. Keep registration changes, readiness state and application
cursor separate. Explain the arrival-between-drain-and-rearm interval using the
inspected Linux contract. Do not generalize this execution to another OS's event
API. Retain HUP separately from unread data: close does not magically consume a
buffered prefix.

Hints: inspect unread bytes after the second wait; retain readiness until actual
would-block; then place the new arrival on a timeline before rearming.

## T24.08.b — a file descriptor is a reusable table index

Use stable registry metadata and tokens `(slot2,generation7)`. Cache an actual
epoll event, explicitly unregister and close the descriptor, and force the same
descriptor number to refer to another owned pipe. Reopen slot2 as generation8.
Deliver the cached generation7 event and reject it before reading connection
payload. Deliver the fresh generation8 event and read the expected new byte.
Record both OS descriptor operations and registry transitions.

The registry must outlive cached tokens; generation checks do not make dangling
registry storage safe. Unregistering future notifications does not invalidate a
userspace event array already returned by the kernel. Duplicated descriptors
also have an open-file-description lifetime distinct from their integer values;
explicitly reconcile every owned reference. Avoid storing an unchecked raw
connection pointer in the event payload.

For the delayed task, reduce the generation range to 1–3. Reopen/close through its
last value and implement permanent retirement. Show why wrap to 1 would alias the
old token. Separately refuse operation-sequence exhaustion inside a still-live
connection. Alternatives need an explicit proof that no old token can remain,
such as a separately verified quiescence protocol; merely widening an integer
does not prove absence of wrap.

The bounded security demonstration permits only injecting cached tokens and
reusing descriptors in this original teaching process. A flawed identity check
can select the wrong synthetic operation; that is the harmless success condition.
Repair with validated incarnation and operation identity, retain lifetime until
quiescence, and reject compiled identity mutants. This does not demonstrate
arbitrary memory access, code execution or privilege escalation.

Hints: distinguish the registry entry from its payload; preserve an old token
while reusing its index; finally shrink the identity space until wrap is reachable.

## T24.08.c — compose quotas without recursive expansion

For three ready participants, combine two successful operations per connection
with eight globally per turn. All-progress callbacks produce six successes;
under this particular three-participant bound, the global success limit is
redundant but still explicit. It becomes consequential if the admitted population
changes. CPU-only callbacks require the separate callback budget from T24.07.c.
Derive the maximum callback count and a conditional time bound from a stated
maximum callback cost; the API cannot enforce that a callback returns promptly.

The changed-condition task allows callbacks to enqueue ready work. Choose the
provided snapshot-at-turn-start rule: new ready publication is retained for the
next turn, never executed recursively in the current callback. Test publication
to a different participant and to the current participant, repeated publication,
and attempted nested dispatch. Show why dynamically extending the current ready
queue without a work cap could make a finite initial queue run indefinitely.

Verify 12,288 combinations of ready set, rotating start, work budget and four
callback result classes for each of the three participants. The oracle uses an
ordered participant set and arithmetic quotas rather than the implementation's
ready bits. Require exact service order, callback/success counts, remaining
readiness and next cursor. Explain the finite alphabet and its limits; arbitrary
callback side effects still need a contract and separate transfer assessment.

Hints: freeze the participant set at turn entry; use a separate next-turn ready
set; then prove how a continuously ready participant eventually becomes first.
