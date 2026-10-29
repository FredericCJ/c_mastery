# PR-T35-05 synthetic secret lifetime successor, version 1

This required contract supplies concrete work for the existing
AX-PR-T35-05-T35.09.a/b/c and their original `-DELAYED` identities. Read it
with the parent, its exact rubric and the legacy shared assessment contract.
Historical acceptance/hours remain unchanged. The minimum seven-day transfer
gate is independent learner work; author controls do not satisfy it.

## Prerequisites, profile and representation

Bring C17 byte/lifetime/alias reasoning, compiler optimization and link audits,
the parent fixture prediction, and your own unfinished comparison/ownership
artifacts. Use only synthetic token[1,2,3,4]. Inputs and output objects are
live and disjoint; no invalid-pointer, freed-storage or register scanning is
permitted. C17 is the implementation baseline; C23 is a separate availability
probe, with N3096 draft wording identified as such, not a read final standard.

`secrets.c` is profileB: caller storage, `<stddef.h>`, no allocation, I/O, TLS
or callbacks other than the explicitly permitted `ss_wipe` platform adapter.
The Linux adapter calls documented `explicit_bzero`, a non-ISO-C17 extension.
Its dependency remains visible in the uninstrumented object. The selected
libc implementation and emitted primitive are inspected; dynamic binding and
opaque libc internals prevent extending the component's own no-allocation
claim to the whole process without further evidence. ASan/UBSan and their
section-boundary/runtime symbols belong only to the diagnostic harness.

`struct ss_compare` owns two unsigned fields (equal/work). `struct ss_pool` owns an actual
four-byte array and live flag initialized to0, no typeless-storage fiction.
Each pool has one owner, no concurrent access, and cannot be copied while
live to create a second ownership claim. `ss_acquire` copies the borrowed
four-byte token only into an empty pool; rejection preserves pool fields.
`ss_release` explicitly clears those four bytes, changes live1→0 exactly
once, and rejects absent/nonlive pool. It clears neither the original nor
any other copies. All pointer validity and exact extents are caller contracts.

## Problems and necessary counterexamples

Implement `ss_compare(token,candidate,length,early,out)`. Accept only exact
length4, nonnull controls and early0/1; invalid admission preserves output.
Length is public metadata. Successful early comparison stops on first unequal
byte. Fixed-work comparison accumulates all four XORs and reports work4.
No string, crypto or comparison library implements this mechanism. Derive the
work/equality outputs for first-different, last-different and equal candidates,
then all256 replacements at each of four positions. Explain the invariant
that accumulated difference is zero exactly when every visited byte matches.
All work is bounded by4 and auxiliary storage is constant.

Implement `ss_indexed` as the deliberately misleading changed mechanism:
for four positions read `table[token[i]]` and `table[candidate[i]]`, record
their eight indices in `trace`, and compare mapped values. The table is the
256-byte XOR165 permutation, declared volatile to retain selected reads in
the experiment. Eight reads still reveal differing address indices. The
teaching observer receives explicit logical traces, not a cross-process cache
probe or an actual timing attack. This counterexample falsifies equivalence
between a fixed loop length, fixed access count and secret-independent
addresses. Volatile does not certify timing or erase register copies.

For lifetime investigation, track distinct original, temporary, pool,
allocator and intentionally unmanaged copies. The component uses no heap;
the hosted harness creates one actual malloc4 copy, checks its cleared bytes
while the allocation is still live, then frees it. Never inspect freed storage.
The original and unmanaged copy deliberately remain unchanged, exposing the
limit of clearing another object. Logs contain only status/work labels and
the separately declared public MAC output; token bytes never enter log calls.
Source contains the synthetic token by design and is not a secret vault.

Compare `ss_dead_store` with `ss_explicit_store`: both copy a borrowed token,
call a separate original consumer and return sum10; only the latter uses the
documented clearing primitive. Inspect complete final functions at O0/O3 and
under diagnostics. A missing memset symbol alone is insufficient: check the
instructions for an inline clear too. The changed C23 probe compiles, links,
executes and inspects `memset_explicit` where available; C17 and musl1.2.5
negative declaration probes preserve actual diagnostics. An accepted language
flag is not evidence that a library declares/implements the function.

## Separate trusted-library transfer

The original .09.c-DELAYED explicitly changes the dependency profile. Its
`provider.c` adapter is **hosted profileA**, with declared OpenSSL3.5.5 EVP_MAC
HMAC-SHA256 and platform clear. It does not implement or replace the educational
comparison or a first-principles cryptographic algorithm. The studied mechanism
here is integration ownership, key lifetime, error admission and publication.
Matching development headers are required; the author privately extracted
the exact distribution package without installing or changing system policy.

Implement `ss_mac(key4,message,length,fault,out32,ledger)` with borrowed key
[1,2,3,4], length0..64, null message allowed only for zero, nonnull distinct
output/ledger and fault0..7. Invalid admission leaves both outputs unchanged.
It owns a four-byte working key, a staged32-byte result, a fetched algorithm
and a fresh context. Initialize with SHA256, update the admitted message,
finalize into staging, require exactly32 bytes, then publish output. No
context reuse or key from environment/command line is permitted.

Fault1 is before fetch,2 after fetch,3 after context,4 after initialization,
5 after update,6 after finalization/before publication. Fault7 supplies a
deliberately nonexistent digest to exercise actual provider rejection.
All failure paths retain caller output, release every acquired provider
handle exactly once, explicitly clear owned working key/result, and publish
a nonsensitive ownership ledger. The borrowed key remains the caller's
responsibility. Successful output is a public authenticator for synthetic
messages, not permission to log real keys. Provider internal key schedules,
allocations, scratch/register copies and thread error queues are opaque;
freeing a context alone does not prove erasure of all copies.

## Deliverables and objective assessment

D1 (.09.a): copy/owner/lifetime/log inventory, pool implementation, defined
before-free allocator observation, dead-store versus explicit clearing
disassembly and an edition/library availability table. Prove the failure
state before testing. Level1 clears one buffer and claims universal erasure;
level2 accurately limits each object/code observation; level3 repeats the
original delayed C23/platform comparison and revises unavailable claims.

D2 (.09.b): exact fixture derivation, 1,024 changed-byte cases, unequal-length
admission, observer inference for work1 versus4, and the eight-read indexed
counterexample. Include actual final comparison/addressing instructions at
changed optimization settings. Level1 equates fixed work with timing;
level2 separates work/address traces from hardware; level3 independently
constructs and diagnoses the original compiler/access-pattern transfer.

D3 (.09.c): an explicit claim boundary separating representation/algorithm
tests, compiler observations, logical leakage, actual timing research,
operational assumptions and deployment approval. Implement the profileA
adapter,65 independently checked message lengths and all seven failures.
Level1 treats known-answer vectors as approval; level2 states exact guarantees
and residual copies/trust assumptions; level3 owns the actual provider state
and cleanup correctly under changed failure/lifetime assumptions.

D4: source/tool/libc/header identities, strict object undefined-symbol audit,
intended mutant failures, compiled learner rejection, raw logs and limitations.
The independent Python fixture/tag oracle is host verification only. Preserve
any failed attempt separately. No criterion passes from an unrelated
diagnostic, a heading or reproducing only the supplied happy path.

The author host harness also composes the frozen original first-principles
`rph_hmac` from `labs/replicated/hashing`, explicitly under the verification
flag SS_FIRST_PRINCIPLES. Its implementation is distinct from OpenSSL;
Python hashlib/hmac may share the installed provider and is not assumed
algorithmically independent. The campaign checks all65 values against both
controls. The Rph representation is two SHA contexts with owned
block/key-derived state; its full contract and clearing limits remain in
HASHING.md. These bounded checks do not approve either implementation for
deployment or discharge that producer's separate algorithm exercises.
Assessor source stays private.

## Hints, variations and security scope

Hint1: draw one node per actual copy and attach an owner/destruction event.
Hint2: separate iteration count from addresses and selected instructions.
Hint3: inspect libc declarations, provider state order and final object before
changing a compiler flag or assuming cleanup happened.

Keep AX-PR-T35-05-T35.09.a-DELAYED (C23/platform availability),
AX-PR-T35-05-T35.09.b-DELAYED (compiler/address changes), and
AX-PR-T35-05-T35.09.c-DELAYED (trusted-provider integration). Independent
assessment uses changed compiler/library/error conditions after seven days.

The target is only these original local functions and synthetic four-byte
token. An observer may select fixed candidate bytes and see declared work or
address traces. Harmless success is distinguishing a prefix through work,
demonstrating the indexed-address difference, or showing an explicitly
unmanaged copy remains. No real credentials, network target, third-party
process or physical side-channel experiment is involved. Repairs remove the
claimed logical leak, reduce/track copies, use supported object clearing and
contain errors. Regression distinguishes those properties; it does not claim
constant-time execution, comprehensive erasure or production cryptography.

## Validation status

Current author execution is source-bound in
`research/v5/expansion/legacy_security_secrets/validation.json`. External
review, independent delayed learner evidence, exact final C17/C23 source
inspection and physical behavior remain distinct open dimensions. Assessor
implementations/analysis are excluded from the learner export.
