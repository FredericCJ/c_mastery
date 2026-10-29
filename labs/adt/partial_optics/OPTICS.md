# T67.08: Partial optics, preserved versions and observable effects

This required successor implements X-T67-008 and its original delayed variant.
Complete X-T67-007 and retain the original `struct ad_pair`, `ad_lens_x`,
`ad_review_number` and `ad_preview_number` implementation from PR-T67-08.
The actual retained functions are imported by the reference tests; a learner
supplies their completed implementation explicitly, never the assessor tree.
Implement the supplied compiling `partial_optics.c` stub without a lens, sum,
reflection or serialization library. The public header, tests and model driver
interfaces are learner artifacts; private implementations/answers are separate.

## Representation and admission

Use hosted ISO C17 with GCC/Clang debug, optimized and ASan/UBSan profiles.
No callback, pointer or concurrency behavior is inferred from Haskell types.
The mechanism allocates nothing. A configuration owns initialized inline
fields: outer presence, inner presence, an endpoint pair, inner audit value,
outer identity and footer. Both presence flags must be0 or1, even when outer
presence is0. All other fields are uint32. Invalid flags are INVALID; a valid
configuration whose outer or inner flag is0 has no selected endpoint and
returns ABSENT. This is a deliberately chosen representation invariant.

All pointers denote live aligned objects and actual extents. Inputs, output,
step array and mutable callback contexts are disjoint. Calls are externally
serialized. All configuration fields are initialized, including inactive ones;
copying an inactive field is therefore permitted. A successful operation copies
values and retains no input or output pointer. No destructor is needed. Null
required output is INVALID. Do not use fabricated pointers as an oracle.

## Optional nested field update

`adl_get` selects the endpoint's x only when both presence flags are1.
`adl_put` returns a copied configuration with x replaced through actual
`ad_lens_x`; it preserves endpoint y, audit, identity, footer and both flags.
Every refusal preserves the entire output object. Missing structure is never
created implicitly, and no default payload is invented. Input(outer1,inner1,
endpoint7/8,audit17,id29,footer41), replacement10 produces exactly the same
fields except x10. Input with either flag0 returns ABSENT unchanged.

Let D be valid configurations with both flags1. On D, get/put satisfy the
three total lens laws with field equality: putting back get(s) yields s;
getting a successful put(s,v) yields v; putting w after v equals putting w
directly. Put preserves membership in D, which makes the composition defined.
Outside D neither get nor put has a selected value; the corresponding equations
are conditional claims, not total equalities. Non-match preservation is an
additional operational contract. Treating an absent get as0 and making put
create a configuration breaks the first equality: put(get(absent)) creates
presence. A setter clamping9 to3 violates put-get even on a present source.

The composition is outer optional selection, inner optional selection, then the
actual pair-x lens. Prove nonselected fields stay unchanged at each layer.
Semantic equality compares named fields, not raw struct bytes: C padding is
not part of the represented configuration. The tests use a stronger fieldwise
preservation check that also includes inactive initialized fields.

## Versioned error prism

The original error sum is a local value representation, not an external wire
format or a library's real error namespace. Version1 admits SUCCESS or IO with
code1..255 and context0..255. Version2 admits SUCCESS, IO with code1..65535 and
any uint32 context, or TIMEOUT with a uint32 duration. Other versions return
VERSION; invalid tag/version combinations return INVALID. Out-of-domain IO
payloads return RANGE. SUCCESS and valid TIMEOUT are nonmatches (ABSENT).

`adl_preview` extracts IO into `struct adl_io {version,code,context}`. `adl_review`
checks this refined focus domain and rebuilds the exact IO branch. Both leave
output unchanged on refusal. On valid IO focus f, preview(review(f))=f. On a
matched source s, review(preview(s))=s under tag/version/active-field equality.
Review is total only over the declared valid focus domain, not every raw C
struct. Previewing a nonmatch retains the original source in the caller; this
API does not claim to expose Haskell's type-changing `matching` residual API.

The version is essential. Extracting only code3 from(version2,IO,3,context4)
and rebuilding canonical(version1,IO,3,context0) loses information and violates
the matched-source round trip. Tests retain this defined counterexample and
exercise255/256,65535/65536,zero,UINT32_MAX,unknown versions and timeout-in-v1.
No inactive union member is used as a semantic equality shortcut.

## Callbacks: transactional output is not effect rollback

`adl_update` accepts zero to two `struct adl_step` entries. Each entry contains a live
callback and context. It validates output, count, complete step array and both
presence flags before invoking any callback. Any missing callback is INVALID
even if an earlier callback would otherwise fail. An absent focus invokes no
callback. With a present focus, zero steps succeeds with an unchanged value.

For each admitted step, call exactly once, in array order, with the current
staged x and a borrowed uint32 result. Return0 means success and initializes
that result; nonzero means failure. On success, the next step sees the newly
staged value. On callback failure, stop and return CALLBACK without publishing
any output. The callback may have written its borrowed result before failure;
that value is discarded. All external effects already performed by callbacks
remain observable. The callback must not retain the borrowed result pointer,
reenter the operation, mutate input/steps/output or invalidate any live object.
Context effects belong to the caller and require their own cleanup policy.

Concrete trace: inputx7, first callback adds3, second writes99 then fails.
The callback inputs are7 and10, each count is1, and the previous output remains
unchanged. An identity callback preserves configuration fields but increments
its external trace counter: value-level identity does not imply effect-level
identity. Arbitrary callbacks cannot be duplicated, dropped, reordered or
treated as pure merely because their C function pointer types match.

All arithmetic in the supplied add callback is checked uint32; overflow is a
callback failure after one invocation. The core itself does not impose a
numeric range beyond uint32, and it does not mistake failure for a valid0.

## Verification, deliverables and criterion-specific rubric

Submit D1 conditional equations and the no-creation counterexample; D2 typed
representations, semantic equality, exact version/focus domains and callback
effect/lifetime assumptions; D3 implementations, independent models, minimized
counterexamples and compiled-control evidence; D4 the changed optional/versioned
case with effect traces and all thirteen costs below. Preserve predictions,
commands, current source hashes and observed outcomes.

The independent Python oracle uses tuple replacement, version-domain relations
and explicit effect traces. It checks every presence combination in a finite
domain, boundary/seeded uint32 fields, every small replacement pair, version/tag
combinations and all supplied callback mode/order/failure combinations. It
does not call the C implementation to derive expected results. The baseline
anchor actually executes256 lens-law combinations and both retained prisms.

Four anchors must pass all six compiler profiles. Eight compiled controls must
fail ordinary semantic assertions: create absent paths, corrupt collateral
audit, discard version, accept TIMEOUT as IO, duplicate callbacks, publish a
partial value on failure, retained collateral-y mutation and retained EMPTY
acceptance. Four compiling learner stubs must fail the selected anchors.
Compilation failure, sanitizer termination or an unrelated process failure is
not a successful semantic-control rejection.

R1 requires the three equations and prism domain/round trips, not only examples.
R2 requires per-field preservation and distinction between value, padding and
effect equality. R3 requires actual baseline composition, all eight controls,
independent results and clamping/effect counterexamples. R4 requires implemented
nested partial updates, version-preserving focus, full admission before effects
and one-call failure traces. Level0 contradicts a contract; level1 passes only
ordinary public cases; level2 supplies each complete argument and bounded
evidence; level3 transfers unaided to the changed conditions and identifies
which old law no longer applies. Finite tests do not prove arbitrary optics.

For X-T67-008-DELAYED, begin with absent outer or inner configuration, then
require explicitly authorized construction as a separate operation before
using the partial optic. Extend the error source from version1 to version2
without dropping its version/context. Add a fallible second callback after a
successful first update, preserving output while recording both external
effects. Existing supplied implementations support these changed assumptions;
the unaided attempt must occur after the original seven-day delay. An external
assessor must distinguish understanding from copying this implementation.

Hints: (1) name the domain on which get exists; (2) retain every component needed
to reconstruct a matched source; (3) separate the staged configuration from
the callback's external state; (4) validate the whole operation before effects.

## Thirteen cost/enforcement dimensions

| Dimension | Required evidence and limit |
| --- | --- |
| Representation | Observed configuration size/alignment, tagged error size, explicit two-level presence and focused version. |
| Reachable storage | Inline fields only; caller-owned step/context extents, no retained borrows. |
| Lifetime | All fields initialized, callbacks live through return, no result-pointer escape; C does not enforce these rules. |
| Mutation/aliasing | Staged copied source and disjoint output; callback context effects remain observable. |
| Allocation | None in the optic core; callback-owned allocation would need its own declared contract. |
| Movement/copying | Fixed structure copies; actual retained pair setter preserves collateral fields. |
| Destruction | No owned heap or destructor in the sum/configuration; external callback cleanup remains caller-owned. |
| Dispatch | Direct partial selectors plus at most two indirect callback calls; separate raw timing series. |
| Concurrency | Externally serialized calls; no transactional isolation or concurrent-context guarantee. |
| Failure | Complete admission before effects; output unchanged on absence/range/version/callback failure, external effects not rolled back. |
| Compilation | Strict C17 diagnostics and actual clean/unchanged build observations; Haskell comparison is inspected source, not runtime execution. |
| Binary/stack | Actual object disassembly, helpers, binary sizes and compiler stack-use records. |
| Boundary | Refined focus domain versus raw C values; conditional laws versus total equations; field equality versus bytes/effects. |

Seven samples compare100000 direct and composed pair-field replacements on the
same already-valid configuration. Both produce checksum3000000; the composed
operation includes its admission checks, so the ratio is not pure dispatch
overhead. A separate one-callback workload produces checksum3100000 and exactly
100000 callback effects per sample; it performs different work and is not a
speedup comparison to the first two. No performance threshold is assumed.

The inspected maintainer source `Control.Lens.Type` states lens and prism laws
and polymorphic function types. Those are semantic obligations, not a claim
that any arbitrary setter implementation is lawful. These C functions have
concrete representations and explicit partial statuses; they do not implement
the Haskell rank-polymorphic encoding. ISO C checks function/structure types,
diagnostics check selected misuse, tests falsify bounded counterexamples, and
the stated ownership/purity laws still require proof or programmer discipline.

All examples are original local teaching values with synthetic fields. No
external target or security-property exploitation is involved. Author execution
is credited only by the current source-bound campaign and its guard. Published
C17/C23 access, Haskell runtime comparison, non-author assessment, independent
learner performance and physical behavior remain separate evidence limits.
