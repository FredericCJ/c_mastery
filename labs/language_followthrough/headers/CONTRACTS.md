# Header definitions, diagnostic authority and observation

This required successor implements the six original PR-T02-04 criteria and their unchanged seven-day variants. IDs, original tasks, prerequisite knowledge, rubric levels and delayed assessment rules remain in the project's assessment supplement. This contract supplies concrete programs and experiments; it does not replace the original tasks or certify a learner from author test results.

## Environment and prerequisites

Use hosted C17/profile A on a local Linux process. Know prototypes, linkage, translation units, unsigned arithmetic, array bounds and separate compilation. Required learner artifacts are two independent consumers, a self-contained header, one external implementation and a diagnostic ledger. The only target dependencies are `<stddef.h>` declarations and the C language; the test driver uses hosted I/O. GCC/Clang, `nm`, `ar`, Python and sanitizers are host verification tools. No parser, library or wrapper implements the mechanism on the learner's behalf.

Label `-std=gnu17` and `-fgnu89-inline` experiments separately; GNU17 alone uses modern inline semantics. Record actual compiler/linker versions and optimization. The N1570 passages are a C11 draft, not the published C17 text. Keep that source-access limitation visible when applying the C17 implementation baseline. Object-symbol spelling and emission are ELF/toolchain observations, not ISO syntax.

## Representation, operations and failure

Complete `headers.h` and `learner.c`. `hh_mix(unsigned)` computes `3*x+1` in unsigned arithmetic; derive the wraparound result. It has internal linkage, a real address and no side effects. `hh_state()` returns the address of a zero-initialized header-local static unsigned object. Calls within one translation unit share that object; the two consumers must have distinct objects. This single-threaded teaching counter is not a synchronization primitive.

`hh_read(values,length,index,out)` returns zero and copies one int when `values` and `out` are nonnull and `index < length`. Otherwise it returns -1 and preserves a live output object. The caller owns actual storage: a nonnull `values` must designate at least `length` live initialized ints; `out`, when nonnull, must designate a live writable int. Length metadata cannot validate a fabricated pointer or repair a false extent. The call borrows storage only until return. It allocates nothing, is constant-time and uses constant additional space. At a successful call the source element is evaluated before assignment, so `out` may designate a source element. Concurrent conflicting access is outside this contract.

## T02.11.a — Definitions and per-unit state

Build `consumer_a.c` and `consumer_b.c` independently, then link `tests.c` with exactly one external definition. Save `nm -P` outputs for each object. Predict helper identity, helper address, API ownership and the unsigned extremum before running selectors 0 and 1. Demonstrate a missing external definition and a duplicate external definition in separate failing links; a compile-only success does not answer either question.

For `AX-PR-T02-04-T02.11.a-DELAYED`, move helper-local state into your header and trace writes 7 and 9 through the two consumer addresses. Explain the number and lifetime of objects. A shared global counter which happens to produce the first expected number fails the identity requirement.

## T02.11.b — Inline model and optimization

The supplied `probes/experiment.h` selects plain `inline` or `extern inline`. Compile both consumer objects and the independent main at O0 and O3 under strict C17, GNU17 and GNU17 with explicit historical GNU-inline semantics. Save actual object symbols, link exit status and the controlling declaration. Address-taking and call-only variants are separate programs. For a missing-definition case add exactly one out-of-line definition from `inline_external.c`, link and run the repaired program. For duplicate bodies explain which declarations must change; suppressing the linker diagnostic is not a repair.

For `AX-PR-T02-04-T02.11.b-DELAYED`, repeat both optimizations with function addresses escaping through separately compiled functions. Compare with call-only objects. Never execute a program whose ISO external-definition obligation is unsatisfied merely because optimized linking succeeded. Account for internal linkage, external linkage and external definition as separate concepts.

## T02.11.c — Header independence

Compile the header first, after `<limits.h>`, and twice in the same unit. Compile both unrelated consumers. Install only the completed header and static library into a fresh directory; compile and run `probes/installed.c` there without repository include paths. Record the actual include search and link command. Inspect exported names so a private helper does not accidentally become a required global ABI symbol.

For `AX-PR-T02-04-T02.11.c-DELAYED`, `missing_masked.c` supplies `size_t` transitively but `missing_first.c` does not. Remove that transitive include and diagnose the first-use failure. Repair the public header's own standard prerequisites, then repeat the isolated installed consumer. Adding the missing include only to one favored client leaves the API incomplete.

## T02.12.a — Classification before compiler policy

Classify `syntax.c`, `redeclaration.c` and `runtime.c` before running anything. Name the source rule, whether ISO requires a diagnostic and whether a nonzero compiler exit is required by the standard or only this build policy. The runtime example's validity depends on a live array and admitted index. Compile that source successfully and run only index3 in the ordinary suite.

For `AX-PR-T02-04-T02.12.a-DELAYED`, compile the same negative sources under permissive GNU17 and explicitly demote integer-conversion diagnostics. Retain diagnostic presence even when exit status is zero. Compare the separate statement-expression extension under strict and declared GNU modes. Acceptance is an observation, not a rewrite of the ISO category.

## T02.12.b — Diagnostic regression matrix

Extend the fixture's synthetic categories with the actual wrong-arity and wrong-argument-type sources. `repair.c` is the positive control for all four translation defects. Save source location, a relevant diagnostic category, exact command and status. A missing include, missing compiler or unknown flag cannot satisfy an intended rejection. The repaired control must translate with the same strict flags.

For `AX-PR-T02-04-T02.12.b-DELAYED`, repeat with GCC and Clang and installed C/C.utf8 locales. Permit wording and caret differences; preserve the semantic category and source association. These two locales need not yield different human languages. Do not claim translated-language coverage unless one was actually available and used.

## T02.12.c — Runtime and analysis counterexamples

The original `runtime_main.c` provides four synthetic ints and accepts a local index. With AddressSanitizer, run index4 only in a dedicated process and require a bounds finding tied to this object. No value oracle is defined for the invalid read. Repair by validating the actual extent before evaluation, then demonstrate both last-valid success and first-invalid unchanged output using `hh_read`.

For `AX-PR-T02-04-T02.12.c-DELAYED`, analyze the three `HH_MODEL` choices: unknown index, branch-constrained index4 and checked index. Use the actual `security.ArrayBound` checker or explicitly document another available checker. Compare its assumptions and diagnostics to the source semantics and the dynamic counterexample. A silent unknown-index result is not a proof that all indices are valid. Preserve capability/version output and the exact analysis commands.

## Deliverables and assessment

D1 is the header/library, two independent consumers, object ownership table and installed-client evidence. D2 is the dialect/optimization/address-taking matrix with prediction, actual symbols, failed links and repaired controls. D3 is the translation/runtime/analysis ledger with source rules, relevant diagnostics and counterexamples. D4 is the delayed packet for each unchanged criterion ID, attempted after at least seven days without copying the original solution.

For each original criterion, level0 means the mechanism or contract is wrong; level1 means a supplied case runs but the explanation or changed condition fails; level2 means correct implementation, invariant, targeted negative control and bounded evidence; level3 additionally requires independent reasoning and transfer to the unchanged delayed task. The criterion-specific rubric in the assessment supplement remains authoritative. A report must associate each claim with its exact source/object/run evidence. Author executions cannot stand in for independent learner attempts.

## Graduated hints

First separate source rules from emitted symbols and runtime executions. Next draw a table with one column per translation unit and another for the final program. For the diagnostic matrix, remove everything except the first violated rule. For the static-analysis comparison, ask which index constraints the analyzer actually knows. Only after those steps compare the repaired control against the failing input. Assessor implementations and exact expected symbol tables are withheld separately.

## Security, validation and limits

The only invalid execution is the original local four-int teaching program with synthetic index4 under AddressSanitizer, in a separate process. It reads beyond the permitted object; the observable success condition is the matching sanitizer finding. There is no external target, credential, privilege change or demonstrated arbitrary execution. The repaired API rejects the index before reading and preserves output; false extent/lifetime metadata and data races remain caller obligations.

Current author commands and outputs are in `research/v5/expansion/legacy_language_headers/validation.json`; export and support reports are separate. The failed first unsigned-wrap expectation and sanitizer exit-code configuration are retained as attempts rather than erased. Only exact source-bound passing reports establish author execution. No physical observations, external review or independent learner assessment are claimed.
