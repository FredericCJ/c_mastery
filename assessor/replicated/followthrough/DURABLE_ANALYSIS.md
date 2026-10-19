# Assessor analysis: durable positioned events

The key proof is relational: metadata count, event keys1..count and RPLog roots
must describe one immutable prefix. Neither a valid event checksum nor a valid
tree page alone establishes that relation. The extra-E3 control leaves a valid
record beyond count2; accepting it silently weakens the authority contract.

The actual page store writes a cleared commit and flushes, writes all32 redo
pages and flushes, then writes the commit page and flushes. Only the last two
steps can expose a new durable generation. The error return cannot decide
whether that generation became durable. Restoring the old visible memory prefix
and poisoning uncertain controls is consistent with later recovery finding the
new prefix; pretending that restoration undid the I/O is not.

During append the actual RPLog can be prepared before the store commit because
the adapter is serialized and callbacks cannot reenter it. On failure restore
count and arena.used. No new root escapes; bytes beyond used are unreachable.
The allocation diagnostic is not a semantic rollback obligation. A concurrent
adapter would need a separate staged publication design: the present internal
transient count cannot be read concurrently merely because scalar writes are
ordinary C operations.

The snapshot is a second projection. Saving prefix1 after assignments7 then9
must store7; using the latest root produces a checksum-valid snapshot of the
wrong state. Exact position/schema and complete eight-chunk coverage precede
publication, and replaying the suffix yields9. Open builds unpublished index
storage and publishes once; this does not legalize copying a live arena with
escaped roots. It also does not treat heap indexes as persistent pointers.

The inner value codec is the actual T20 signed32 codec, including its arithmetic
decode. The new envelope specifies byte order, version, identity, position,
type-specific zero fields and CRC32. Neither inner XOR nor CRC32 authenticates
an attacker. A writer that can construct fully valid replacement records can
change this unsigned history; an authentication/authorization protocol would
be a separate mechanism.

For whole-page callback prefixes, append events1,3..34,36 are writes and2,35,37
are flushes. Only all4096 bytes at36, or a nonempty dirty-page prefix at37,
can leave the new commit valid with a durable redo batch. Recovery writes home
pages1..32, flushes33, writes super34, flushes35, clears commit36 and flushes37.
Before home/super durability the redo commit remains; after it, clearing that
commit is safe. Every interrupted recovery can therefore be retried. This
argument depends on successful flush persistence and the checksum corruption
model; no experiment has established a physical power-loss theorem.

Cost: an append allocates three of64 fixed HAMT nodes, while actual store_commit
writes all32 redo pages even if only two logical records changed. That is a
substantial write-amplification cost, not an O(1)-byte append log. Snapshotting
writes nine logical records in one such transaction. Open/validation scans the
bounded keyspace, replays at most16 events and validates32 snapshot values.
Compiler frame/map and host operation/timing evidence must still be attached
before claiming the full thirteen-dimension cost investigation complete.

The supplied replay has no external query callback: an admitted QUERY_RESULT
event records the answer. Re-querying99 when the record contains9 changes the
meaning. Similarly, naively repeating a synthetic effect on every replay
duplicates it. Disabling effects during local rebuild is useful but does not
solve process failure between a remote receiver's effect and its acknowledgment;
the durable inbox/outbox successor must demonstrate that boundary separately.

All current tests are author support. Do not assign independent-review or learner
mastery credit from this analysis, from a sanitizer pass, or from the number of
enumerated persistence prefixes.
