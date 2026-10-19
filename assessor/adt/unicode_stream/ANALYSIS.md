# Assessor analysis: copied prefixes, transactional chunks and text ownership

OPEN states represent a concatenation of completed scalars followed by either
no prefix or one valid incomplete scalar prefix of length1..3. bytes includes
that prefix; scalars does not. The section4 restrictions on second bytes make
the prefix extendible. Completing a sequence checks actual ad_text and increments
one scalar. Each byte updates only a local stream copy until the entire chunk
succeeds. Thus a failed suffix cannot consume the preceding ASCII or overwrite
old pending bytes. Size admission precedes reading and bounds both counters.

Finish is possible exactly when pending is empty. It need not inspect or retain
the prior chunks. Its idempotence does not authorize further feed. Cancellation
discards the entire logical attempt, including counts and every pending byte;
the next attempt needs init. A full-capacity incomplete prefix has no completion
space but still has a legal cancel transition. No implicit error latch exists.

Do not compare incremental error timing directly with CPython without specifying
the policy. The first independent pilot differed for ED A0: Python can keep the
inevitably invalid prefix until a later byte, while this contract refuses the
chunk immediately. The corrected independent oracle uses all encoded scalar
prefixes to test possible completion, while Python remains responsible for its
separate complete/stream decoding logic. All1112064 scalar encodings contribute
17652 distinct proper prefixes including empty. This does not establish every
possible stream history or normalization correctness.

Owner replacement first validates complete text, then invokes the actual owning
sum's allocate-copy-before-drop operation. A failed clone never changes old
scalar metadata. A successful clone receives independent storage and only then
invalidates old borrows. Empty TEXT owns a drop callback even with no allocation.
The test's emergency allocation cleanup runs only at process exit after an
intended mutant assertion; it does not change semantic live/drop counters or
make a leaking implementation pass. Refusing before clone is observably different
from allocating a malformed string and then rejecting it.

The C buffer-alias example is defined memory access: it changes bytes while the
array remains live and shows cached semantic validation no longer holds. The
owner's copy remains valid under mutation of the original. Safe Rust disallows
mutation while a later use of the str borrow remains; a last-use or independent
String makes mutation legal. C's const member and nominal wrapper provide no
equivalent temporal rule. Conversely, Rust's positive examples do not establish
the arbitrary C clone backend's failure guarantees.

Sealed delayed scenario: a receive buffer is overwritten after every successful
feed. Feed F0 then90 then80, cancel, overwrite it, initialize and feed E2/82/AC.
Require0 counters after cancellation and one scalar after restart. Change the
owner source to a subview of its old valid payload, force the next clone to fail
after temporary allocation, and verify the old view remains valid and exactly
one owner survives. Finally remove failure and replace, retiring all prior views
without reading/comparing their pointers afterward. Ask the learner to explain
the distinction between input chunks, retained copied bytes and owner borrows
before seeing this solution.

Encoding validity alone does not imply one grapheme per scalar, normalized
equality, safe identifiers or authorization. Composed e-acute and decomposed
e+combining-accent both pass with different scalar counts. Security controls only
witness the original teaching state/property violation, not external exploitation.
