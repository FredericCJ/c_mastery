# T65 research: error models and executable contracts

Inspected on 2026-10-17. The census and SWE catalog are discovery tools, not
authorities. The broad SWE query `railway|refinement|contract|applicative|error`
returned 33 bibliographic/concept records. Its private search output is retained
separately. No book passage was inferred from those records.

The first comparison exposed a misleading equivalence: a C tagged union does
not acquire Rust's exhaustiveness, move checking, or restricted niche-layout
guarantees. Focused inspection therefore read Rust `std::result` sections
Representation, Transforming contained values and Boolean operators, then
`std::option` sections Representation and Boolean operators. Both distinguish
eager value arguments from callbacks that run only on a selected alternative.
These are rolling official documents, observed as Rust 1.99.1 documentation;
compiler execution and cross-version compatibility remain separate experiments.

N1570 printed pages 186–187 and 261–264 were read through the public PDF text.
The selected diagnostics and nonlocal-jump clauses motivate dedicated tests for
disabled assertions, live jump destinations, changed automatic locals and
cleanup ownership. The draft is C11, not the published C17 standard. The exact
C17 access gap from B01 remains open; do not silently substitute N1570 for it.
The arithmetic comparison reuses B01's separately inspected bounded passages.

Rust Reference destructors introduction, operation and drop-scope introduction
were read to separate language-provided destruction from a C callback registry.
The RefCell replacement and borrow/try_borrow documentation was inspected for
T66's next producer wave; it is not evidence that the C borrowing laboratory
already exists. The operator-expression page was inspected only for raw-borrow,
dereference and try-propagation passages; unrelated operator semantics are not
assigned here.

Authorship decisions: use a total finite error domain, preserve output on failed
construction, expose partial progress as a product, distinguish short-circuit
composition from accumulation, and keep nonlocal-jump cleanup a separate hosted
boundary. An observe-mode contract may report a violation and still return a
safe rejection; observation must not turn invalid arithmetic into executable
undefined behavior. Performance claims require equivalent failure contracts.

The three-source requirement is satisfied for the T65 pilot by the four records
in errors_resources.json. This does not close research for T66–T76, the 188 census
links, all SWE elements, or the local library. Independent technical review,
learner assessment and board evidence are not represented by this author gate.

Additional inspection: N1570 6.2.5p19, 6.7.4p8–9 and 6.8.2p1–2 were read for the assessed C08/C12 non-equivalences. The original laboratory contract is separately registered as a course authority, never an ISO authority. All thirteen cost-review dimensions in census/modules/07_engineering_selection.md were read and turned into the shared modern-cost contract.
