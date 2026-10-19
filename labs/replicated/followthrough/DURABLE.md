# T76.01: positioned events on the actual page store

This is the required implementation successor for X-T76-001, including its
delayed variant. It does not replace the accepted in-memory RPLog or its public
identifiers. Complete PR-T70-07/X-T70-007 (HAMT roots), PR-T20-01 (record codec)
and PR-T47-10 (the integrated page store), retaining their actual source,
contracts and dependency fingerprints. Read the original X-T76-001 brief and
modern-cost-contract.md. A bibliography entry alone is not source inspection.

The actual imports are `rp_log_*` and `d_hamt_*`, `store_*` from store.h, and
`ar_encode/ar_decode` version2 from the additive T20 architecture artifact.
The latter encodes one signed32 value as four big-endian octets and their XOR.
The adapter adds a version, stream UUID, schema identity, position, kind and
CRC32; it does not pretend that the old five-byte record already had these.
All imported private implementations are explicitly selected only for author
validation. A learner uses completed learner workspaces and records adaptations.

## Inputs, outputs and admitted state

Implement durable.h and the unsolved durable.c in C17 on hosted Linux. Require
8-bit bytes, uint32_t/uint64_t and 32-bit int as the retained HAMT does. No
database, serialization or hashing library may implement these mechanisms.
Host-side Python dictionaries/zlib are independent verification tools only.
The prior page store may use its allowed byte-copy library operations.

One RplLog exclusively borrows an already-open `store` at a stable address.
`rpl_create` requires that store to be empty; it does not format or erase an
existing store. Nonzero UUID64 and schema32 are caller-assigned identities,
not generated credentials. `rpl_open` expects exact identities and reconstructs
the sixteen-event HAMT from the durable authority. The store and callback
contexts outlive the log; no outside code mutates that store while the log is
live. Controls, inputs and outputs are disjoint live objects. Calls are externally
serialized. No callback reenters either log or store, mutates controls, or retains
a borrowed byte buffer. Public structs must be constructor/operation-admitted.

An event has position1..16, key0..31, SET or QUERY_RESULT kind and int32 value.
Both kinds assign the recorded value to the key; QUERY_RESULT records an answer
already obtained before admission. Recovery never repeats that query. A new
external answer is a new event, not an excuse to reinterpret an old event.
Event history is immutable. The seventeenth append returns RPL_FULL. Prefix0
is empty; a missing key returns RPL_STATE without changing the output value.
Invalid position/key returns RPL_INVALID. An admitted log must have exactly
count event keys, with no hole, duplicate, extra future event or unknown key.

The public memory view uses the actual retained RPLog and its64-node arena:
each append consumes three nodes and sixteen consume48. Every root belongs to
this live arena. No escaped root/control is copied or relocated. Open stages an
unpublished index-only arena, with no borrowed roots, then constructs the output
by value. That construction does not authorize relocating a published arena.

## Exact persistent format

The database keys are one-byte `M` (log metadata), two-byte `E,position`, optional
one-byte `S` (snapshot metadata), and eight two-byte `C,chunk` keys where chunk
is0..7. Every value is64 octets, within STORE_VALUE_MAX:

| Octets | Meaning |
| --- | --- |
|0..3|ASCII RPL1|
|4|format version1|
|5|type1 event,2 log metadata,3 snapshot metadata,4 snapshot chunk|
|6..7|zero, reserved|
|8..15|UUID64, big-endian|
|16..19|schema32, big-endian|
|20..23|event count or snapshot/event position, big-endian|
|24..59|type-specific payload; unused octets zero|
|60..63|CRC32 of octets0..59, reflected polynomial0xedb88320, initial/final complement, big-endian stored result|

An event payload stores key at24, kind1/2 at25, codec version2 at26, and the
actual five-byte `ar_encode(2,...)` output at27..31. Metadata payload is zero.
A snapshot chunk stores chunk index at24, four-bit presence mask at25, zero
at26..27, and four actual five-byte records at28..47. An absent value is
canonically zero. Snapshot metadata and every chunk have the same position.
All eight chunks are required. Record validation rejects reserved bits,
versions, identity mismatches and malformed inner codecs before publication.
CRC32/XOR are corruption checks, not authentication or proof of uniqueness.

## Commit, snapshot and failure contract

Append prepares the next actual HAMT path, then begins a page-store transaction,
writes the event and new metadata, and commits through the real WAL. The
nonreentrant serialized contract prevents observation of staged state. On
success, event/count/root/kind describe the same prefix. On any store failure,
the published memory count and arena-used boundary remain at the prior prefix;
unreachable bytes and diagnostic counters need not be byte-identical. Injected
HAMT allocation refusal returns RPL_FULL before any page transaction.

Store errors distinguish ordinary I/O and uncertain final commit. An uncertain
append may already be durable despite unchanged visible memory. Poisoned store
or log controls reject further operations until the owner reopens the store and
constructs the log again. The callback contract makes accepted partial writes
and successful flushes meaningful; no rollback of persisted bytes is promised.
`rpl_open` preserves its output on rejection, but imported read statistics may
change and the earlier `store_open` recovery may itself write/flush pages.

`rpl_snapshot_save(position)` writes all eight chunks and snapshot metadata in
one actual store transaction. An old-position snapshot uses that position's
root, not the current root. Load checks exact schema/position/chunk coverage and
compares the snapshot against retained event authority before publishing it.
`rpl_state(...,1,...)` starts with that snapshot and applies only its suffix;
`rpl_state(...,0,...)` rebuilds the whole prefix. Both yield the same dictionary.
The snapshot accelerates reconstruction only after validation in this bounded
design; it does not replace immutable event authority or prune old events.

## Byte-to-page adapter

Implement RplPages over RplBytes callbacks. Each read/write callback accepts a
positive prefix no longer than requested, -2 means retryable/no progress, and
other nonpositive values terminate. The call limit counts every invocation,
including retries, separately for each page. Reads stage a whole4096-byte page
before publishing output. Writes resume at exactly the accepted offset; replaying
an accepted prefix is a defect. Failed writes may partially persist. Successful
flush makes every prior accepted write durable. The byte extent must cover all67
image pages; no callback is asked to address beyond it. The adapter does not own
or close the byte device. Formatting is separately authorized only for the
original disposable fixture.

## Supplied experiments and independent results

Run the three CTests durable_log, durable_snapshot and durable_bytes. They
exercise failed publication, all sixteen appends, signed extrema, exact schema,
extra valid future records, prefix snapshots, short writes, bounded retries,
partial-read output preservation and zero progress. Six independently compiled
controls omit rollback, use the latest snapshot root, omit CRC checking, omit
schema checking, replay short-write prefixes or admit a future event. A control
passes only by reaching a normal RP_CHECK semantic rejection; sanitizer/runtime
failures do not count.

Run `tools/verify_replicated_durable.py --driver PATH/replicated-durable`.
Its dictionary/prefix model and Python byte construction are independent of
the C algorithm. Its709 wire rows,3976 prefix queries,192 whole-state results,
64 reopens,617 append admissions and64 snapshots include changed identities,
signed extrema and malformed arguments. Fixture resets are additional rows.
The actual resulting count is reported by the tool, not inferred here as a
mastery claim.

`durable_faults` cuts every one of139 short-write/flush callbacks for append and
recovery under three admitted persistence outcomes: discard pending bytes,
persist all accepted pending bytes, or tear the addressed chunk halfway. There
are417 append and417 recovery executions. For append, a complete persisted
commit after the WAL flush can recover the new prefix even when the call failed;
otherwise the old prefix survives. Recovery interruption must retain the already
committed new prefix after a later successful recovery.

`durable_tears` additionally tests every byte prefix0..4096 at every one of34
whole-page write boundaries, plus every ascending dirty-page prefix0..67 at
each of three flush boundaries, for both append and recovery. Each phase has
139502 cases. In append the new event can survive a failed call only at complete
commit-page persistence or a final flush prefix that includes that page. The
recovery phase must always recover the committed new event. These are exact
enumerations of the stated prefix model, not all possible unordered sector
subsets, checksum collisions, dishonest flushes or physical media behavior.

## Required reasoning, hints and assessment

D1 supplies a byte table, abstraction function, ownership graph, event/snapshot
authority distinction and a replay counterexample for a changed external query.
D2 implements all operations and explicit producer adapters. D3 records the
independent dictionary/byte comparisons, all cut classifications, mutant
rejections and source/dependency identity. D4 repeats the investigation with a
snapshot behind the current event position and an effect receiver that can stop
after applying but before acknowledging; the latter composes the separate
durable inbox/outbox investigation, not a claim that this log alone deduplicates
an external effect. Apply the original four objective-specific rubric rows.
Passing nominal CTests alone earns at most level1; level2 requires the complete
reasoning/failure evidence, and level3 requires an unaided changed-condition
attempt and a correct counterexample to the old assumptions.

Graduated hints: first draw which root corresponds to every count; next distinguish
the WAL's durable commit from the caller seeing success; finally ask whether a
snapshot checksum proves it belongs to the requested event position. Assessor
implementation and technical analysis remain outside the learner package.

The source interpretation is bounded by the inspected Event Sourcing passages
on snapshots, external updates/queries and code changes. They motivate these
questions, but do not specify this WAL protocol. The actual store implementation
and its own contracts supply that protocol. No draft is relabeled as published
ISO C17/C23 wording. Physical failures, non-author review and independent delayed
learner performance remain separately unobserved.

Security scope: only this original teaching program, disposable memory/file
fixtures and synthetic bytes/identities. The attacker may corrupt/truncate
records or replay a valid old/future record within the fixture. Success means a
wrong recovered prefix, wrong snapshot or duplicate synthetic effect, never a
claim of arbitrary execution. Admission/identity/checksum repairs and exact
regressions constrain these defects; checksums cannot authorize hostile writers.
