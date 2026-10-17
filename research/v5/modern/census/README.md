# Modern Programming Techniques — Systems-Oriented Census

**Edition:** 2026-10-13  
**Coverage:** 100 techniques; four mandatory analytical fields per technique.  
**Primary focus:** C++20/23/26, Rust, Go, Swift, Zig, Jai, V, Nix, Odin, C3, and Roc, with additional reference languages where they clarify a distinct mechanism.

## Read the reference

Start with [KERNEL.md](KERNEL.md) for the scope, guarantee classes, and cost model. Use [INDEX.md](INDEX.md) to locate a technique, or read [COMPLETE_CENSUS.md](COMPLETE_CENSUS.md) as one continuous document. The modular and complete editions contain the same substantive census.

| File | Contents |
|---|---|
| [01_memory.md](modules/01_memory.md) | M01–M26: ownership, views, allocation, layout, lifetime, and reclamation |
| [02_control.md](modules/02_control.md) | C01–C24: errors, vocabulary types, cleanup, iteration, effects, and concurrency |
| [03_polymorphism.md](modules/03_polymorphism.md) | P01–P26: constraints, dispatch, generics, type erasure, and metaprogramming |
| [04_data_modeling.md](modules/04_data_modeling.md) | D01–D24: ADTs, invariants, typestate, protocols, capabilities, and boundaries |
| [05_language_crosswalk.md](modules/05_language_crosswalk.md) | Language-specific guarantees, C++ edition map, and maturity caveats |
| [06_layout_and_examples.md](modules/06_layout_and_examples.md) | Layout models, complete examples, local observations, and device-state design |
| [07_engineering_selection.md](modules/07_engineering_selection.md) | Legacy-risk routing, cost-review sheet, migration, and misconceptions |
| [08_glossary.md](modules/08_glossary.md) | Terminology and distinctions that are often conflated |
| [SOURCES.md](SOURCES.md) | Linked primary-source registry |
| [VALIDATION.md](VALIDATION.md) | Document checks and narrowly scoped example-test record |

Every numbered entry covers **modern entities**, **classical antecedents**, **mechanism/layout**, and **trade-offs/footprint**. Reference labels are clickable in standard Markdown renderers. Explicit HTML anchors provide stable technique links.

## Examples

The worked examples are embedded in the Markdown and also provided as [C++23 source](examples/packet_view.cpp) and [Go source](examples/go_representation.go). The C++ example passed compilation and focused behavioral checks with GCC 14.2.0 on x86-64. The Go example passed with Go 1.23.2 on linux/amd64. These tests are not a cross-language conformance suite, an ABI guarantee, or a benchmark.

## Evidence boundaries

C++26 facilities are identified from working-draft documentation, not assumed available on all toolchains. Rolling documentation is tied to the research date rather than guaranteed immutable. Jai receives an explicitly evidence-limited profile rather than invented current implementation details. Performance discussions are mechanisms and engineering analysis unless an observation is specifically labeled as measured.
