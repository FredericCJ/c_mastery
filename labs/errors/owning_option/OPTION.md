# Required T65.01 tagged and owning optional-value investigation

This completes original X-T65-001 R1-R4, C08 and X-T65-001-DELAYED. Keep the
early scalar lesson before T66. Return to the owning stage after completing
X-T66-009: it explicitly imports that actual ow_allocator implementation and
accepted V4Allocator declaration. This is a later consumer stage, not a reverse
prerequisite that makes the original T65/T66 sequence cyclic. Supply the completed
ownership workspace; do not copy an assessor implementation into learner sources.
Implement src/owning_option.c and retain your actual er_optional_pack/unpack and
result constructors. No optional-value or ownership library supplies the mechanism.

Use hosted ISO C17 with exact uint32_t/int32_t and GCC/Clang. OwAllocator provides
eight actual typed int32_t[16] arrays; this exercise requests one cell per resource.
Host Python/Rust/compiler tools are verification dependencies. The Arm comparison
compiles only actual option_types.h layouts; it does not run or link this hosted
family. Native errors.h includes setjmp.h, and its absence from the available Arm
sysroot is not disguised by a fake header or a claim of target result execution.

## Representation, cardinality and access

The retained zero-reserved encoding has2^32 states: absence and2^32-1 nonzero
values. An unrestricted optional uint32 needs2^32+1 states. EopScalar has an
explicit unsigned tag, NONE0 or SOME1, and a uint32 value. make(value,present,out)
requires present0/1, stores canonical value0 for NONE and the complete value for
SOME, including0 and UINT32_MAX. Invalid arguments leave output unchanged.
get validates the tag before selecting its payload, returns ABSENT unchanged for
NONE, and publishes the value only for SOME. Do not use padding as an assumed tag.

eop_result_get accepts the actual er_result representation. Unknown tags or null
required outputs return INVALID without reading a union member or changing either
output. ER_OK publishes only value; every valid non-OK tag publishes only offset
and returns EOP_ABSENT to identify the error branch. The caller still has the
original result tag; this accessor does not convert all error causes into one
equivalent error value. Well-formed tag/payload consistency is a caller premise.

All pointers denote live objects with declared extent/alignment. Outputs, owners,
allocator state and input controls do not overlap. Calls are externally serialized.
Public C structs can be copied and forged; checked tags cannot validate arbitrary
pointer provenance or infer unique ownership. Never read inactive union members
or compare native padding to establish semantic equality.

## Owning option and failure semantics

EopOwner begins zero-initialized in fresh nonowning storage. NONE owns nothing;
SOME owns one distinct live int32 cell, its arbitrary uint32 resource ID (zero
is valid), and the exact V4Allocator needed to release that cell. The allocator
context remains live until drop. Its take/release callbacks obey T66's no-reentry,
no-nonlocal-exit, nonfailing-release contract. A release observer may inspect the
owner's tag but must not mutate/reenter/destroy its control or retain the cell.

create requires an empty destination and valid callbacks. An occupied destination
returns BUSY before allocating. It requests one cell, initializes its value and
publishes the complete SOME state only after success. A null allocation returns
NOMEM with destination unchanged. The resource ID is independent of pointer
address or pool slot, and ID0 does not mean absence.

clone requires distinct source/destination and an empty destination. Cloning NONE
does not allocate; cloning SOME requests and copies a new cell through the same
allocator. Failure preserves both owners and all source values. A successful
clone owns a different cell, so later writes to one cell do not change the other.
The caller may access its own unique cell, but may not retain that access across
move/drop or manufacture another owner by raw assignment.

move requires distinct valid owners and an empty destination. It transfers the
whole state, then empties the source. Moving NONE succeeds without effects;
self-move is INVALID and occupied destination is BUSY. No allocator callback runs.
read returns copied ID/value only for SOME, otherwise preserves both outputs.
drop caches the cell/allocator, empties the owner before invoking release once,
and is idempotent for NONE. There is no implicit destructor in ISO C. A plain
struct assignment may duplicate a pointer and compile; do not consume both copies
as if that established two allocations. The deliberate shallow-clone control
is rejected by pointer/owner accounting before any duplicate release experiment.

Use actual OwAllocator fail_at1 for construction and fail_at3 after one failed
construction plus a successful source construction to reject cloning. Verify
unchanged destination, live source, attempted-call count, release count, zero
invalid releases and final live0. At SIZE_MAX attempts, refusal must leave the
owner empty and report the allocator's exhausted counter. Fixed backing arrays
make poison/accounting observable, not a memory-safety enforcement mechanism.

Concrete trace: create(id0,value-7) owns one cell; clone owns a second; changing
the clone to20 leaves source-7; moving clone empties it; dropping source and moved
releases exactly two cells. A release observer sees the owner already NONE. The
independent state model tracks first-free backing indices as well as logical
values, so shallow copying cannot pass by returning equal payloads.

## Language comparison and C08

Read the registered Rust Option representation passage. Its documented layout
guarantees apply to listed types and conditions, including NonZero integers,
sized references and the specified Box allocator. They are not promises for
every T, nor an ABI identity with EopOwner or er_result. The actual Rust2021
experiment checks selected listed sizes/alignments, moves an Option containing
ID0, takes it to NONE and observes exactly one Drop. A separate moved-value
translation is rejected. C's copying permission and callback protocol remain
different even if one measured size happens to coincide. Report documentation
version and compiler version separately; no invalid transmute is executed.

Void is an incomplete type with no values in the inspected C11 draft; a struct
member needs a complete object type. Compile the invalid void member and attempted
assignment of a void function result to EopUnit under strict C17 diagnostics.
Also compile a falsely returning _Noreturn function as an expected diagnostic,
without linking/running it. _Noreturn is a function specifier, not a bottom type
that supplies a payload or a general coercion. A named one-state EopUnit enum
models one admitted protocol state, not an uninhabited language type. C still
allows representable casts to other enum values, which eop_unit rejects at runtime.

Measure actual native sizes/alignments and compile the same option types for
32-bit Cortex-M4. Inspect the emitted constants/object metadata; call that a
compiler layout observation, not target execution or a hosted32-bit campaign.
The entire result type remains native-only in this comparison because the
available Arm sysroot lacks its hosted nonlocal-jump header.

## Deliverables, verification and assessor criteria

- D1/R1: derive both cardinalities and admitted tagged states; distinguish
  representation bytes, resource ID, allocation identity and ownership.
- D2/R2: implement total scalar/result accessors and create/clone/move/read/drop
  with unchanged failures, exact active member and explicit caller preconditions.
- D3/R3: compare all8-bit payloads plus UINT32_MAX at present0/1/2 against an
  independent model; test unknown tags and exactly preserved output canaries.
  Compare both real data-model layouts and reject the retained niche controls.
- D4/R4: implement the zero-ID owning stage with actual T66 allocation failures,
  deep clone and moved-from empty state; execute C08 and Rust comparisons and
  explain counterexamples to language/ABI/move-only equivalence. Complete all13
  cost dimensions and the independently attempted delayed variant.

Run run_errors_owning_option.py, verify_errors_owning_option.py,
run_errors_option_investigation.py and check_errors_owning_option.py --self-test.
Four anchors must pass across GCC/Clang debug/release/ASan+UBSan, ten compiled
controls must fail ordinary ER_CHECK assertions, and four compiling learner stubs
must reject. The independent owner model enumerates three-operation histories
over12 operations at three allocation schedules, plus longer clone/move/reuse
traces. It tracks values, backing indices, attempts, live cells and releases;
it is not a proof of arbitrary C lifetimes. Preserve raw logs/tool/source pins.

Public tests alone earn at most level1. Level2 requires each explicit criterion
and failure accounting; level3 requires a fresh changed-condition attempt at
least seven days later, without the old solution and with assistance recorded.
Hints: (1) count absence separately; (2) separate a resource ID from its cell;
(3) clone allocates, move transfers, drop empties before release. Private analysis
and a sealed variant are kept under assessor/, outside learner exports.

## Costs, enforcement and scope

| Dimension | Required evidence |
| --- | --- |
| Representation | Actual scalar/owner/result size and alignment; native and actual32-bit option layout constants. |
| Reachable storage | Inline tag/allocator plus one int32 cell per SOME; actual bounded8-slot backend. |
| Lifetime | Allocator context outlives owner; borrowed cell access ends at move/drop. |
| Mutation/aliasing | Unique-owner premise, disjoint controls, clone-distinct storage; plain struct copy remains possible. |
| Allocation | Exact real T66 attempts, failure schedule and releases; scalar operation makes no allocator calls. |
| Movement/copying | Full scalar copy, one-cell deep clone, fixed-size ownership transfer and emptied source. |
| Destruction | One nonfailing release per SOME; tag empty before observer; no automatic C destructor. |
| Dispatch | Actual typed allocator callbacks and retained result/optional calls; inspect emitted object imports. |
| Concurrency | External serialization only; no shared reference count or concurrent drop guarantee. |
| Failure | Invalid/occupied/admission and allocation failure preserve ownership; no partial clone escapes. |
| Compilation | C invalid-member/value/Noreturn probes, actual Rust moved-value refusal, clean/unchanged builds. |
| Binary/stack | Six-profile code/stack/object reports; Arm layout compile is not a link/runtime audit. |
| Boundary | No C/Rust ABI equivalence or native serialization; exact published C17/C23 still inaccessible. |

All operations are O(1) except the supplied allocator's bounded8-slot search.
Cloning copies one cell; drop's bound assumes the specified callback. Seven10000
iteration samples separately measure scalar construction and owning create/drop
with actual allocation and cleanup. Their contracts differ: do not label their
ratio abstraction overhead. Include checks/observer overhead, zero separate
warmup, resolution/noise, raw CPU times and allocation counts. The scalar series
may be optimized heavily; emitted code is evidence, not a universal speed claim.

Only original local code and synthetic resources are targets. No credentials,
external service or operational hardware is involved. A duplicated-owner mutant
shows a protocol defect, not arbitrary code execution. Author execution, source
review, independent learner work and physical evidence remain separate statuses.
