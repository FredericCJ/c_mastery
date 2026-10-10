# Language, libraries, algorithms, and architecture: research synthesis

Research date: 2026-10-04. Assigned scope: T01–T06 and T12–T22, with a narrow T35 bridge from inspected allocation passages. This is a research contribution and selection rationale. It does not author the curriculum or certify that every taxonomy objective already has an inspected reading.

## Search, reflection, and second search

The first search used read-only local library catalog queries for modern language texts, reusable C interfaces, historical C, and algorithms, plus the local SWE bibliography for foundational papers and allocator research. Catalog candidates were treated as limited evidence. Six ordinary indexed held PDFs were selected explicitly, with private hashes and bounded text inspection; no opaque package or `_old` traversal took place. Governing rules, role guidance, current pointers, and manifests were pinned privately. The library was not modified.

Primary web discovery initially emphasized author and publisher pages, committee material, institutional archives, and original paper copies. This found a usable language pair, Hanson’s reusable interfaces, CLRS, and the foundational Ritchie/Hoare/Dijkstra/Parnas/allocator works. Reflection against the fine-grained taxonomy exposed three weaknesses in a books-only design: undocumented resource effects, numerical problem conditioning, and the gap between an abstract algorithm and a valid C implementation. The focused second search therefore selected libc documentation and tagged source, Higham, byte-encoding rules, narrow parser material, and documentation contracts. Failed access was recorded rather than promoted into inspected coverage.

The result is 24 selected resources, each with a role, prerequisites, exact inspected locators, and inspection limits. `assignable: true` means that the specified substantive passages can be assigned. It does not mean every chapter in that book, every topic listed in its contents, or every objective in its `domains` array has been read. Assignment scope is deliberately explicit. A failed or bibliography-only candidate remains a search-log lead, not an assignable record.

## Selection decisions

| Need | Selected evidence | Design implication |
|---|---|---|
| Modern C teaching | Modern C third edition; Effective C second edition | Use a modern explanation before historical idioms; attach standard/dialect and support requirements to examples. |
| Exact semantic disputes | Committee draft with precise identity; live committee issue status | A textbook, compiler result, and draft are different forms of evidence. Do not silently turn a later-development draft into the published standard. |
| Reusable components | Hanson, Parnas, concrete SQLite lifetime documentation | Evaluate representation, ownership, failure behavior, and change propagation at the interface. |
| Algorithm reasoning | CLRS, Hoare, Dijkstra, Morin | Require invariants, termination, cost assumptions and a C representation argument; pseudocode does not establish bounds or lifetime safety. |
| Allocator engineering | Hanson arenas, Wilson survey, Lea design account | Separate lifetime strategy, placement policy and implementation mechanism; compare realistic ordered traces as well as adversarial fixtures. |
| Hidden effects | glibc safety/qsort manuals and musl v1.2.5 qsort source | Record heap, automatic workspace, callback and transitive-call effects with implementation/version evidence. |
| Numerical correctness | Goldberg, Higham; narrow DSP supplement | Choose the problem and error measure before choosing the implementation or tolerance. |
| Byte-oriented parsers | RFC 3629; Nystrom scanner narrative | Distinguish byte spans from terminated strings and give borrowed tokens an explicit lifetime. |
| Independent integrator use | Modern C interface documentation; Doxygen commands | Documentation must express preconditions, ownership, errors and effects; generated pages do not prove them. |

The chosen editions are substantive distinctions. The [Modern C author page](https://gustedt.gitlabpages.inria.fr/modern-c/) explains the relationship between its C23 manuscript and the 2025 print edition and warns that numbering/layout can differ. The [Effective C publisher](https://nostarch.com/node/790) gives a September 2024 publication date, despite the held edition’s 2025 copyright. Publication dates and copyright dates must remain separate. Higham’s [author bibliography](https://nhigham.com/accuracy-and-stability-of-numerical-algorithms/) and SIAM listing establish the second edition as 2002.

## Findings that affect project design

The object model must precede increasingly generic code. The inspected Modern C passages separate declared/effective type, alignment, scope, linkage and lifetime; these cannot be reduced to “an address is valid.” A resizing container or arena is therefore an ownership/invalidation problem as well as an allocation exercise. The inspected Effective C allocation pages offer a useful T35 bridge through initialization, terminated-buffer construction, `calloc` size concerns and preserving ownership on failed `realloc`. They do not substantiate unseen security, integer or format chapters.

Historical works are valuable when their assumptions are visible. K&R’s allocator explicitly assumes meaningful ordering across separately obtained blocks. Its function-pointer example and the held extraction also require careful critique. Hanson’s public [Atoms chapter](https://drh.github.io/cii/documents/atom.pdf) includes an exercise that deliberately accepts undefined behavior. These examples should inform a compare-and-repair exercise, followed by a contemporary contract, rather than become a reference implementation. Hanson’s exception macros and global atom/arena state also make useful context and cleanup questions.

The original foundational reasoning works belong beside small implementations. Hoare’s inspected pages make arithmetic assumptions and side-effect restrictions explicit; Dijkstra’s [correctness/implementation discussion](https://www.cs.utexas.edu/~EWD/transcriptions/EWD02xx/EWD249/EWD249.html) shows why proving an ideal program is insufficient. Parnas’s [KWIC example](https://www.cs.lafayette.edu/~gexia/cs301/resources/parnas.html) supports an architecture exercise whose observable evidence is the scope of a representation or policy change. A slogan about “modularity” would lose that testable argument.

Library effects deserve a measured strand. The [glibc qsort documentation](https://sourceware.org/glibc/manual/latest/html_node/Array-Sort-Function.html) describes auxiliary allocation and a fallback. Its [safety concepts](https://sourceware.org/glibc/manual/latest/html_node/POSIX-Safety-Concepts.html) distinguish thread safety from atomicity and mark some annotations as preliminary. The contrasting tagged musl source uses smoothsort and fixed automatic arrays. Those observations establish a useful audit question; they do not prove linked-binary heap freedom, a stack bound, signal suitability, or the effects of user callbacks. Build identity, source revision, instrumentation limits and transitive calls must be part of the later verifier.

Allocator comparisons should retain temporal structure. Wilson’s [survey](https://www.cs.hmc.edu/~oneill/gc-library/Wilson-Alloc-Survey-1995.pdf) explains why object-size/lifetime distributions alone can miss fragmentation caused by ordered allocation patterns. A later fixture suite should combine phase-shaped traces, realistic traces, and named adversarial cases. Lea’s [article](https://gee.cs.oswego.edu/dl/html/malloc.html) helps explain tradeoffs but explicitly says it is outdated; historical thresholds cannot be recast as current glibc facts.

Numerical projects need an independent problem oracle. Higham distinguishes precision, accuracy, conditioning and backward/forward error and gives explicit limitations of compensated summation. The later curriculum should use exact small cases, analytically known behavior, residuals/backward errors, and a high-precision or independent reference where appropriate. Agreement between two binary64 implementations alone is insufficient. The [DSP guide](https://www.dspguide.com/ch16/2.htm) is a narrow practical supplement; its historical integer-size/hardware comments need separation from ISO C semantics.

## Coverage model and progression implications

The research supports a layered progression: diagnose prerequisites; establish declarations/arithmetic/object semantics; build bounded byte/span components; add containers and algorithms; introduce allocator/lifetime choices; then evaluate reusable API and architecture changes. Numerical, implementation-audit and release-quality work recur across that progression. This is sequencing evidence, not a fixed calendar or authored project sequence.

Every fine-grained objective should eventually have a ledger entry distinguishing explanation, inspected reading, implementation, verification evidence, and independent revisit. Core mastery needs a construction or repair task and an observable acceptance condition. Elective awareness needs an honest locator and an explanation of its depth, with deeper construction triggered only by the chosen specialization. A source-domain association alone cannot close the ledger.

Small project families can reuse evolving artifacts without becoming one unbounded capstone. The evidence suggests bounded representation-change tasks, standalone algorithm kernels, arena checkpoints, parser state machines, fault-injected initialization, and independent integration probes. Each 4–12-hour brief needs an explicit stop condition and can be split if new prerequisites appear. Later fixtures should contain normal, boundary, invalid, empty and adversarial cases, plus failure injection where relevant. Briefs and fixtures are compatible with independent study and do not require starter repositories or phase reference solutions.

Independent feedback can come from self-contained invariant checkers, standard/library oracles with documented limits, differential implementations, delayed repair/reimplementation, and a fresh-client integration test. Verifiers should validate properties such as permutation preservation, invalidation guarantees, rollback, bounded work, and numerical residuals. They should not merely reproduce the learner algorithm. Manual reasoning remains necessary for premises that dynamic tests cannot establish.

Mathematical remediation should be just in time: induction and logarithms before invariant/cost work; elementary probability before expected hash behavior; conditioning/error measures before numerical kernels; linear algebra or sampling/convolution before the corresponding elective. Higham and CLRS are references, not an expectation that a learner reads them cover to cover before any coding.

## Rejected alternatives and remaining gaps

- K&R as the primary modern spine: retain historical value, but require modern prototypes, error handling, object rules and edition-aware interpretation.
- Hanson code as a production reference solution: retain design examples while reviewing arithmetic, alignment, lifetime, globals and deliberate undefined-behavior exercises.
- A catalog or contents match as verified chapter evidence: discovery only; exact assignments must stay within the inspected passage set until further inspection.
- A standards project link as the exact published edition: the [WG14 table](https://www.open-std.org/jtc1/sc22/wg14/www/projects) explicitly describes N3220 and N2310 as early later-development drafts. Published C17/C23 full texts were not inspected here.
- A libc function name as a resource contract: use documentation, source, call graph and targeted measurement, with callbacks and compiler transformations accounted for.
- Independent uniformly random allocator traces as the sole benchmark: retain temporal/lifetime structure and workload-specific limits.
- Numerical success as “same as another double result”: name the mathematical problem, error model and oracle.
- Whole-book or whole-interpreter prerequisites: use bounded selections and keep larger interpreter architecture elective. Nystrom’s raw manuscript was inspected, but embedded C listings were not fully expanded.
- GSL and CERT passages as assigned readings based on search results: direct substantive retrieval failed, so these remain leads. No inspected CERT standard record is included.

Further inspection is needed before assigning untouched textbook chapters, advanced data structures/graphs/string algorithms, sophisticated parsing, the full numerical menu, or complete T22 packaging/FFI/release workflows. The current contribution has domain-level selection strength; it does not close every leaf. Published standard access, precise compiler/libc source revisions and executable verification must be resolved by the research/authoring gate. No source implementation was compiled or measured in this research stream.

The public files contain bibliographic metadata, paraphrased selection reasoning and inspection boundaries. Held raw text, personal-use markings, source paths, navigation derivatives, hashes and governing-input pins remain in the private workspace; no book extracts are published.
