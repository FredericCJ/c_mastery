# Coherent-memory and changed-turn analysis packets

These are required concrete analysis implementations for X-T39-001-V1/V2 and
X-T40-002-V2. Retain their original seven-day delay, rubric, learner ownership
and independent assessment rules. The imported C model and target-adapter
tests remain required; these arguments do not replace executable obligations.

## X-T39-001-V1: shared cache lines

Use a declared32-byte maintenance granule and a64-byte aligned arena. BufferA
occupies offsets0..15, B offsets16..31. They share one line even though their
C array extents do not overlap. After A's line is cleaned/published, a CPU
write to B dirties the same line. A subsequent clean can overwrite an independent
device write to A; an invalidate can discard B's dirty update. Exercise these
consequences with the actual `EvVisibility` CPU/device arrays in the ring
variant laboratory, and distinguish logical byte ownership from maintenance
granularity.

Implement the allocation **policy**: round each admitted allocation's reserved
extent to a whole granule, require both address and reserved length aligned,
and place A at0..15 within reservation0..31 and B at32..47 within32..63.
No other mutable object may occupy either reserved line while its buffer is
device-owned. Checked rounding must prove the reservation remains within the
arena before forming a pointer. Two16-byte payloads now cost64 reserved bytes;
the32-byte overhead buys independent line ownership. A packed alternative
must transfer the entire shared line as one ownership unit, denying both CPU
writers until returned ownership/invalidation. Identify the throughput/space
tradeoff and explain why alignment of only the first pointer is insufficient.

## X-T39-001-V2: explicit coherent-DMA profile

Specify a changed machine contract, rather than assuming a cache-coherent
target exists: CPU and the selected device participate in one coherent data
domain for these admitted buffers. Before publication, all payload/descriptor
writes are ordered before the ownership doorbell by the device's documented
release mechanism. After device-returned ownership, the documented acquire
mechanism orders its status and payload writes before CPU observation. Only
under **both** visibility and ordering premises are explicit data clean and
invalidate unnecessary. C17 atomics alone do not define a device doorbell or
make a nonparticipating DMA engine coherent.

The permitted history remains CPU-write → release-publication → device-use →
returned-ownership → acquire-observation → CPU-reuse. Coherence does not allow
concurrent mutation of a borrowed TX payload, early free after timeout, stale
generation completion, invalid reachability, misaligned descriptors, or treating
local completion as peer delivery. Counterexample: coherent bytes are current,
but the doorbell becomes visible before the final length field; the device
reads the previous extent. Another: software timeout followed by allocator
reuse permits a late coherent device write to corrupt the new object.

The supplied Python finite witness checks the ordering graph and both invalid
histories. It is an analysis model, not an executed coherent-DMA adapter. A
selected real target needs freshly inspected cache/interconnect/device rules,
actual barrier/descriptor code, link placement and separate physical evidence.

## X-T40-002-V2: baud and turn policy change

At PCLK42MHz and admitted OVER16 divisor729, actual baud is42000000/729,
not exactly57600. An8N1 byte consumes10 bit periods. A conservative escaped
96-byte maximum occupies960×729/42000000 seconds, or116640/7 microseconds;
rounding upward gives16663us. It cannot fit the original10000us completion
budget, as the actual physical producer demonstrates before raising DE.

Define a separately negotiated changed policy:200us peer guard,4000us
additional first-byte window,20000us whole-reply completion allowance from
the first byte, and50000us minimum next-turn interval from the actual POLL
TC/DE-low origin. First byte4200 is admitted;4201 is late if no start occurred.
Even a first byte at4200 plus the full20000 budget and200 guard ends by24400,
before50000. This is a bounded protocol-time argument; ISR/polling latency,
actual PCLK tolerance and line propagation remain additional premises.

Do not pass these constants to the unchanged E7Grant and call it compatible:
its2ms/10ms/25ms contract is fixed. A changed peer must explicitly select this
new policy, and a new scheduler adapter must retain separate start/end clocks
if implemented. The original variant asks for recomputation and explanation;
this packet supplies both, while the original adapter continues refusing an
incompatible transmission. Capture actual final TC, DE edges, first byte and
final delimiter on both peers before claiming physical timing. Current evidence
contains no such capture.

## Deliverables and independent assessment

Submit the two buffer maps, arithmetic overflow obligations, ownership/order
graph, two counterexamples and exact baud fractions. Compare them to the actual
parent C model observations. Produce a fresh delayed case with48-byte granules
and a non-power-of-two alignment policy, then a changed baud/divisor whose
rounding reaches the deadline boundary. An assessor scores the original
criterion separately: reproducing numbers is level1; deriving premises and
falsifiers is level2; independently defending unseen changes is level3. Python
checks are author analysis evidence only, never proof of absolute mastery or
independent learner success. All bytes/events are synthetic local fixtures.

Graduated hints: first draw reserved extents rather than payload extents; then
identify the release/acquire edges; finally carry the baud as a fraction until
the final conservative rounding. Keep the unaided attempt before using hints.
