# Required X-T66-010 investigation: revalidate at the authority boundary

This completes author support for the original R1 through R4 and delayed
X-T66-010-DELAYED. It preserves the original sixteen-cell teaching target.
The public header and compiling unsolved source are learner material; reference
code and worked analysis remain under assessor/ownership/authority_boundary.
Passing the supplied tests establishes neither whole-process memory safety nor
independent mastery.

## Prerequisites, profiles and supplied artifacts

Retain the X-T66-009 allocation/lifetime ledger and implement the actual earlier
ow_checked_write API from PR-T66-10 in the same ownership workspace. This slice
uses no allocation, callbacks, threads, OS services or third-party capability
library. The hosted C17 driver supplies observations and independent Python
oracles; GCC and Clang debug, optimized and ASan+UBSan builds are separate
experiments. CHAR_BIT=8, exact uint32_t/int32_t and 32-bit unsigned are asserted
for the original wrap comparison. The wire profile has explicit octets and
does not serialize a C structure. Six anchors, eight compiled controls,
tests/tests.c, include/authority_boundary.h, src/authority_boundary.c and the
campaign recipe are supplied. No assessor implementation enters an export.

## Representation and authority

OafRoot is trusted live caller storage with arena pointer, tenant0 or1,
rights READ=1 and WRITE=2, epoch initially1 and a retirement flag. Each tenant
owns eight logical cells in the same actual int32_t[16]. A view contains a
borrowed root pointer, epoch, tenant-relative interval and attenuated rights.
Initialize only fresh root storage; reinitializing a live/retired authority is
outside the contract. Its arena and root remain at their addresses until all
views and calls have ended. All operations are externally serialized; input
control objects remain unmodified during the call. Root/view storage and
outputs must be live, correctly typed and disjoint from the arena and each
other, except subview may replace its own parent and decode may overlap its
input bytes. Read output contains at least eight int32_t objects. No retained
borrow is created from wire input or read output.

Native C callers cooperate with those ownership/lifetime rules. A writable C
struct is not an unforgeable capability. At the foreign boundary, the server
selects a trusted parent view out of band; attacker bytes cannot choose its
pointer, rewrite it or select an unrestricted parent. A request can forge every
numeric field. Reconstructing a typedef from those fields establishes no
authority. No cryptographic authenticity, IPC credential check, hardware
memory protection or Rust borrow-checker equivalent is supplied or claimed.

## Operations, wire format and failure order

oaf_view issues the whole root interval. oaf_subview admits offset<=parent.count,
count<=parent.count-offset and rights contained in the parent's rights. Zero
rights are valid but confer no access. The resulting absolute tenant offset is
parent.offset+offset; a zero-length one-past view is valid. Out-of-range/unknown
rights return INVALID, widening returns DENIED. Publication occurs last. Parent
replacement is valid because all fields are staged first.

The exact 16-octet request is version1, tenant, rights, reserved0, followed by
epoch, offset and count as three unsigned32 big-endian values. Offset is relative
to the trusted parent. Encode checks syntactic tenant/rights and stages all
bytes; it deliberately allows arbitrary epochs/extents for adversarial input.
Decode requires exactly16 bytes, version1, tenant<=1, rights<=3 and reserved0.
It copies all fields before output, including when output overlaps the byte
representation. Invalid input leaves the output unchanged. NULL is invalid even
at length0. This boundary does not expose an in-process pointer in the wire.

Read and write first validate the live trusted view. An expired view returns
STALE. Next they reject malformed tenant/rights and extent, then reject a
request epoch mismatch as STALE, then enforce matching tenant, rights subset
and the requested operation's right as DENIED. This order defines statuses for
inputs invalid in multiple ways. Empty access still requires its operation's
right. Only after admission does a translation-unit-private Validated value
reach the simple indexing core. Read stages and copies exactly count cells;
its remaining output cells are unchanged. Write changes exactly that interval.
Every failure preserves the whole arena and output. Native requests and decoded
requests pass through the same validator.

Revoke advances the epoch; old views and old requests refuse. At UINT32_MAX it
sets permanent retirement without wrapping. No new view may be issued; another
revoke returns FULL. Cleanup requires no new epoch. Finite epoch values cannot
be silently recycled. The MAX test sets trusted state explicitly as an assessor
boundary fixture, not an attacker API.

## D1 through D4 and acceptance criteria

D1/R1: derive the original wrapped sum at UINT_MAX-3,count8. Show exact modified
indices12..15 and0..3 using synthetic canaries. Every store remains inside the
actual16-cell array; a quiet sanitizer does not imply tenant isolation.

D2/R2: implement the original subtraction check and private validated core,
then the root/view/request split. State the trusted-parent premise and provide
a same-process forged-struct counterexample to an overstated capability claim.
Correctness must not depend on attacker-provided epoch being a secret.

D3/R3: enumerate ordinary tenant/offset/count combinations around8 and large
unsigned cases, then run the independent set-membership oracle. Require
unchanged state/output on refusal, zero-length boundary correctness and named
semantic rejection of all eight controls. Compilation failure or sanitizer
termination does not count as rejection. The oracle covers a fixed parent;
separate anchors cover codec/overlap, nested views, revocation and retirement.

D4/R4/delayed: deliver a foreign byte request through the actual adapter, derive
a read-only nested view and show that it cannot widen rights. Compare native
and wire admission. Revoke a root while retaining both old native view and old
wire bytes; both must refuse. For the delayed unaided assessment, change which
principal chooses the parent and explain why numeric validation alone cannot
authenticate that principal. Analyze a trusted principal-to-parent selection table with lifetime and
revocation rules; explain why letting the client name an unrestricted native
pointer defeats the boundary. The supplied implementation supports the original
required read/write and read-only subview change. This further analysis variant
requires a concrete authority counterexample, not an invented authenticated
service or an unobserved learner execution.

Rubric: level0 violates isolation/publication/lifetime or overclaims safety;
level1 passes nominal examples without derivation; level2 meets the entire
corresponding R1-R4 contract with source-bound observations and evidence limits;
level3 independently adapts the authority model and falsifies a former premise.
All four require level2, with independently assessed changed conditions as
required by the original rubric. Record hints and delayed assessment separately.

Hints: (1) distinguish actual object bounds from logical tenant bounds;
(2) compare against remaining length before adding; (3) determine who controls
the parent before trusting any reconstructed descriptor. The worked status
order, arithmetic proof and mutation explanations are assessor-only.

## Security scope

Target: original local OwTenantArena and Oaf adapters with synthetic integers.
Environment: isolated hosted process and its unit-test harness, no network
listener, credential, device or external service. Attacker controls request
bytes and numeric original offset/count/value for tenant0, not native pointers
or trusted parent storage. Defect: wrapped extent admission permits a logical
cross-tenant write. Exposing transition: UINT_MAX-3 plus8 wraps and masked stores
modify tenant1 canaries. Harmless success: exact canary99-to7 (or the driver's
distinct indexed canaries) changes only inside the teaching array. Exploitability
requires the specified unsigned width, masking and logical partition; it is
not arbitrary code execution, privilege escalation or an ISO out-of-bounds
store. Repair: subtraction bounds and authority validation before indexing;
regressions include forged rights/tenant/epoch, nested attenuation and empty
requests. Residual limits: arbitrary same-process writes, invalid pointers,
concurrency without synchronization and caller impersonation outside parent
selection remain outside this faÃ§ade. Sanitizers enforce neither authorization
nor language-level borrowing.

## Thirteen cost and enforcement dimensions

1. Representation: one16-cell arena, root/view metadata, fixed16-byte wire;
   report actual sizeof and padding, not a packed-ABI assumption.
2. Allocation: no explicit or callback allocation; inspect each selected
   object's imports and do not infer a whole-process allocator claim.
3. Copy/move: wire encoding copies16octets; read stages up to8cells. Root/view
   lifetime restrictions are programmer obligations, not automatically owned.
4. Locality: two logical tenants share one contiguous array; there is no MMU
   boundary or measured cache-traffic guarantee.
5. Dispatch: ordinary calls and branches, no virtual or callback dispatch.
6. Work: validation O(1), read/write O(count<=8), revoke O(1).
7. Failure: no partial publication; no rollback of external effects is needed
   because the core has none. Finite epoch retirement is a refusal.
8. Concurrency: externally serialized; no atomic/lock-free or scheduler claim.
9. Code/stack: retain actual compiler object, disassembly, sizes and stack-use
   records for both compilers and optimization modes.
10. Latency: seven process-CPU samples over10000 equivalent eight-cell wire
    write/read cycles; checksum490000. No physical deadline inference.
11. Build: record cold and unchanged build timings, with exact dependencies.
12. Enforcement: C types/diagnostics validate ordinary declarations only;
    runtime checks enforce numeric bounds/rights under trusted-parent/lifetime
    premises; tests and bounded models falsify selected claims.
13. Composition: explicit reuse of the actual original ownership source;
    wire syntax validation and authorization are separate. Native imports,
    serialization and later dispatch capabilities do not add authentication.

## Validation boundaries

Authored implementation and finite campaigns are recorded by matching source
hashes. No physical target, independent human review, learner mastery or proof
of all states is implied. N1570 unsigned-arithmetic passages are reused within
their recorded C11-draft limits; the exact published C17 text is still an access
gap. The C17 execution profile is not relabeled standards inspection.
