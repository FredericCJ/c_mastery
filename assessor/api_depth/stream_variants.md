# Exact T19.08 changed-condition analysis

For .a, the existing signed callback can report progress or an error, but cannot
report both. The new bridge calls the actual original `sf_pump` once per
external invocation. A positive count therefore advances the original cursor;
the wrapper then gives `SV_ERROR` precedence over producer completion. This
extra terminal status is necessary when the original producer has already
entered DONE. Starting another request cancels only an old active producer
request, and validates new admission first. It must not erase a prefix already
committed at the peer. A count exceeding the offer terminates the wrapper but
cannot establish what the misbehaving callback actually did.

For .b, let segment ends be1, n+1, n+2 and cursor x. Segment i contributes
[max(x,start_i),end_i) only when its length is positive. This skips whole
segments and preserves the exact within-segment suffix. The data pointer and
length must be trimmed together; shortening the length while retaining the
original pointer duplicates data. n=0 still has a length byte and checksum.
The independent test decoder accumulates the checksum from supplied payload,
not from the adapter's internal fields. The omitted-checksum control must fail
as a semantic assertion, not access an invalid span.

For .c, FIFO applies to complete frames, not callbacks. Conditional progress
is enough to serve two bounded requests, but finite capacity cannot promise
unbounded admission or unconditional latency. Each slot's generation survives
reuse. A globally increasing ticket chooses the oldest active request; refusing
before ticket wrap keeps the comparison valid. Cancellation matches both live
slot owner and generation. Returning the event by value separates a completed
request's result from the slot which a new admission may reuse.

After two bytes of [3,1,2,3,6], immediately emitting [1,8,8] creates a different
byte sequence that the old peer still interprets as the first frame. A local
cancel flag cannot undo that prefix. Taint therefore blocks all subsequent
pumping until an external quiescent transport/decoder reset is asserted. The
cancelled frame is removed; queued unstarted frames keep cursor0. Reusing the
partially transmitted frame's old cursor on a new connection would lose its
prefix, so this implementation does not resume that cancelled request. Error
after all bytes also taints: complete local acceptance is not remote semantic
acknowledgment. This reset model is deliberately distinguished from an executed
socket shutdown/reconnect experiment.

The seven mutations each preserve well-formed local memory and violate one
observable property. The source-bound campaign also substitutes the unsolved
original producer to establish an actual dependency. ASan/UBSan and optimized
profiles provide bounded author evidence, not independent review, a delayed
learner attempt, strict freestanding closure or a concurrency proof.
