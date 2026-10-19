# FW-S1: verified signature verdict and key-policy composition

This required PR-T59-02/03 continuation maps T59.02.a/b/c and T59.03.a/b/c,
families A/B/C/D/E/F/G/I/J. It incorporates `curriculum/v5/contracts/b06.md`.
Prerequisites are the immutable image extent, version floor, key policy and
transactional `lce_admit` artifact from EXTENSIONS.md. Use hosted C17 and OpenSSL3.5
Ed25519 as an explicitly permitted external primitive; implement parsing, key
selection, ownership, result interpretation and rollback yourself. This is a
host adapter experiment, not freestanding target linkage or board secure boot.

Implement `signature_adapter.h` using the public stub. The trusted keyring holds
two32-byte public keys with IDs1/2 and a presence mask. Inputs are immutable
image bytes and a64-byte signature. Return `{accepted=1, verified_key=id}` only
after exact cryptographic success for the selected key; every other outcome is
`{0,0}`. The bridge additionally requires that returned identity equal the
requested identity. Backend errors, invalid signatures, unsupported/missing key,
allocation failure and unexpected verdict values all reject. Free every context
and key object on every path. The authenticated extent includes LE32 version,
key ID and payload; it cannot be replaced between verification and admission.

The supplied test uses two deliberately public synthetic seeds to construct
original teaching keys and signatures. It accepts key1/version7, rejects every
single-byte header/payload change, wrong key, corrupted/truncated signature,
revoked key1, stale version and an injected verifier error. It then accepts
key2/version8 under a policy that still revokes key1. A valid signature does not
override current revocation or rollback policy. The harmless attacker success
condition is publication of a synthetic version under a failed/wrong-key verdict;
it never flashes hardware, modifies host trust or uses production credentials.

D1 is the exact signed-byte/key/ownership contract and error matrix; D2 is the
implementation and tamper/rotation cases; D3 is a source-bound tool/provider log,
compiled fail-open and wrong-key controls and cleanup evidence. Run the public
test with learner code, then opt into the withheld author runner
`python3 tools/run_b06_signature.py`. Require corrected acceptance/rejection and
unchanged output on every failed admission; a crash is not the success oracle.
The report is `research/v5/expansion/b06/signature_validation.json`.

Assess each `.a` derivation, `.b` implementation/boundaries and `.c` transfer.
Level0 treats truthy error as valid; level1 checks only a valid signature; level2
satisfies every exact verdict/identity/policy/failure obligation; level3 completes
FW-S1-DELAYED after seven days with a new algorithm identifier, unknown-critical
field rejection and an independently designed key-rotation trace. Hints: inspect
the backend's exact return values; separate cryptographic success from policy;
then bind the key identity and immutable full extent at the bridge.

Inspected primary bindings are R-B06-FOLLOWUP-OPENSSL-SIGN: Ed25519 NULL digest,
one-shot verification and exact success/error rules. The provider implements
cryptography; using it is not a first-principles cryptographic algorithm exercise.
Signing/verifying here share one provider, so these checks validate integration,
not independent Ed25519 correctness, certification, side-channel resistance,
fault resistance or physical boot-chain integrity. Target adaptation, dependency
closure, hardware key storage and actual reset/flash evidence remain distinct.
