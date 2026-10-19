# PR-T46-01: values, images, and changed representation rules

This package completes the three original criteria and their unchanged delayed
variants. Families A/B/C/D/F/I/J apply: representation reasoning, construction,
malformed boundaries, diagnosis, falsification, target changes and transfer.
Prerequisites are T03 checked integer conversions, T04 live byte extents, T07
object/link inspection, and the learner's CVM1 raw codec. The optional section
exercise uses that codec; it does not require the later compiler or verifier.
Author comparisons name the separate `assessor/runtime/cvm.c` reference explicitly.

## 01a: meaning before storage

Prepare a table for INT0, BOOLfalse, NIL and REF(slot0,generation1), including
which fields have meaning, equality and permitted operations. Equal payload bits
are insufficient to establish equal guest values. A generation distinguishes
slot reuse within one VM lifetime; it neither validates a native pointer nor
distinguishes resets of separate VM instances. Reference equality need not
dereference the referent. Unknown tags and malformed Boolean/reference payloads
are rejected by the successor import boundary. Import normalizes ignored fields.
Write `rv_import`, `rv_equal`, and a counterexample to comparing entire structs.
For example, an integer's ignored generation field may differ while its value
does not. Padding is another reason object bytes are unsuitable for this test.

Nystrom's inspected *Types of Values* narrative distinguishes the tag from its
payload, checks the guest type before arithmetic, and explains why struct byte
comparison can disagree with value equality. Its Lox representation and numeric
semantics are different from this original checked-integer design. The raw
manuscript's code-inclusion markers are not inspected implementation listings.
[Assigned primary passage](https://raw.githubusercontent.com/munificent/craftinginterpreters/master/book/types-of-values.md).

For the original changed condition, implement the separately versioned numeric
API in `representation.h`. Q16 stores the mathematical number `payload/65536` in
a signed int32 payload. INT/INT arithmetic keeps checked int32 semantics. If
either operand is Q16, first convert both operands exactly to representable Q16;
an INT outside -32768..32767 fails this admission even if subsequent cancellation
would fit. Add/subtract preserve the scale; multiply divides the product by65536;
divide multiplies the numerator by65536 before dividing. Both rounding operations
truncate toward zero. Divide by zero and an out-of-range result fail unchanged.
An int64 intermediate suffices because admitted operands are int32 units: their
product is at most2^62 in magnitude, and the scaled numerator at most2^47.

Work through `Q16(-1)*Q16(32768)`: mathematical stored result -1/2 truncates to0,
whereas an arithmetic right shift would commonly produce -1. Mixed arithmetic
does not imply coercing equality: INT1 and Q16(65536) remain unequal under this
tag-strict equality contract. BOOLtrue is not an arithmetic integer. Supply a
rule matrix, valid neighbors beside refusals, and a rational-number oracle.
No CVM1 opcode, compiler grammar or accepted image changes. This is an explicit
API-level changed-condition numeric mechanism, not a claim that old bytecode
can execute a new tag. A later opcode integration must update every consumer.

ISO C17 does not enforce correspondence between tag and payload. The checks,
private invariants and caller ownership do that work; a language with exhaustive
algebraic data-type matching may diagnose omissions that this C API cannot.
Successful tests do not make forged structs impossible. C23's changes to signed
representation do not supply the application's checked-overflow, scaling, tag,
ownership or version rules; retain the C17 build and explicit byte conversions.

## 01b: bytes independent of the machine

Derive this complete CVM1 container without serializing a C struct:

```
43 56 4d 31 01 00 00 06 00 00 00 01 2a 00 00 00 11
```

It has one contiguous six-byte function, no arguments/locals, INT42 then RET.
INT32_MIN has immediate bytes `00 00 00 80`. The raw codec may carry opcode255;
it must not claim instruction validity. Derive every truncation verdict and
retain a byte-for-byte canary destination and written-length sentinel. A
failed encoder must not leave a header prefix in the destination. Decode and
encode reference inputs only while their complete admitted spans remain live.

For the delayed changed data model, run the same literal bytes on a32-bit
little-endian Cortex-M4 and64-bit big-endian PowerPC64 ELFv2. Inspect ELF headers,
`sizeof(size_t)`, object imports and exact results. Eight-bit bytes and exact
int32/uint32 are explicit admission requirements; this is not a proof for all
C implementations. The supplied M4 builder links only named original objects,
including the T11 unsigned-limb division provider and a signed ARM ABI adapter.
Do not copy the historical Thumb runner's `-lgcc` link into the strict profile.
Its old receipt remains historical. A successful host run, cross-build and
actual emulator execution are distinct evidence dimensions.

## 01c: structure, evolution, and atomic publication

For CVM1, independently corrupt magic, count, descriptor gap/overlap, region
length, arity/local bound and oversized length. State the first rejecting phase;
do not substitute an instruction-verifier error for a malformed container.
The supplied tests keep the accepted raw unknown-opcode example next to these
negative cases. No portable expected execution result is assigned to malformed C.

Implement the exact RVE2 envelope in the header. A nine-byte header carries
version, zero flags, explicit CVM1 length and at most four sections. Each section
has type, required flag, little-endian length and at most64 bytes. Type1 is a
32-byte digest; type2 is a1..31 printable ASCII name; known duplicates reject.
An unknown optional section skips after bounds checking, an unknown required
section refuses, and reserved flags/trailing bytes refuse. The digest is metadata,
not a signature or authenticated proof of source identity. The old raw CVM1
reader rejects RVE2; silent reinterpretation would be a versioning defect.

Derive the maximum input:9+1077+4*(4+64)=1358 bytes. Check total size before any
read for oversized claims, then use `remaining >= required` before advancing
each offset. Stage the decoded image locally and publish only after the last
section and exact end check. Encoding is canonical (digest then name); an image
with skipped unknown sections cannot be re-encoded without an explicit loss
policy, so the supplied encoder rejects it. Decoding never grants execution
permission; use the original verifier before a subsequent run.

## Deliverables, checks, and progression

Submit `learner.c`, the completed original CVM raw codec, the representation/rule
tables, byte derivations, error-phase map, complexity and live-storage analysis,
and criterion-indexed evidence. All functions use fixed caller-owned storage,
no allocation or hidden global state, and no output publication on refusal.
Output may alias by-value arithmetic inputs; codec source/destination extents
must be disjoint. The encoder's temporary1358-byte buffer and decoder's staged
image count toward stack cost. Work is linear in admitted image bytes and fixed
numeric operations; target division cost is128 limb iterations, not one cycle.

Run `python run.py --cvm your_cvm.c --source learner.c --build build/rv` from
the exported package, adjusting paths. `target_build.py --help` lists every
explicit original runtime provider. Host Python Fraction oracles and sanitizers
are verification tools, not firmware dependencies. Reject missing providers;
an unfinished learner stub must compile and fail a semantic assertion. Inspect
the actual target objects, final image and stack reports, not only source flags.

Graduate hints: first list meaningful fields; then write an exact unit/scale
equation before C arithmetic; finally draw the decoder's local staging object
and its sole publication point. Keep initial predictions before consulting a
hint. The separately packaged assessor analysis and mutations are withheld.

Rubric per original criterion: level0 for a violated contract or false target
claim; level1 for matching supplied output without the derivation; level2 for
the complete implementation, independent expected results, boundary diagnosis
and a fresh falsifier; level3 requires an independently assessed changed rule
after at least seven days. For01a assess numeric admission/rounding as well as
tags;01b assess actual data-model/endian evidence;01c assess unknown-required,
partial publication and version boundaries. Author runs cannot award level3.

Malformed images target only these original local programs using synthetic
metadata. Harmless success for the intentionally flawed variants is an incorrect
local value, accepted required section or changed canary. Do not equate this
with arbitrary code execution. Repair the specific check/publication ordering,
keep regression fixtures, and state that metadata remains unauthenticated.
No board flashing, external targets, physical measurements or learner attempt
is implied by this package. Exact campaign results live in the linked author
support; unavailable independent and physical evidence remains pending.
