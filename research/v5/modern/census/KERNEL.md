# Reading kernel: guarantees, representations, and cost

**Research snapshot:** 13 October 2026. **Scope:** 100 distinct techniques in four categories, with language comparisons and engineering appendices. This is a broad architectural census, not a claim that every programming-language technique has been enumerated or that all listed implementations provide equivalent guarantees.

## What “modern” means here

The organizing direction is to move implicit contracts into explicit representations: ownership into resource types, absence and failure into vocabulary types, valid alternatives into sums, behavioral requirements into constraints, and protocol sequencing into state-indexed interfaces. Many foundations—RAII, algebraic data types, regions, parametric polymorphism, statecharts, and persistent collections—are older than their current implementations. Contemporary uptake is not a historical invention date.

The antecedent in each entry identifies a specific mismatch or risk. It does not declare that inheritance, raw pointers, C ABI interfaces, exceptions, mutable records, or integer flags are universally inappropriate. An open plugin boundary may require dynamic dispatch; a hardware register demands explicit low-level access; a C ABI can be the most stable integration boundary. The objective is to confine the obligations of those mechanisms and make their contracts reviewable.

## The four-point entry contract

Every numbered technique contains the same four fields:

1. **Modern approach & concrete entities:** terminology and actual language or library primitives.
2. **Classical antecedent / historical anti-pattern:** the specific design weakness or risk being addressed.
3. **Underlying mechanism & memory layout:** representation, lowering, dispatch, lifetime, and storage behavior.
4. **Engineering trade-offs & footprint:** runtime, compile-time, binary-size, predictability, and maintenance consequences.

Stable identifiers are `M01–M26`, `C01–C24`, `P01–P26`, and `D01–D24`. These identifiers refer to techniques, not languages or chronological generations. Neighboring entries deliberately separate mechanisms often conflated in discussions: a slice versus a borrow, a result versus propagation syntax, a constraint versus a code-generation strategy, or typestate versus proof of physical device state.

## Classify the guarantee before selecting the feature

| Guarantee class | What it establishes | What remains outside it |
|---|---|---|
| Static language rule | Admissibility of a program under the language's typing, ownership, or effect rules | Assumptions at unsafe/foreign boundaries; external behavior; resource budgets |
| Library invariant | Properties established by restricted constructors and invariant-preserving methods | Bugs in trusted implementation; bypasses; undocumented mutation paths |
| Runtime enforcement | A check, lock, counter, collector, or scheduler enforces a condition while executing | Associated instruction, memory, latency, and failure costs |
| Optimization opportunity | A compiler may erase, inline, specialize, reuse, or elide work | No unconditional cost guarantee without stronger specification or measurement |
| Documented layout/ABI guarantee | A specified representation relationship or calling convention | Other targets, representation modes, language versions, and wire semantics |
| Working-draft or evolving facility | A documented design available for evaluation | Uniform compiler support, stable APIs, and production qualification |

Rust's lifetime checks normally do not add runtime lifetime metadata, but bounds checks and allocations are separate matters. C++ `span` packages access without establishing the same lifetime proof. The Rust layout reference explicitly distinguishes guarantees from current implementation observations; this census follows that distinction rather than turning an observed fat-pointer size into a universal ABI rule. [RUST-BORROW] [RUST-LAYOUT] [CPP-SPAN]

## Treat ownership, access, storage, identity, and effects independently

For any value, record five independent answers. Who destroys it? Who may read or mutate it, and for how long? Where is its storage, and can that storage move? What identifies it across time? Which effects can operations perform? A heap object can have value semantics. An inline object can borrow memory owned elsewhere. A logical handle can remain stable while an allocation moves. A pure function can still need substantial storage. A noncopyable object can still be leaked.

Similarly, distinguish a **closed** alternative set from an **open** implementation set. A sum is effective when the domain cases are intentionally enumerated. An existential interface is effective when new implementations must be added without changing the consumer. Neither dimension alone specifies allocation or dispatch: a sum may contain pointers; a borrowed trait object may avoid allocation; a concrete generic may still carry runtime metadata. [RUST-DYN] [CPP-VARIANT] [SWIFT-CALLING]

## Evaluate zero-cost claims against an equivalent contract

A useful cost ledger is:

```text
runtime work = useful algorithm work
             + validation and bounds checks
             + dispatch and metadata access
             + allocation and reclamation
             + copies and ownership bookkeeping
             + synchronization and scheduling
             + cache/TLB and branch effects

engineering footprint = runtime work and peak live storage
                      + binary/ROM size and stack/frame size
                      + clean/incremental compile cost
                      + diagnostic and verification complexity
```

This is a classification of costs, not an additive timing model: processors overlap work, optimizations interact, and memory effects can dominate instruction counts. Compare implementations with equivalent safety, failure, and lifetime contracts. An unchecked raw-pointer loop is not a fair performance baseline for an abstraction required to reject invalid bounds unless the raw implementation includes the same obligation.

“Zero-cost” also differs from allocation-free, lock-free, wait-free, deterministic, real-time-safe, and small-binary. A specialized generic can have no dispatch overhead while expanding code substantially. A scope destructor can be deterministic but take unbounded time because the owned graph is unbounded. An arena can make allocation cheap while retaining excessive dead storage. A nonblocking algorithm can still depend on a reclamation scheme that delays memory reuse. [RUST-MONOMORPH] [CPP-ARENA] [CROSSBEAM-EPOCH]

## Source and maturity policy

Language facts are linked to specifications, official manuals, implementation documentation, project-authored library documentation, or original research. The engineering analyses are architectural deductions rather than benchmark claims. Source labels such as `[RUST-PIN]` are stable reference identifiers and clickable links in rendered Markdown. The complete source registry is included in the package.

C++26 entries refer to facilities documented in the current working draft, not to a promise that every compiler or standard-library combination implements them. Rolling `main`, `master`, and `latest` documentation is identified by the research snapshot; URLs can subsequently change. V's memory-management modes, Roc's new-compiler syntax, and Jai's evidence limitations receive explicit treatment in the language crosswalk. No all-language conformance or performance test was performed. Only the narrowly identified C++ and Go examples in the validation appendix were compiled and run.

<!-- REFERENCE DEFINITIONS -->
[CPP-ARENA]: https://eel.is/c++draft/mem.res.monotonic.buffer "C++ working draft: monotonic_buffer_resource"
[CPP-SPAN]: https://eel.is/c++draft/views.span "C++ working draft: span"
[CPP-VARIANT]: https://eel.is/c++draft/variant "C++ working draft: variant"
[CROSSBEAM-EPOCH]: https://docs.rs/crossbeam-epoch/latest/crossbeam_epoch/ "Crossbeam epoch-based reclamation documentation"
[RUST-BORROW]: https://doc.rust-lang.org/book/ch04-02-references-and-borrowing.html "The Rust Book: references and borrowing"
[RUST-DYN]: https://doc.rust-lang.org/reference/types/trait-object.html "Rust Reference: trait objects"
[RUST-LAYOUT]: https://doc.rust-lang.org/reference/type-layout.html "Rust Reference: type layout"
[RUST-MONOMORPH]: https://rustc-dev-guide.rust-lang.org/backend/monomorph.html "Rust Compiler Development Guide: monomorphization"
[RUST-PIN]: https://doc.rust-lang.org/std/pin/index.html "Rust standard library: pinning"
[SWIFT-CALLING]: https://raw.githubusercontent.com/swiftlang/swift/main/docs/ABI/CallingConvention.rst "Swift ABI: calling conventions"
