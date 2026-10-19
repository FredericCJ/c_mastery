# PR-T24-03: framed streams and datagram policies

This is the required executable successor for T24.05.a–c and T24.06.a–c,
including their existing seven-day changed-condition assessments. Original
criterion IDs, prerequisites, and delay remain in `ASSESSMENT.json`. Read
`STREAMS.md` and `DATAGRAMS.md` before implementing the deliberately incomplete
`stream.c`. `stream.h` is the operation contract; `tests.c` is public feedback,
not the independent assessment. Do not copy assessor implementations.

Use hosted C17, profile A, GCC or Clang, Linux nonblocking local sockets, and
synthetic byte strings. Core parsing, queueing, deduplication and sequence
acceptance must use only the supplied fixed objects and C operations. No framing,
queue, RPC, retry or serialization library may implement the assigned mechanism.
Host verification may use Python, libc, pthread facilities and binutils. These
are not freestanding target dependencies. No C23 feature is required: socket
behavior is an OS contract, not a guarantee newly supplied by an ISO C edition.
The exact published ISO texts are not represented as inspected by this package.

Prerequisites are T07 build evidence, T18 ownership/failure contracts, T23
descriptor lifetime, and T24.03–.04 deadline and connection work. Bring the learner
byte decoder, descriptor ledger and absolute-deadline adapter; compare their
contracts before reuse. Author native tests import the explicitly identified
assessor `event_depth/signals.c` deadline provider. This does not establish that a
learner adapter works. Independently integrate and validate that adapter later.

For every operation, caller-provided objects and output arrays must be live,
correctly aligned, correctly sized and non-overlapping where mutation would
otherwise invalidate an input. Do not call an initializer on an object while
another operation owns it. These are programmer obligations; C pointer types do
not establish object lifetime, exclusive access, or absence of aliases. All core
objects are serialized by one owner. No data-race freedom or concurrent queue
contract is implied. Internal corruption detection assumes valid pointers; it
cannot safely inspect an arbitrary forged pointer.

The wire assumes octets and an `unsigned` capable of representing 65536; establish
these environment premises before building. One-byte and two-byte big-endian
lengths encode payloads of zero through eight octets. Integer objects are never
sent by casting their representation to a wire structure. A parser borrows each
input span synchronously and copies only a complete frame to the output. A
malformed length or truncated end-of-stream requires discarding that connection's
parse state. Fresh connection state is initialized explicitly.

An outbox has four connection slots and eight message references per slot. A
successful enqueue transfers exclusive access to the message until delivered or
cancelled. Rejection leaves it caller-owned. The message object remains live
while queued, including while a peer is stalled. Only the actual committed
prefix advances the cursor. The writer callback borrows the offered range for
one call and may not retain it, reenter the box or mutate its messages. This
contract does not apply to an asynchronous kernel operation retaining buffers.

The global payload budget is at most 256 octets. It includes the full payload
of every queued message until its entire frame commits, even after partial
progress. It excludes headers, the fixed message and queue objects, stack,
allocator bookkeeping in a learner wrapper, and kernel socket storage. Report
those separately. Empty frames still consume a queue slot. Limit zero therefore
admits at most 32 empty frames, not an unbounded stream of allocations.

Produce a pre-execution prediction, implementation, invariant/ownership argument,
raw native traces, an independent expected-result method, a minimized defect and
repair, cost accounting, and changed-condition reasoning for each assigned leaf.
Record compiler/version/options, kernel/libc, exact source identities, bounds and
exit statuses. Keep successful model cases, actual syscalls, deliberate semantic
rejections and unsupported environments distinct. A compiled negative control
must fail at the intended check; a crash or failure to compile is not equivalent.

The native exercises use only processes and descriptors created by the lab.
Do not contact external peers. Restrict queued objects, datagrams, descriptor
transfers and retry counts to the declared bounds. Close every received descriptor
reference, even on rejection. Harmless observable failures are a rejected frame,
retained ownership, an incorrect synthetic counter, or a leaked owned descriptor
detected by the ledger. No arbitrary code execution or privilege gain is claimed.

Build public feedback, then substitute your source and add your native adapter:

```sh
cc -std=c17 -O2 -Wall -Wextra -Werror -I labs/hosted_systems/stream_depth \
  labs/hosted_systems/stream_depth/tests.c \
  labs/hosted_systems/stream_depth/stream.c -o stream-tests
./stream-tests
```

The shipped scaffold intentionally compiles and fails. Public tests cover bounded
inputs; passing them is insufficient without the leaf arguments and independent
assessment. Assessor-only native probes, mutations, sequence oracle, solutions
and delayed package are excluded from learner exports.
