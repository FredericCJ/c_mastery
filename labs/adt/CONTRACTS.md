# Original mechanism laboratory contract

Use ISO C17 with extensions disabled and -Wall -Wextra -Wpedantic -Werror.
Public src files are compiling unsolved learner stubs. Reference solutions and
semantic mutants reside only under assessor/. Explicit assessor selection must
never be used as learner evidence. Python, CMake, compilers and diagnostic
runtimes are host verification tools, not implementations of the taught mechanism.

Unless an operation explicitly states otherwise, all pointer/control/output
objects must be live, correctly aligned, initialized to their required
representation and disjoint from input/control storage. Nonzero extents require
real backing objects of that extent; numerical range checks cannot establish
that a fabricated pointer is valid. Null allowance is stated per operation.
Output failure atomicity refers to admitted inputs. There is no general promise
to diagnose invalid native pointers, data races or lifetime violations.

Views borrow stable backing storage. The caller retains ownership and must not
destroy, relocate or mutate it while the view's validated interpretation is in
use. Value copies of handles/views do not extend lifetime. Public representation
fields remain constructible in C; they are not unforgeable proof objects.
Operations are single-threaded unless explicitly synchronized by an external
contract; adding atomics to one counter does not provide reclamation safety.

No library may replace the specific constructor, validator, property runner,
token expander or graph mechanism being implemented. Declared prerequisite
components may be reused through their documented contracts. At final
composition, retain the actual learner source identities and an adapter test;
merely listing an import does not establish working integration.

All security investigations use only these original local teaching programs,
synthetic bytes and local state. A harmless violation is an invalid input
accepted, a wrong synthetic value returned, or a stated property falsified.
This does not establish arbitrary code execution, privilege escalation, or a
property of any deployed program. Never use third-party services or real data.

Every project requires D1-D4 from its exercise, all thirteen dimensions in
modern-cost-contract.md, a prediction before execution, exact tool/source
identity and independent expected results. Level2 requires the entire written
contract and original implementation/analysis; public tests alone earn at most
level1. Level3 requires a delayed unaided changed-condition attempt after at
least seven days. Record assistance and distinguish self-report from review.
Author campaigns are not learner acceptance, independent human assessment,
physical validation or proof of absolute mastery.

## PR-T67-01: Products, tagged sums and checked elimination

Implement ad_measure and ad_number over ADValue: EMPTY maps to0, NUMBER to its uint32 payload, PAIR to the exact uint64 sum. Invalid enum values fail; ad_number accepts only NUMBER. Every failure preserves the output. No union payload may be read before validating the tag. Supply constructor/accessor conventions and a deliberately incomplete switch translation unit.

Draw all admitted tag/payload states, establish uint64 sufficiency for two uint32 values, and explain which inactive-member operations are avoided. Derive a counterexample to the claim that adding a default branch makes future variants exhaustively handled. Compare Rust enum constructors and binding behavior with C aggregate initialization and private-header discipline.

## PR-T67-02: Newtypes and checked dimensional conversions

Implement ad_milliseconds(ADSeconds,ADMillis*). Conversion succeeds exactly for seconds<=UINT32_MAX/1000, with exact multiplication and unchanged output otherwise. Separate nominal dimensional distinction from runtime range validation. Add diagnostic probes for passing ADMillis to the ADSeconds parameter and for the unsafe typedef-only alternative.

Derive the division-before-multiplication admission test and show that the largest accepted seconds value maps below UINT32_MAX. Compare the inspected C++ duration_cast ratio formulas; static_cast or a distinct duration type does not itself establish arbitrary destination representability. Specify round/truncate behavior before adding fractional units.

## PR-T67-03: Typed flag sets and reserved-bit admission

Construct ADFlags from uint32 raw input. Bits0,1,2 are independently allowed; zero is a valid empty set. Any other bit fails transactionally. Provide union/intersection operations in the learner extension and show that their closure depends on validated operands.

Define the valid mask invariant raw & ~7 ==0, all eight admitted states and ownership-free value semantics. Explain integer promotions when complementing a narrow mask; use an explicit uint32 mask. A public struct can be forged, so only constructor-admitted values have the promised invariant.

## PR-T67-04: UTF-8 smart constructors and validated borrowed text

Implement ad_text over at most65535 bytes. Return a borrowed immutable pointer/length and Unicode scalar count. NULL is permitted only for length0. Reject truncated continuation sequences, stray continuations, overlong forms, surrogates and scalars aboveU+10FFFF. No normalization, grapheme segmentation or allocation is performed. Failure leaves output unchanged.

Track input position and remaining continuation count without forming an out-of-range pointer. For each sequence derive minimum scalar value and maximum scalar value; compare scalar count with byte count. State storage lifetime and mutation restrictions explicitly, including a safe constructed example where another alias changes a previously validated buffer.

## PR-T67-05: Branded pool handles with checked generations

Implement the fixed4slot ADPool API. init requires a live fixed-address pool with no surviving handles. acquire takes first free nonretired slot and returns owner pointer/index/generation; full pool fails unchanged. get/release validate exact live pool identity, index and generation. Release increments generation, or permanently retires a UINT_MAX generation slot. Caller controls synchronization; no concurrent calls.

Derive FREE/LIVE/RETIRED transitions and count available slots. Explain why copying a handle is permitted by C yet does not duplicate ownership. Pool objects must remain live at fixed addresses and cannot be reinitialized or reused while handles survive: pointer identity does not prevent ABA across object lifetimes. Compare PhantomData nominal/lifetime roles without claiming equivalent static checks.

## PR-T67-06: Read-only strided views and ownership boundaries

Implement ad_view and ad_view_get for2D uint32 data. Nonempty views require positive strides and required_span=1+(rows-1)*row_stride+(columns-1)*column_stride<=capacity with checked arithmetic. Empty rows or columns permitNULL and zero strides; no access succeeds. Overlapping indices are allowed for this read-only view. Input storage must be live and stable; view fields are not freely forgeable.

Prove multiplication and addition guards before computing the last element offset; explain why logical element count need not equal required span. Compare span contiguous access and mdspan layout_stride preconditions. Define when two logical indices alias and why a writable transfer needs stronger validation or an explicit alias contract.

## PR-T67-07: Recursive arena values and typed indices

Implement ad_eval for1..32ADNodes. Kind0 is uint32 literal; kind1 adds two child indices, each strictly less than the current node index. Validate every node, even if unreachable from root. Root must be in range. Addition is mathematical uint32 with overflow rejection, output unchanged on failure. Use iterative evaluation and fixed32word scratch storage, no recursion or library graph engine.

Prove by induction on node index that each value is available before its parent; separate index category from actual range validity. Give O(count)time/O(32)stack and compare a recursive tree that duplicates shared children. For validate-once extension, make the arena immutable or invalidate its certificate after every structural mutation.

## PR-T67-08: Lenses and prisms with falsifiable laws

Implement ad_lens_x preserving the y field of ADPair, plus ad_review_number and ad_preview_number over ADValue. Preview is partial and leaves output unchanged on non-NUMBER. Enumerate x,y,replacements0..3 and all variant classes; state purity and equality assumptions explicitly.

Write each law as an executable predicate and derive which information an update may change. Compare the C functions with the inspected lens type laws; C types do not enforce those laws. Distinguish field equality from raw struct memcmp because padding need not encode semantic identity.

## PR-T67-09: Evolution-aware sums and lossless tolerant readers

Implement original teaching TLV [tag:u8,length:u8,payload]. Tag1 has exactly2payloadbytes,tag2 has0; tags3..127 are unknown advisory variants retained as borrowed bytes;0 and128..255 fail. Require exact input extent and lossless re-encoding. Write into disjoint caller storage only after complete capacity validation, with used/output unchanged on failure. This is not a Protobuf-compatible wire implementation.

Define a sum of known-number, known-empty and unknown bytes; retain tag and payload even when semantics are unavailable. State lifetime, disjointness, maximum255payloadbytes and overflow-free size calculation. Compare binary unknown-field preservation with the inspected Protobuf guide and demonstrate a field-wise rebuilding path that loses unknown data.

## PR-T67-10: Zero-copy packet view with precise error sum

Implement ad_packet for [kind:u8,length:u16 big-endian,payload]. Kind1 data accepts any declared payload includingempty;kind2 acknowledgement requireszero. Check header first, then kind, exact extent, then ack constraint. Return distinct ADPacketError values and publish ADPacket only on success; keep payload pointing into stable caller backing. Compose the parser with the retained io.frame/cursor interfaces via a learner adapter and reject fragmentation until a complete frame is available.

Derive a state table for every failure stage and prove length-3 subtraction is reached only after header validation. No struct overlay, unaligned integer load, allocation or payload copy is permitted. Compare C tag/error conventions with census C++ variant/expected/span without treating the example as an authoritative language specification.
