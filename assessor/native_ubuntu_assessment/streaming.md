```json
{
  "schema_version": 2,
  "kind": "assessor-packet",
  "packet": "streaming",
  "path": "streaming-to-perf",
  "route": "contextual-v2",
  "tasks": [
    {
      "id": "T-STR-1",
      "title": "Fragmented acquisition log",
      "role": "primary",
      "initial_follows": {
        "nodes": ["U-T14-01-04", "U-T20-01-04", "U-T15-08-04", "U-T24-03-03"],
        "episodes": ["EP-byte-ring", "EP-record-ports", "EP-wire-framing", "EP-framed-streams"]
      },
      "changed_follows": {
        "nodes": ["X-T15-019", "U-T25-03-04", "U-T55-01-03"],
        "episodes": ["EP-nested-events", "EP-publication-order", "EP-spsc-ring"]
      },
      "also_required_nodes": [],
      "mechanisms": ["rings-incremental-parsing"],
      "gap_mechanisms": [],
      "requires_selection": true,
      "min_rejected_alternatives": 2,
      "requires_revision_under_changed_assumption": true,
      "released_after_seal": true,
      "changed_assumption": "The producer and parser become separate threads, the consumer may suspend indefinitely holding two borrowed outputs, and shutdown can arrive at any moment.",
      "canaries": [
        {"public": "one-byte length frame with payload limit 8 and a CRC-8 (EP-wire-framing exit; U-T15-08-0x)", "task": "two-byte big-endian length that includes the kind byte, limit 32, no checksum"},
        {"public": "struct cd_ring with split spans and rf_feed over socket chunks (EP-byte-ring, EP-framed-streams)", "task": "an eight-record bound and a 128-byte input bound stated together, with accepted-record retention across a full output"}
      ]
    },
    {
      "id": "T-STR-2",
      "title": "Foreign sample region with out-of-band loss count",
      "role": "gap",
      "requires_test_fixture": true,
      "initial_follows": {
        "nodes": ["U-T55-01-03", "U-T43-09-01", "U-T43-09-02", "U-T43-09-03"],
        "episodes": ["EP-spsc-ring", "EP-pmu-counting", "EP-sampled-records"]
      },
      "changed_follows": {
        "nodes": ["U-T55-01-03", "U-T25-03-04"],
        "episodes": ["EP-spsc-ring", "EP-publication-order"]
      },
      "also_required_nodes": [],
      "mechanisms": ["rings-incremental-parsing"],
      "gap_mechanisms": ["pf_ring_drain"],
      "requires_selection": true,
      "min_rejected_alternatives": 2,
      "requires_revision_under_changed_assumption": true,
      "released_after_seal": true,
      "changed_assumption": "Downstream processing becomes asynchronous: workers release delivered events in any order and at any time.",
      "canaries": [
        {"public": "pf_ring_drain: 8-byte header (u32 type, u16 misc, u16 size), SAMPLE 40 B, LOST 24 B, LOST_SAMPLES 16 B, 64-bit head/tail, 64-byte test ring (labs/performance_followthrough/pmu.h, PMU_PROTOCOL.md, pmu_tests.c)", "task": "4-byte header (u16 kind, u16 size), event 12 B, overrun marker 8 B, 32-bit free-running positions, 128-byte area, loss also reported by a separate cumulative counter"}
      ]
    }
  ],
  "criteria": [
    {
      "id": "STR-M1",
      "name": "Parser state machine and progress invariant",
      "dimensions": ["mechanism"],
      "tasks": ["T-STR-1"],
      "earns_2": "Draws an explicit state machine (length high, length low, kind, payload, terminated) and states a progress invariant that ties the reported consumed prefix to bytes whose ownership transferred; every transition is justified for a call boundary at any byte.",
      "earns_3": "Also presents a minimal counterexample to an abandoned design, with the exact input and the clause it breaks, and says which invariant the final design adds.",
      "rejects": [
        "Parsing each call independently and resetting state at the call boundary on the assumption that delivery is frame aligned.",
        "Reporting consumed as bytes read from the caller's span rather than bytes whose ownership transferred, so a full output loses or duplicates bytes."
      ]
    },
    {
      "id": "STR-C1",
      "name": "Every split, stall and failure",
      "dimensions": ["correctness"],
      "tasks": ["T-STR-1"],
      "earns_2": "Produces the same record sequence and the same concatenated consumed prefix for one byte per call, all at once and every two-part split of the assessor stream, with a stalled consumer; rejects lengths 0 and 33, a final lone length byte and a truncated payload as distinct outcomes from clean end of input.",
      "earns_3": "Also builds an oracle or metamorphic property that was not copied from the implementation (for example split-invariance over random cut points plus prefix equivalence) and uses it to kill the assessor's seeded mutants (length excluding the kind byte; limit 32 accepted as 31 or 33).",
      "rejects": [
        "Testing only one byte per call and whole-stream delivery, which never splits inside the two length bytes.",
        "Reading the length as payload length, so a record with length 3 is believed to carry three payload bytes."
      ]
    },
    {
      "id": "STR-O1",
      "name": "Accepted records survive pressure and shutdown",
      "dimensions": ["memory_ownership"],
      "tasks": ["T-STR-1"],
      "earns_2": "Keeps an accepted record valid and unmodified until its explicit release through a full output and repeated stop and release, so every accepted record has exactly one terminal ownership result and nothing is freed under a live borrow.",
      "earns_3": "Also names the publication edge that makes a record visible and the reuse edge that returns its storage in the threaded variant, and shows the execution in which removing either one lets the producer overwrite a borrowed record.",
      "rejects": [
        "Dropping the oldest record when the output is full, which overwrites a record the consumer may still be reading.",
        "Releasing storage at shutdown while a borrow is outstanding because the stop request is treated as an implicit release."
      ]
    },
    {
      "id": "STR-R1",
      "name": "Byte and object bounds with stalled borrowers",
      "dimensions": ["resources_dependencies"],
      "tasks": ["T-STR-1"],
      "earns_2": "Proves both bounds: at most 128 retained input bytes (the longest partial frame is 2 + 32 = 34 bytes) and at most eight complete records, with no allocation outside the supplied storage.",
      "earns_3": "Also accounts for the stalled consumer (two borrows leave six usable record slots), states the effect on the producer's offers, and lists what lies outside the accounting (kernel socket buffers, the consumer's own copies) rather than hiding it.",
      "rejects": [
        "Counting the bound in records only, so a worst-case record size makes retained bytes exceed 128.",
        "Claiming the bound holds on average or after the consumer drains, which fails while the consumer is suspended."
      ]
    },
    {
      "id": "STR-D1",
      "name": "First invalid transition and ordering versus corruption",
      "dimensions": ["failure_diagnosis"],
      "tasks": ["T-STR-1", "T-STR-2"],
      "earns_2": "From a failing trace names the first invalid transition (state and byte index, or position) and does not equate a crash, timeout or compile error with the expected malformed-frame rejection.",
      "earns_3": "Also separates a publication-ordering fault from parser corruption with a controlled experiment: the same bytes replayed single-threaded pass while the threaded run fails, or the failure disappears under a stated delay, with the prediction recorded before the run.",
      "rejects": [
        "Treating every failure as a parser bug and adding retries or sleeps.",
        "Counting a rejected malformed frame, a crash and a timeout as the same outcome."
      ]
    },
    {
      "id": "STR-T1",
      "name": "What transfers and what must be reread",
      "dimensions": ["communication_transfer"],
      "tasks": ["T-STR-1", "T-STR-2"],
      "earns_2": "Adapts to the changed condition unaided and states which earlier storage and invariant arguments carry over and which ordering or ABI rules must be reread.",
      "earns_3": "Also states exactly which earlier proof is replaced (the single-writer index argument, the in-place borrow) and which remains valid (the frame grammar, the byte bound), with the changed-condition prediction made before running and matching the outcome or the mismatch explained.",
      "rejects": [
        "Asserting that the earlier proof still applies because the data structure is unchanged although the writer and consumer are now different threads.",
        "Restating the task as the prediction."
      ]
    },
    {
      "id": "STR-M2",
      "name": "Snapshot and ordering of a foreign region",
      "dimensions": ["mechanism", "memory_ownership"],
      "tasks": ["T-STR-2"],
      "earns_2": "Loads head once per drain with acquire ordering, consumes no more than head - tail with unsigned 32-bit arithmetic, finishes every read of the data before storing tail with release ordering, and never re-reads head mid-drain.",
      "earns_3": "Also draws the execution graph for each edge and shows the failing execution when it is weakened (relaxed head load reads stale bytes; tail stored before the copy lets the device overwrite unread bytes).",
      "rejects": [
        "Treating volatile as the ordering mechanism.",
        "Masking head and tail to the area size, which makes a full region look empty."
      ]
    },
    {
      "id": "STR-C2",
      "name": "Wrap, partial and invalid records",
      "dimensions": ["correctness"],
      "tasks": ["T-STR-2"],
      "earns_2": "Handles a header split across the end of the area, a body split across it, a partial record (tail unchanged), size 0, 3 and 65, and an event of size 16 (tail left at the start of the bad record, earlier records consumed), unknown kinds (skipped and counted) and positions that wrap at 2^32.",
      "earns_3": "Also differential-tests against a separately written byte-level model with random sizes and wrap offsets and uses it to catch the assessor's seeded mutants (header read through a pointer cast that ignores the wrap and the alignment of an arbitrary start byte; a wrap test written as offset + size >= area).",
      "rejects": [
        "Copying a whole record from the tail offset with no wrap handling.",
        "Skipping a corrupt header by scanning for the next plausible header, which turns a framing failure into silent loss."
      ]
    },
    {
      "id": "STR-R2",
      "name": "Loss accounting from two sources",
      "dimensions": ["resources_dependencies", "failure_diagnosis"],
      "tasks": ["T-STR-2"],
      "earns_2": "Keeps the in-band overrun total and the out-of-band cumulative counter separate, reports their difference as the loss not yet reported in-band, and checks every sum for overflow.",
      "earns_3": "Also computes the assessor's case correctly (total loss 5, not 8), states when a complete claim is unavailable (counter unread, hardware denied) and reports a lower bound instead.",
      "rejects": [
        "Reporting total loss as in-band markers plus the counter, which counts reported drops twice.",
        "Concluding that no loss occurred because no overrun marker was seen."
      ]
    },
    {
      "id": "STR-O2",
      "name": "Out-of-order release and the tail",
      "dimensions": ["memory_ownership", "communication_transfer"],
      "tasks": ["T-STR-2"],
      "earns_2": "Advances tail only across the contiguous released prefix and predicts the tail after each release before running (0, 0, 36 for the assessor order).",
      "earns_3": "Also derives the consequence for the device (one held record can block all later space) and chooses and defends a bounded policy, rejecting a second with a trace.",
      "rejects": [
        "Advancing tail to the highest released offset, which lets the device overwrite a record still being read.",
        "Tracking only a count of released records."
      ]
    }
  ],
  "worked_numbers": [
    {"id": "s10-bytes", "expr": "10*4", "expected": 40, "text": ["40 bytes"], "occurrences": 1},
    {"id": "s10-held-prefix", "expr": "8*4", "expected": 32, "text": ["32 bytes"], "occurrences": 1},
    {"id": "area-events-fit", "expr": "128//12", "expected": 10, "text": ["ten fit"], "occurrences": 1},
    {"id": "area-free-after-ten", "expr": "128-10*12", "expected": 8, "text": ["8 bytes remain"], "occurrences": 1},
    {"id": "design-b-free", "expr": "128-9*12", "expected": 20, "text": ["free space becomes 20"], "occurrences": 1},
    {"id": "design-a-dropped", "expr": "20-10", "expected": 10, "text": ["`dropped` = 10"], "occurrences": 1},
    {"id": "design-b-dropped", "expr": "20-11", "expected": 9, "text": ["`dropped` = 9"], "occurrences": 1},
    {"id": "f4-pending", "expr": "(0x10-0xFFFFFFF4)%2**32", "expected": 28, "text": ["28 pending bytes"], "occurrences": 1},
    {"id": "f4-final-tail", "expr": "(0xFFFFFFF4+24)%2**32", "expected": 12, "text": ["0x0000000C"], "occurrences": 1},
    {"id": "f6-wrapped-difference", "expr": "(5-10)%2**32", "expected": 4294967291, "text": ["0xFFFFFFFB"], "occurrences": 1},
    {"id": "release-order-tail", "expr": "12+12+12", "expected": 36, "text": ["0, 0, 36"], "occurrences": 2},
    {"id": "longest-partial-frame", "expr": "2+32", "expected": 34, "text": ["2 + 32 = 34"], "occurrences": 1},
    {"id": "loss-total", "expr": "3+2", "expected": 5, "text": ["total loss 5"], "occurrences": 2},
    {"id": "loss-double-count", "expr": "3+5", "expected": 8, "text": ["gives 8"], "occurrences": 1}
  ],
  "leak_fingerprints": ["00 03 11 A0 A1 00 01 22 00 02 33 B0", "A device writes event records into a shared data area of 128 bytes", "F8 loss: the device discards 3 events"],
  "entry_check": {
    "accepted_artifacts": [
      {"artifact": "bytes.checked", "producer_node": "U-T04-01-04", "needed_for": ["T-STR-1", "T-STR-2"]},
      {"artifact": "bytes.cursor", "producer_node": "U-T03-02-03", "needed_for": ["T-STR-1"]},
      {"artifact": "containers.ring", "producer_node": "U-T14-01-04", "needed_for": ["T-STR-1"]},
      {"artifact": "codec.records", "producer_node": "U-T20-01-04", "needed_for": ["T-STR-1"]},
      {"artifact": "sync.spsc", "producer_node": "U-T55-01-03", "needed_for": ["T-STR-1", "T-STR-2"]}
    ],
    "source_identities": [
      "SHA-256 of curriculum/v5/routes/contextual-v2.json and the repository commit",
      "SHA-256 of every learner source file and of each imported bundle (containers.ring, codec.records, sync.spsc)",
      "the learner's own pf_ring_drain source and the build record naming whether it was labelled",
      "compiler and version, ThreadSanitizer availability, python3 version"
    ],
    "tools": ["C17 compiler with sanitizers", "ThreadSanitizer", "python3", "perf_event_paranoid value (recorded, never changed)"],
    "probes": [
      {"id": "P-STR-1", "ask": "Draw a ring where head equals tail and say how your representation distinguishes empty from full, without opening old code.", "tests_node": "U-T14-01-03", "remediation": "Redo the full/empty representation decision and the capacity-3 and capacity-5 traces."},
      {"id": "P-STR-2", "ask": "A record's two length bytes arrive in separate calls. What does your parser report as consumed after the first call and what does it retain?", "tests_node": "U-T24-03-03", "remediation": "Redo segmented-header feeding with exact consumed-prefix accounting."},
      {"id": "P-STR-3", "ask": "Name the store and load that make a slot's bytes visible, and the pair that makes the slot reusable.", "tests_node": "U-T55-01-03", "remediation": "Redo the slot-reuse chains for capacity 4."},
      {"id": "P-STR-4", "ask": "Classify a zero count with positive running time, a group with zero running time and a denied event.", "tests_node": "U-T43-09-01", "remediation": "Redo the PMU protocol classification before any hardware experiment."},
      {"id": "P-STR-5", "ask": "Why can the event's cumulative lost count not be added to the in-ring lost records?", "tests_node": "U-T43-09-03", "remediation": "Redo the loss-accounting table in PMU_PROTOCOL.md and its worked trace."}
    ],
    "known_unstated": [
      "The route review lists no criterion for pf_ring_drain (U-T43-09-03 scores sampling attribution), so the learner may reach T-STR-2 with no assessed kernel-ABI ordering or wrapped-record work. T-STR-2 supplies that evidence; a failed probe P-STR-3 or P-STR-5 here is a route gap and is recorded as an unstated prerequisite.",
      "U-T43-09-03 gates on U-T55-01-03, U-T24-03-03, X-T15-019 and U-T24-05-03 as concept edges only; the route does not force the learner's own sync.spsc or ring bundle into the PMU workspace, so the bundle hashes must be requested."
    ]
  },
  "imitation_probes": [
    {"check": "IM-VERBATIM", "applies_to": ["T-STR-1", "T-STR-2"], "compare_with": ["labs/hosted_systems/stream_depth/", "labs/performance_followthrough/pmu_tests.c", "labs/performance_followthrough/PMU_PROTOCOL.md", "curriculum/v5/routes/v2/streaming.md"], "probe": "Look for the public constants (8-byte header, 40/24/16-byte records, one-byte length, limit 8, 64-byte area) in the learner's traces, then ask the learner to change the header to 4 bytes live and name the lines that change."},
    {"check": "IM-ADAPTER", "applies_to": ["T-STR-1", "T-STR-2"], "compare_with": ["assessor/performance_followthrough/", "labs/support/"], "probe": "Hash the learner's pmu_sampling, pf_ring_drain and ring sources against every assessor C/H/S file and every shipped lab file; inspect the build record for TEST IMPLEMENTATION."},
    {"check": "IM-REJECTED", "applies_to": ["T-STR-1", "T-STR-2"], "compare_with": [], "probe": "Ask for the smallest stream or schedule on which each rejected alternative breaks, then run the assessor's model of it."},
    {"check": "IM-PREDICT", "applies_to": ["T-STR-1", "T-STR-2"], "compare_with": [], "probe": "Compare the sealed prediction with the worked answers below; then mutate one learner line offline and ask which trace fails first."}
  ]
}
```

# Withheld packet: streaming input to sampled-record consumption

Path `streaming-to-perf` in `contextual-v2`. This packet follows the episodes
EP-byte-ring, EP-record-ports, EP-wire-framing, EP-framed-streams,
EP-nested-events, EP-publication-order, EP-spsc-ring, EP-pmu-counting and
EP-sampled-records, and the nodes in each task's manifest entry. The transfer
here is unassisted selection under requirements that name no structure; the
route's own encounters for this mechanism end at U-T43-09-03 with no criterion
for `pf_ring_drain`, which T-STR-2 supplies.

Administer T-STR-1 (primary) and T-STR-2 (gap). Run the entry check in README.md
first; this packet's probes and artifacts are in the manifest above. For the
optional native extension require perf ABI protocol competence separately: a
successful byte-ring test does not establish it, and a denied or unscheduled
hardware group is a recorded state.

### Task T-STR-1

#### Release: initial

An acquisition device produces records with a 2-byte big-endian length,
a 1-byte kind and 0 to 31 payload bytes. The length counts the kind byte and the
payload. Zero or length above 32
is malformed. Delivery may split anywhere, including between length bytes.
At most 128 input bytes and eight complete output records may be retained by the
service; a record the consumer holds counts as retained. A consumer accepts one record or refuses progress. It may retain an
accepted record until an explicit release, and the service must not overwrite it.

Design and implement an adapter. Report the exact consumed input prefix and
preserve accepted records while the output is full. No read past the admitted
span, silent byte loss, duplicate delivery or allocation outside the supplied
storage is permitted. Malformed framing terminates that stream; already accepted
records remain valid. End of input distinguishes clean completion and truncation.

Before implementing, record the requirements you derived, the representation you
select, two rejected alternatives with the smallest input that defeats each, and
one experiment whose outcome would distinguish your design from the nearest
alternative, with its predicted outcome.

#### Release: changed condition

After the delay, the producer and the parser are separate threads and the consumer
can suspend indefinitely holding two borrowed outputs. Shutdown can be requested
at any moment, including while outputs are borrowed. Adapt your design. Before
running, predict how many further records can be accepted while two are borrowed,
what the producer observes when it offers more input, and what shutdown reports.
State which of your earlier arguments still hold and which must be replaced.

#### Assessor material (not released)

Split and framing trace. Use bytes `00 03 11 A0 A1 00 01 22 00 02 33 B0`. The three
records have kinds 11, 22, 33 hex and payload lengths 2, 0, 1. Supply one byte per
call, then all bytes at once, then every two-part split. Verify the same record
sequence and exact accepted-prefix accounting independently of internal buffer
positions: the concatenation of reported consumed prefixes must equal a prefix of the
stream and re-offering the unconsumed suffix must reproduce the all-at-once sequence.

Pressure trace. Use stream S10, ten records `00 02 K P` for i = 1 to 10 with kind
K = 0x40 + i and one payload byte P = i (40 bytes, kinds 41 to 4A hex). A record the
consumer holds counts against the eight retained. Offer S10 and accept record 1 but
do not release it: records 2 to 8 are accepted (seven further), record 9 is not
accepted and stays unconsumed or retained as input. The consumed prefix is at least
32 bytes (records 1 to 8) and at most 40 when the unparsed bytes are retained inside
the 128-byte bound; any value in that range is acceptable if the invariants hold.
Release record 1: record 9 is accepted, record 10 after another release. The delivered
sequence is records 1 to 10 in order, once each, for every offer schedule.

Reject lengths 0 and 33, a final lone length byte and a truncated payload. Retain
a consumer borrow during shutdown. Force repeated stop and release and verify
that every accepted record has one terminal ownership result. A crash, timeout or
compile error is not equivalent to the expected malformed-frame rejection.

Worked answers for the changed condition: with eight slots and two borrowed (records 1
and 2 of S10 held), six further records are accepted (records 3 to 8); the producer's offer stops, with a consumed
prefix that ends at a frame boundary or a retained partial frame within the 128-byte
bound and nothing lost; shutdown reports two outstanding borrows and frees nothing
until they are released (it may wait with a bound or return busy, but not free).
Single-writer index arguments from the one-thread ring must be replaced by a
publication and slot-reuse pair; the frame grammar and byte bound carry over.

Seeded mutants for STR-C1: length read as payload length; limit test `> 33` and
`> 31`; reset of parser state on a zero-byte call.

### Task T-STR-2

#### Release: initial

A device writes event records into a shared data area of 128 bytes and keeps
three 32-bit unsigned values beside it. `head` counts the bytes the device has
ever written; the device stores it after the bytes. `tail` counts the bytes you
have finished with; you store it. `dropped` counts the records the device has
discarded for lack of room. All three are free-running and wrap at 2^32. A
position maps to data offset `position mod 128`.

A record may start at any byte. It begins with a 4-byte header, native
byte order: a 16-bit kind, then a 16-bit size giving the whole record in bytes.
A valid size is from 4 to 64. Kind 1 is an event whose size is exactly 12: a
32-bit timestamp and a signed 32-bit value after the header. Kind 2 is an overrun
marker whose size is exactly 8, with a 32-bit count of the records discarded since the previous
marker. A record of any other kind with a valid size is unknown: skip it and count it.
A known kind with any other size is invalid. A
record may cross the end of the data area at any byte, including inside its header.

The device never overwrites bytes between `tail` and `head`. When a record does not
fit it discards the whole record and increments `dropped`; when room returns it first
writes an overrun marker for the records discarded since the previous marker and
then the record.

Write a consumer. For a snapshot of `head`, the data and `tail`, handle at most a
given number of records, pass a copy of each event to a downstream function that
may take arbitrarily long and cannot fail, and report records consumed, events
delivered, unknown records, the overrun total seen in the data, and what the final
`dropped` value implies about total loss. A partial or invalid record stays
unconsumed with the earlier records accepted; say which of four outcomes applies
(all pending consumed, stopped at a partial record, stopped at an invalid record,
snapshot inconsistent). Then say when the device may reuse consumed bytes.

Record the requirements you derived, how consumed bytes are held while downstream
runs, two rejected alternatives with the smallest failing input, and a predicted
discriminating experiment. For a device that writes twenty 12-byte events while
your consumer is blocked in the downstream function for the first event, predict
how many events the device wrote and how many it discarded.

#### Release: changed condition

Downstream processing becomes asynchronous: your consumer hands events to workers
that release them in any order and at any time. Adapt your design. Before running,
predict `tail` after each release when three consecutive 12-byte events starting
at `tail` = 0 are released in the order second, third, first; and how many more
12-byte events the device can write when ten are pending and (a) only the first is
still held, (b) only the second is still held.

#### Assessor material (not released)

The synthetic layout is deliberately not the kernel's. If real perf is used, replace
it with the actual kernel contract (PMU_PROTOCOL.md; Linux perf ring documentation)
and keep lost-record and denied-hardware states explicit; record `perf_event_paranoid`
and whether the group opened. The assessor prepares a writer fixture for the live
variant from this specification before the sitting, labels it TEST IMPLEMENTATION and
records its SHA-256 on the form; it is never learner work.

Fixtures (all with area 128, positions 32-bit):

- F1 wrapped header: `tail` = 126, one event, `head` = 138. The header occupies
  offsets 126, 127, 0, 1 and the body offsets 2 to 9. One event, `tail` = 138.
- F2 wrapped body: `tail` = 120, `head` = 132. Header at 120 to 123, timestamp at
  124 to 127, value at 0 to 3.
- F3 partial: `head` = `tail` + 11 for a 12-byte event, and `head` = `tail` + 3. Both
  stop at a partial record with `tail` unchanged.
- F4 position wrap: `tail` = 0xFFFFFFF4, `head` = 0x00000010 (28 pending bytes): two
  events, then a header whose body is absent. Two events consumed, partial,
  `tail` = 0x0000000C.
- F5 invalid headers at the first record: size 0, size 3, size 65, and kind 1 with
  size 16: invalid, `tail` unchanged; the same header after one valid event: `tail`
  advanced by 12 only.
- F6 inconsistent snapshot: `tail` = 0, `head` = 129, and `tail` = 10, `head` = 5
  (unsigned difference 0xFFFFFFFB). Nothing consumed; the second is not empty.
- F7 unknown kind 7, size 8, between two events: skipped, counted once, both events
  delivered.
- F8 loss: the device discards 3 events (`dropped` = 3), later writes marker(3) and an
  event, then discards 2 more (`dropped` = 5, no marker). The consumer sees overrun
  total 3, `dropped` 5, 2 not yet reported, total loss 5. Adding 3 and 5 gives 8:
  wrong.

Worked answers. Initial prediction, device writes twenty 12-byte events (ten fit,
since 10 x 12 = 120 and 8 bytes remain) while the consumer is blocked on the first:

- Design A (events processed in place; `tail` advances after the downstream call
  returns): the device wrote 10 and discarded 10; `dropped` = 10.
- Design B (copy the event out and advance `tail` by 12 before the downstream call):
  free space becomes 20, one more event fits, so the device wrote 11 and discarded
  9; `dropped` = 9.

Either is acceptable; the criterion tests that the learner's prediction follows from
its own choice and that the rejected alternative is defeated by a stated trace.

Changed condition: released second, third, first gives `tail` 0, 0, 36. With ten
pending 12-byte events: (a) first held, `tail` = 0, 8 bytes free, none can be written;
(b) second held, the first is released so `tail` = 12, 20 bytes free, one event.

Seeded mutants for STR-C2 and STR-M2: header read through a pointer cast that
ignores the wrap and the alignment of an arbitrary start byte; wrap test `offset + size > 128` replaced by `offset + size >=
128`; head loaded relaxed; tail stored before the copy; head re-read per record.
A hardware observation, if attempted, is recorded separately from every claim
above.
