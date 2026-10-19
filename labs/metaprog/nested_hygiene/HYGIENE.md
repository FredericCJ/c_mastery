# Required T68.09 nested expansion, repetition and capture investigation

Apply original X-T68-009 R1-R4 and X-T68-009-DELAYED. Complete X-T68-008 and
implement src/nested_hygiene.c plus your retained mp_expand. The reference
actually calls that primitive for each terminal token; no parser/macro library
implements expansion. Host Python/Rust tools are separate verification tools.
Use hosted ISO C17 with unsigned having32bits for the fixed oracle boundary
cases; the C operations use UINT_MAX rather than assuming a native wrap value.
Record actual width and rerun a changed-width oracle before transferring.

## Representation, authority and admission

MphProgram owns1..4 definitions, each0..16 instructions. Two MphArguments own
0..4 initialized MPToken values each. Output owns at most64 tokens plus counts
for invocations, visited steps and peak depth. Inputs/outputs/next_context are
live, aligned, initialized, disjoint and externally serialized; no pointers
are retained. Public structs are not immutable types, but must not change during
expansion. Unused instruction fields are ignored and still initialized.

Identifier identity is exactly(symbol,context); token kind does not enter name
lookup. Token kinds in argument lists are0..2. Context0 is admitted for existing
global identifiers; next_context must be nonzero. The caller owns a namespace
authority in which no live identifier has a context at or above next_context.
All argument contexts, literal contexts and the caller context must be below
the initial next value; the implementation checks these visible values, but
cannot prove the authority's claim about other retained identifiers. Resetting
or copying that authority while old tokens survive can cause capture.

Before expansion validate all definitions, even unreachable ones: counts,
instruction kinds, ARG index0/1, CALL/REPEAT target definition, REPEAT mask1..3,
and visible context bounds. Null required controls, missing definitions/entry,
bad budget values and malformed structural data return INVALID. Extents refer
to actual array objects; invalid native pointers are not runtime-test inputs.

Each macro invocation consumes one new context, including an empty definition.
The last issuable context is UINT_MAX-1. A successful empty invocation may leave
next_context equal to UINT_MAX, which is an exhausted sentinel, never issued.
All expansion state, output tokens and next_context are staged. Every failure
leaves the caller's entire output and next_context unchanged, even after nested
work. Successful output and advanced authority are published together.

## Instructions and nested repetition

Operations are LITERAL0, ARGUMENT1, INTRODUCED2, CALL3, REPEAT4 and ESCAPE5.
LITERAL retains symbol/context but sets kind0. INTRODUCED retains symbol,
sets kind2 and assigns this invocation's fresh context. ARGUMENT selects the
named argument list and preserves the complete chosen token. Outside an active
repetition for that list, it must contain exactly one token or return SHAPE.
Inside a repetition, it selects the bound iteration index. CALL invokes its
target with new context, unchanged argument lists and current repetition indices.

REPEAT selects lists by its two-bit mask. Selected lists must have equal length
or return SHAPE; each iteration invokes the target definition with a new context
and binds those lists' indices to that iteration. Zero equal lengths produce
no target invocation. An already active list cannot be rebound by a nested
REPEAT (SHAPE). A nested repetition over the other list is allowed, forming a
Cartesian product; outside masks retain their existing index. This explicit
two-list model is not Rust's general nested fragment tree or matcher grammar.

ESCAPE is an explicit intentional-capture operation. It sets kind0 and uses the
immediate invocation parent's context, or caller_context for the entry. It
requires allow_capture=1, otherwise CAPTURE. It does not silently retag every
argument, and an escape's meaning changes at nested call boundaries. A normal
introduced binding cannot capture a caller token solely because symbols match.

Example: root context12 repeats twice over two length2 lists; each body emits
INTRO7,ARG0,ARG1. Body contexts are13 and14; arguments retain contexts5 and6;
output has6tokens, next15, invocations3, steps10 and peak_depth2. With masks1
then2 in nested bodies, both length2 lists produce four pairs and7invocations,
next19 and peak_depth3. The two meanings must not be confused with a truncated
zip that silently discards unmatched arguments.

## Work budgets and failure precedence

Budget depth is1..8, token count0..64, step count1..256, allow_capture0/1.
Each invocation and each visited instruction consumes one step. Depth is the
number of active invocations, entry1. Every emitted token consumes one token
unit. Repetition iteration dispatch is charged by its child invocation; a
zero-length repetition still consumes its instruction step. The call stack
never exceeds8, output never exceeds64, and work stops before step257.

At invocation entry check depth/step budget before context exhaustion, then
reserve the context and invocation step. At an instruction check/consume its
step; calls/repeats then validate their dynamic shape and invoke children in
order. Before a terminal token check token budget, then its argument shape or
escape permission. These define deterministic first failures. BUDGET is not a
claim that a recursive program terminates without the bound, nor an automatic
diagnosis of the original source's semantic intent. Direct/mutual cycles are
structurally admitted and bounded at execution; even a zero-output cycle stops.

## Environment resolver and concrete capture

This is not a compiler: it transforms already typed instruction records and does
not parse C. The supplied mph_resolve scans0..64 MphBinding entries from last
to first, comparing symbol/context, and returns the last matching value. Required
null output, null nonempty environment or count>64 is INVALID. No match returns
NOT_FOUND unchanged. The environment order is the caller's explicit shadowing
model, not a proof of complete lexical or module scope rules.

Use caller binding(7,5)=11 and introduced binding(7,12)=99. Correct argument
(7,5) resolves11 even if the introduced binding is later in the environment.
An original deliberately flawed transformer retags it(7,12) and resolves99.
This is an observable harmless capture defect in the local teaching model.
The repair preserves argument context and exact-pair lookup; regress both
transform and resolver defects. No invalid native pointer or external target
is required, and the result does not demonstrate arbitrary code execution.

Read the registered Rust primary repetition and mixed-site hygiene passages.
Actual Rust2021 compiler experiments distinguish definition-site locals from
invocation-site item lookup, nested argument identity, explicit identifier
binding and equal-length repetition. Negative translations reject unequal
repetition lengths, cross-expansion local sharing and bounded compiler recursion.
Our uniform fresh-context introduced-token scheme and explicit ESCAPE differ
from Rust mixed-site lookup; neither is an implementation of ISO C macros.

## Deliverables, assessment and variants

- D1/R1: derive identifier identity and namespace authority, implement the nested
  transformer and resolver, and demonstrate correct11 versus flawed99 capture.
- D2/R2: prove argument preservation, per-invocation freshness, exhaustion and
  three budget bounds; compare the actual Rust lookup/repetition counterexamples.
- D3/R3: execute retained hygiene plus macro_nested/budgets/capture/base in six
  profiles, reject eight compiled semantic controls and four compiling stubs;
  compare an independent explicit-work-stack oracle and guard-filled failures.
- D4/R4: implement nested calls, zip and independent-list nested repetitions,
  explicit escape permission, empty expansion/cycles and unchanged publication
  after partial work. Supply all13cost rows and an unaided delayed changed case.

Run run_metaprog_nested_hygiene.py, verify_metaprog_nested_hygiene.py,
run_metaprog_hygiene_investigation.py and check_metaprog_nested_hygiene.py
--self-test. The oracle uses a separate explicit task stack, immutable token
tuples and Python integers; it does not call the C recursion or expected outputs.
Retain exact sources/tools, finite input limits and first divergences. Mutants
must compile and fail an ordinary MPO_CHECK rather than sanitizer termination.
Public tests alone earn at most level1; level2 requires each specific criterion,
level3 an independently attempted changed case with assistance recorded.

Hints: (1) carry binding indices separately from syntax context; (2) reserve
contexts in staged state, including empty calls; (3) zero output does not mean
zero work. Private analysis and the sealed case are separate under assessor/.

## Costs and enforcement

| Dimension | Required evidence |
| --- | --- |
| Representation | Actual program/expansion sizes, definition/list counts and explicit unsigned context sentinel. |
| Reachable storage | Four16-instruction definitions, two4-token lists, one64-token staged output and at most8frames. |
| Lifetime | No retained input pointers; context authority outlives every token it allocated. |
| Mutation/aliasing | Disjoint controls and immutable input during the call; no concurrent namespace allocation. |
| Allocation | None in the transformer/resolver; Rust compiler/Python are host tools only. |
| Movement/copying | Complete output publication after bounded copies; argument token kind/symbol/context preserved. |
| Destruction | No owned heap resources; authority cannot be reset while old identifiers survive. |
| Dispatch | Direct recursion plus actual retained mp_expand leaf calls; inspect emitted objects. |
| Concurrency | External serialization; no atomic unique-context allocator or concurrent mutation claim. |
| Failure | Every dynamic failure leaves authority/output unchanged; no partial expansion becomes visible. |
| Compilation | Real Rust positive/negative translations, clean/unchanged builds and explicit edition limits. |
| Binary/stack | Six-profile size/object/stack evidence; dynamic recursion bounded8; no freestanding-helper claim. |
| Boundary | Original finite token language and reverse environment, not C parsing or full Rust macro/module hygiene. |

Validation scans at most64 instructions and8 argument tokens. Execution is
O(steps+tokens), steps<=256, each terminal conversion bounded; resolver O(bindings)
with at most64. Seven10000-expansion CPU-time samples use the same2-iteration
zip and full validation, zero separate warmup, and exact operation counts.
Record noise/resolution; no universal zero-overhead or physical timing claim.
Exact published C17/C23, independent human review and learner attempts remain
separate evidence limits. No operational devices, credentials or services are
targets; all deliberate capture demonstrations use original synthetic bindings.
