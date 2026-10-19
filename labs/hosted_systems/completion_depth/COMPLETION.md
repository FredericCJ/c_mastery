# Completion changes the ownership graph

Readiness says that trying an operation may progress. Your caller still owns the
buffer while a synchronous call executes. A completion interface publishes an
operation that can retain that buffer after submission returns. Draw a separate
edge for each resource: the slot's generation, target request, cancellation
request, registered region, source file and receiver's outstanding bytes. Clearing
one edge does not clear the others.

Start with a blocked pipe read. Its slot is BUSY. Request cancellation, then
consider both dispatch orders. In target→cancel order the slot is DONE but a
cancel record remains; in cancel→target order the target loan remains. The sole
reusable state requires both events followed by release. Now complete a target
in the kernel but delay application dispatch: an ENOENT cancel result cannot tell
the application that it has already processed the target. Two independent sets
of pending event IDs make a useful model against which to test a state machine.

The inspected [cancel operation documentation](https://man7.org/linux/man-pages/man3/io_uring_prep_cancel.3.html)
distinguishes successful cancellation, an absent request and a request too far
advanced to cancel. Target completion remains an obligation. Do not assign a
portable order to the two CQEs, and do not require EALREADY to occur in a finite
test. The public deterministic cases model it; native cases record what happened.

Generation tokens solve a different problem. With slot index alone, an old
completion can finish a new request after reuse. Reserve a generation field and
retire before exhaustion. Derive the available space:31 generation bits plus8
low slot bits stay below the separate bit63 cancellation tag. Increasing the
field changes exhaustion cost, not the need for a proof. An overflowing epoch is
not made safe by calling it unlikely.

Registered storage adds a longer lifetime. The inspected
[registration contract](https://man7.org/linux/man-pages/man2/io_uring_register.2.html)
describes memory registration, resource charging, operation probing and explicit
unregistration. The [fixed-read contract](https://man7.org/linux/man-pages/man3/io_uring_prep_read_fixed.3.html)
requires the requested span to fit its indexed region. A successful read returns
its operation loan; it does not unregister the memory. This lab registers once,
drains every operation and cancel CQE, unregisters, then unmaps. Dynamic resource
replacement and multishot operations require additional protocols outside this
single-shot package; the retained T62 exercises cover their own contracts.

For copy avoidance, distinguish bytes copied in user code, bytes retained, kernel
references and measured memory traffic. [sendfile](https://man7.org/linux/man-pages/man2/sendfile.2.html)
can return short progress or EAGAIN. Keep the source immutable until the peer has
consumed the transferred data; submission success is not that acknowledgment.
In our stalled experiment the application can cease submitting immediately, yet
queued bytes remain and must be drained or abandoned under an explicit protocol.
Without a bound on scheduling and peer progress, cancellation-to-drain has no
finite wall-clock upper bound. The experiment reports a sample, not a theorem.

Derive storage bounds before timing. With batch depth d and span n, at most d*n
response bytes are borrowed in the copying/ring implementation. It allocates
16KiB regardless of active depth; registered mode holds that region until explicit
unregistration. Sendfile allocates no producer payload region here, but the source
file and kernel queue references remain. Receiver scratch, request metadata,
latency arrays, ring mappings and kernel memory are additional costs. Mapping
length is not page-rounded RSS. A count of adapter calls is not an instruction
count or an exact count of underlying system calls.

Predict the small-scattered result separately from the large-sequential result.
Setup, CQ dispatch and verification can dominate a tiny payload. Warm cached file
reads may already be cheap. The controlled first-batch stall can dominate the
whole latency mean; retain it and show the steady distribution separately.
The [`posix_fadvise`](https://man7.org/linux/man-pages/man2/posix_fadvise.2.html)
hint and [`mincore`](https://man7.org/linux/man-pages/man2/mincore.2.html) snapshot
support a narrowly labeled guest page-cache comparison, not a physical disk
claim. Counterfactual questions matter more than a fastest-backend label: what
happens when the peer stalls, registration fails, an ID retires, the batch is
partially submitted, or the cache assumption fails?

Graduated hints: (1) list every event that can still name a slot; (2) model pending
target events and pending cancellation events as different sets; (3) compare
those sets to BUSY/DONE/cancel_pending after every transition. For the benchmark,
first prove byte/offset equality, then label the clock interval and every retained
resource. Only then interpret timing differences.
