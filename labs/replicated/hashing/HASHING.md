# Streaming hash construction and keyed authentication

This required support extends X-T76-007 / PR-T76-07, especially
T76.07.b and X-T76-007-R2/R3. It also supplies the primitive needed for
T76.04's authenticated admission investigation. Implementing this preparatory
primitive does not require completing the later Merkle/anti-entropy exercise;
do not create a circular X-T76-004 to X-T76-007 prerequisite. The original
Merkle proof, framed repair and delayed obligations remain required separately.
Families A/B/C/D/E/F/H/I/J apply; no allocation, I/O, timeout or recovery
mechanism is invented inside this synchronous, bounded-state primitive.

## Prerequisites and inspected passages

Retain T04 unsigned arithmetic/shift reasoning and T03 byte-array lifetime and
alias contracts. Begin with PR-T76-07's byte lengths 0..65 hash implementation
and expected empty/abc digests. The supplied retained `rp_sha256` is a
comparison producer, not a streaming implementation and not a usable HMAC
provider: an inner HMAC computation already starts with a 64-byte pad.
The test executable explicitly imports that producer and its containing source
file's DOD dependency; the new hash implementation itself imports neither.
Do not copy assessor files into learner workspaces. Supply completed learner
dependency workspaces through the established CMake/CLI contract.

Read R-RP-SHA, RFC6234 sections 4.1, 5.1, 6.1 and 6.2; R-RP-HMAC,
RFC2104 sections 2–4 and the final paragraph of section6; and
R-RP-HMAC-VECTORS, RFC4231 sections4.2/4.3/4.7 and5. These exact passages were
inspected for this support. They specify algorithms, not final ISO C wording,
an audited production library or a proof that the learner's implementation is
secure. Preserve the published-C17/C23 access gaps.

## Environment and concrete representation

Use ISO C17 with 8-bit bytes and exact uint32/uint64 types on hosted Linux,
GCC/Clang, extensions disabled. The header exposes `struct rph_sha`: eight uint32
chaining words, uint64 byte count, 64-byte partial block, used count and final
flag. `struct rph_hmac` owns two such values. Contexts own no pointers and can be copied
by value to fork a prefix; copies of keyed state are additional secrets.
Objects and byte spans must be live and correctly aligned. Distinct parameters
and their object spans are disjoint; in particular input/output cannot overlap
a context. Null input with zero length is legal and performs no null arithmetic.
Null required control/output is INVALID. Operations on one context are externally
serialized; concurrent independent contexts are allowed. Public fields are for
inspection, not permission to mutate a live context. Explicit test-only state
injection checks admission arithmetic and length encoding.

Implement locally from integer/byte operations. No target cryptographic,
container, serialization or dynamic-allocation library may implement a studied
mechanism. Python hashlib/hmac, diagnostics, CMake and the hosted timing/test
harness are verification dependencies only. This support's host campaign does
not claim a strict freestanding link, board execution or a timing guarantee.
Inspect actual object disassembly, undefined symbols and stack reports; any
implicit compiler helper must be declared before adapting to a restricted target.

## Operations, invariants and refusal

`rph_init` admits writable context storage and sets the RFC initial words,
zero count/used/final and zero block. A live active context has final0,
bytes at most floor(UINT64_MAX/8), used below64 and used=bytes modulo64.
The words summarize exactly the complete blocks; block[0..used) is the suffix.
`rph_update` must preflight all arguments, state and remaining length before
changing anything. It consumes exactly count bytes, retains no input pointer,
and returns RANGE without modification if the byte-count bound would be exceeded.
This bound ensures the encoded bit count is less than 2^64. Empty updates are
accepted only on active contexts. A final or structurally invalid context gives
STATE, preserving it; this is not a native-pointer validity detector.

`rph_final` stages padding and digest on a copy: append 0x80, zero to byte56
of the final block (possibly requiring another block), append the original bit
length as eight big-endian bytes, compress and emit eight big-endian words.
Success writes exactly32 bytes and marks the original context final1. Repeated
final/update returns STATE without modifying context/output. Finalization does
not append padding to the public byte count or permit silent reuse. `rph_hash`
is the one-shot adapter with the same input/count/output rules. No allocation
failure exists; argument/count/state refusal is transactional.

Implement both a 64-word schedule and a 16-word circular schedule. For the
latter derive why the overwritten slot at round t holds W[t-16], and read all
four source terms before the assignment. Every addition is modulo 2^32, and
all actual rotate counts are in 1..31. Derive constants independently from
fractional cube roots and initial words from square roots; do not replace
construction with a call to a hashing library.

`rph_hmac_init` hashes keys longer than64 bytes, pads the resulting key to64,
and initializes separate inner/outer states with XOR0x36/XOR0x5c blocks.
Keys of length0 are admitted for algorithm testing, not recommended key policy.
The input key is borrowed only until return. Active HMAC has active states,
inner.bytes at least64 and outer.bytes exactly64. Update uses inner admission;
therefore the maximum message length is floor(UINT64_MAX/8)-64, not the raw
SHA limit. Finalize a staged inner state, append its32-byte digest to the outer,
finalize and publish both final flags and the tag. Repeated use refuses.
The one-shot adapter preflights key/message bounds before consuming either.

`rph_equal` compares exactly32 bytes, returning false on a null argument. All
32 bytes influence the result. Its fixed source work and branch structure are
an audit obligation, not an ISO C constant-time guarantee. `rph_clear` performs
volatile byte stores over exactly the live writable object span (NULL/0 is
allowed); it does not erase unknown copies, registers, swap, compiler temporaries
or previous logs. Clear contexts after finalization/cancellation in the caller;
the function does not discover other owners or revoke retained copies.

## Required experiments and evidence

D1: Supply the two schedule implementations and state-machine invariant proof,
including 55/56 and63/64 boundaries, original bit-length encoding and finite
count admission. Keep the existing bounded hash and compare all lengths0..65.
Fork one prefix into different suffixes and explain the value-copy semantics.

D2: Implement streaming/one-shot HMAC and demonstrate long-key reduction,
independent pad states and key/input lifetime. Include three published full
SHA256 HMAC vectors (RFC4231 test cases1,2,6), empty key/message, key lengths
63/64/65/131, and repeated-final rejection. Explain why authentication is not
authorization, confidentiality, replay prevention or identity provisioning.

D3: Run four public anchors with both schedules, the independent oracle and
all eight compiled controls. The stream anchor compares every two-piece split
of lengths0..257 (33,411 splits). The host oracle checks 1,786 combinations of
length, key length, chunk size and byte pattern against hashlib/hmac for both
schedules (7,144 digest comparisons). A separately written 64-word Python
compression algorithm derives constants with exact integer roots, checks itself
against hashlib on130 short messages, then checks three injected large byte
counts. These injected states test encoding, not processing huge real streams.
Retain exact commands, raw logs, source/toolchain/include/link identities and
normal semantic mutant rejection. Sanitizer termination does not count as an
intended mutant kill. Run the compiling learner scaffold and observe its named
semantic failures before replacing it with the learner implementation.

D4: Conduct the local authentication experiment below; inspect generated
comparison/compression code, code size and stack use. Measure both schedules
over lengths0/55/56/64/65/1024/8193 with identical digest contracts and1,000
iterations each. Preserve raw elapsed samples and checksum preventing unused
result elimination. Explain the limitations of TIME_UTC, scheduler noise and
host-only samples. Never report faster code as better before checking identical
results and resource contracts.

## Security target and harmless success condition

Target only `hash_security` in this original local process, with synthetic
eight-byte key and byte requests. No external endpoint or real credential is
used. The intentionally flawed verifier accepts a tag computed as
SHA256(key || request). The attacker sees a tag and original request and knows
the laboratory key length, but the continuation routine uses no key bytes.
Resume the exposed chaining words after the original padded64-byte block,
append eight chosen bytes and produce a forged tag. The target independently
hashes key || original || glue-padding || extension and accepts it. Acceptance
of this new byte message is the complete harmless violation; the bytes are not
parsed as commands and no privilege escalation or code execution is claimed.

Repeat continuation from the genuine HMAC tag: the HMAC verifier rejects it.
Repair the construction with the implemented HMAC, compare all32 bytes, and
test every single-bit tag alteration (256 rejections). Compiled controls replace
HMAC with secret-prefix hashing and compare only one tag byte. Exploitability
of the flawed construction requires exposed hash state, guessed/known key
length and a target admitting those extended bytes; structured-message policy
could reject the glue bytes independently. HMAC correctness in these examples
does not establish general unforgeability, safe key distribution, constant-time
execution, replay defense or protection against an authorized malicious writer.

## Rubric, hints and changed assumptions

Use the original X-T76-007 rubric and assign R2/R3 to D1/D3 here, while R1/R4
retain their tree/repair requirements. Level0: missing or incorrect core/result.
Level1: vectors pass but boundaries, state ownership or failure proof is absent.
Level2: D1–D4 and all named boundaries pass with independently explained
evidence and enforcement limits. Level3: independently derives the changed
schedule, prefix fork, high-count admission and length-extension analysis from
the contracts without consulting the assessor solution. Every required criterion
must reach2; unaided/delayed learner evidence is recorded separately. An
incorrect security guarantee or changed output on admitted refusal fails.

Hints: (1) store the unpadded count separately from the temporary suffix;
(2) derive the circular schedule's slot meanings at rounds15/16/31/32;
(3) HMAC's outer hash consumes a digest, not the original message;
(4) an exposed unkeyed chaining value can be resumed, but it is not an HMAC
inner state. Separate assessor analysis is in `assessor/replicated/hashing/ANALYSIS.md`.

The changed-condition variants are the circular/full schedule comparison,
stream chunking versus two-piece splits, independent prefix copies, long HMAC
keys and high-count state injection. X-T76-007-DELAYED still requires the
separate unseen Merkle/framed-repair variant; this primitive does not substitute
for it. Revisit generated shifts, helpers, stack and comparison code on a new
data model/compiler/target before asserting portability or execution costs.

## Thirteen cost and enforcement dimensions

1. Representation: eight words+64-byte suffix+counter/state; record actual
   sizeof/alignment for each target instead of assuming no padding.
2. Ownership: value-owned contexts, transient input borrows, independent cloned
   secret copies; no allocation or hidden shared pointer.
3. Operations: update linear in consumed bytes; final at most two compression
   blocks; comparison exactly32 source iterations; HMAC adds two pad blocks.
4. Invariants: used/count/state and original length are runtime checked;
   actual historic prefix validity and disjointness remain caller obligations.
5. Failure: preflight INVALID/STATE/RANGE preserves outputs/state; no memory
   allocation, system I/O or recovery mechanism exists in this primitive.
6. Work: 64 rounds/block, full schedule64 words versus circular16; derive exact
   compression counts including padding and long-key reduction.
7. Storage: bounded context and call stack; report actual stack/disassembly,
   including staged HMAC copies and compiler-induced copies/helpers.
8. Locality: sequential input, small schedule/state, fixed public K-table
   indices; no secret-dependent table lookup in the authored source.
9. Timing: raw host samples for both equivalent schedules; no worst-case,
   physical latency or constant-time claim follows from the samples.
10. Code/toolchain: size, stack, undefined symbols and comparison/round code;
    compiler, optimization and diagnostics are recorded, not ISO properties.
11. Composition: retained short-hash equivalence, later HMAC admission and
    domain-separated Merkle commitments; none imply sender authorization.
12. Enforcement: C types/layout and unsigned arithmetic versus API runtime
    checks, tests/oracles and caller lifetime/key policy are distinguished.
13. Alternatives/transfer: compare schedule representations under the same
    digest contract; contrast a language-owned byte buffer's bounds checks
    with C span preconditions, without calling either a cryptographic proof.

## Validation scope

Authored support; executions require a matching `replicated_hashing_validation`
report and guard. Initial GCC release tests/oracle passed; final profile status
is in the source-bound report, never inferred from this paragraph. No independent
human assessment, learner mastery, physical execution or production security
certification is claimed. Remaining Merkle transport/repair and authenticated
counter policy obligations remain visible until their own evidence is supplied.
