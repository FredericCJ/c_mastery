# Dependent validation, nested paths and publication

This required X-T65-005 follow-through supplies original R1-R4 and the delayed
variant, after X-T65-004. Bring the actual `er_validate` implementation and its
bytes.checked reasoning artifact. C17 hosted Linux, exact uint32_t/int32_t
representations, GCC/Clang debug/optimized/ASan+UBSan; no library implements the
validation mechanism. Host Python unbounded integers are an oracle, not a target
dependency. The retained scalar API and historical accepted identities remain.

## Input, representation and exact contract

`struct evv_input` holds count0..3, three initialized er_config objects and a uint32_t
limit. All three objects, including unused entries, contain initialized int
values: a successful publication copies the complete value. `root` is a nonzero
numeric path token. `struct evv_diagnostic` has field/error ID, depth3 and the exact
path `[root, zero_based_item_index, field]`. Fields1/2/3 name age/quota/denominator;
field4 names the ratio constraint. Each complete diagnostic occupies one slot;
there are no partial paths, strings, allocation or hidden formatting.

Pointers name live aligned objects with declared extent. Input, output and
diagnostic backing are disjoint; shared access requires external serialization.
Control objects are ordinary values, not capability proofs. C cannot validate
dangling pointers, races or caller overlap. There are no callbacks or retained
borrows. Capacity is0..9; capacity0 permits a null diagnostic pointer. Null
input/output, root0, count>3, capacity>9 or null nonzero-capacity diagnostics are
structural misuse: return `{total=SIZE_MAX,stored=0,truncated=0,ratio_checks=0}`
without modifying any output or inspecting item values beyond structural checks.

First visit each item's independent age0..150, quota0..1024 and denominator1..4096
checks in item order, then field order. Accumulate every error. In a second
phase visit each item whose quota **and denominator** passed. Its age need not
have passed; neither do unrelated items' fields. Increment ratio_checks exactly
once for each admitted pair. Require the *exact mathematical rational*
quota/denominator <= limit, not truncated integer division. For q,d admitted,
write q=kd+r with0<=r<d. Reject k>limit or k==limit and r!=0. This avoids limit*d
overflow and never divides by an invalid denominator. Limits0 and UINT32_MAX
are meaningful. The Python oracle independently compares q to limit*d using
unbounded arithmetic.

Append each ratio error after **all** independent errors, in item order. The
diagnostic output is the first min(total,capacity) complete records. Stored and
total are separate; truncated means stored<total even at capacity0. Preserve
the remaining diagnostic slots. Only total0 publishes the complete EvvInput.
An empty batch is valid and publishes count0; truncation never makes invalid
input valid. The maximum total is9: per item, either at most3 independent errors
or valid quota/denominator with at most age+ratio errors. No count can overflow.

## Concrete implementation and experiments

Retained scalar boundary input sets are age[-1,0,1,150,151], quota[-1,0,1024,1025],
denominator[0,1,4096,4097], across every capacity0..3 plus structural capacity4.
Compare exact IDs, count/truncation and config/diagnostic canaries against the
independent predicate list; execute actual ER_MUTANT9/10 controls separately.

For the nested model, root42, limit1 and items(-1,3,2),(151,1025,0),(0,4,3)
produce six ordered errors: `(item0,age)`, `(item1,age)`, `(item1,quota)`,
`(item1,denominator)`, `(item0,ratio)`, `(item2,ratio)`. Exactly two ratio checks
run. Capacity2 returns the first two complete paths, total6 and truncated1,
preserving the old configuration. Input(0,3,2), limit1 fails the rational check
although integer division yields1. Input(-1,3,2) reports age then ratio; an invalid
age does not suppress an otherwise admitted dependent check. Quota1024 with
denominator4096 and limitUINT32_MAX is valid without an overflowing product.

Implement `evv_validate` from the header. Submit D1 validation graph/invariants,
D2 source and failure contract, D3 independent model/metamorphic prefix checks
and exact commands, D4 the nested delayed case. The oracle combines all original
boundary representatives, signed extrema, exact-fraction cases, one/two/three
item groups, empty and malformed counts, root changes and capacities0..10.
It calculates full diagnostics before truncation independently of the C phases.
Changing capacity must preserve total/validity/ratio_checks and expose a prefix
of the same complete ordered list. Changing root changes only the path prefix.

Named anchors separately assess retained validation, ratio admission, nested
ordering/truncation, structural refusal and adversarial publication. Seven new
compiled controls publish early, stop after the first error, round the ratio,
gate all ratios on unrelated errors, erase item indices, hide truncation or
accept invalid capacity. Each must fail a normal ER_CHECK assertion; a divide
trap, sanitizer abort or compile error is not semantic-control success. The
unsolved core and retained scalar stubs must compile and fail the anchors.

## Reasoning, comparison and assessor rubric

Derive the maximum error count, exact predicate domain and two-phase order.
Explain why a skipped dependent check is not a successful check and why an empty
diagnostic buffer is not an empty error set. ISO C enforces declared member and
pointer types; runtime numeric admission enforces this bounded model. Neither
provides refinement types, exhaustive validation or automatic immutable records.

The inspected Cats Validated passages distinguish accumulating independent
errors from sequential admission through andThen. Its discussion shows why
combining accumulating application with short-circuit flatMap can violate a
consistency law. This C contract chooses deterministic bounded diagnostics and
does not claim a general Cats implementation, Scala type-class enforcement,
parallel execution or a proved arbitrary applicative instance. No Scala compiler
execution is claimed; this comparison is source analysis and original C tests.

R1 assesses independent/dependent admission and truncation reasoning. R2 assesses
exact bounded paths, structural misuse and complete publication. R3 requires the
independent ordinary grid, canaries and two retained compiled controls. R4 requires
the exact rational predicate, nested paths and capacity/root transfer. Level1 is
nominal behavior, level2 a justified complete bounded implementation with actual
evidence, level3 an unaided changed-condition argument and counterexample. Every
criterion is separate; premature publication or an unexplained skipped predicate
fails regardless of other scores. Attempt the held-out delayed input at least
seven days later and report assistance. Author execution cannot award that score.

Graduated hints: list all independent predicates; draw prerequisite edges for
the ratio; separate the full diagnostic sequence from its stored prefix; use
quotient and remainder for exact rational comparison. Assessor solutions are
separately packaged under assessor/errors/validation.

## Security scope and containment

Target: this original local in-memory configuration program; environment:
unprivileged Linux process with synthetic data. The attacker chooses field
values and batch content, not pointers, memory or scheduling. Isolation is the
process/test workspace with no sockets, files, credentials or deployed target.
The defect is publication before complete validation; the protected property
is that live configuration changes only for admitted input. Transition from
the canary configuration to(-1,1025,0) with capacity0 exposes the flawed version;
observable harmless success is an unauthorized synthetic config change. This
requires the flawed path and attacker-controlled fields; it establishes neither
code execution nor privilege escalation. Repair stages publication after all
admitted checks. Regression checks old fields unchanged on every error and a
valid replacement positive control. Residual limits are live/disjoint storage,
external serialization and the defined validation policy, not universal security.

## Cost review and limits (thirteen dimensions)

1. Representation: report actual input/diagnostic/summary size/alignment; no
   serialized or padding-byte equivalence.
2. Work: at most9 independent checks and3 ratio checks, plus at most9 emissions.
3. Space: three quota/denominator flags and staged summary; caller error storage
   is0..9 records; no recursive traversal or allocation.
4. Allocation: no allocator in this core; inspect emitted imports separately
   from harness or runtime allocations.
5. Memory traffic: input field reads, two small admission arrays, complete-path
   diagnostic writes and one complete config copy only when valid.
6. Dispatch: no callbacks or virtual dispatch; tag/condition branches select
   predicates, publication and capacity.
7. Code size: retain object disassembly and executable sizes for each profile.
8. Stack: retain compiler stack-usage observations; callbacks are absent, but
   ABI/compiler changes require renewed evidence.
9. Latency/throughput: seven samples of10000 identical three-item invalid batches,
   six diagnostics/two ratio checks each, checksum980000; no arbitrary validation
   workload or cross-language overhead claim.
10. Failure: structural misuse modifies nothing; semantic failure may publish
    only its bounded diagnostic prefix, never partial configuration.
11. Contention/scheduling: externally serialized synchronous work; no parallel
    validation, lock-freedom or real-time scheduling claim.
12. Build: clean/unchanged build timings, actual compiler dependency closure and
    source/log hashes; no fabricated cache or incremental result.
13. Portability/transfer: exact unsigned/signed widths, size_t sentinel, output
    layout and future larger bounds require revisiting arithmetic and models.

Source-bound author receipts, independent learner assessment, external review
and physical observation remain separate. C17/C23 published normative text is
inaccessible; inspected drafts are not relabeled. Finite tests falsify defects
in the stated model and do not establish absolute mastery.
