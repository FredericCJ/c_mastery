# Independent review: language, assessment and publishing

Status: accepted. All three findings are resolved after inspecting the actual corrections. No open blocking or advisory findings remain in this review scope.

Scope: T01–T06, T12, T18–T22 (129 subtopics and 387 original objectives), all 49 linked project briefs and fixtures, both four-milestone capstones, common entry/assessment/laboratory/audit/profile/inventory guidance, and publishing/export/check scripts.

Review completed: 2026-10-04T07:31:34+00:00. The JSON receipt records 140 final source/evidence hashes and the earlier snapshot. Final snapshot SHA-256: 44a1b8d78f30ef86a4d7593fa3d0c408bdfb113c2e8d2bc80907b071664f6709.

## Resolved findings

### LA-01: Prefix representability is weaker than final-total representability (medium; resolved)

Location: curriculum/projects/PR-T01-01.md.

Reread revised PR-T01-01/T01 teaching, three signed cancellation boundary cases and the checked-prefix oracle; PR-T01-03 also rejects unrepresentable prefixes. Checks occur before addition, bounded staging preserves output on failure, and success/failure-index/publication expectations are explicit. All 49 fixtures and 116 top-level fields recompute without failure.

### LA-02: Required embedded-library backend reading has no concrete source or inspected locator (medium; resolved)

Location: curriculum/projects/PR-T12-05.md.

Reread T12.16.b, PR-T12-05, assigned locator metadata and the merged Picolibc 1.8.10 record. Independently opened the tagged primary os.md/build.md documents: selected globals/FILE/callback, fopen/fdopen, POSIX-console and stdio-option passages support both exact configuration rows and named allocator/POSIX backends. The task explicitly remains documentation/source analysis, with no embedded-library build, transitive-dependency or physical claim. Pinned glibc qsort source and musl source remain distinct from mutable manuals.

### LA-03: Make the updated-keyword-spelling probe explicit (low; resolved)

Location: curriculum/projects/PR-T05-04.md.

Reread T05 teaching and PR-T05-04. The probe explicitly pairs _Bool/bool, _Alignas/alignas and two-argument _Static_assert/static_assert; it separates C17 header macros from C23 keywords, records no-header rejection/extensions and retains underscore alternatives. The bounded N1570 header definitions and N3096 keyword-table inspection are registered with draft identity limits.

The Picolibc recheck used the primary tagged [operating-system interfaces](https://github.com/picolibc/picolibc/blob/1.8.10/doc/os.md) and [build options](https://github.com/picolibc/picolibc/blob/1.8.10/doc/build.md). Inspection covers the named stdio/backend passages, with the documentation-only limit retained.

The spelling recheck retains the authority limits of the [N1570 C11 draft](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf) header definitions and [N3096 working draft](https://open-std.org/jtc1/sc22/wg14/www/docs/n3096.pdf) keyword table. They do not substitute for exact published C17/C23 text.

## Positive evidence

Teaching and topic/project fit: All 129 reviewed subtopics have an original mechanism, concrete investigation, misconception and observable evidence; each is linked to a local brief that repeats topic-specific tasks and evidence. Projects use bounded implementation or explicit source/design-analysis activities.

Readings and source authority: Assigned locators match the registry and remain bounded. Historical K&R/Hanson, drafts, mutable manuals and measured behavior are qualified. Picolibc 1.8.10 documentation now supplies exact embedded stdio configuration/backend rows; pinned glibc and musl qsort source comparisons preserve their distinct authority and limits.

Arithmetic and fixture oracle: Inspected all 49 fixture input/expected pairs and every oracle branch; the independent command recomputed 116 top-level expected fields with zero failures. Prefix arithmetic now checks every intermediate candidate before addition and preserves output on rejection, including both cancellation-shaped boundary failures and a successful boundary trace.

Dependencies and progression: C17 baseline and A/B/C component-versus-harness boundaries are explicit. T18 precedes abstraction/architecture and container consumers. T20 physical adapter evidence is deferred to the relevant embedded gate without granting synthetic conformance.

Short diagnostic and mathematics: T01 six tasks are capped at three assessment hours; failed groups alone trigger remedial projects. Entry guide separates targeted numeric/scheduling remediation from language progression.

Mastery and capstone rubric: All six dimensions must score at least two, with two transfer scores of three; correctness, ownership, dependencies and failure handling cannot be offset. Both four-milestone capstones require provenance, failure/overload, shared-core reuse, perturbation and physical evidence where applicable.

Publishing implementation: Publisher validates before rendering, inserts exact objective text/anchors and inspected readings, copies fixtures as downloads and emits same canonical prose into site/Markdown. Guarded source cleanup is confined to build/source. HTML/Markdown checks inspect destinations, objective fidelity and unconverted roles; package excludes private data and independently rebuilds extracted public source.

The revised acceptance and fixture guards were also statically inspected. They enforce pass/accepted statuses, bind actual supplied fixtures to the global catalog, and keep curriculum acceptance separate from archive extraction/rebuild evidence. Rendered asset references remain checked while theme Jinja templates are excluded as browser pages. The final MathJax path and equivalent target/existence caches were reread.

## Verification and limits

The language fixture command recomputed all 49 fixtures and 116 top-level expected fields without failure. Current scoped module/project contracts and exact registered readings pass validation. Every supplied input/result pair and oracle branch was inspected; no other author’s source was edited.

This acceptance covers authored teaching, project fit, fixture correctness and static publication implementation in the stated scope. It does not establish learner mastery or physical hardware behavior. The integrator owns the clean build, rendered browser/equation review and source-export reproduction receipts.


## Final publication reproducibility recheck

Status: accepted (2026-10-04T07:41:01+00:00).

Read the actual publish.py cleanup and package.py implementation plus the diagnosis receipt. Generated HTML is removed only after preparation/validation and after resolving and checking the exact intended ROOT/build/html target; the source cleanup uses the same exact literal-directory boundary. Both checks precede existence, creation and deletion branches. Fresh output prevents stale incremental search postings. Packaging excludes .buildinfo, .buildinfo.bak and .doctrees as build metadata, while retaining byte comparison of every exported public asset including searchindex.js. Current generated directories were observed as ordinary directories. No broad directory deletion or comparison exemption is introduced.

The integrator runs the clean build and fresh extracted-source reproduction; this static recheck does not claim their results in advance.

Updated JSON snapshot SHA-256: 6d40c7fb33ede8f9b20d6503428189801d00c25ea698d728bb17596aed67d263. The initial completion snapshot above remains an earlier review checkpoint.
