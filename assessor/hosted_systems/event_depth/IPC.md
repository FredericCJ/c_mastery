# Assessor analysis: PR-T24-02 IPC/local successor

Private package. Pair with the original six criterion rows and their preserved
DELAYED IDs; give the changed-condition prompt only under that assessment's
conditions, at least seven days after the original attempt. Author reference
execution is not a learner attempt. Do not include this directory in learner
initialization. Public stubs remain intentionally incomplete.

## Technical solution and counterexamples

03.a: `ei_reply` uses an unsigned sum bounded by2040, so the independent643
answer is `00 00 02 83`. The three native experiments retain the identical4-slot
logical admission policy and request/result function. Pipes are byte streams;
the selected8/4-byte writes fit PIPE_BUF, but this neither creates read message
boundaries nor proves arbitrary stream writes cannot be partial. Use cursors.
The shared version has4 request/response slots, per-slot epochs and a mutex
initialized with PTHREAD_PROCESS_SHARED. MAP_SHARED establishes shared mapping,
not mutual exclusion; a process-private default mutex is the wrong contract.
The notification byte selects a slot; protected slot state is authoritative.

Restart variant: `ipc_restart.c` creates a uniquely named0600 shared object,
checks size/version, and uses three exec workers. The first effects epoch1;
after it is collected, the coordinator destroys synchronization, increments
epoch and initializes a fresh mutex. A stale epoch1 request is rejected under
epoch2; an epoch2 request then succeeds. The descriptor is closed after mapping,
and unlink prevents new attachment without invalidating that existing mapping.
A high-number CLOEXEC pipe endpoint is absent in each new executable before
new descriptors are acquired. This demonstrates why an anonymous pipe does not
automatically reconnect independently restarted programs. An explicitly
inherited descriptor or descriptor-passing channel would be a different design.
The fixed shared record is a same-build host ABI, not a wire format. Mutex bytes
cannot be persisted/reinitialized while another process owns them. This experiment
proves normal worker-exit continuity under coordinator ownership; it does not
establish crash-owner recovery, reboot durability or automatic epoch allocation
after coordinator loss. Require that limitation in the learner's answer.

03.b: The fact model records eight independent historical facts (queued,
accepted, effect, completion, disconnection, cancel, acknowledgment, stopped).
It derives owner/state/outcome from those facts, rather than evaluating the C
transition switch again. Its26 reachable fact sets have208 outgoing event checks.
The actual controlled-process traces add causal witnesses:

1. Before acceptance, an owned server confirms it purged the unread request.
   Only then may disconnect mean definitely-not-done. Merely closing the client
   read end leaves a counterexample: the request may remain in a pipe.
2. After the server's acceptance acknowledgment, the client closes its reply
   channel. The server is released to effect the sum and sends an assessor-only
   witness. The client's outcome remains uncertain despite that observer knowing
   the result. A production protocol cannot use a nonexistent witness channel.
3. The server acknowledges cancellation, then the gate permits an effect. Its
   final stop confirmation occurs after its last request access. Until that
   confirmation the SERVER ownership remains; free-on-ack is premature.

The reference uses an original sum as a harmless effect witness; no production
service is involved. An alternate implementation may make cancellation prevent
the effect, provided its acknowledgment contract actually guarantees quiescence
and the rubric includes evidence for that stronger contract. Renaming a seen
acknowledgment does not meet it.

03.c: Full rejection preserves the fifth caller's record and its owner. The
32 retained payload bytes exclude request metadata,4-byte results, queue pointers,
mutex/epoch storage and kernel buffers. The actual report records mapping size
for this ABI; changing ABI requires remeasurement. Compare optimized observations
only against optimized observations with the same function and batch size.
Seven rotating runs plus a warmup establish a sample, not a significance claim.
Explain scheduler activity, wakeups and copied bytes before drawing conclusions.

Deadline variant: model one successful byte operation as one millisecond. With
one injected EAGAIN/wait or EINTR before each positive operation, each byte costs
two milliseconds. For budget B and per-byte cost C, committed bytes are
`min(12,floor(B/C))`; sent is min(committed,8), received is max(committed-8,0).
Success requires `B>12*C` because observation exactly at the deadline is late.
This gives81 expected cells independently of the C loop. Timeout may leave a
fully collected late response in private progress while keeping caller output
unpublished. Check time after EAGAIN before waiting. Resetting a one-second
deadline on each attempt permits an arbitrarily long sequence of slow progress;
the semantic mutant must fail, not be declared valid because it eventually
returns all bytes. The native port confirms a real blocked response retains2
bytes and times out; a watchdog catches broken tests but is not the API deadline.

04.a: Numeric resolver hints contain only declared fields, with others zero.
Copy selected address and length before freeing the entire list. The first
candidate is held bound without listen, preventing another process from taking
over that test port. The second is an original listening loopback endpoint.
Nonblocking connect can report EINPROGRESS; writable readiness may report an
asynchronous failure. Read SO_ERROR and reject it before publishing. Confirm
the actual peer port equals candidate2. The deliberately wrong writable-success
implementation fails this test. A syscall trace can complement but not replace
the resource/peer assertions. A deadline must span all candidate attempts; DNS
is deliberately excluded with numeric-only resolution, not hidden in the budget.

04.b: The ledger records acquisition incarnations, not one permanent entry per
integer descriptor. Faults occur before each actual selected operation; all
earlier acquisitions are real. Five fault boundaries and the postaccept cancel
boundary must restore live count to zero with no double close. The cancellation
callback sees a live borrowed descriptor exactly once; only the adapter decides
publication/cleanup. An alternate ownership token is valid with equivalent
proof. Setting the output to -1 after publishing does not revoke another owner's
borrow. This selected close callback consumes once and preserves the original
error; secondary fallible-close diagnostics are outside its narrow API and
must not be claimed as equivalent to the MC-FALLIBLE-CLOSE contract.

04.c: IPv4 and IPv6 use different structures and sizes. Network-order port
0x1234 must be bytes12,34 regardless of host endianness; sockaddr_storage is
storage for the largest supported address, not a universal passed length.
Use memcpy into a declared type before C-level inspection. Pathname Unix
addresses include their terminator in this contract; reject overlong paths.
Linux abstract naming starts with a zero byte and uses the supplied length,
not strlen. The private executable creates names whose printable prefixes are
identical and whose significant suffixes after an embedded NUL differ. Both
bind and receive their own byte; a duplicate exact name fails EADDRINUSE, then
can bind after the last owner closes. The abstract namespace has no filesystem
inode permission boundary. Do not claim that a private-looking string provides
authentication or that these rules are portable to all Unix systems. A pathname
directory's ownership/permissions are a distinct containment tool.

## Rubric and evidence map

Score each row separately, 0–3. A pass requires at least2 in every applicable
row and no ownership, containment or fabricated-evidence failure. A3 requires
independent derivation and successful changed-condition reasoning; author runs
never assign a learner score. Provide feedback per criterion, not one aggregate
“IPC test passed.”

| Criterion | 0–1: superficial or incorrect | 2: competent implementation | 3: independent transfer |
|---|---|---|---|
|03.a|Only socketpair demo or identical pointers assumed across processes|Three actual mechanisms, exact codec, synchronization and equivalent workload|Independent exec/restart package distinguishes name/data/ABI/mutex lifetime and rejects stale epochs|
|03.b|Every disconnect means no effect; ack frees state|All state/owner transitions and actual before/after acceptance witnesses justified|Explains observer knowledge, composes cancellation and proves last-access-before-release under changed sequence|
|03.c|Timing with unequal correctness, unbounded queue or restarted timeout|Bounded queue ownership plus labeled raw resource/cost observations|Derives clock oracle, handles late full response/partial progress, explains what elapsed samples cannot prove|
|04.a|First resolver result only, writable means connected|Full list cleanup, fresh failed-candidate sockets, correct second peer|Preserves global budget and rejects asynchronous failure with a causal SO_ERROR explanation|
|04.b|Happy path only or cleanup count by fd number|All5 failure cuts and cancellation boundary balanced|Handles reused numbers and changed publication point without premature release or duplicate owner|
|04.c|sizeof(sockaddr) everywhere or host-endian port|Correct3 family layouts, limits and actual local endpoints|Derives embedded-NUL abstract identity/lifetime/access limitations without unearned portability claims|

Required evidence: public `ipc_tests.c`, `ipc_budget_tests.c`, `local_tests.c`;
private `ipc_oracle.py`/`ipc_trace.c`; native transport, protocol, restart,
deadline and local probes; eight **compiled** semantic/stub rejections.
Inspect a selected incorrect trace before revealing the reference. Sanitizers
cover only executed paths and cannot establish the protocol's knowledge or
quiescence invariant. The finite fact oracle does not enumerate arbitrary OS
schedules or prove every environment's behavior.

Graduated assessor hints: (1) ask which party can still access the record;
(2) show an accepted request whose reply is lost; (3) supply the ordering
accept→cancel-seen→effect→stop and ask for each owner. For budget mistakes,
reveal the B/C counting model only after requesting a hand-worked three-byte
example. For address mistakes, show two byte arrays with equal strlen but
different supplied lengths. Valid alternate designs must state their stronger
assumptions; silently changing the task's failure or timing model earns no credit.

Validation status is source-bound in the current event support receipt. Physical
observations, external review, actual independent attempts and delayed attempts
remain separate. Original failed drvfs and early pilot records are retained;
running pathname sockets on native Linux temporary storage preserves the
requested interface rather than silently replacing it.
