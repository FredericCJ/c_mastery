# Owned composition and bounded error conversion

This is required follow-through for X-T65-004/R1-R4 and X-T65-004-DELAYED.
The early scalar stage uses the actual `er_map`/`er_then` contract after
X-T65-003. Return for the owning stage after X-T66-009/PR-T66-09
`memory.adversarial_allocator` and X-T65-001's owning-option stage. Supply the
completed `ownership` workspace explicitly. Reuse the learner's actual
`struct eop_owner` implementation in the errors workspace. This later return does not
create a reverse prerequisite from early T65 to T66. C17 hosted Linux with exact
int32_t/uint32_t, GCC and Clang; no strict freestanding or hardware claim.

## Representation, operations and invariants

`struct erc_result` is EMPTY, VALUE with one live uniquely owned EopOwner, or ERROR with
an `struct erc_error`. The error contains 1..4 causes, oldest/innermost first. Each cause
has nonzero domain and code and an exact size_t offset; SIZE_MAX is meaningful.
No error string, allocator, pointer or padding is part of error equality. Empty
destination controls are zero initialized or previously dropped. Controls and
allocator contexts stay live and at stable addresses; owning controls are not
copied. Source, destination and callback context are disjoint. All operations
are externally serialized. Numeric/tag checks cannot validate dangling pointers,
forged owner pointers or uniqueness.

`erc_adopt` moves a SOME EopOwner into an EMPTY result and empties the source.
Admission errors preserve both. `erc_error` copies a valid bounded error into
an EMPTY result. Occupied destinations return BUSY; malformed inputs INVALID.
`erc_map_clone` borrows its source. ERROR copies without calling the callback
or allocator, even if the callback is null. VALUE requires a callback and first
deep-clones through the source's actual allocator. Allocation failure returns
NOMEM with input/output unchanged and zero map invocations. Only after successful
clone does it call the nonfailing map exactly once, assign the clone's cell and
publish. The input ID/value and ownership remain unchanged. The map receives a
value copy, not ownership; it returns any representable int32_t. Its body must
not overflow signed C arithmetic. Self-map is INVALID. This is an explicitly
allocating, borrowing operation, not Rust's consuming `Result::map`.

`erc_drop` empties the result before calling the actual owner destructor. Empty
and error drops allocate/free nothing. An observer may read the result tag
during its release callback; no callback may mutate, retain a borrowed input,
reenter any participating control, destroy live context or escape nonlocally.
Allocator take/release follow the imported contract and release does not fail.
There is no automatic destructor or unwinding in this C representation.

`erc_cause` constructs a one-cause record. `erc_wrap` validates the complete
input and outer cause before appending. A fourth existing cause returns FULL
without truncation or output modification; invalid inputs also preserve output.
The exact same input/output error object is allowed, otherwise storage must be
disjoint. Inner domains, codes, offsets and order are preserved. This operation
allocates nothing and does not silently replace an inner cause with an outer
code. An error record is not a serialized wire representation.

## Concrete tasks and independent expectations

1. Derive map identity/composition and bind associativity using tag cases and
   an equality relation. First require total pure callbacks on the admitted
   domain. Then refine equality to include callback order, side effects,
   allocation and failure. Sequencing the same effectful callbacks need not
   invalidate associativity, but eager evaluation of an unused branch does.
   On IO(offset7), lazy bind calls zero callbacks; eagerly computing a discarded
   candidate calls one, despite equal returned errors. Do not claim any side
   effect automatically disproves every law.
2. Implement the header from permitted primitives and actual EopOwner/T66
   allocator. On owner(ID0,INT32_MIN), the supplied map yields INT32_MAX in a
   distinct clone; the source remains INT32_MIN. A scheduled second-allocation
   failure preserves the source and empty destination, with zero callbacks.
   Dropping two distinct owners performs two releases, each after EMPTY is
   observable. Do not run a double-free as the test for copying an owner.
3. Use the independent table oracle over payloads0..7: sixteen total tables
   (eight modular additions and eight XORs), seventy-two result tables
   (nine thresholds times eight modular additions), all listed pairs and
   OK/RANGE/IO/unknown-tag inputs. These are exhaustive *within the stated
   function family*, not all 8^8 total functions or arbitrary callbacks.
   Compare values, exact errors, callback counts and input traces. Run bounded
   error-product and scheduled allocator tables separately. Sanitizers do not
   prove universal laws or uniqueness.
4. For delayed transfer, change a scalar error to the four-layer cause product.
   Wrap inner(1,2,SIZE_MAX) with three outer contexts, verify order and full-width
   offset, then reject a fifth cause unchanged. Decide separately whether an
   owning transformation borrows/clones or consumes. Demonstrate why identity
   under numeric equality is weaker than resource/effect equivalence: even an
   identity clone can allocate and fail.

`compose_laws`, `compose_base`, `compose_effects`, `compose_owner`,
`compose_failure`, `compose_cause` are distinct anchors. Every compiled control
must fail its named ER_CHECK assertion, not compilation or sanitizer execution.
Seven owner/cause controls cover alias clone, error callback, leaked destructor,
lost inner offset, silent truncation, callback before failed allocation and
duplicated move ownership. Two controls compile actual ER_MUTANT7/8. The learner
stubs compile and must reject all anchors. A pilot caught a positional allocator
initializer mismatch at compilation; the harness now uses designated fields.
The first full campaign also found a vacuous law-equality anchor: an all-stub
result matched itself. The corrected anchor requires actual successful callback
counts as well as equality; the failed campaign is retained separately.

## Language comparison and enforcement

Inspect Rust Result's map, map_err, and versus and_then passages. The supplied
Rust2021 specimen actually consumes a non-Copy owner and drops once, preserves
an inner cause/offset, and observes eager-one versus lazy-zero. Its negative
translation attempts reuse of the moved result and is never executed. The
installed Rust1.93.1 compiler and rolling1.99.1 documentation are separate
evidence. C compatible function-pointer types get compiler checking; active
union members, unique ownership, callback totality and borrowing discipline do
not get Rust-equivalent enforcement. External tests and runtime guards cover
only their stated inputs. Published C17/C23 access remains unresolved.

## Cost review (all thirteen dimensions)

1. Representation: report sizeof/alignment of result/error/owner and inactive
   union capacity, without padding comparisons or cross-language ABI promises.
2. Work: scalar composition takes at most two callbacks; clone-map has one
   allocation, one cell copy and one callback; cause operations inspect at most4.
3. Space: stack staging is bounded; each successful owner/clone owns one typed
   int32_t cell from the actual bounded allocator, whose control is separate.
4. Allocation: measure failed and successful calls/releases through actual T66;
   errors and wrapping make none. Callback internals need their own audit.
5. Memory traffic: distinguish input-value copy, deep cloned cell, full bounded
   cause copy and pointer/control transfer. No generic memcpy implementation.
6. Dispatch: inspect emitted er_map/er_then and clone-map indirect call sites
   before the companion's timing run; dynamic counts differ on error and OK.
7. Code size: retain executable sizes and actual object disassembly, with
   optimization/compiler recorded; static call sites are not dynamic frequency.
8. Stack: retain actual compiler stack-usage reports, not a platform-wide bound.
9. Latency/throughput: seven samples of10000 complete scalar compositions and
   create/adopt/clone/drop sequences. These have different contracts; never
   divide their timings into a purported abstraction overhead.
10. Failure: callback does not run before failed clone; source stays owned;
    error propagation is allocation-free; disposal is exactly once.
11. Contention/scheduling: externally serialized synchronous model; concurrent
    ownership, cancellation or scheduler guarantees are not applicable here.
12. Build: record clean and unchanged incremental builds, dependencies, compiler
    versions and actual imported producer objects.
13. Portability/transfer: hosted data model changes require renewed size/offset,
    allocator/alignment and code-generation evidence. Rust consuming semantics
    are a changed contract, not a C ABI or automatic-destructor guarantee.

## Assessment, hints and evidence limits

D1 supplies the law derivation and ownership/failure table; D2 the implementation;
D3 the oracle, exact logs, mutants and cost evidence; D4 the delayed cause/owner
transfer. Score original R1-R4 separately: nominal tests score1; complete bounded
contract/implementation/independent oracle score2; unseen changed assumptions
with a counterexample and defended limits score3. Duplicate ownership, erased
error or unreported callback effect is a failure regardless of other scores.
Hints: enumerate tags; distinguish an unevaluated callback from a precomputed
argument; stage a clone before a nonfailing map; preserve causes as data before
formatting. Separate assessor technical analysis is not exported to learners.

Local original programs and synthetic counters only; no credentials, network,
privilege, deployed systems or exploitability claim. These are resource/semantic
integrity examples. Source-bound author execution is distinct from independent
human review, delayed learner success and physical evidence. Finite tests do not
prove absolute mastery or arbitrary callback laws.
