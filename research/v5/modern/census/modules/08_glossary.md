# VIII. Terminology and non-equivalences

The definitions below provide a compact reading aid. Entries elsewhere in the census provide the language-specific qualifications and primary sources.

| Term | Precise meaning in this census |
|---|---|
| ABI | The binary contract governing calling conventions, layouts, symbols, and related interoperability rules. Source compatibility does not imply ABI compatibility. |
| Affine type discipline | A usage discipline permitting at most one consumption of a value; unused values may be discarded under the relevant rules. |
| Algebraic data type | A data model built from product and sum constructions, potentially recursively. “Algebraic” describes compositional structure, not necessarily numeric content. |
| Algebraic effect | An operation treated through an effect system and corresponding handlers; not synonymous with returning `Result`. |
| Alias | Another access path to the same storage or resource. Copying a descriptor can create aliases without copying elements. |
| Arena / region | Storage managed according to a collective lifetime, often with bulk reclamation. Object cleanup and memory reclamation are distinct. |
| Borrow | Temporary access whose validity depends on another owner or scope; the degree of static enforcement depends on the language. |
| Brand | Type-level identity intended to distinguish values from different categories or issuing scopes. Uniqueness requires a sound construction scheme. |
| Capability | A value conveying access and authority over a resource or operation set; a narrower concept than an arbitrary dependency parameter. |
| Closure environment | The captured state accompanying a callable. It can contain values, references, owners, or shared handles. |
| Coherence | Restrictions ensuring implementations of an abstraction can be selected consistently, including constraints on overlapping or external implementations. |
| Copy-on-write | Sharing backing storage until a write requires separation to preserve value semantics. |
| Dictionary passing | Supplying operation/type evidence as additional runtime arguments to generic code that may be shared across types. |
| Discriminant | Information identifying the active case of a sum; it need not occupy a separately stored tag byte. |
| Drop glue | Compiler-generated destruction logic that invokes the relevant cleanup for a value and its owned components. |
| Erasure | Removal of compile-time distinctions or evidence from runtime representation; alternatively, hiding a concrete type behind a runtime abstraction. Specify which meaning applies. |
| Escape analysis | Analysis of whether an object's lifetime/access escapes a scope, used among other purposes to select allocation placement. |
| Existential type | Packaging a hidden concrete type together with enough interface evidence to operate on it. |
| Fat pointer | A pointer-like representation with additional metadata, such as slice length or dynamic dispatch information. |
| GADT | A generalized algebraic data type whose constructors can establish type-index relationships refined by pattern matching. |
| Generational handle | An index or identity combined with an incarnation counter to detect selected stale references after reuse. |
| Higher-kinded abstraction | Abstraction over type constructors, such as an operation generic over a family `F<T>`, not merely over an ordinary type `T`. |
| Interior mutability | A controlled mechanism for mutation through an otherwise shared interface; enforcement may be dynamic or synchronization-based. |
| Linear type discipline | A usage discipline requiring exactly one use under its formal rules; not a blanket guarantee that an external resource is eventually released. |
| Monomorphization | Producing implementations specialized to concrete substitutions of generic parameters. |
| Newtype | A distinct type identity wrapping or otherwise reusing an existing representation, typically to separate domain meanings. |
| Niche | A bit pattern unavailable to valid values of a type that can be used to encode another alternative without separate tag storage. |
| Nominal typing | Type identity or compatibility depends on declarations/names and specified conformance relationships, not only on structural shape. |
| Opaque concrete type | An unnamed or hidden concrete type fixed by a declaration or instantiation, distinct from a runtime existential choice. |
| Phantom parameter | A type parameter that influences static reasoning without requiring a runtime value of that parameter. |
| Pinning | A contract restricting selected movement of a pointee; distinct from physical-page pinning or DMA addressability. |
| Persistent collection | A structure that preserves access to earlier versions after updates, usually through sharing; not necessarily disk persistence. |
| Product type | A value containing all components simultaneously; finite-domain cardinalities multiply. |
| Provenance | The relationship between an access path and the allocation/permissions from which it originates; not merely its numeric address. |
| Refinement type | A base type constrained by a predicate available to a verification system. |
| Row polymorphism | Abstraction over extensible record or variant structure while tracking known fields/cases; not ordinary dynamic dictionary access. |
| Session type | A type-level protocol describing communication direction, payload, continuation, and potentially branching or recursion. |
| Structural typing | Compatibility derived from operations or structural shape according to the language's rules. It does not imply equal physical layout. |
| Sum type | A value holding one of several alternatives; finite-domain cardinalities add. |
| Type erasure | Hiding concrete type identity at an interface, usually retaining runtime operation metadata when behavior is needed. |
| Typestate | Encoding the operations permitted in a protocol phase using different types or type parameters. |
| Value semantics | Operations behave as independent logical values; implementation sharing can remain invisible until required. |
| Vtable / witness table | Runtime operation or conformance metadata used to invoke behavior for an otherwise abstract concrete type. |
| Zero-cost abstraction | An abstraction intended not to add unavoidable overhead relative to a comparably correct lower-level implementation; it is not a universal timing or memory guarantee. |

The crucial non-equivalences are **ownership ≠ allocation**, **view ≠ lifetime proof**, **noncopyable ≠ guaranteed destruction**, **value semantics ≠ deep copy**, **generic ≠ monomorphized**, **explicit failure ≠ no other failure channel**, **cancellation ≠ rollback**, **pinning ≠ DMA mapping**, and **declarative ≠ reproducible by itself**. The mechanisms underlying those distinctions are documented in the corresponding language and runtime references. [RUST-OWNERSHIP] [CPP-SPAN] [RUST-FORGET] [SWIFT-ARRAY] [GO-DICTIONARIES] [CPP-EXPECTED] [TOKIO-SELECT] [RUST-PIN] [NIX-DERIVATIONS]

<!-- REFERENCE DEFINITIONS -->
[CPP-EXPECTED]: https://eel.is/c++draft/expected "C++ working draft: expected"
[CPP-SPAN]: https://eel.is/c++draft/views.span "C++ working draft: span"
[GO-DICTIONARIES]: https://raw.githubusercontent.com/golang/proposal/master/design/generics-implementation-dictionaries-go1.18.md "Go compiler design: generic dictionaries and gcshape sharing"
[NIX-DERIVATIONS]: https://nix.dev/manual/nix/2.34/language/derivations "Nix 2.34 reference manual: derivations"
[RUST-FORGET]: https://doc.rust-lang.org/std/mem/fn.forget.html "Rust standard library: mem::forget"
[RUST-OWNERSHIP]: https://doc.rust-lang.org/book/ch04-01-what-is-ownership.html "The Rust Book: ownership"
[RUST-PIN]: https://doc.rust-lang.org/std/pin/index.html "Rust standard library: pinning"
[SWIFT-ARRAY]: https://raw.githubusercontent.com/swiftlang/swift/main/stdlib/public/core/Array.swift "Swift standard library: Array implementation and documentation"
[TOKIO-SELECT]: https://docs.rs/tokio/latest/tokio/macro.select.html "Tokio: select and cancellation safety"
