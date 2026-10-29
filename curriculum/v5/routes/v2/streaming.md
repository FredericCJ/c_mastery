# Pilot A: bounded input to sampled-record consumption

In `contextual-v2` this path is a sequence of authored episodes:
EP-sequence-representations (U-T14-01-01, U-T14-01-02), EP-byte-ring
(U-T14-01-03, U-T14-01-04), EP-record-ports (U-T20-01-01..04), EP-callbacks
(U-T19-01-01..04), EP-wire-framing (U-T15-08-01..04), EP-framed-streams
(U-T24-03-01..04), EP-nested-events (X-T15-019), EP-publication-order
(U-T25-03-01..04), EP-spsc-ring (U-T55-01-01..03), EP-pmu-counting
(U-T43-09-01, U-T43-09-02) and EP-sampled-records (U-T43-09-03). Other
episodes are interleaved between them where their prerequisites or motivation
belong; the card for each unit names its episode, step and transition.

The recurring consumer is the telemetry service, with firmware input as a later
adapter. The input producer may stop at any byte; the consumer may refuse progress.
The contract must preserve accepted input, report pressure and avoid publishing
partial records. The stages below add different obligations.

1. Use the bounded arrays and views from the foundation and PR-T14-01
   (U-T14-01-01 seals the full/empty decision; U-T14-01-04 exports the ring). Implement
   the caller-owned byte ring and prove full/empty, capacity and split-span rules.
   Test every head position, exact capacity and one beyond capacity with an
   independent sequence model. No concurrency is implied.
2. Compose the ring with PR-T15-08 wire records (U-T15-08-01..04) and PR-T24-03
   framed streams (U-T24-03-01..04).
   `labs/hosted_systems/stream_depth/CONTRACTS.md` defines copied complete frames,
   retained message ownership, segmented headers and a finite payload budget.
   Try every split point, a full destination and malformed/truncated input. A
   retry may consume no bytes and must not duplicate an already committed record.
3. Use the nested-event parser X-T15-019 when the consumer needs structural events
   rather than a complete flat frame. State which spans are borrowed during a
   callback, whether callbacks can reenter and how backpressure suspends parsing.
   Consumer rollback is a distinct obligation from parser recognition.
4. At T25/T55 (U-T25-03-01..04, then U-T55-01-01..03), introduce SPSC
   publication. Give each index one writer; separate
   making bytes visible from publishing the index that permits their reading.
   Test bounded histories and shutdown while stating what the model excludes.
   Changing to multiple producers invalidates the single-writer argument.
5. Enter the Linux PMU strand (U-T43-09-01 protocol, U-T43-09-02 hardware groups,
   U-T43-09-03 sampled records) using
   `labs/performance_followthrough/PMU_PROTOCOL.md`. The perf mmap metadata and
   record layout are a kernel ABI. Use its required data-head/data-tail ordering,
   validate headers and lengths, handle wrap and account for lost records. The
   kernel ring is not a replacement implementation of the earlier ring API.

The supplied PMU scaffold is `labs/performance_followthrough/pmu.c`; its header
and Linux adapter document the public boundary. Run public feedback with
`python3 labs/performance_followthrough/build.py --protocol --output /tmp/pmu-learner`.
The default learner implementation remains incomplete. Assessor source is a
separate explicit author input and is never exported as a learner component.

Before native sampling, exercise zero/short/oversized headers, a wrapped header,
a wrapped body, unknown record types, lost-data records, stale snapshots and
controlled shutdown with deterministic fixtures. Bounds checks must precede
accesses and a rejected record must not advance publication beyond validated
bytes. Record parser, protocol and hardware results separately.

For transfer, supply an unfamiliar event source with a documented framing ABI,
finite retention and intermittent downstream stalls without naming a data
structure. Require the learner to choose storage and a publication policy,
reject an alternative, and adapt when cancellation leaves an outstanding borrower.
Do not assess this by asking for a memorized ring function.

The route records this transfer's assessment gap explicitly: the independent-
transfer encounter of the rings mechanism is U-T43-09-03, but no existing
criterion scores `pf_ring_drain` itself (AX-PR-T43-09-T43.10.c scores sampling
attribution). Until one exists, only the withheld streaming packet assesses it.

The assessor packet must withhold the transfer trace and score: defined C access,
state/ownership invariants, preserved progress, failure cleanup, ABI ordering,
and independent diagnosis. Author tests cannot establish independent learner
performance or delayed retention; those remain separate observations.
