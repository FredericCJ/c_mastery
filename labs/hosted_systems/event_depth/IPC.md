# IPC ownership, restart and local connection construction

This packet supplies the teaching and construction path for
`AX-PR-T24-02-T24.03.a` through `.03.c` and `.04.a` through `.04.c`, including
their existing `-DELAYED` changed-condition assessments. Families B/C/D/F/G/H/I/J
are selected because representation, partial progress, failure, measurement and
changed lifetimes differ materially here. The original criterion IDs, rubric
and minimum seven-day delay remain authoritative. Follow `CONTRACTS.md`.

Prerequisites: T18 ownership and cleanup ledgers, T19 state machines, T21 partial
failure, T23 descriptors/process collection, and T24.01–02 absolute waits.
Bring those learner artifacts, an acquisition ledger and a byte codec. Reuse
`es_remaining`; compare it against the integer-time oracle before integrating it.
The author campaign uses the explicitly named assessor provider, never labels
that execution as integration of an actual learner's code.

Use C17 on hosted Linux with pthreads, `_GNU_SOURCE`, numeric loopback and
original child processes. These POSIX/Linux adapters are intentionally hosted;
they are not permitted dependencies in the strict freestanding profile. No C23
feature is required and C23 does not supply these OS protocols. Compile GCC and
Clang at O0/O2 and an address/undefined sanitizer configuration. Keep syscall
contracts, the C object model, the selected libc ABI and observations distinct.
Primary passage identities are in `research/v5/expansion/legacy_events/resources.json`:
R-EVENTS-CONNECT, GETADDRINFO, PIPE, UNIX, SHM_OPEN, MMAP, MUTEX_PSHARED,
IP, IPV6, SOCKADDR_IN and SOCKADDR_IN6, plus the earlier POLL entry.

## Representation and derivation

The application request is exactly8 bytes, interpreted as unsigned octets. The
answer is their unsigned sum in exactly4 big-endian bytes. Derive the maximum
sum2040 and prove each shift is within the unsigned representation. For
`00 01 02 03 80 fe ff 00`, independently derive643 and `00 00 02 83`.
Do not write a C struct, enum, pointer or mutex as a portable message.

`EiRequest` is a serialized authoritative ownership ledger. Epoch and request ID
are1..65535, chosen without reuse while older references remain live. Its states
are allocated, queued, accepted, completed and abandoned. Client owns allocated
and completed records; the transport owns queued records; the server owns
accepted records. An abandoned record has no owner. “Owner” means the party
responsible for the request's remaining accesses and release, not that separate
processes share the same C pointer. A pipe/socket copies bytes; an observer may
learn about server transitions only later. Draw that knowledge delay separately.

Implement `ei_init`, `ei_transition`, `ei_invariant`, `ei_reply` and the four-slot
`EiQueue`. An invalid transition must leave every field unchanged. Admission
transfers client responsibility only on success; a full queue returns0 and
retains the caller's responsibility. Duplicate addresses and live `(epoch,id)`
pairs are rejected. Dequeue transfers one queued record to the server. Prove
`head<4`, `count<=4`, `bytes==8*count`, slot uniqueness and ownership preservation.
The fixed-capacity scans have constant bounds; a generalized capacity N requires
stating the duplicate-validation cost rather than silently calling it O(1).

Disconnect before acceptance is definitely-not-done **only when transport purge
has been confirmed**. Absence of an acceptance reply is insufficient: the server
may have accepted and effected work before the reply was lost. After acceptance,
disconnect records uncertainty while the server retains responsibility. A
cancel request and its acknowledgment are distinct transitions. Acknowledgment
means seen, not stopped. Only confirmed quiescence permits release. An effect
may occur between acknowledgment and quiescence; cancellation cannot undo it.

## Concrete construction and investigation

| Objective | Required ordinary artifact and experiment | Changed condition |
|---|---|---|
|T24.03.a|Implement the8→4 protocol through two pipes, an AF_UNIX stream socketpair, and a MAP_SHARED record protected by a process-shared mutex. Each uses the same sum and queue contract. Send512 requests in batches of4 and compare every result with the independently derived bytes.|Run separately exec'd workers that attach named shared data. Distinguish the object name and versioned request data from descriptors, mappings and synchronization lifetime. Demonstrate stale epoch rejection and explain which state survives normal worker exit.|
|T24.03.b|Draw all legal/illegal ownership transitions. Force confirmed purge before acceptance and disconnect after confirmed acceptance. Record a separate assessor effect witness without pretending the disconnected client knows it.|Force cancel-seen, then a real effect, then stop-confirmed. Show the server retains access through the acknowledgment and cleanup happens only after its last use.|
|T24.03.c|Gate the server before consumption, admit4 and reject the fifth. Report retained application bytes, kernel transport buffers, shared metadata, blocking points and rejection ownership separately. Compare raw wall time, parent CPU, child CPU and bytes for equivalent workloads.|One absolute request deadline spans the8-byte send and4-byte receive. Time spent making progress and interrupted/would-block operations counts. Preserve cursors on timeout; no unbounded restart of a relative timeout.|
|T24.04.a|Resolve two numeric loopback candidates; reserve the first port without listening and create an owned listener on the second. Release the full resolver list after copying valid addresses. Close a failed socket before trying a fresh one under the same deadline.|Use nonblocking connect, readiness, and SO_ERROR. Writability alone cannot prove a connection succeeded. Record actual peer identity after selecting the second listener.|
|T24.04.b|Inject failure at socket, bind, listen, connect and accept. Account for each acquired descriptor incarnation, even when numbers are reused. Preserve the output on failure and the primary error across cleanup.|After accept succeeds but before publication, invoke a borrowed cancellation callback. It may neither close nor retain the descriptor. Cancellation closes once; publication transfers once.|
|T24.04.c|Construct IPv4/IPv6 loopback and private pathname Unix addresses with family-specific lengths. Verify port0x1234 becomes bytes12,34, reject unsupported family/overlong path, and exchange data on actual endpoints.|Analyze Linux abstract addresses as length-delimited byte names. Compare two names with identical C-string prefixes but different bytes after an embedded NUL; distinguish name lifetime and access controls from pathname sockets.|

## Deadline and connection contracts

Implement `ei_exchange` in `ipc_budget.c` using its injected `EiIo` operations.
Begin with zero `EiProgress`. Each positive transfer advances exactly that cursor;
zero denotes end-of-stream/no progress failure in this selected8→4 exchange.
EINTR retries the operation and EAGAIN waits; obtain current time after a failed
I/O before computing the wait allowance. Check the same absolute deadline before
every attempt and after collecting the complete response. A response observed
at the deadline is late. Publish the caller's response only on timely completion;
retain internal partial bytes on all failures. Resume only with the identical
request, channel and deadline. This is serialized ownership, not concurrent
cancellation or an ISO C checked lifetime.

Callbacks must perform nonblocking I/O, report honest byte counts and use the
same nondecreasing clock. The explicit clock tests calculate positive byte
progress independently for direct, interrupted and would-block operations.
An actual stalled child supplies two response bytes and retains the write end;
the caller must time out with8 sent and2 received. Record elapsed time, but do
not infer a hard Linux scheduling bound from it. Rejecting late completion does
not stop remote work. Use T24.03.b's separate quiescence protocol to release it.

`ElAddress` stores a sockaddr_storage plus its actual length. Numeric getaddrinfo
suppresses external name resolution. Copy into properly declared sockaddr_in,
sockaddr_in6 or sockaddr_un objects for inspection; casting an arbitrary buffer
does not itself establish ISO C effective type. The casts at the socket API
boundary use its specified OS interface. Pathname addresses use
`offsetof(sockaddr_un,sun_path)+name_length+1`; use the actual platform capacity.

`el_listener` publishes only after socket/bind/listen succeed. `el_connect`
accepts at most8 owned local candidates. `el_accept` is a nonblocking **try**:
EAGAIN after apparent readiness is a valid result, so an outer deadline loop
must retry. The cancellation callback borrows only until return. `ElOps.close_fd`
must consume the owned descriptor exactly once; this small adapter preserves
the primary failure and does not provide a secondary cleanup-error channel.
It is not equivalent to a general fallible resource-release abstraction.
Document a richer diagnostic design if that contract changes; never blindly
retry close on a descriptor that may already have been reused.

## Verification and evidence

Submit source, state/ownership table, full acquisition traces, independent sum
derivation, raw commands, addresses and lengths, deadline/cursor traces, and
measurement tables with compiler/kernel/libc identities. The public tests cover
six disconnect cuts, bounded admission and81 integer-clock cells. Derive a
history-fact model independently of the C transition switch, enumerate its
reachable states and compare every outgoing operation, including rejected ones.
Predict how early release, treating acknowledgment as quiescence, restarting
budgets, treating writable as success and leaking a cancelled accept will fail.
A compiled specimen must fail the intended contract check; compilation failure,
an unrelated crash or a sanitizer report is not that semantic demonstration.

For costs, use one warmup and seven recorded512-request runs per transport under
each optimized compiler, rotate transport order, and report dispersion. Setup
is outside the parent wall/CPU interval, while child resource accounting includes
its process lifetime. Each shared slot requires synchronization and notification;
shared memory is not a claim of zero copying or zero kernel activity. The host
and finite workload do not establish a universal fastest transport.

Containment: all processes, shared objects, private filesystem paths, abstract
names and ephemeral loopback listeners are original teaching targets with
synthetic bytes. No external addresses, credentials, deployed services, privilege
changes or global IPC settings. Obtain private native-Linux temporary storage
for pathname sockets; drvfs may reject that interface. Delete only objects this
run created and join all owned children before destroying shared mutexes. The
harmless failure observations are stale-message rejection, incorrect result
classification and unmatched resource accounting, not code execution.

Hints, in order: separate a request's authoritative state from the client's
knowledge; next draw the last access before each release; then label the absolute
deadline at every syscall and wait. For restart, list what exec removes before
choosing a rendezvous object. For addresses, begin with bytes and explicit lengths.
Private solutions and transfer rubrics remain assessor-only. Current execution
status comes from the source-bound support record; passing these bounded cases
does not establish independent learner mastery or physical timing.
