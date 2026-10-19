# C99/C11 capability migration with a C17 implementation baseline

## Exact scope and prerequisites

This required companion supplies the twelve ordinary and delayed tasks in
`PR-T05-02`: `AX-PR-T05-02-T05.04.a` through `.04.c`, and `.05.a` through
`.05.c`, with the existing `-DELAYED` identifiers. Families A, B, C, D, F, H,
I and J are selected for semantic migration, construction, refusal boundaries,
link diagnosis, falsification, resource inspection and changed environments.
Retain the parent teaching and assessment card. Prerequisites are completed
PR-T05-01 source-authority and prototype work, basic object lifetime and
translation-unit linkage from T02/T03, checked arithmetic from T04, and the
learner's own prediction notebook. No assessor implementation is a prerequisite.

Implement `capability.c` in C17 from the supplied header. The supplied stub must
compile; its semantic tests initially fail. Use no allocator or hidden global
state. Hosted formatting may use `snprintf`; formatting syntax and ABI migration,
not a first-principles decimal conversion algorithm, is the mechanism here.
The strict target companion admits only its explicitly listed startup and
semihosting adapter. Host Python, CMake, QEMU and linker tools are verification
dependencies, not target runtime libraries.

## Representation, operations and publication

`LpValue` owns no allocation: `present` is 0 or 1, and empty records have value
zero. `lp_record` accepts a live caller-owned output, rejects invalid presence
without modifying it, and copies a designated compound literal into that
storage. The internal automatic literal does not escape. When `LP_LAYOUT2` is
defined the two fields change order; construction and semantic output must be
unchanged. No structure byte representation or cross-layout binary ABI promise
is made. Compile both producer and consumer with the same selected layout.

`lp_format` writes `empty` or signed `intmax_t` decimal plus a terminating zero.
It stages into 128 bytes and refuses a formatting error, a result too large for
that staging bound, or insufficient caller capacity. Every refusal preserves
the caller's bytes. This is a bounded string operation, not stream I/O. Inputs
and outputs must be valid and disjoint; no racing calls may share writable
storage. `lp_transform` doubles 0..16 `int` values. Zero length admits NULL
spans without pointer arithmetic. Nonempty live source and destination spans
must be disjoint, as required by this API's `restrict` contract. Preflight every
value against `INT_MIN/2` and `INT_MAX/2` before any output write. The array and
record operations are O(n) and O(1) space, except the fixed 128-byte formatting
stage. The no-alias and lifetime obligations are programmer premises; ISO C
does not dynamically check them.

`lp_capability(required,observed,out)` uses bits 1,2,4,8 for language, header,
link and execution. Masks above15 or NULL output refuse unchanged. A valid row
is usable exactly when every required bit is observed; the empty requirement
is true. This recomputes a declared synthetic model, not compiler support.
`LP_TRACE(function,...)` invokes a compatible callable exactly once, evaluating
each argument once, without imposing left-to-right evaluation. The test uses
two different counters, so no unsequenced conflicting modification is executed.

## Twelve concrete tasks and changed conditions

| Exact objective | Ordinary deliverable and expected observation | Delayed changed assumption |
|---|---|---|
| T05.04.a | Implement designated `LpValue`, block literal copying, the checked disjoint transform and variadic trace. Empty record is `{0,0}`; late overflow leaves all three destination values91,92,93. Trace returns9 with one call and counters3,8. Explain the returned-pointer lifetime defect and overlapping restrict call as reasoning-only counterexamples. | Replace an interface returning a pointer to its automatic literal with the supplied caller-storage API. Prove the output outlives the internal literal; repeat bounds/refusal tests. |
| T05.04.b | Compile separate static-inline header consumers in C99, C17 and GNU89, recording object symbols and linked values5,7. Admit VLA lengths1..16 only; reject0/17 before declaring the array. Observe exact-width type availability and use its actual PRI macro. | Move the public inline body into two translation units. Record C99/C17 external-provider linkage, GNU89 duplicate-definition failure, and the explicit GNU89 extern-inline repair. Preserve identical function bodies and exact values. |
| T05.04.c | Compare original strict C90 `long` construction/printing with the C17 designated/intmax port for empty,LONG_MIN,0,LONG_MAX. Compare actual textual output, not padding or unrelated wider values. Exercise INTMAX_MIN/MAX separately. | Compile producer and consumer again with the field order reversed. Named initialization must retain meaning; positional old initialization would not. Rebuild all affected objects, not only the header consumer. |
| T05.05.a | Isolate alignment, `_Generic`, `_Static_assert`, atomic increment and actual two-thread creation. For each C11 and C17 probe retain preprocessing, semantic translation, object, link and run results. Atomic increment returns old0/new2; joined workers return7 and sum20000. | Use an atomic aggregate large enough to emit actual helpers. Inspect undefined object symbols; require the intended missing-helper link rejection and an explicitly linked `libatomic` positive run. This changes the dependency profile; it is not an inline or lock-free claim. |
| T05.05.b | Capture optional-feature macros alongside actual header, object and runtime evidence. Exact32 availability is conditional; a minimum-width integer changes the contract and is an explicit fallback, not the same representation. | Repeat selected operations in freestanding C17 on Cortex-M4: inspect object and final-image symbols, execute the library-free 32-bit increment/alignment/generic/VLA case in QEMU, and reject the unresolved 64-bit atomic helper. Probe actual thread-header availability. Do not infer unsupported facilities from one unrelated macro. |
| T05.05.c | Recompute the original fixture: required all four layers with observed language/header only is false; language-only is true. Compile the original `lp_optional_runtime` consumer, then observe its intended link failure. | Compile the supplied original helper separately, link it explicitly, and run the expected42 result. Keep the previous unavailable row and append the changed dependency configuration. A successful new link cannot rewrite the earlier receipt. |

The C99/GNU89 comparisons are isolated compiler modes. The C17 implementation
baseline is unchanged. GNU89 VLA is an extension probe and retains pedantic
warnings. Clang's strict GNU89 inline-extension rejection is retained as its
own negative observation; the extension build uses `-pedantic` explicitly.
GNU89 external-inline semantics are a GCC/Clang implementation-mode
contract. The C90 routine uses prototypes and defined argument types; no
undefined historical behavior is executed to create a golden result.

## Verification, resource evidence and pass criteria

Run the public CMake tests against your implementation, then the independent
`tools/verify_portability_capability.py` driver protocol. Its324 mask rows and
1,331 boundary triples use independent Python arithmetic and all-or-nothing
expected output. Four semantic controls must be rejected: partial writes
before late overflow, language-only capability inference, duplicated trace
evaluation, and nonzero empty-record payload. A crash or sanitizer finding is
not the intended mutant success condition.

The three unfinished C operations reject their corresponding learner anchors.
The supplied trace macro is an executable syntax probe; it already passes its
positive test and its separately compiled duplicated-evaluation control fails.
It is not counted as an unfinished implementation rejection. The wide-helper
probe uses a volatile atomic three-word aggregate so optimization cannot erase
the observable accesses. A prior unescaped static aggregate was actually
optimized away by Clang at O3, demonstrating why source spelling alone cannot
establish an emitted helper dependency.

The author campaign runs GCC and Clang debug, release and AddressSanitizer plus
UndefinedBehaviorSanitizer profiles under C17. It records actual compiler
include closure, linked host libraries, object disassembly, stack reports,
code size, exact commands and raw logs. Stack and code size observations are
configuration-specific; `inline` is not a promise of inlining or lower cost.
The companion records C90/C99/C11/GNU89 modes separately, including all positive
and negative link controls. Target object helper audits and final unresolved
symbol checks are mandatory: `-ffreestanding` alone proves neither closure nor
absence of compiler-generated runtime calls.

Pass each task only with its named observations, correct source-based semantic
argument, an independently selected boundary and the changed-condition result.
D1: predictions and authority/mode table. D2: implementation and isolated probe
translation units. D3: manifest, commands, source hashes, raw diagnostics,
oracle results and cost/dependency analysis. D4: delayed variant after at least
seven days, using a fresh argument and without the assessor solution. The
retained criterion rubric grades 0 missing/wrong,1 reproduction without a
correct mechanism,2 justified ordinary task plus independent falsification,
3 successful changed-condition transfer with bounded conclusions. All six
leaf objectives are assessed separately, even when one build supplies evidence
for several of them.

## Graduated hints and containment

1. Classify each failure by phase before changing code or linker flags.
2. An external inline body and an external definition are different obligations.
   Take its address to prevent an optimized-away call from hiding the symbol.
3. Preflight the entire transform and use named fields; do not repair a failed
   alias/lifetime contract by merely disabling optimization.

Only original local programs and synthetic records are involved. Reasoning
counterexamples with expired pointers, overlapping restricted accesses or
incorrect variadic types are not executable expected-output demonstrations.
No host security changes, external target access or physical board operation is
authorized by these exercises.

## References and validation limits

The source register pins actually inspected N1570 passages and the versioned
GCC15.2 inline/atomic helper manual. N1570 is the 2011 C11 committee draft, not
an inspected copy of published C17. Exact published C90,C99,C17 and C23 remain
access gaps. Actual execution can establish only the named configuration and
finite tests. QEMU semihosting is an explicit emulator adapter, not a standalone
board runtime or physical observation. The readiness envelope distinguishes
authored, executed, independent review and learner assessment; passing author
campaigns does not establish universal conformance or learner mastery.
