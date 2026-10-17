# B01 source inspection and authoring reflection

Inspection date: 2026-10-16. These are authoring records, not independent review
or learner acceptance. The exact assignable passages and identity limits live in
`b01_resources.json`. Existing accepted source records were not broadened.

## Actual primary passages inspected for this batch

| Record | Inspection and use |
|---|---|
| R-B01-N1570-OBJECT-ARITH | Actual N1570 PDF text: diagnostics; lifetime; unsigned arithmetic; representation; alignment; promotions/conversions; effective type; call sequencing; pointer arithmetic; shifts; logical AND; assignment constraints; restrict; exact/greatest-width typedefs. See exact paragraph ranges in the resource record. Web passage text and an in-memory pdftotext check were read; fetched PDF SHA-256 is recorded. This is a C11 committee draft. |
| R-B01-C17-DR452 | Actual issue identity, Fixed in C17 status and final temporary-object effective-type corrigendum read. Earlier proposal wording was distinguished from the final correction. |
| R-B01-C17-DR400 | Actual issue identity and final zero-size realloc correction read. Its final ownership rule differs from early suggested language; zero-size realloc is not used by the slot implementation. |
| R-B01-GCC-STANDARDS | Actual official C-language mode/correction and hosted/freestanding passages read. Mode selection is not historical semantic reconstruction; compiler-runtime dependencies require image evidence. |
| R-B01-GCC-OVERFLOW | Actual generic add/subtract/multiply overflow builtin descriptions read, particularly result-write and mathematical-operation behavior. Comparative wrappers must restore transactional semantics. |
| R-B01-N3096-CHECKED | Actual N3096 7.20–7.20.1, printed page311, read after fetching into memory and extracting with pdftotext. Header contract and complete checked-operation semantics inspected. It is a pre-publication C23 draft, not exact ISO/IEC9899:2024. |
| R-B01-RUST-DROP | Actual official reference opening destruction rules and explicit suppression/non-unwinding termination passages read. Used to bound, rather than overstate, C-versus-Rust cleanup comparisons. |
| R-B01-RUST-MOVE | Actual official book ownership rules and moved-String compile-rejection explanation read. Used for one concrete language-enforcement comparison, not a claim of full Rust equivalence. |

No source passage is reproduced in the learner publication. The exercise
mechanisms, fixtures, counterexamples, state machines and rubrics are original
curriculum work. Source references identify authority for language/tool claims;
they do not claim that the source authored these exercises.

## Access attempts and reference gaps

The repository audit reported that exact N2176/C17 text could be fetched but its
password protection blocked ordinary inspection. This is carried forward as
`G-B01-ISO17`, not silently replaced with N1570 or an early C23 draft.
The separate audit's held Modern C and Effective C inspections are useful
discovery/teaching evidence but were not repeated or newly claimed by this author.
No Hanson passage was assigned or claimed inspected here.

N3096's web text search did not find checked-arithmetic strings, and a screenshot
fetch timed out. An initial Python extraction attempt failed because pypdf was
absent. The installed pdftotext tool then successfully extracted the primary
PDF directly from in-memory fetched bytes. Only the actual bounded page311
reading is assigned; the failed attempts establish no inspection by themselves.
The exact published C23 edition remains distinct from the inspected draft.

Rust slice documentation was opened as a discovery lead but no slice-specific
reading was assigned; the required comparisons use the inspected move/destructor
passages. Live manuals require installed-version matching for new measurements.

## Depth decisions and unresolved questions

The eight exercises are separated by mechanisms: semantic inference; arithmetic
admission; object-bounded views; wire/value representation; staged construction;
overlap/access promises; finite identity; and authorized composition. Public
tests intentionally cannot grade a semantic explanation or delayed transfer.
Source-bound exercises specify objective-level rubric criteria and separate
author execution from independent assessment.

The raw-storage baseline's four interval-overlap fixture cases did not establish
a typed construction lifecycle, aliasing proof or generation exhaustion behavior.
Those omissions motivated explicit callback states, restrict analyses and finite
identity histories. A broad original objective may remain only partly deepened:
the overlap exercise does not alone close all memory-defect repair skills, and
the handle exercise does not settle formal provenance models.

Open research/validation work includes exact C17 authority access, independent
content review, learner trials and calibration, delayed-assessment results,
physical validation where applicable, and broader indeterminate-value/provenance
models. No single-answer execution puzzle is based on unresolved indeterminate
value behavior. DR0451 is mentioned by the broader audit as a closed committee
response, not recorded here as an adopted C17 wording correction or newly
inspected source. Corpus-wide discovery and the T57–T76 progression remain in the
root completion ledger; B01 is not the entire curriculum expansion.

## Traceability and evidence

Each canonical exercise maps exact source locators to existing leaf IDs, D1–D4
artifacts, four coverage facets and explicit rubric criteria. Every facet is a
specified bounded claim, not evidence that a learner passed. Assessor solutions
and precise delayed cases are separate under `assessor/foundation_depth/analyses`.
The shared learner contract fixes C17, dependency separation, non-reentrant
callbacks, object/disjointness assumptions, status meanings and 0–3 assessment.
The research gate hashes this reflection and the additive resources; it can
establish current authoring inputs, never independent reviewer acceptance.
