# T24.05: bytes, ownership and replay

Read the inspected `recvmsg`, `sendmsg`, `shutdown` and `unix` passages identified
in `research/v5/expansion/legacy_framing/resources.json`. Linux permits a receive
to return the bytes currently available; a send boundary is not an application
message boundary. A successful send does not acknowledge application processing.
`shutdown(SHUT_WR)` terminates one direction; closing the descriptor is a different
lifetime transition. The original local-socket probes below establish the stated
Linux observations, not TCP retransmission, congestion control or network timing.

## T24.05.a — derive an incremental decoder

Implement `rf_parser_init`, `rf_push`, `rf_feed` and `rf_eof`. Begin with one
length octet and payload length 0–8. Derive a state table with header progress,
decoded length, body progress, and a latched malformed state. `rf_feed` stops
after the first frame and reports exactly how much input it consumed; the caller
retains and resubmits the suffix. An empty input span does not synthesize a frame.

For example, `[0, 3, 0x41, 0, 0x42]` contains an empty frame and a three-octet
frame. Splits `[0,3] / [0x41] / [0,0x42]` and a single five-octet input must
produce the same two outputs. EOF immediately after `3` is truncated body, not
an empty frame. Show why the output is unchanged on incomplete and rejected
input. Explain the difference between parser failure and a transport error.

Construct all adjacent length pairs 0–8 with embedded zero bytes. Enumerate every
nonempty chunk partition of each resulting wire and compare with a complete-buffer
decoder that walks absolute frame extents. Also classify every prefix as a frame
boundary or truncation. Do not derive that expected classification by calling the
incremental parser. Include invalid lengths, repeated calls after failure and
output sentinels. Each input octet is processed once; derive O(n) decoding and
fixed payload storage. Explain why using `strlen` or casting a packed structure
would invalidate the representation argument.

The changed-condition task uses a two-octet big-endian header. Derive its
partial-header state and arithmetic bounds before coding it. With octets, a
completed header can be 65535 even though the accepted body is at most eight.
Reject oversized values before body storage. Test all header values, including
zero and 65535, and EOF after only the first header octet. Do not assume that
changing one array size establishes the altered contract.

Hints: first separate consumed input from published output; next describe the
state immediately after a complete zero-length header; finally write the
complete-buffer oracle before generating chunk partitions.

## T24.05.b — half-close and uncertain completion

Use an owned socket pair in nonblocking mode. Send the final complete request,
then call `shutdown(SHUT_WR)` on the client while keeping its read direction
open. The server must parse the request, observe EOF only at a frame boundary,
and send a framed response. Retain the client's received response as evidence
that write half-close did not destroy the opposite direction.

On separate connections, end input mid-header and mid-body. Reject both without
publishing a partial message. Reconnect with a freshly initialized parser;
do not append new bytes to the abandoned old connection's body. State who owns
each descriptor and buffered request at every exit, including deadline expiry.
Reuse the absolute deadline contract so interruptions do not replenish a budget.

The delayed task adds request identities. Define a live-server session and a
four-entry result cache. A new `(session, id, delta)` adds an unsigned delta to
a synthetic 32-bit total. A retry of the same tuple returns the original result;
reuse of an ID with a different delta, another session, cache exhaustion or total
overflow rejects without mutation. After committing the effect on the first
connection, deliberately lose its reply. Retry over a second connection and
show both the returned result and the count of actual effects.

Derive the difference among known failure before admission, unknown outcome after
connection loss, and a cached successful result. A cached result is associated
with the original effect, not the current total. Explain why eviction, process
restart, concurrent requests and storage failure need additional protocols.
The required cache is bounded and volatile; do not claim durable exactly-once
execution. Alternative policies may reject all ambiguous retries if they state
that reduced service contract explicitly; they do not satisfy this task's
recover-the-lost-reply requirement without a separate implementation.

Hints: draw two independent socket directions; place effect and reply on separate
timeline events; then identify the information retained across a connection loss.

## T24.05.c — bound the complete retained set

Implement `struct rf_message` and `struct rf_outbox` operations. The fixed limits are four
connections, eight frames per connection, eight payload octets per frame, and a
separate global payload limit 0–256. Enqueue returns 1 on transfer, 0 on capacity
rejection and -1 on an invalid contract. A fifth connection, ninth frame, live
duplicate ID or duplicate object must not silently acquire ownership. A partially
sent message remains exclusively queued until every encoded octet commits.

Stop all four peers reading. Fill the owned nonblocking socket buffers within a
bounded prefill limit until EAGAIN, then enqueue the maximum frames. Demonstrate
that an unsuccessful write leaves both cursor and owner unchanged. Resume peers,
advance frames in one-octet progress steps and check exact frame order at each
receiver. Separately supply a deterministic writer returning alternating short
positive prefixes and would-block; distinguish requested bytes from committed
bytes. State whether your kernel run actually produced a short positive syscall
return; do not rename a model return as an OS observation.

Disconnect at every relevant cursor boundary. Every queued object becomes
cancelled exactly once; delivered objects stay delivered; repeated disconnect
does not decrement a budget twice. No callback may retain the borrowed wire.
Derive the invariant `bytes = sum(payload_length of queued messages) <= limit`.
Headers and object storage remain separately bounded even for zero-length frames.

For the delayed task, run a producer faster than all peers, including a phase
with no drain at all and variable payload sizes. Show why per-connection frame
counts alone do not implement an arbitrary global byte limit. Compare admission
to an independent count-and-byte ledger for every limit 0–256 and length 0–8.
State the maximum retained payload, headers, object storage and kernel prefill.
Account for the cost of invariant validation: with generalized capacity K,
pairwise duplicate checking is O(K²), even though this lab fixes K at 32.

Hints: keep a message until its final byte, not its first successful send; make
admission a transaction; then test zero-length frames under a zero-byte budget.
