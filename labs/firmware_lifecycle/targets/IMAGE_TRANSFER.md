# T59 image completion and exhausted-counter recovery

Required continuations for PR-T59-01 and PR-T59-06, plus their composition with
PR-T59-02/03/05. Families A/B/C/E/F/G/I/J and the B06 shared contract apply.
Prerequisites are the learner one-to-zero flash model, journal commit marker,
immutable authenticated-image admission and finite-generation state machines.
Use C17, fixed storage and permitted byte operations; no allocator or flash/update
library implements this mechanism. Public `image_transfer.h`, stub and tests are
separate from assessor implementation/analysis. Tests run hosted; strict target
dependencies and physical controller behavior remain separate obligations.

## FW5: commit metadata only after image program completion

An image is exactly32 immutable bytes in an erased32-byte destination, programmed
as four- or eight-byte units. `struct fi_image` stores bytes, selected unit and one
completion bit per unit. `fi_init` admits only4/8 and otherwise preserves state.
`fi_program` takes an indexed unit, bit-prefix0..unit×8 and trusted controller
completion status0/1. Reject invalid index/status/extent or any requested zero-to-
one transition without mutation. Beginning any admitted program attempt clears that unit's prior completion
acknowledgment, including reprogramming identical bits. Apply only the selected
bit prefix; set the
unit-completed bit only for full attempted extent and successful controller
completion. A failure cut may leave all desired bits visible without the
controller having reported completion: visible equality alone cannot authorize
the next metadata operation. Reprogramming identical bits is allowed in this
digital model; a physical controller may impose additional restrictions.

`fi_commit` writes a24-byte journal record through the earlier `lce_record` only
when every unit is completed and the entire destination matches the intended
image. Sequence is nonzero; record value is reflected CRC32 over all32 bytes,
using the earlier polynomial/initial/final convention. Reject unchanged output
otherwise. `fi_record_matches` validates both metadata and the image CRC, not just
the marker. It detects the tested accidental changes; CRC is not authenticity.
The actual hosted composition in signature_adapter_test.c applies the real
signature/trusted-key policy from SIGNATURE.md to
the immutable recovered image before boot admission, and derives recovery epoch
from a signed payload byte and floor from the verified version. Run
`python3 tools/run_b06_signature.py` for this actual Ed25519 composition; it still
uses the explicitly modeled trusted atomic policy store. Do not infer that adding CRC
replaces that separate cryptographic operation.

D1: implement these operations and the complete-image-before-commit invariant;
enumerate every unit and every next-unit bit-prefix for both unit sizes, holding
the old committed image/record intact. D2: test withheld completion acknowledgment,
image corruption after metadata formation, invalid zero-to-one attempts and
unchanged rejection. Reuse the earlier294 metadata cuts for the subsequent
record-program phase. The composition is image completion, then record body,
then marker; neither an early marker nor a completed metadata CRC certifies
unfinished image programming. Complexity is O(32) fixed work and storage.

## FW6: a new trusted epoch after finite counter exhaustion

`struct fi_rotation` contains epoch, sequence, nonzero limit, minimum image version,
active key1/2 and a persistent revoked-key mask. Valid initial policy in this
exercise is epoch1,sequence0,limit3,floor7,key1,revoked0. `fi_advance` increments
only below the limit; exhaustion rejects unchanged. Recovery is an explicit
trusted operation, not sequence wrap. It requires exhausted sequence, next epoch
exactly current+1 without epoch wrap, another unrevoked key1/2, a nondecreasing
version floor, and a trusted verifier's exact success1 and matching verified-key
identity. These verifier outputs must come from the signature adapter, never
from bytes an attacker labels 'verified'.

Construct the candidate with sequence0, new epoch/key/floor and the old key
revoked. Call the existing persistence boundary with the immutable candidate;
only exact durable-success1 may publish the in-memory policy. A failed callback
leaves the current policy unchanged and retains the old committed policy. The
callback is non-reentrant and may not retain a stack pointer; its separate contract
must implement atomic durable policy publication and recovery. The supplied
failure model denies each of six staging boundaries and permits only a final
commit. It tests caller ownership/publication ordering, not flash-controller
durability. Compare it with the earlier two-record/marker mechanism when
implementing a target policy store.

D3: execute exhaustion, failed recovery persistence, successful epoch2/key2/floor8
publication, reboot from the callback's committed policy, and subsequent attempts
to use revoked key1 or wrap epoch. Include the security counterexample: resetting
sequence alone permits replay under the same epoch; keeping a smaller version
floor permits rollback despite a new key. With only two keys this demonstration
eventually refuses further rotation; extending the trust set requires a newly
authenticated provisioning policy, not recycling a revoked key.

## Verification and assessment

Run `python3 tools/run_b06_image_transfer.py`; require GCC debug/release and Clang
ASan/UBSan reference passes,524 image program-prefix cuts, all policy boundaries,
and semantic rejection of the completion-status, missing-revocation and
publish-before-persistence mutants plus compiled learner stubs. D4 retains exact
source/command evidence and the trust boundary between this model, real signature
verification and actual flash metadata recovery. Assess each a/b/c objective:
FW5 uses D1/D2 for T59.01 and T59.05; FW6 uses D3/D4 for T59.06 and T59.03.
Level0 commits unfinished data or wraps/reuses revoked identity; level1 normal
success only; level2 complete invariants, failures and controls; level3 independent
changed-condition implementation after seven days. Required variants add a
nonuniform erase region, a power cut during policy-store commit, and a third key
authenticated by a separate provisioning authority. Hints: distinguish visible
bits from controller completion; keep old commit ownership; order epoch/sequence
lexicographically and keep the image-version floor independent.

Only synthetic images and original state machines are in scope. No flashing or
deployed bootloader interaction is authorized. Source inspection is limited to
the registered MCUBOOT passages and the explicit original model above; no device
manual semantics are invented. `image_transfer_validation.json` records author
execution, not independent learner, reviewer or physical acceptance. Assessor
analysis is `assessor/firmware_lifecycle/targets/IMAGE_TRANSFER_ANALYSIS.md`.
