# Consistent-hash ring and membership views

This required T76 refinement implements placement, distinct replica selection,
and a bounded migration plan. It reuses the actual T76 `RphSha` component for
byte-key hashing. It does not transfer records, authenticate membership, run
consensus, elect a leader, implement Chord finger routing or promise Dynamo
compatibility. Existing anti-entropy, outbox and storage exercises retain those
separate contracts. A placement plan is not evidence that data moved safely.

## Sources and algorithm choice

Karger et al. (1997), sections4.1–4.3, distinguish monotonicity from probabilistic
balance, spread and load. Their construction uses nearest points on a unit
interval. Our clockwise successor ring follows the later Dynamo2007 sections4.2
and4.3; these constructions must not be conflated. Dynamo's replication passage
specifically skips virtual positions with a repeated physical owner. This small
finite ring adopts an explicit equality/tie convention and claims no theorem
about uniformly random independent hash families for adversarial chosen tokens.

## Representation and ownership

The space is all uint16 positions0..65535. A token is (position,owner,ordinal),
with physical owner1..16 and uint8 ordinal. There are at most32 tokens and a
nonzero uint32 membership epoch. Token identity is (owner,ordinal); identities
must be unique even when positions differ. Equal positions are permitted and
ordered by owner then ordinal. An empty admitted ring is meaningful but cannot
answer placement queries. Zero and65535 are ordinary positions, not sentinels.

`cr_build` validates an arbitrary input token list and copies it into a canonical
sorted value. Read operations require a live admitted ring that the caller has
not modified. They check extent and epoch, but do not revalidate the whole token
ordering on each lookup. This keeps lookup logarithmic; treating forged public
fields as an admitted ring violates the precondition. Struct privacy is not
enforced by ISO C. All pointer spans must be live, aligned, sufficient and
disjoint; `cr_change` alone updates its supplied owner object. No function
allocates heap memory or retains caller arrays. Calls and hash callbacks must
not concurrently mutate a live ring or output. Callback context is borrowed
for the call, never stored. A valid function pointer does not establish its
semantic contract or prevent reentrant mutation.

## Operations and exact behavior

`cr_owner` returns the first token at or after the key's position, wrapping to
the first token if none qualifies. For a position shared by several tokens it
chooses the lowest owner/ordinal. Binary-search count and number of visited
tokens are published in `CrCost`. Expected epoch mismatch yields STALE before
lookup. EMPTY, STALE and other failures leave output and cost unchanged.

`cr_change` adds one previously absent physical owner with1..32 explicitly
provided tokens, or removes an existing owner when count is0. An addition must
contain only that owner, valid distinct ordinals and fit the32-token capacity.
Tokens for existing owners never change. Epoch advances exactly once on success;
UINT32_MAX rejects rather than wrapping. Removal of the last owner produces an
empty ring. To change positions of an existing owner, explicitly remove then
add under separate epochs and analyze the intermediate state. A cut value in
0..final_token_count requests an abort at a prepublication checkpoint; SIZE_MAX
disables injection. Complete old state remains visible on every rejection or
cut. These checkpoints model interruption before publication, not power-loss
atomicity of a physical struct copy. A successful update is only logically
atomic under the single-threaded/external-serialization precondition.

`cr_replicas` starts at the same successor and scans at most one complete ring.
It skips already-selected physical owners and owners marked by the16-bit
unavailable mask (bit owner-1). It returns exactly wanted distinct owners1..16,
or UNAVAILABLE without partial output. This bounded skip policy is a stated
degraded placement view, not evidence of durability or quorum intersection.
A replica list containing three virtual positions of one machine still has
only one physical failure domain. Physical owners need not be distinct racks
or sites, another assumption the representation does not encode.

`cr_route` composes a caller-supplied32-byte digest function with placement. The
first two digest bytes in big-endian order give the16-bit key. A callback
failure yields HASH_FAILED with unchanged placement outputs. The supplied
`sha_adapter.c` calls the actual `rph_hash` interface from
`labs/replicated/hashing/hashing.h`; author tests explicitly compile
`assessor/replicated/hashing/hashing.c`. The16-bit truncation intentionally
destroys cryptographic collision resistance. The exercise proves placement
semantics, not secure identifiers or an authenticated membership digest.

`cr_plan` composes two admitted adjacent epochs and up to32 hash positions. It
returns only changed (key,old_owner,new_owner) rows, preserving input order and
duplicate keys. If output capacity is insufficient, neither rows nor count
change. Both rings must be nonempty for nonempty key input; a last-owner
removal cannot invent a destination. Epoch adjacency is necessary bookkeeping,
not proof the two views represent one authorized change. The caller must bind
view identity and migration authorization before using a plan operationally.

## Derivation and changed assumptions

For nonempty views the successor minimizes clockwise modular distance, then
the stable tie tuple. Inserting only tokens for a new owner can change a key
only to that owner: old candidate order does not change. Removing an owner can
change a key only when its old owner was removed. Neither argument assumes a
balanced token distribution. Re-seeding old tokens during an addition breaks
the premise. Equal-position ownership can give some owners zero keys; virtual
nodes improve some sampled distributions but cannot prove balance against
chosen colliding positions. Measure all65536 keys, not just a few examples.

Construction is O(V squared) insertion sort with O(V) staging. Owner lookup is
O(log V), distinct replicas O(log V+V), membership O(V squared), and a bounded
plan O(K log V) with O(K) staging. State costs depend on target padding; measure
sizeof rather than assuming the serialized layout is the C struct layout.
Compare evenly spaced, tightly clustered, identical and fixed-seed positions
at4/8/16/32 tokens. Report exact load min/max and lookup comparisons separately
from host CPU ticks. A uniform hash over keys does not repair a terrible token
layout, while evenly spaced tokens do not cure a workload concentrated on one
key. Measure these mechanisms under the same placement and failure contracts.

Delayed transfer adds unavailable owners and migration-plan capacity after
membership changes, reusing the actual learner's ring and hash adapter. Explain
why two clients with different epochs can route to different machines, why
simply discarding an old copy before acknowledged handoff loses availability,
and why a matching Merkle root requires the same key-range/view interpretation.
This exercise creates the plan; existing T76 anti-entropy/storage work assesses
the actual repair and persistence mechanisms.

## Environment, enforcement and security

C17 with8-bit bytes and exact-width integers is the active baseline. C23 does
not add automatic ring validity, ownership or distributed membership checks.
The target core is profile C: inspect Cortex-M4 objects and a linked image,
allowing only explicitly linked original memory helpers. Host Python oracles,
stdio, clocks and sanitizers are verification dependencies, not target code.
The separately compiled real SHA adapter is author-side integration evidence;
it does not substitute for future learner-artifact integration or target run.

Only this original local program is a security target. An adversary controls
synthetic token positions, labels, unavailable masks and key bytes. Demonstrate
load capture, duplicate-physical-replica acceptance, stale-epoch use or
prepublication mutation in a compiled defective specimen. Harmless success is
an observable violated placement/publication property; no external network,
credentials, deployed cache or host security change is involved. Deterministic
tie rules, admission and staging repair their stated bugs; they do not supply
Sybil resistance, Byzantine agreement, failure detection or key secrecy.
