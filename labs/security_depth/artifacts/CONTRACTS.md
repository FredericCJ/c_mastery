# T35 artifact policy and complete service review

This versioned successor completes the author implementation for the six original
criteria `AX-PR-T35-06-T35.11.a` through `.11.c` and `.12.a` through `.12.c`.
The original criterion text, delayed identifiers, baseline bindings and historical
hours remain unchanged. The parent legacy assessment shared contract supplies the
assessment process; this contract supplies the concrete mechanism and environment.
Families A/B/C/E/F/G/I/J apply. No finite regression campaign establishes mastery or
the absence of vulnerabilities.

## Prerequisites and environment

Use active C17, Linux hosted dependency profile A, 8-bit bytes, exact uint32_t,
GCC and Clang with strict warnings, O0/O3 and separate AddressSanitizer/Undefined-
BehaviorSanitizer builds. The original PR-T35-06 expressly permits a mature SHA256
implementation and requires no homemade signature algorithm. Here OpenSSL 3.5.5
implements the declared SHA256/Ed25519 boundary; the learner implements framing,
trust, admission, ownership and failure policy. This does not assess Ed25519
internals. Provider allocation and dynamic libcrypto are target dependencies.
Python/hashlib, disassemblers and receipt tools are host-side verification tools.

Import learner-owned PR-T35-01/03 command parser, authority and copied queue from
`labs/security_depth/command/command.h` and its `CONTRACTS.md`; PR-T35-02's
`labs/security_depth/worker/worker.h` and descriptor policy; and PR-T35-05's
`labs/security_depth/secrets/secrets.h` with explicit clearing primitive
`platform.c`. Their tests and contracts must pass before composition. The supplied
`artifacts.h`, `learner.c`, `tests.c`, `release_probe.c` and original
`fixtures/embedded/PR-T35-06.json` are learner material. Private implementations and
analysis remain under assessor. All objects must be live, correctly aligned and
disjoint unless an operation explicitly permits aliasing. Calls and trust updates
are serialized; no callback may mutate or reenter the service during an operation.
Initialized service invariants are caller obligations, not arbitrary-byte validation.

## Problem and representation

Implement `SATrust`, `SAArtifact` and `SAReport` operations exactly as declared.
Payloads contain 0..32 bytes; null is allowed only with zero length. Digest output
has 32 bytes. The signed artifact message has 44 bytes: ASCII `CM35`, version and
epoch as four-byte big-endian unsigned values, then the 32-byte digest. A candidate
contains a raw 32-byte public key and a 64-byte Ed25519 signature. Its `committed`
flag is a trusted local installation fact, either 0 or 1; it is deliberately not
signed remote input. A trusted owner supplies the initial key, nonzero epoch and
minimum version. Never derive that trust from the candidate's key or adjacent hash.

`sa_verify` stages its report and returns 0 after classification, including ordinary
signature mismatch. Return -1 on invalid arguments or provider operational failure
without changing output. Keep seven distinct booleans: digest match, valid signature,
trusted key, version at least minimum, current epoch, local committed state and
readiness. Readiness is the conjunction of the other six. Compute the signature
over the complete message with Ed25519's one-shot interface and no digest parameter.
Borrow candidate/trust/payload; release every acquired provider object on every path.
`sa_manifest`/`sa_transition` require valid nonnull objects and exact output extents.

Rotate trust with a 49-byte message: ASCII `ROT35`, old epoch, new epoch, new minimum
(each big-endian 32-bit), and new public key. The current trusted key signs this
message. Admit only next epoch exactly old+1, no wrap, a different key and a minimum
that does not decrease. Commit all new trust fields together after verification;
reject without mutation otherwise. Rotation immediately revokes the old key. There
is no dual-key interval, revocation server, persisted transaction or power-loss claim.

Compose the actual queue and descriptor operation in `SAService`. Ordinary opcodes
retain their producer contracts. Add opcode 6 with bytes `[6,width,count,payload...]`:
width is 1 or 2, count is 0..4, and total length is exactly `3+width*count`.
Its opaque copied staging payload consumes a separate four-byte global resource
budget, in addition to the existing three jobs/six work-unit budget. Charge
`max(1,count)` work units and `width*count` retained bytes; each slot can hold at
most four bytes. A successful finish performs the existing authorized four-byte
log read and privileged synthetic counter increment. Payload retention itself is
the new resource being assessed; payload bytes do not grant descriptor authority.

`sa_submit` requires a trusted report previously produced for the installed image;
it does not cryptographically authenticate arbitrary report structs. Reject a
non-ready report before processing commands. Validate framing before effects,
admit both quotas before publication, and publish the ticket last. Preserve caller
ticket on rejection. Authority generation is checked at submission and finish.
`sa_finish` rejects a stale/reused ticket before touching its slot, denies revocation
before descriptor access, stages read output, and publishes it only on success.
Terminal completion, cancellation, descriptor error and shutdown each release their
owned charge exactly once and clear the full staging slot. Repeated shutdown is
idempotent; a stale finish must not clear a new occupant. No descriptor is acquired
or closed by this component: the caller lends a live log descriptor through finish.

Use `sa_verified_submit` at the application boundary: it freshly verifies the supplied
artifact bytes against current trust and only then admits a command, within the same
serialized operation. Provider/argument failure maps to SC_INVALID; a classified
unready artifact maps to SC_DENIED. No cached report is a revocable capability.
The authenticated payload represents the installed teaching artifact, not command
bytes: different valid commands are authorized by the separate SCAuthority policy.
Binding a command's identity into a signature is a different protocol. Keep trust,
artifact, payload and authority stable through the call. A cached ready report still
passes low-level `sa_submit` after rotation; the adapter rejects the old artifact.
Already queued requests are not retroactively revoked by artifact-key rotation:
the artifact policy is checked at admission, while operation authority is rechecked
at finish. Stronger queued-artifact revocation would require a new generation check.

## Necessity and reasoning

A matching checksum is only consistency with a supplied value. A mathematically
valid signature can belong to an untrusted key, an authenticated version can be too
old, and a compatible image can remain uncommitted. Reproducible malicious source
remains malicious. Queue capacity does not bound a newly introduced resource.
State each pre/postcondition and derive `used_bytes = sum(active slot charges) <= 4`
and queue work <= 6. Show why width*count is safe after these small bounds, rather
than generalizing to unchecked multiplication. Explain output staging and acquire/
release ownership, callback exclusion, terminal-counter headroom and generation
exhaustion inherited from the producer. C provides no cryptographic authenticity or
isolation for mutable structs; diagnostics, runtime policy and programmer discipline
have different roles. Verification work is O(payload length) plus provider cost;
service bounds are fixed, while provider heap/stack work is opaque and must be named.

## Deliverables and verification

- D1: source/tool/dependency/release identity ledger, hand-derived framing and all
  seven report fields for the fixture, adjacent-hash replacement, untrusted valid
  signer, old version, changed epoch and uncommitted image.
- D2: learner implementation and ownership/state proof, signed rotation/revocation
  traces, provider-boundary failure and output-preservation traces.
- D3: two same-input builds per compiler, exact commands and binary hashes, actual
  behavior of the original flawed and repaired release probe, plus section and
  instruction analysis explaining a cross-toolchain disagreement.
- D4: eight-row review (extent, arithmetic, lifetime, authority, quota, descriptor,
  secret and artifact), each with invariant, specific source transition, discriminating
  input, observed repair regression and residual limitation. Add opcode 6's separate
  charge ledger; a row containing only a topic name is incomplete.
- D5: attacker-capability table and integrated successful/malformed/denied/overbudget/
  stale/cancel/descriptor-failure traces; replay count/width and authority-source changes.

Run each public case separately so a process-local synthetic key or file is always
released, including intentional assertion failure. Check abc against the independent
fixture, and the 12-byte manifest/17-byte rotation prefixes against manually derived
bytes. Actual signatures must distinguish valid-but-untrusted from invalid. Inject
failures after key acquisition, after context acquisition, after verification init,
and before digest execution; each must preserve all output fields. These injections
are application-boundary failures, not evidence of actual provider out-of-memory.
Use successful sanitized cleanup as bounded corroboration, not universal leak proof.

Check valid sum 7 and authorized log `SAFE` still succeed; malformed length and denied
authority preserve effects; a second four-byte retained request fails despite spare
work quota; two width-one/two-count requests fit. Revoke before finish with fd=-1 to
distinguish denial before I/O from descriptor failure. Cancel and shutdown must erase
staging and release all charges. Keep a harmless mutant omitting each of trusted-key
admission, transition verification and new-resource quota; each must fail its intended
assertion, and the original repaired implementation must pass. Do not count compile
failure, sanitizer failure in a test harness, or unrelated rejection as a killed mutant.

## Rubric and hints

For each original objective, level 0 is missing/incorrect behavior; level 1 reproduces
the supplied case without the following causal argument; level 2 requires all listed
deliverables and an independently chosen discriminator; level 3 additionally derives
the corresponding delayed changed-condition contract unaided after at least seven days.

| Objective | Level-2 discriminator and reasoning | Level-3 changed condition |
| --- | --- | --- |
| T35.11.a | Explain why replaced bytes plus replaced hash still fail authenticated-origin policy; bind actual inputs/tools/dependencies | Replace both adjacent values and separate digest authority from signature authority |
| T35.11.b | Derive all predicates, ownership and unchanged-output failure; distinguish current-key trust from signature validity | Implement authenticated rotation, monotonic minimum and immediate revocation without wrap |
| T35.11.c | Reproduce an original flawed effect, inspect both compiler outputs and explain different hashes | Investigate differing compiler artifacts through actual structure/semantics, not preferred digest |
| T35.12.a | Eight concrete boundary rows and a killed quota defect with a successful repair | New opcode introduces byte retention distinct from work/job quotas |
| T35.12.b | Bound packet/timing/callback/host capabilities and exact harmless observation | Direct shared-process-memory mutation invalidates mutable authority as an isolation boundary |
| T35.12.c | Authorized success plus each named failure and exact cleanup; no reject-all solution | Recompute width/count charges and revalidate authority at effect time |

Hints: (1) list who controls every input field and when each becomes authoritative;
(2) write the ownership/charge equations before coding, then mark the publication point;
(3) separate signature math, trust root, version and local commit, and compare a four-byte
charge with two two-byte charges. Private technical analysis supplies solutions separately.

## Variants and security scope

Retain exact delayed IDs `AX-PR-T35-06-T35.11.a-DELAYED`, `.11.b-DELAYED`,
`.11.c-DELAYED`, `.12.a-DELAYED`, `.12.b-DELAYED`, `.12.c-DELAYED` (each full prefix
`AX-PR-T35-06-T35`), all with minimum seven days. Author execution supplies worked
changed-condition evidence, not a learner's delayed assessment or assessor independence.

Targets are these original local programs only. Synthetic payload abc/abd, published
test seeds 1/2, temporary SAFE log and small counters have no outside authority. The
attacker may replace candidate bytes/key/signature/version, choose malformed packets,
order valid sequential requests or supply a misbehaving local callback. No network
listener, real credential, production target, host policy change or flashing is used.
The shared-memory variant deliberately writes a live synthetic authority object:
observable success is a counter increment after the original denied request. It
demonstrates loss of the assumed logical boundary, not arbitrary code execution or
host privilege escalation. Repair the primary cases with immutable caller-controlled
trust, independent admission predicates, authenticated rotation and separate quotas.
Moving an untrusted callback behind an actual isolation boundary requires a separate
process/protocol design; these tests do not establish that boundary.

## Validation and limits

Current source-bound execution is recorded separately in
`research/v5/expansion/legacy_security_artifacts/validation.json`; failed earlier test
harness evidence is retained as attempt history. Export and negative-control guards
bind the exact canonical criterion objects without rewriting their acceptance history.
No external review, independent learner performance, persisted firmware installation,
physical observation, constant-time guarantee, provider certification or final published
C17/C23 source inspection is inferred. ISO semantics, OpenSSL contracts, Linux descriptor
behavior and actual compiler output remain distinct evidence layers.
