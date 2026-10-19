# PR-T02-04 assessor analysis

Read the unchanged original six criteria and delayed tasks together with the public contract. This file is withheld; the public negative programs are deliberately original teaching targets. A passing process alone never earns the reasoning or delayed-assessment criterion.

## T02.11.a

Two translation units define two internally linked `hh_mix` functions. Their escaped function pointers compare unequal. Each local static object has static storage duration but is a distinct object: A=7 cannot alter B=0, then B=9 leaves A=7. The external `hh_read` definition appears once. `nm -P` marks selected ELF local text `t` and global text `T`; those letters are tool conventions. Missing `hh_read` fails the final link; supplying the same definition twice also fails. Three unsigned-max plus one reduces modulo UINT_MAX+1 to UINT_MAX-1. The initial author test mistakenly expected UINT_MAX-2; its failed receipt/source are retained. Accepting that old result would teach the wrong arithmetic.

The API copies a selected live int or preserves output on failure, with a caller-owned truthful extent. It does not validate arbitrary address metadata. Helper-result, shared-state and output-clobber mutants exercise distinct properties; all must fail at their intended assertion, including optimized and sanitized builds.

## T02.11.b

For this exact declaration set, C17 and GNU17 plain-inline consumer objects have an undefined external reference when the address escapes. Neither supplies an external definition. Exactly one separate ordinary definition repairs the link and yields one external address. Modern extern-inline emits an external body in each consumer, so both together duplicate it. Explicit GNU89-inline reverses these two emission patterns. All combinations are inspected at O0 and O3 with actual `nm` and actual links.

Call-only optimized modern code may contain no remaining reference and may link despite the source-level external-definition requirement. Such objects are evidence of optimization, not permission to execute the incomplete program. The campaign never runs that executable. Address-taking prevents this particular apparent disappearance. This conclusion is confined to observed compiler/ELF behavior; do not generalize exact emission to every optimization, linker or object format.

## T02.11.c

`<stddef.h>` supplies the public `size_t` requirement directly. Include guards allow repeat inclusion. The broken header depends accidentally on its caller's transitive include; the masked control compiles and the first-include control fails on `size_t`. The corrected reference header succeeds first and in the opposite order, and a fresh installation includes only the completed header/library and independent client. Public export instead contains unfinished learner bodies and no assessor tree. Moving a private header into the installed tree or adding repository include paths would invalidate isolation.

## T02.12.a

The malformed function declarator violates syntax. Conflicting declarations in one scope violate the compatible-declaration constraint; the prototyped arity and integer-to-pointer argument cases violate call constraints. N1570 5.1.1.3 requires a diagnostic for a syntax/constraint violation but does not universally require translation failure. `-Werror` is local policy. Both selected compilers can diagnose the type case and return success after explicit diagnostic demotion. GNU mode does not make the other malformed sources valid ISO C.

The indexed read is syntactically valid and constraint-correct. Index3 in an actual four-element array is valid; dereferencing the one-past pointer at index4 is not. A diagnostic need not occur during translation. This distinction is not recoverable from compiler exit status alone. The statement-expression case is an explicitly declared extension and is rejected under the selected strict policy, accepted under GNU17; no equivalence to ISO syntax is claimed.

## T02.12.b

Require each failing command to identify its actual source and intended defect, plus a passing repaired source under the same flags. The retained verifier checks semantic identifiers/categories and rejects missing-file/unknown-option failures. C and C.utf8 were available; both produced English diagnostics. A learner must allow location/caret/wording variation without accepting an unrelated error. This is a negative-test integrity exercise, not a compiler conformance test suite.

## T02.12.c

Successful compilation of `hh_unchecked` does not prove the dynamic precondition. Separate-process AddressSanitizer output demonstrates the selected stack-buffer overread and nothing stronger. Its finding is not an arbitrary-code-execution or privilege-escalation demonstration. The repaired contract still depends on truthful storage and lifetime supplied by the caller.

Clang's documented `security.ArrayBound` assumes an uncertain index is within bounds rather than reporting every possible invalid access. In the actual selected compiler, unknown-index and guarded-index models produce no finding; the explicit index4 branch produces the matching finding and path. The unknown result plus the separate dynamic counterexample falsifies the overclaim that analyzer silence establishes memory safety. The guarded model establishes only the analyzed path property under the tool's assumptions; inspect the guard and source argument independently. ISO C, a diagnostic, runtime instrumentation, and programmer discipline enforce different parts of this contract.

## Evidence and independent transfer

The source-bound campaign covers two compilers, six behavioral profiles, O0/O3 inline matrices, two installed locales, isolated repaired installation, deliberate missing/duplicate links, three compiled semantic mutants and unfinished-learner rejection. These are author observations. The minimum-seven-day criterion variants remain learner work. Evaluate each original criterion's argument and changed condition independently; do not promote the finite command count to complete language conformance or mastery.
