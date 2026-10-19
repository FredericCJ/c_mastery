# Assessor analysis: PR-T35-06 artifact and review successor

Keep this file out of learner exports. The six original objective-specific rubrics
and delayed tasks remain canonical; this analysis defines the concrete solution.

## T35.11.a

The fixture's abc digest is ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad.
Replacing abc with abd invalidates digest match. Replacing the adjacent digest restores
that predicate but invalidates the old signed manifest. Re-signing with synthetic
attacker key 2 restores signature validity while trusted_signer remains false. Thus
neither consistency nor mathematically valid signature supplies its own trust root.
The runner binds source, compiler, headers, libcrypto, libc and emitted binaries;
these hashes identify its scoped inputs, not the legitimacy of their origins.

## T35.11.b

Use separate booleans and a final conjunction. Versions 1/2/3 under minimum 2 yield
false/true/true; a compatible signature with committed=0 remains unready. Commit is
local trusted state and intentionally excluded from the signed manifest. Epoch must
match the current owner; the signature's key must match the trusted public key. Pure
Ed25519 verifies the complete 44-byte manifest with NULL digest, not a prehashed
Ed25519 variant. Classify a normal invalid signature while preserving outputs on
operational error. Four injected acquired-resource boundaries return unchanged report;
key/context release is checked under the sanitizer profiles without claiming exhaustive
provider allocation-failure testing.

Rotation signs the domain-separated 49-byte transition with the old key. Stage new
trust, reject epoch wrap/skips, decreasing minimum and unchanged key, then commit once.
Tampering with the transition is the intended ROTATE mutation discriminator. After
rotation, a perfectly valid old-key signature is untrusted. No durable write protocol
is present: persisted trust rollback, power interruption and key compromise remain
separate problems. Fixed public synthetic private seeds are teaching fixtures only.

## T35.11.c

Both same-compiler builds of release_probe reproduce a defined unauthorized synthetic
effect of 1 from the same original flawed source. Removing the flaw yields 0 and a
different binary. This directly falsifies reproducibility-as-trustworthiness.

Actual first campaign output was inspected: GCC main starts at 0x1060, loads effect 1
into edx and calls __printf_chk with flag 2; Clang main at 0x1140 loads effect 1 into
esi and calls printf. The GCC .text size is 0x119 versus Clang 0x108; GCC has .plt.sec
and a combined .got, Clang a separate .got.plt. Both use the same format string and
produce the same synthetic value. These instruction/section/default-library differences
explain concrete sources of hash disagreement; they are not evidence that either
compiler is malicious or independent of common libraries. The final campaign retains
fresh commands/sections/disassembly and verifies these discriminators. Equal contract
and observed behavior, not preferred hash choice, govern the comparison.

## T35.12.a: complete boundary review

| Boundary | Actual mechanism and discriminator | Repair and residual limit |
| --- | --- | --- |
| Extent | sc_decode rejects `[1,2,0]`; opcode6 requires exact 3+width*count before copying | Validate count/width first, retain exact length; live input pointers remain caller preconditions |
| Arithmetic | width in 1..2/count<=4 bounds product to8; physical slot<=4 and used<=4 | Check new global quota before adding charge; corrupted internal structs are outside initialized invariant |
| Lifetime | Queue copies requests; resource payload copied into owned slot; stale finish before clear | Generation/current-slot check and one terminal release; concurrency is excluded by serialized contract |
| Authority | Submit/finish generation checks; revoked request with fd=-1 returns DENIED | Revalidate before effect and I/O; mutable shared memory is not an isolation boundary |
| Quota | Two four-byte requests fit work6 but exceed retained-byte4 | QUOTA mutant admits second; corrected admission rejects while two two-byte requests succeed |
| Descriptor | Existing sd_request performs pread of four SAFE bytes; fd=-1 error terminally releases | Borrow descriptor, stage read and publish only success; caller owns descriptor lifetime/identity |
| Secret | Full staging and temporary read buffer are explicitly cleared on terminal paths | Actual post-clear slot bytes zero; no erasure claim for provider internals, registers, caller copies or swap |
| Artifact | Actual verified report gates service submit; uncommitted report blocks valid command | Separate signature/key/version/epoch/commit predicates; trusted report must not be caller-forged |

The new opcode has a distinct retained-byte resource. Its payload is opaque copied
data; the authorized effect is the existing log read and synthetic counter. The
demonstration is deliberately not an arbitrary path reader. All eight rows cite
actual composed source/tests rather than claiming prior independent modules imply
their integration automatically.

## T35.12.b: attacker table

| Capability | Actual probe / consequence | Boundary retained or reopened |
| --- | --- | --- |
| Arbitrary bounded packet | malformed, width/count, denied opcode, candidate key/signature replacement | Exact parser/admission and no granted effect on error |
| Timing / ordering | revoke after submit; late/repeated finish; cancel before effect | Sequential event ordering only, no concurrent scheduling proof |
| Local callback misbehavior | original hostile_callback writes the live authority.allowed field | Actual shared memory can bypass logical authorization; no isolation is claimed |
| Shared process memory | denied request, then defined authority write, then successful synthetic effect | Trust/report/queue fields likewise require protected ownership; a new process boundary needs separate validation |
| Arbitrary host compromise | excluded, not tested | No statement that this library protects against host/kernel compromise |

Do not equate this defined counter increment with memory corruption, arbitrary code
execution or operating-system privilege escalation. It is the intended specific
security-property violation under the changed attacker model.

## T35.12.c

The integrated case obtains SAReport from real signature verification and feeds it
through the actual composed service adapter, then blocks the same operation when
uncommitted. Parent-agent source review identified the cached-report trust boundary
before freeze. The completed adapter freshly verifies before admission. An explicit
counterexample shows raw sa_submit accepts a cached ready report after rotation;
the adapter rejects the old artifact, then admits a different authorized command
under a freshly signed current artifact. This is artifact readiness plus separate
command authorization, not an authenticated command protocol. Artifact revocation
is admission-time only; queued operations still use their operation-authority
generation. Caller-owned mutable trust/report objects are not unforgeable capabilities.
The ordinary service trace sums 7, reads SAFE, increments privilege once, rejects
malformed/denied/overbudget/stale operations, and clears resources after revocation,
descriptor error, cancel and shutdown. Output is unchanged on denied/error read.
No reject-all implementation passes these successes. The changed width-one/count-two
case permits two active requests, whereas width-two/count-two permits one. Authorization
is rederived from current generation at effect time; a previously accepted request is
not a permanent grant. The TRUST/ROTATE/QUOTA mutants fail exact named assertions in
all selected profiles and learner stubs fail semantically after successful compilation.

## Evidence bounds

The initial sanitized negative harness leaked its own test keys after an intentional
assertion return. It was fixed with process-level owned-key cleanup rather than
disabling leak detection; the failed report remains. Later fault controls test the
component's own error branches. Finite tests do not certify cryptographic providers,
full-service isolation, all C executions or learner mastery. Seven-day delayed
assessment remains a separate learner obligation. Actual external, physical and
published-edition source gaps remain explicit in completion dimensions.
