# Assessor analysis for X-T11-002

Keep this file out of learner exports. The reference has no maximum limb count;
only the host driver chooses bounded arrays. Reject an implementation replacing
the runtime count with128bits or converting the whole value to uint64_t.

Prefix addition satisfies out_low+carry*B^k=x_low+y_low. Subtraction uses unsigned
modular subtraction masked to one digit and propagates whether xdigit<ydigit+borrow.
Since B divides2^32, masking the unsigned wrapped subtraction gives the intended
digit for both supported radices. Multiplication admits the exact worst-case
accumulator B^2-1; using a signed32 temporary is a real defect at16bits.

Binary restoring division appends each input bit to the consumed prefix. If
old_r<y, next_r=2*old_r+bit<2*y. Thus one comparison and at most one subtraction
give next_r<y; that decision is the next quotient bit. A high remainder carry is
stored in an actual extra limb. This simpler proof trades up to BL_BITS iterations
per dividend limb against the inspected normalized quotient-digit algorithm;
do not call it HAC14.20 or infer that algorithm's two-correction bound applies.

Result lengths are trimmed before capacity admission. In particular q may fit
capacity0 when x<y, while r still needs storage. Both are checked before either
publication. Signed division applies signs after successful magnitude division:
-7/3 gives q=-2,r=-1;7/-3 gives q=-2,r=1. Python floor divmod is therefore not the
signed oracle without explicit conversion to truncation semantics.

Owner replacement allocates before touching old storage and copies while a
self-borrow remains valid. The old pointer becomes invalid only after successful
replacement. A failed allocation leaves the exact old identity usable. The
nonreentrant/nonfailing callback premise is essential; ISO C provides no ownership
type. The test allocator records matching extents and drains a deliberately leaked
mutant allocation before returning the intended BL_CHECK failure, preventing a
sanitizer symptom from being misreported as a semantic control.

Six independent controls omit addition carry, subtraction borrow, final product
carry, equality subtraction in division, dividend remainder sign and release of
the replaced owner. Four arithmetic and two ownership controls run at both radices.
They test distinct mechanisms, not changed constants. Require compiler success,
the exact assertion exit41, and absence of sanitizer reports before crediting a
mutant rejection. The unfinished stubs compile and reject every declared anchor.

Independent Python integers cover unequal runtime lengths and values beyond2048
bits. Their fixed test limit is evidence scope only. A target with a different
size_t or no uint16_t/uint32_t may fail the stated profile; diagnostics do not prove
it supports the same representation. Generated helper inspection is mandatory
even though the source calls no arithmetic library. Storage/time observations
remain separate from physical performance and learner competence.
