# Systems research evidence and decisions

Research captured on 4 October 2026. The contribution contains 35 bounded teaching resources and two separate, unassignable bibliographic leads. It covers T07–T11, T13 and T23–T26. These are research findings and source-selection constraints, not completed platform experiments.

The strongest result is a usable chain of evidence from C translation to loaded process: the held CSAPP linking chapter, versioned compiler/linker manuals, explicit ELF/AMD64 definitions, selected Linux loader code, and contrasting glibc/musl startup files. Linux/POSIX and concurrency likewise have a textbook explanation layer and an independently inspected contract layer. Historical papers explain design choices without becoming present-day API authority.

## Discovery and trust

The broad SWE pass searched the union of `corpus.nodes` and `elements.works` across all collections: 1,257 unique works, 148 systems-related candidates. All top-level report documents were also searched; 20 contained relevant hits. This is a reference index, not the actual library. Its verification labels, citations and collection placement are discovery evidence. They do not establish that a passage exists in a particular edition or that a linked implementation still matches current behavior.

Knowledge control documents, the shared contract and the metadata-provenance role were read before discovery. The actual catalog was opened explicitly in immutable read-only mode, and historical root paths were rebased only after existence checks. The catalog has 4,803 documents and limited-evidence search enrichment, not book full-text search. Broad title/author queries returned 13 distinct records. Exact-title and conjunctive metadata misses are therefore not absence proofs. No library edits, `_old` traversal, opaque-package traversal or builds occurred.

The held TLPI and CSAPP files were matched to their catalog hashes and inspected in bounded private extracts. A critical edition correction emerged: the CSAPP file is Third Edition **GLOBAL EDITION**. Its Chapter 7 uses printed pages 705 onward; US-edition page numbers cannot be reused. The imprint gives a 2016 copyright, while publication year is left null because that field was not separately established. TLPI is the 2010 first edition; physical PDF pages have a +44 offset from printed pages. Its explanatory examples need current POSIX and Linux cross-checks.

Raw book text, captured web bodies, rendered inspection images, archives, SQL results and resolved library paths remain in private research evidence. Public records expose bibliographic facts, bounded locators, inspection limits and hashes; they do not redistribute book extracts. Version identifiers and content hashes anchor the web evidence. Downloaded-source hashes identify inspected bytes, not an independent cryptographic authenticity audit.

## Reflection and focused pass

After discovery, a written reflection identified four failure modes: selecting a title without opening its passages; assigning pages from the wrong CSAPP edition; treating old textbook behavior as a current contract; and collapsing ISO C, POSIX, hardware/ABI, libc and the kernel memory model into one semantics. The focused pass then searched those gaps directly. A second all-collection SWE query returned 56 records. A focused Knowledge networking query found two UNIX Network Programming volumes, while a conjunctive concurrency query returned no results. Those uninspected volumes remain leads; current socket and epoll excerpts supplied the bounded networking claims.

Focused primary-source inspection recovered official glibc 2.41 and musl 1.2.5 release archives, with only three named startup files read from each. GNU make HTML access failed; the official 4.4.1 release Texinfo supplied the inspected prerequisite sections. Clang HTML arrived compressed on one endpoint and was decompressed before semantic inspection. Lamport’s OCR lost negation marks, so printed page 559 was rendered and visually checked before interpreting concurrent events. These corrections are material evidence checks, not merely download bookkeeping.

## Source roles and subject coverage

| Domains | Evidence selected | Boundary for later assignment |
| --- | --- | --- |
| T07 builds/toolchains | GCC 15.2, Clang 21.1, GNU make 4.4.1, CMake 3.31.12, Ninja 1.13.1 | Assign inspected dependency/stage/runtime subsections; do not assume target defaults or reproducibility from documentation alone. |
| T08–T09 ABI/ELF/linking | CSAPP Global Edition Chapter 7, binutils 2.45 scripts, dated gABI and AMD64 drafts | Keep symbols, sections and load segments distinct; old ABI drafts are a bounded baseline, not the latest specification. |
| T10 startup | AMD64 initial-stack excerpt, Linux v6.12 `binfmt_elf.c`, glibc/musl entry files, `execve(2)` and loader search order | Separate kernel entry, runtime initialization and `main`; selected source ranges are not a complete startup proof. |
| T11/T13 minimal runtimes and hidden costs | GCC suppressed-library options, Clang freestanding/runtime documentation, concrete libc initialization paths | Freestanding flags can leave memory helpers, compiler helpers and atomic runtimes. A small source file does not prove a library-free or allocation-free binary. |
| T23 processes/mappings/files | TLPI errors/mappings, POSIX read excerpt, current Linux read/exec documentation | Explain partial transfer, offsets and state replacement using contracts; do not turn a historical or textbook example into a portability guarantee. |
| T24 signals/network/events | Linux socket-layer overview, epoll LT/ET subsection, signal-safety opening discussion; UNIX history | Readiness needs a state machine and partial-I/O handling. Signal-handler safety is a separate contract from ordinary reentrancy. |
| T25 threads/memory ordering | TLPI mutex/condition-variable passages, POSIX.1-2024 §4.15 and condition-wait specification, Boehm history/rationale | Predicate rechecking is required after condition waits. Ordinary racing accesses have undefined behavior; mutex order, atomic modification order and sequential consistency are different claims. |
| T26 advanced concurrency/kernel | Michael–Scott queue safety/algorithm excerpt, kernel barrier guide and LKMM relation explanations | Historical pseudocode needs modern atomic orders, lifetime/reclamation and target-width proofs. Kernel primitives and LKMM are optional separate semantics. |

CSAPP and TLPI serve as primary exposition. Official versioned manuals and POSIX provide selected contract oracles. Source files let later work test claims against an implementation. Lamport, Ritchie/Thompson and early Boehm work are history; Boehm–Adve gives historical language-design rationale. The queue paper is an advanced case study whose assumptions must be made explicit.

## Substantive findings

Compiler and linker suppression flags are independent controls. Removing startup objects is different from removing default libraries, and freestanding compilation changes assumptions without proving that generated code has no external symbols. GCC and Clang both document memory-helper requirements; Clang additionally documents non-native atomic and sanitizer runtime calls. Later minimal-runtime work should inspect undefined symbols and the linked dependency closure, rather than infer dependencies from source spelling.

The entry boundary is equally important. The kernel constructs a stack and auxiliary vector; platform entry assembly recovers arguments and aligns the stack; libc prepares facilities before calling the application and handling exit. The inspected glibc and musl files make this sequence concrete, but configured macros, TLS implementation and loader paths remain outside the selected closure. Neither a direct syscall example nor a small CRT file establishes full runtime equivalence.

Memory-ordering explanations require several separate orders. POSIX defines conflict, per-object modification order, release/acquire relations and happens-before, and specifies which thread operations synchronize memory. Lamport’s 1978 distributed relation is causal history, not a substitute for those rules. The Boehm–Adve race-free theorem is qualified; it does not make arbitrary relaxed-atomic programs sequentially consistent. Linux’s barrier guide explicitly disclaims being an infallible hardware specification. LKMM further documents limitations involving compiler transformations and mixed-size accesses. Those qualifications must remain visible in later examples.

Current interface pages constrain older system-programming prose. A successful read can be short; an edge-triggered event loop must consume readiness correctly; exec preserves some process attributes while replacing others; and stdio is unsuitable for the inspected signal-handler use case. The selected pages support these particular claims, not every error path or every platform.

## Rejections, gaps and next evidence requirements

Levine remains a **bibliographic/ToC lead, not an assignable teaching resource**. The publisher confirms first edition, 1999, while the author page says manuscript chapters were withdrawn. No ordinary held catalog match or chapter passage was established. Do not recover guessed historical download URLs or use third-party book dumps. CSAPP, binutils and ABI excerpts cover the verified linking foundation until an authorized edition can be inspected.

The original 2005 Boehm paper likewise remains a separate unassignable lead after inaccessible/authentication-returning copies. The approved resource is explicitly his author-hosted presentation, and does not claim paper inspection. Robert Love’s held second edition was discovered but not selected as another primary spine: the inspected material did not add necessary verified coverage beyond TLPI/current contracts. The historical UNIX resource is the **1978 BSTJ revision**, explicitly distinguished from the 1974 CACM original.

No blocker prevents the core systems research gate. Important remaining limits are: current non-x86 ABI coverage; full cross-build/reproducible-build configuration; bounded-stack/allocation call-closure proofs; complete signal/cancellation/robust-mutex treatment; network protocol correctness beyond socket/event contracts; and a modern, independently inspected memory-reclamation treatment for advanced lock-free implementations. These are topic-specific evidence needs. They should constrain future required readings and proof claims, not be hidden by assigning an entire uninspected book or by labeling a model run a proof.

No experiments or builds were run in this research contribution. Later empirical checks need their own target, tool versions, artifacts and interpretation. A stress test, sanitizer result or litmus outcome can supply evidence within its tested scope; none alone establishes portable correctness.

## Authoring follow-up inspections

Nine precise primary records now close specific source gaps discovered while authoring: CMake 3.31.12 cross/sysroot search, Configure Preset fields and install/export/relocatable-package passages; Linux man-pages 6.19 dlopen initialization/ownership and flock/fsync cooperation/persistence passages; GNU ld 2.45 symbol versions; GCC 15.2 GNU TLS extension; and tagged LLVM 21.1 MOV-immediate declarations. These inspections support bounded mechanism assignments without claiming successful builds, plugin runs or crash experiments. The CMake examples use an old toolchain/C++ illustration, while the authored C exercise supplies its own current environment and separate consumer. Linux captures are version/footer/hash anchored, despite moving URLs. The LLVM source is a declarative backend example, not a complete processor specification.

Complete modern reclamation implementations remain gated on additional independently inspected protocols and review. Core T26 awareness uses explicit abstract models and never upgrades their results into hazard-pointer/RCU safety claims.


Review closure inspected four additional sources. The pinned qsort pair supplies concrete allocation/callback and fallback distinctions; the local replacement deliberately handles only at most sixteen integer elements. Kernel work drain is taught with the no-racing-enqueue and calling-context limits, separate from a memory barrier. C/POSIX thread lifecycle mapping remains an interface study around one worker implementation. The later WG14 draft is labeled as a cross-check, with the selected C17 baseline and actual support reported independently; protected N2176 content is not assigned.
