# Release and migration laboratory

Release1 permits requested buffer capacities0..8; release2 permits0..16. Existing
requests remain valid. A release1 descriptor requires exactly16 bytes, while
release2 accepts a larger capacity and leaves its unknown tail untouched. This
is a documented admission extension, not evidence that arbitrary layouts work.
`tests/abi.c` and the separate ctypes client execute both actual versions.

The private representation2 inserts a private field. It must retain installed
consumer behavior; changing rejected-append output instead is a semantic break
even if symbols/layout remain compatible. The deliberate mutant is rejected,
not shipped under the existing contract. Known limits: only serialized calls,
bounded buffers and the selected Linux ABI are implemented. No promise covers
hostile pointers, thread-safe lifetime, arbitrary Unicode text or network input.

The migration example is `examples/client.c`: copy embedded-zero bytes, check
each status and release the original owned handle even on failure. The optional
deferred migration uses `lc_start` plus retained callback/context, then `lc_poll`
or `lc_cancel`/`lc_ack`, followed by `lc_release_operation`. The synchronous call
may be deprecated for a deferred consumer, but keeping its spelling/signature
while silently deferring is a lifetime-breaking change, not a pure refactor.

The persisted teaching record is original synthetic data, not an external file
format. Version1 is `L1` + one length byte + that many raw bytes, maximum8.
Version2 is `L2` + one length byte + raw bytes, maximum16. Readers require exact
extent and their own magic. Upgrade copies to a new file and retains the old
artifact; downgrade is permitted only at length<=8. A length9 record yields an
explicit rollback refusal and leaves both files intact. Successful file copying
does not establish atomic deployment or power-loss durability.

Build outputs include an unstripped binary, separately extracted debug file,
stripped library with GNU debug link, build identifier, and SHA-256 manifest.
Controlled same-compiler builds normalize debug paths and compare actual bytes;
changed-compiler builds repeat consumer checks and report their actual differing
hashes. Matching bytes are an input/output observation, not a correctness proof.
