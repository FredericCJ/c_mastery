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

## PR-T68-01: Template-header monomorphization and associated types

Implement one unguarded private ring template instantiated for uint32_t and MPPair. Public MPU32Ring/MPPairRing each hold4elements,head,count. Push fails unchanged when full; pop fails without modifying output when empty. FIFO order must survive wraparound. Prefix, element type and ring type are explicit template parameters; undefine all private macros after each inclusion. Reuse the containers.ring contract but implement the type-generation mechanism yourself.

Derive head<count/capacity invariants, occupied logical slots and output ownership. Compare generated specialization with Rust generic/associated-type obligations and show a mismatched ring/element instantiation diagnosed by C. Explain why a template include guard would silently suppress the second instantiation and why internal macro names can collide.

## PR-T68-02: Constraint probes and constant storage parameters

Implement mp_storage_bound(count,width,limit,out): positive count/width, multiplication representable in size_t, and product<=limit. Fail without output changes. Create positive/negative translation units using _Static_assert for compile-time capacity and type compatibility, with array extents derived from constant parameters. A runtime input must use the checked function rather than a false compile-time guarantee.

Derive count<=SIZE_MAX/width before multiplication and the independent budget bound. Explain why C17 _Static_assert requires an integer constant expression, VLA support differs from fixed arrays, and syntactically valid macros do not impose an abstract trait. Compare the inspected Rust const-generic type/use restrictions without claiming arbitrary compile-time evaluation equivalence.

## PR-T68-03: Policy composition with explicit failure semantics

Implement mp_reduce over uint32 inputs and MPPolicy{limit,saturate}. Starting from0, sum mathematical values up to limit. saturate0 rejects any exceeding prefix and leaves output unchanged; saturate1 clamps tolimit. Invalid policy values fail. Empty input returns0; NULL input is allowed only when empty. Then generate two named specialized wrappers whose policy is fixed and compare with the dynamic function.

Prove value<=limit before subtracting limit-value, and define how a single input greater thanlimit is handled. Inputs/outputs are disjoint. State no allocation, single-threaded execution and failure publication at the end. Compare a policy parameter with Rust const parameters/trait-associated behavior only at the documented boundary.

## PR-T68-04: Portable C17 variadic machinery and single evaluation

Implement mp_variadic as an inspectable demonstration of a nonempty1..4argument macro selecting typed inline functions; arity0 uses a separate explicit empty operation. Each argument is evaluated exactly once and arithmetic is uint32 modulo. The supplied demo evaluates seed+i and reports the exact count. Use separate counters for distinct argument expressions so the test itself does not contain unsequenced writes to one scalar.

Show token substitution, rescan, selector position and parentheses for each admitted arity. Explain why an empty token argument differs from omitted variadic arguments in C17, and why this lab deliberately has a distinct zero-argument entry. Compare N3033 proposal semantics without presenting a proposal as the inspected final C23 standard.

## PR-T68-05: Generic customization with cv-preserving receivers

Implement a _Generic receiver dispatcher selecting functions for uint32_t* and const uint32_t*. The mutable receiver returnsvalue+100, const receiver returnsvalue+200, using modulo arithmetic; neither modifies the pointed value. mp_generic_receivers invokes both through pointer-producing expressions and reports exactlytwoevaluations. Unsupported receiver categories receive a compile-time diagnostic; there is no default erased fallback.

Distinguish top-level pointer qualifiers from pointed-to const and state the compatible-type association restrictions. The controlling expression is not evaluated; only the selected call argument is evaluated. Explain why the generic selection should choose valid function names rather than embed branch-specific invalid field operations.

## PR-T68-06: Single-source field reflection and explicit wire operations

Use MP_RECORD_FIELDS to describe code:uint16 and value:uint32 with wire widths2/4. Generate MPRecord and the MPField descriptor table from that list; implement exact6byte big-endian encoding. A short output capacity fails unchanged. Do not memcpy the struct to wire bytes; do not serialize padding or assume offsetof values equal wire offsets. Add a field through the single registry and rebuild every generated consumer.

Explain stringification/token substitution and why the field list is a local schema, not arbitrary introspection of undeclared C types. Distinguish native field offset, native sizeof, wire width and schema identity. Compare the declared generated interface with language macro/derive facilities; no C++26 reflection claim is inferred without inspected final specification.

## PR-T68-07: Derived equality, hashing and shrinking law counterexamples

Implement MPKey equality overx,y only; cache is explicitly nonsemantic. Hash canonical little-endian64bit words forx theny with the supplied specified FNV-1a recurrence, not native struct bytes. Implement mp_find_counterexample for pure MPProperty over bounded pairs0..bound, bound<=255: deterministic lexicographic enumeration, first failing input, then decrement-one-coordinate shrinking while failure persists. Return1with locally minimal witness,0if all admitted pairs pass,-1on invalid property/bound; no failure output publication for0/-1.

Prove reflexivity/symmetry/transitivity obligations separately from hashing and collisions. Specify FNV parameters and unsigned64 wrap; this is not a cryptographic integrity claim. The shrinking order finds a local minimum under decrement moves, not necessarily a globally simplest witness for arbitrary nonmonotone predicates. Callback/context must remain live, deterministic and side-effect controlled.

## PR-T68-08: Typed formatting and transactional bounded sinks

Use the supplied MP_ARG category-selection scaffold from the earlier generic-selection work, implement its uint32/text factories, and implement mp_format for decimal uint32 or literal zero-terminated text. Derive the scaffold expansion independently in D1. MPSink appends raw bytes with no terminator guarantee. Validate used<=capacity; failure leaves bytes/used unchanged. Text is live terminated storage, disjoint from output; percent sequences are literal data. Unknown argument kinds fail. No printf-family routine may implement the conversion.

Derive the10digit bound for UINT32_MAX and reverse temporary digits only after capacity admission. For text, prove scan stops on insufficient capacity without publishing a prefix. Compare C++ basic_format_string constant-evaluation compatibility with this narrower C factory: it enforces argument categories, not arbitrary formatting grammar or backing lifetime.

## PR-T68-09: A bounded hygienic token expander written in C

Implement mp_expand for at most32template tokens. kind0 is a literal token whose context is retained;kind1 substitutes argument indexsymbol and preserves the argument token completely;kind2 is an introduced binding/reference token whose symbol is retained but context becomes one fresh expansion context. Validate kinds, argument indices, capacity and context exhaustion before publishing output/used or incrementing next_context. Context0 andUINT_MAX are rejected; all supplied objects are disjoint and live. This bounded token model does not parse C or implement the complete Rust macro grammar.

Define identifier equality as(symbol,context), track binding/reference identities through an expansion, and show how call-site symbol7/context5 remains distinct from introducedsymbol7/context12. Explain nested expansion context retention and finite namespace exhaustion. Compare the inspected Rust mixed-site hygiene rules precisely; the model uses an explicit stronger uniform context scheme for introduced names.

## PR-T68-10: Declarative build graphs and deterministic table generation

Implement mp_build for0..8nodes with dependency bitmasks, source bytes, tool identity and configuration identity. Reject out-of-range dependencies/cycles; choose the lowest ready index deterministically. Compute each64bit FNV-1a key from length, source bytes, tool, configuration and ordered(dependencyindex,dependencykey) words. Encode words little-endian8bytes; output order/keys only after full success. Implement mp_emit_table for1..8keys as a bounded generated C uint64 array and compile the generated file in a temporary directory.

Prove Kahn-style readiness and termination, show how dependency content propagates into consumers, and distinguish source identity from timestamp ordering. Compare Ninja explicit/implicit/order-only dependency semantics and command-line rebuild tracking. The educational FNV key is not collision-resistant; require full input equality or a suitable cryptographic digest before any untrusted shared cache use.
