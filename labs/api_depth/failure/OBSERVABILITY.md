# T21.07–09: diagnostics are executable resource contracts

This packet supplies `observability.h/c/tests`, `diagnostics.h/c/tests` and the
original PR-T21-03 criteria. Families A/B/C/D/F/G/H/I/J apply where their mechanisms
occur. Prerequisites are the bounded T18/T19 record, ownership and API contracts,
T20 transport boundaries, and the earlier T21 failure packets. The reusable core
has no allocator or I/O dependency. Fault-process experiments, ELF inspections and
the separate-process diagnostic consumer are hosted assessor tools.

## Assertions, policy and optional work

External capacity is0..3; capacity4 must return INVALID through an ordinary branch
with NDEBUG both defined and absent. The original faulty internal count4 is a
separate invariant defect. Test four deliberately distinct policies: an enabled
assert terminates through the hosted runtime; error-return records CONTRACT and
performs one modeled cleanup; process-panic exits73 without cleanup; simulated
safe-state sets an output-enable variable to0 and retains ownership. A process
exit is not a board safe state. The driver supplies these original synthetic
defects without invalid pointer accesses. State what a physical adapter would need
to establish before claiming outputs are safe.

The panic sink deliberately fails. The bounded reporter stores a fallback record;
it never allocates or retries the failing sink recursively. Keep required work out
of assertions. In the defective specimen `assert(++work==1)` increments only with
assertions enabled. The repair increments first, then asserts the invariant. Both
forms reject invalid capacity before doing work. Preserve the defective specimen
as a diagnosis fixture, not as a portable recommendation. Inspect actual
preprocessed source and executable state, including uninstrumented O2 builds.
N1570 7.2 supports these draft-era semantics; the bounded N3096 comparison changes
assert's macro parameter to variadic, not its permission to omit evaluation under
NDEBUG. Neither draft is relabeled as the published C17 standard.

## Records, time, admission and reporting

An `AeRecord` encodes to12 octets, never a struct dump: version1, severity0..3,
unit1(milliseconds) or2(microseconds), timestamp width8 or16, operation generation
1..3, payload length0..4, timestamp little-endian in two bytes, and four payload
bytes. Unused payload bytes are zero. Decode requires exactly12 bytes and rejects
noncanonical unused bytes, unknown versions and out-of-range fields without
changing output. Encoding accepts capacity>=12. An empty payload and maximum
timestamp are valid. Native `sizeof(AeRecord)` is an implementation cost to measure,
not the wire size. Require CHAR_BIT8 and an unsigned type able to represent65536
for this selected implementation; other data models need a deliberate adaptation.

Timestamp comparison returns equal, before or after only under a known elapsed
distance strictly below half a cycle; exactly half is AMBIGUOUS and preserves the
output. Width8 wraps at256 units, width16 at65536. For instance0 follows255 only
under the stated bounded-distance premise. No pairwise modular comparator creates
a total order over arbitrary historical observations. Supply a three-point cycle
counterexample, then derive the maximum sampling gap for both timestamp units.

The log holds three records. DROP_OLDEST admits A..E as[C,D,E], dropped2. A separate
token limiter has capacity3 and earns one token per two elapsed ticks, preserving
a one-tick remainder. Each refill accepts elapsed0..127 from a caller that has
already resolved clock wrapping; it is not a time source. Rate-limited admission
increments `limited`, distinct from dropped/rejected counts. PROTECT_HIGH rejects
an incoming high-severity record if full and refuses to evict any queued high
record. A full all-low queue may still drop its oldest low record for another low
record. Rejection does not consume a token. Explain this explicit choice and its
starvation tradeoff; it is not a universal logging policy.

The sink receives one complete12-byte record and returns1(accepted),0(stalled)
or-1(failed). A stall retains the head and increments `stalls`; failure retains it
and returns DEVICE. Successful acceptance removes exactly one head. This sink
contract has no partial acceptance; a real byte-stream adapter must implement
that separately. Reporting may be reentered by the sink's failure path. A busy
guard routes nested reports directly to two fallback slots. A failed outer send
also records its failure. Once unavailable or full, fallback increments `lost`,
saturating at65535. The last-resort counter is observable only while its owning
memory survives; there is no lossless or durable reporting claim.

## Feature boundaries and stable diagnostics

Compile all four AE_DEV/AE_INTEGRATOR combinations at O0/O2. Enabled developer
traces retain four unsigned entries. Enabled integrator diagnostics retain another
four and filter by runtime threshold. The macro evaluates a payload expression
only when the feature and severity admit it; an omitted feature cannot be restored
by changing a runtime threshold. Keep ordinary capacity validation identical.
Measure argument evaluations, emitted categories, symbols and sections, rather
than merely testing an if statement. Under the selected section-GC link, disabled
call sites also allow the unused emit function to disappear. Taking its address in
another translation unit keeps that function and makes an explicit call evaluate
its argument even when the macro feature is absent; its dedicated buffer remains
compiled out. Code elimination is a compiler/linker observation, not an ISO size
guarantee. Concurrent emission requires synchronization not supplied by this
serialized diagnostic buffer.

Reuse the actual T21 public error/snapshot provider from TAXONOMY.md. A cross-process
frame is16 bytes: little-endian32-bit public code and operation, then its versioned
8-byte diagnostic snapshot. Read exactly16; reject trailing bytes, truncation,
unknown version, reserved bytes or inconsistent operation. Two private layouts
must feed an unchanged independently built consumer. This file-based process
exchange tests serialization; it does not promise IPC authentication or durability.

Deliver the nine criterion-specific explanations, public implementation, fixed
wire vectors, wrap proof/counterexample, ring traces, recursive-sink trace,
four-switch build table, process-boundary rejection evidence and cost observations.
Hints: separate admission from transmission; record loss without reusing the
broken transport; inspect macro expansion before assembly. Level1 is nominal
output reproduction; level2 requires contract-specific counterexamples and actual
positive/negative checks; level3 derives changed timing, fallback and linkage costs
without copying the private analysis. The original minimum-seven-day packets and
independent learner evidence remain separate from this author campaign.
