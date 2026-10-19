# Bit contract assessor analysis

Reference/provider code stays private. `bits.c` uses the actual completed integer
cell codec; `oracle.c` uses bit lists, remainder/division by2 and per-bit writes.
Public tests, native oracle, intrinsic comparison, layout observation, plugin
execution and M4 closure support different claims. The ordinary nine task IDs and
original delayed IDs are mapped separately in objective_mapping.json.

## t0405a

Logical width16 admits counts0..15; reject-1 and16 unchanged before evaluation.
The carrier has at least32 value bits; casting1 to FbWord before shifts avoids
16-bit signed-int1<<15. Mask input before right shift/rotation, not only afterward:
0x1234 at width8 right4 is3; an unmasked input incorrectly gives0x23 after masking.
Rotation left4 is0x43; width16 rotation gives0x2341. Zero rotation returns masked
input. The general full-carrier idiom `x>>(carrier_width-count)` is UB for zero;
the reference selects zero first. Its logical range is deliberately1..16, not
arbitrary full-width arithmetic. Every intermediate left shift is within the
at-least32-bit carrier after masking16bits.

## t0405b

Field mask has high-low bits; empty mask0, full logical16 mask65535, both safe in
carrier. Reject value greater than that mask. Preserve all bits outside field
inside logical width, and remove high carrier bits. Insert value10 at[4,8) into
0x1234 gives0x12a4. At width8, first[0,4)=15 then[2,6)=0 applied to0xa0 yields0x83.
Simultaneous assignments disagree on overlapping bits2/3 and return CONFLICT
without publication. Setting second value3 agrees on overlap, giving0x8f. A
different simultaneous policy could give priority, but that would be a different
contract, not a proof of this implementation. Both edits are validated before
output. An independently valid nonoverlapping permutation commutes; overlapping
sequential updates need not. Oracle checks every4-bit word and admitted pair of
fields/values via per-bit assignment,51984 combinations.

## t0405c

0x1234 has5 ones, highest bit12, bit-width13 and16-bit leading count3. Zero logical
value has0 population,0 width and exactly logical-width leading zeros. Clear
out-of-domain bits before checking zero. GCC clz(0) has no defined result;
`intrinsics.c` bypasses it. Native unsigned carrier32 clz is adjusted by32-width;
static capability/range checks are part of that backend, not ISO portability.
The zero-policy mutant remains defined C and is rejected at width1,value0.
Actual1,048,576 width/value comparisons provide bounded equivalence to the loop;
the independent bit-list oracle separately checks stats2048 cases, so equality of
two copies of an error is not the only evidence. Intrinsic runtime helpers must
be audited before any strict target adoption; this comparison is host-side.

## t0406a

BIG octets18,52,86,120 and LITTLE reverse them; the inverse sums octet_i*256^j in
an unsigned at-least32-bit value. Do not promote a signed char into a high shift.
Reference reorders cells around actual `fi_encode_cells`/`fi_decode_cells`; it
does not invent a duplicate prerequisite. Explicit CHAR_BIT8 capability applies
to these spans; PR-T04-01's wider-character model is a separate adaptation.

## t0406b

Character indexing never requires uint32 alignment. The split decoder first
checks `a.size<4 && b.size<4-a.size`, avoiding overflow of length sum. For each
index0..3 it chooses a only when index<a.size, otherwise b at index-a.size.
It never subtracts or orders pointers from distinct arrays. Null empty spans
are allowed and never indexed. Required lengths still rely on the caller's live
extent guarantee. Short input must not touch output: the early-publication
mutant exposes this independently of endian correctness. Host sanitizers, exact
allocated plugin extents and QEMU checks supplement the source proof.

## t0406c

Layout experiment writes actual typed objects, reads their representation through
unsigned characters, and records padding as observation only. Reordering the
declaration changes member offsets/meaning even if a particular size remains
equal. A declared packed object populated from known bytes has a host-endian
interpretation: on the recorded little-endian host bytes18,52,86,120 interpreted
as a uint32 yield2018915346, whereas BIG decoding yields305419896. Packed removes
some padding; it supplies neither network endian nor general lifetime/effective
type/bounds permission for a forged overlay. The forged-char-buffer cast remains
compile-only; no crash or convenient value is used as its correctness oracle.

The header-only unknown record `{2,255,0,4}` can be skipped without any payload
read. Reject policy yields UNSUPPORTED unchanged. Skip yields SKIPPED, zero
value/flags and consumed4. Unknown flag in version1 uses the same unknown policy.
Declared8 with only4 available rejects SHORT even under skip. Known version1
must size8 and use only bit0; policy cannot excuse malformed known records.
This is parser extensibility, not an integrity or authentication mechanism.

## t0407a

Fields unsigned-int3/5/9 store values5/17/257. N1570 allows implementation-defined
allocation direction and unit crossing, unspecified unit alignment, and specified
constraints on field types. Do not take a bit-field address or infer an offsetof.
Observe entire object bytes/size/alignment under actual compiler/target/options.
The named GCC/Clang native profiles show a different underlying unsigned-long
container and pragma-pack1 layout; this is not a portable expected byte string.
Padding after member stores may be unspecified even after an initial memset.
The native manifest compares actual sizes; each exact observation remains pinned
to compiler version, optimization and packing options. Rebuild both consumer
and producer before changing a shared C struct ABI, or use explicit wire fields.

## t0407b

Initial0xa5: read returns0xa5; writing `(read&~15)|3` sends status ones0xa0,
clearing both high statuses and leaves0x03. Correct no-read update(low3,clear0)
writes0x03 and leaves state0xa3. Update(low4,clear0x20) leaves0x84. With read-to-clear
enabled, read returns0xa5 and leaves0x05; a second read returns5. Merely logging
via a read would destroy pending status. Correct update still leaves0xa3 with
reads0. Explicit status injection/interleaving would need a separately stated
atomic hardware contract; the sequential synthetic model does not prove it.
The defined faulty RMW branch is compiled and must fail the no-unrequested-clear
assertion. Volatile alone would not change this state-machine defect.

## t0407c

Distinguish C object ABI manifest from wire schema. The former records actual
sizes, alignments, member order/offsets, type widths, compiler target/options and
endian; the latter fixes octet offsets independent of those native layouts.
Consumer can validate available prefix, declared size, supported version,
required field lengths and feature mask. It cannot validate arbitrary producer
pointer liveness or machine calling convention by reading a version number.

The actual shared-library experiment builds eight separately compiled providers
(GCC/Clang×four versions), loads them through normal ELF dependency binding into
sixteen independently compiled consumers, and allocates exactly the queried
descriptor length. Version1 genuinely supplies8 bytes, version2 supplies12.
Unknown-version3 supplies only4 and is rejected without reading features; the
fourth provider claims12 in an8-byte object and is rejected before trailing read.
The interface uses a typed exported C function, not a void-pointer/function-pointer
cast or a shared packed struct. Host C ABI remains a prerequisite. Only one
entry is exported; symbol and dynamic dependency inspections are recorded.
ASan/UBSan and sentinel checks run in consumers. Private payloads remain
synthetic; a plugin is executable code trusted at the loader boundary, so size
negotiation is not a sandbox. The version1 fabricated-tail mutant returns999
without UB and must be rejected as a semantic failure.

## Rubric and transfer

Apply the public0–4 rubric per ordinary/delayed criterion. Require explicit proofs
for masked shifts and split-object indexing, failure-atomic publication, unknown
record semantics, stateful read/write behavior and ABI enforcement boundaries.
Reject “packed means portable”, “a register read is harmless”, “clz zero is32
everywhere”, “byte pointers make any typed cast valid”, and “version validates
plugin ABI”. Valid alternate implementations may use repeated per-bit loops,
different mask construction or staged byte cursors with the same contracts.
Assess their costs and real generated runtime closure separately.

`oracle.c` independently checks2048 stats,39936 shift outcomes,548864 field
insertions and51984 edit-pairs per profile. Native object and four direct M4
image audits include actual integer provider and original helper object. The
stack estimate sums live nonrecursive C frames plus1024 for this IRQ-masked
driver only. It is not a physical watermark or an arbitrary callback bound.
No physical MMIO, external review or independent learner evidence is supplied
by these author runs. Preserve all original delayed minimum_days=7 records.
