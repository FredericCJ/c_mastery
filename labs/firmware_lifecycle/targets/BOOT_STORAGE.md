# FW-STORAGE: read the bytes that will execute

This required T59 continuation binds BOOT_PAIR.md and ECC_GUARD.md to actual
slot reads. Preserve original PR-T59-03/04 objectives and delayed variants.
Prerequisites are the learner's guarded-word read, transactional record
snapshot, current-policy signature admission, paired journal and handoff
artifacts. The B06 common contract and the explicit FW-BOOT permitted crypto
provider profile apply; no library implements the storage/admission coordinator.

Implement boot_storage.h from its stub. Trusted BSLayouts contains exactly two
candidate slots per core. For each slot, the32-byte signed header is at
base-32; up to2048 payload bytes start at the512-aligned base; the64-byte
signature is at base+2048. The complete owned immutable slot and layout limit
must cover through base+2112. Read callbacks return exactly1 only for a valid
word; any failed read rejects that image. Header extent is checked before
payload reads; a final partial word is read only within admitted slot capacity,
and only its declared bytes enter the authenticated envelope. Use a fixed2080-
byte staging buffer and preserve output on every failure.

bs_select reads two guarded journal snapshots. For each structurally valid
record, match its vector for each core to exactly one trusted slot choice,
read the actual bytes through EGRead, authenticate them under current policy,
and require full digest/version/vector/generation match. Only then may a record
participate in highest-sequence selection. Unequal equal-sequence valid records
are conflict; no admissible pair returns0 and leaves output unchanged. The
output identifies the selected journal and both admitted images. Duplicate
trusted layout entries are a configuration error, never an arbitrary tie-break.

The fallback policy is explicit: an older authenticated pair may boot only if
it still meets current trusted version/generation floors and revocation policy.
An attacker corrupting the new record can cause availability fallback within
that policy. A product that requires a higher antirollback floor must persist
that authority separately; CRC/sequence alone cannot enforce it. Preserve old
slots until the paired update transaction is committed and safe to reclaim.

D1: derive address/extent arithmetic, ownership of both records and four slots,
snapshot atomicity and policy-bounded fallback. D2: implement the coordinator
using actual prior components; no injected untrusted `signature_ok` replaces
cryptographic verification. D3: fault each of168 header/payload/signature words
in each newest component and all64 record-word positions; recover the exact
surviving pair. Reject a CRC-valid digest mismatch and revoked latest key;
raising the floor while that key is revoked must yield no selection unchanged.
D4: compile with strict C17 diagnostics, run six host profiles and reject the
metadata-binding and early-output mutants plus learner stub. D5: run original
MPS2 M4/M7 O0/O2 images that first recover an actual guarded BusFault, then read
RAM-backed teaching slots through that same adapter, reject tampered stored
payload, select a valid paired record and execute the admitted reset vector.
Inspect linker reserved-slot bounds and final helper closure.

Level0 copies a trusted-looking metadata record. Level1 verifies a separate
buffer but does not bind storage or current policy. Level2 satisfies D1–D5
with exact output/failure evidence. Level3 independently designs the original
delayed revoked-key or peer-failure variant after seven days and derives which
old pair remains admissible. Hints: decide which bytes are authenticated before
reading payload; bind each journal vector to one trusted layout; select among
fully admitted pairs last. Assessor analysis is separate.

Only original synthetic slots/public teaching keys/local emulator addresses
are used. A harmless attacker success is a mutant selecting a mismatched
image pair; real flash or host trust is never modified. The emulator's slots
are RAM-backed fixture storage, not STM32 program/erase cells. Single-core CPU
execution cannot demonstrate H745 two-core start/health behavior. Board reset,
clock state, other-core start, recovery transport, exact silicon errata and
physical power-loss/retention remain separately required. Fixed stack storage,
maximum4 image verifications and bounded record scans give finite resource
costs; signatures do not prove the admitted program safe. Exact commands and
current source fingerprints are in boot_storage_validation.json; independent
review and learner mastery are not inferred.
