# CST2 page and recovery contract

Every page is exactly 4096 octets. Integer fields are unsigned little endian;
encoding/decoding uses byte shifts, never a packed C structure. CRC-32/IEEE uses
reflected polynomial `0xedb88320`, initial all-ones and final complement, over
octets 0 through 4091. The last four bytes hold the checksum. Tests treat checksum
collisions as outside the admitted corruption model.

The established image contains 67 pages: checkpoint records 0–1; data pages 2–33;
redo images 34–65; commit record 66. Logical data page 0 is the tree metadata;
logical pages 1–31 hold nodes or free-page encodings. The allocation bitmap owns
logical page 0 and every reachable node exactly once. Format initializes a fresh
image; an interrupted format is not a transaction and must be retried on a fresh
disposable image. Opening never silently formats a corrupt image.

Common fields are magic `0x43535432` at offset 0, kind at 4, 64-bit generation at 8,
and a kind-specific word at 16. Kind 20 is checkpoint metadata, with data-page count
32 at 16. A valid record with the greatest generation wins. Generation wrap is
rejected. Kind 30 is a commit record: count 32 at 16, and the 32 image checksums at
offset 32. Its generation must be exactly one beyond the accepted checkpoint.

Data kind 10 is logical page 0: page ID at 16, root ID at 20, allocation bitmap at
24. Kind 11 is an allocated node; kind 12 is a free node page. All data-page
generations match the image generation. A node stores page ID at 16, leaf flag at
20, key count at 21, next leaf at 24, eight key lengths at 32, eight value lengths
at 40, eight 16-byte key slots at 64, eight 64-byte value slots at 192 and nine
32-bit child IDs at 704. Only seven entries are persisted; the eighth is in-memory
split scratch. Only live key/value extents and the admitted internal child
interval are encoded; unused slots, padding within slots and leaf child fields
are zero. Bounds and ordering are
checked before following disk references.

Tree invariants: all leaves have equal depth; non-root leaves hold 4–7 records;
non-root internal nodes hold 3–7 separators and one more child; the root is a leaf
or has at least one separator. Separator i is the minimum key in child i+1.
Equality descends right. Leaves form one ordered, acyclic chain matching tree
traversal. Every allocated node is reachable, and each reachable node is allocated.

For a transaction, checkpoint the preceding committed batch first. Keep modified
nodes in the caller-owned workspace. Commit performs these ordered stages:

1. Write an invalid commit page and flush it before reusing any redo space.
2. Encode all 32 after-images into redo pages and flush them.
3. Write the checksummed commit page and flush it. Only then acknowledge success.
4. Reads on the live handle use redo pages until checkpoint finishes.

Checkpoint validates the whole redo batch before the first home-page write,
copies all images to home pages and flushes, writes and flushes the alternate
generation record, then invalidates and flushes the commit page. It never reclaims
the only recovery evidence before all home pages and the new checkpoint record
are durable. A checksum-invalid attempted new marker before home writes can be
ignored. Valid newer metadata means home write-back has completed. Older valid
metadata plus a newer valid commit means recovery must replay the complete batch.

Recovery validates every referenced image and its generation/checksum before
write-back. Replaying a complete page is idempotent: replaying it twice has the same
result as once. A crash during replay leaves the intact commit evidence for another
replay. Errors after publication poison the handle. Unavailable/corrupt/read-error
states must not be reported as an empty successful database.

The proof is conditional on successful flush persisting all preceding writes and
on addressed-page-only torn/reordered unsynchronized writes. If no new valid commit
survives, home pages still describe the old checkpoint. If a commit survives, all
its images preceded its publication barrier and recover the complete new state.
If the new checkpoint survives, its home images preceded its barrier. Log reuse
occurs only after that checkpoint. The finite harness tests selected cuts of this
ordering argument; it does not prove arbitrary devices satisfy the assumptions.
