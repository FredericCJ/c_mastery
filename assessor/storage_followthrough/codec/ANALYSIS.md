# PR-T47-01 separate technical analysis

For criterion .a, derive the CST2 layout from FORMAT.md: the leaf kind is 11, page
ID is7, full generation is0x0102030405060708, keys occupy fixed16-byte slots and
values fixed64-byte slots. The empty leaf has no next pointer; the maximum leaf
has7records and next31. The Python oracle deliberately constructs these bytes
without calling either C codec and uses zlib's independent CRC implementation.
Do not compare native structure padding. The format3 marker differs from kind11;
simply accepting version2 in the new decoder is a semantic mutation.

For .a's changed version, the actual old decoder feeds the new encoder, and the
reverse path feeds the actual old encoder. Staging makes overlap safe. The
conversion is leaf-only: internal nodes reject, and no current CST2 engine can
open CST3L pages. A whole-image upgrade would require a separately specified
atomic authority/rollback protocol. That is the learner's scope distinction,
not a claim that the component silently supplies it.

For .b, CRC is independent of count, identity and bounds. A malformed page can
carry its correct CRC. The retained decoder stages a node and validates internal
children before any caller follows them. For the changed format, admit offset
before subtracting from4092; validate lengths, then pairwise intersection of
positive-length spans. A zero-length value at4092 is legal and does not intersect
a key ending there. Equality between two keys rejects after full decoding but
before output. The early-publication control exposes this ordering precisely.
The overlap control uses one key and value both at128, with all vacated bytes
zero; otherwise a separate hole check would mask the missing-disjointness defect.

For .c, an opened established image is distinct from a disposable creator input.
The actual retained formatter has35writes and3flushes. The fresh factory transfers
one candidate; every failed stage consumes it through discard before the next
fresh identity is acquired. Successful verification publishes once. A failed read
may leave an unpublished temporary store handle partly initialized, which does
not grant authority to erase its file. The test keeps an established nonempty
image and compares every byte and write count after failed reopen. A corrupted
older checkpoint beside a valid newest checkpoint allows a real reopen; corrupt
both checkpoint records and the result must be corruption. Exact guarantees are
conditional on the supplied synchronous IO callback contract, not actual device
power behavior.

The oracle's1024extent cases are one-entry boundary combinations; multi-entry
ordering and overlap are covered by separate fixtures. Five controls and six
compiling-stub failures demonstrate sensitivity to selected mistakes only. Review
the original IDs separately; no family pass proves every property or independent
mastery. Delayed learner administration, external review and physical validation
remain distinct dimensions.
