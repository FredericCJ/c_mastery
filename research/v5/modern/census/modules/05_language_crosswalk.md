# V. Language crosswalk and maturity boundaries

This crosswalk maps the census to concrete language families. It is not a feature-count ranking. A language can emphasize explicit data layout without a borrow checker, static algebraic modeling with runtime memory management, or declarative dependency construction outside the native execution domain.

<a id="language-cpp"></a>
## C++20, C++23, and C++26

C++ combines value-oriented resource wrappers, static generic programming, explicit low-level access, and optional dynamic polymorphism. RAII, moves, and standard containers predate the requested editions. Modern library vocabulary improves signatures, but raw-pointer and reference lifetimes are not generally checked as Rust borrow relationships. Concepts constrain generic interfaces; they do not prove semantic laws such as comparator ordering. [CPP-GUIDELINES] [CPP-CONSTRAINTS]

| Edition grouping | Census-relevant facilities | Important boundary |
|---|---|---|
| C++20 | `span`, concepts and `requires`, ranges, coroutines, modules, `consteval`, `[[no_unique_address]]` | Views are not a borrow checker; modules are not ABI stability; coroutines are not a scheduler |
| C++23 | `expected`, optional/expected monadic operations, `mdspan`, explicit object parameters, `if consteval`, `move_only_function` | Value operations may throw or allocate; explicit object parameters do not make moves consuming in Rust's sense |
| C++26 working-draft facilities | Contracts, static reflection, execution/senders, `inplace_vector`, `hive`, `indirect`, `polymorphic`, `function_ref`, data-parallel `<simd>` | Support is compiler/library-specific; consult the cited draft sections and feature tests |

The facilities and their semantics are documented in the linked draft sections rather than inferred from edition names. Some referenced sections contain later evolution of an older facility; the edition grouping identifies introduction of the named core facility, not every overload visible in a rolling draft. [CPP-SPAN] [CPP-EXPECTED] [CPP-MDSPAN] [CPP-FUNCTION] [CPP-CONSTEXPR] [CPP-MOVEFUNCTION] [CPP-CONTRACTS] [CPP-REFLECTION] [CPP-EXEC] [CPP-INPLACE] [CPP-HIVE] [CPP-INDIRECT] [CPP-POLYMORPHIC] [CPP-FUNCTIONREF] [CPP-SIMD]

For bounded embedded use, inspect the actual element types, allocator fallback, exception policy, callback representation, and object/frame size. `std::pmr` selects allocation policy through a runtime resource interface; it does not mean allocation-free. `std::inplace_vector<T,N>` embeds capacity, but `T` may allocate. `expected<T,E>` avoids a success/error out-parameter protocol, not every possible exception in its payload operations. [CPP-PMR] [CPP-INPLACE] [CPP-EXPECTED]

<a id="language-rust"></a>
## Rust

The characteristic combination is affine ownership, borrow/lifetime checking, ADTs, constrained generics, and explicit unsafe boundaries. Relevant entities include `&T`, `&mut T`, `Box`, `Vec`, `Option`, `Result`, trait bounds, `dyn Trait`, `impl Trait`, `Pin`, `MaybeUninit`, `Send`, and `Sync`. General resource lifetime and aliasing checks are compile-time rules; runtime policies remain available through `Rc`, `Arc`, `RefCell`, locks, and allocation. [RUST-OWNERSHIP] [RUST-BORROW] [RUST-UNSAFE] [RUST-CELL]

A Rust design review should distinguish ownership of a handle from authority over its referent, memory safety from deadlock freedom, and pinning from physical memory constraints. `Pin` is not DMA page locking. Lifetimes do not extend the owner's storage. A safe abstraction may rely on a small unsafe implementation whose invariants must survive panic, cancellation, forgetting, and partial initialization. Public Rust type layout is not automatically a stable FFI contract. [RUST-PIN] [RUST-FORGET] [RUST-MAYBEUNINIT] [RUST-LAYOUT]

<a id="language-go"></a>
## Go

Go emphasizes garbage-collected storage, structural interface conformance, ordinary value records, slices, explicit product-return errors, and lightweight concurrency. `[]T` is a descriptor over shared backing storage, not a borrow-checked unique region. `(T, error)` permits both components to carry information; `io.Reader` makes this useful for partial progress. `defer` is function-scoped. [GO-SLICES] [GO-IO] [GO-DEFER]

Generic type-set constraints should not be confused with first-class runtime sums. The documented compiler design combines representation-shape sharing with dictionaries; the source language does not require one fully specialized machine-code body per type. An interface containing a typed nil pointer can itself be non-nil. The GC manages memory reachability, not timely release of device handles or files. Go also does not statically enforce Rust-like ownership transfer between goroutines. [GO-SPEC] [GO-DICTIONARIES] [GO-FAQ] [GO-GC]

<a id="language-swift"></a>
## Swift

Swift combines value-semantic structs/enums, reference types managed by ARC, protocol abstraction, and increasingly explicit ownership and isolation controls. `some P` preserves an opaque concrete type; `any P` packages an existential. `~Copyable`, borrowing/consuming operations, `throws(E)`, parameter packs, actors, and `Sendable` expose different static obligations. [SWIFT-OPAQUE] [SWIFT-NONCOPY] [SWIFT-THROWS] [SWIFT-PACKS] [SWIFT-ACTORS] [SWIFT-SENDABLE]

Swift 6.2 documents `InlineArray` and lifetime-safe `Span`. Value semantics does not imply eager copying: `Array` can share backing storage until mutation. Existential storage can use inline buffers or out-of-line representation depending on the value and ABI rules. Actor isolation does not make an entire method atomic across `await`; invariants must be re-established after suspension. ARC traffic, copy-on-write separation, and task scheduling must be budgeted separately from static checking. [SWIFT-RELEASE] [SWIFT-ARRAY] [SWIFT-LAYOUT] [SWIFT-CONCURRENCY]

<a id="language-zig"></a>
## Zig

Zig's relevant tools include explicit allocator passing, slices, optionals, error unions, `try`/`catch`, `defer`/`errdefer`, `comptime`, reflection through builtins, and explicit external/packed representations. These support deliberate low-level interfaces and compile-time specialization without requiring inheritance-heavy modeling. [ZIG] [ZIG-ALLOCATOR] [ZIG-ARENA]

A slice does not carry Rust's general lifetime proof, and `defer` does not prevent a freed allocation from remaining reachable through an escaped pointer. Runtime safety settings affect which checks execute. The language-reference and allocator links use rolling sources: method names and APIs should be pinned to the actual compiler release before adopting an example. Explicit allocation is a visibility mechanism, not evidence that an arena or allocator has a bounded latency or no upstream heap dependency. [ZIG]

<a id="language-jai"></a>
## Jai: a metaprogramming reference with narrower evidence

Jai belongs in the comparison principally as a reference point for compile-time execution and programmer-controlled code generation. An original language demonstration explicitly concerns base-language and compile-time execution. It is historical evidence, not a current language specification or ABI contract. [JAI-DEMO]

A sufficiently current, public primary specification was not established for this document. Consequently the census does not present contemporary Jai allocator details, exact reflection APIs, ownership guarantees, generics ABI, or compiler distribution status as verified facts. The corresponding techniques are fully analyzed under staged programming, reflection, allocator composition, and data-oriented layout using independently documented implementations. A Jai implementation assessment should first obtain the exact compiler build and its bundled documentation, then inspect generated code, safety modes, metaprogram permissions, and reproducibility. This limits language-specific certainty; it does not limit the general technique census.

<a id="language-v"></a>
## V

V's documentation distinguishes optional `?T` from fallible `!T`, and describes structural interfaces, sum types, and generics. Its C-backend documentation describes inline option/result payload storage, while error objects and the payloads themselves can introduce allocation. This is more precise than assuming that concise failure syntax is allocation-free throughout an operation. [V-OPTIONS] [V-INTERFACES]

The memory-management documentation identifies default garbage collection and marks `-autofree` as work in progress. These should not be relabeled as Rust-style compile-time lifetime enforcement or unconditional automatic resource safety. Assess the chosen compiler/backend/memory mode explicitly, including foreign calls and generated C when applicable. Generic, interface, and sum-type syntax alone does not establish dispatch cost, stable ABI, or mature toolchain qualification. [V-MEMORY]

<a id="language-nix"></a>
## Nix

Nix is useful here for immutable declarative values, parameterized dependency graphs, and effect separation at the build/configuration level. Its language is lazy and dynamically typed; a function accepting an attribute set does not imply a statically checked row-polymorphic record. A derivation describes a build rather than directly representing a native object. [NIX-LANGUAGE] [NIX-DERIVATIONS]

It is therefore inappropriate to compare Nix attribute sets against C++ records using only `sizeof` or native dispatch overhead. Relevant costs include evaluator memory, deferred work, retained values, dependency closure, and rebuild scope. Explicit dependencies improve auditability, but evaluation purity, sandbox policy, fixed inputs, and deterministic builders remain separate questions. This is complementary tooling for systems software, not a substitute for a low-level ownership type system.

<a id="language-odin"></a>
## Odin

Odin's documented facilities include slices and dynamic arrays, explicit context allocators, tagged unions, distinct types, `bit_set`, structure-of-arrays declarations, and composition through `using`. These are concrete ways to make data shape and storage policy more visible than a hierarchy of heap-allocated objects. [ODIN]

The implicit context is convenient policy plumbing, not equivalent to strict capability confinement or statically proven lifetime ownership. A slice still depends on valid backing storage. `using` is composition/member promotion rather than a mandate for virtual dispatch. SoA helps selected bulk access patterns but changes element-reference semantics and may be worse for record-oriented access. Allocator context, container mutation, and temporary-storage lifetime deserve explicit review.

<a id="language-c3"></a>
## C3

C3 documents fallible optionals, generics, slices, methods, contracts, and dynamic interfaces. Its `T?` failure model should not be translated mechanically into a nullable pointer, and its interface mechanism should not be assigned another language's vtable layout merely because both offer method dispatch. [C3-OPTIONALS] [C3-GENERICS] [C3-INTERFACES] [C3-SPEC]

The useful architectural direction is a C-adjacent interface style with stronger vocabulary and compile-time facilities. It is not a general claim of borrow-checked memory or erased dynamic dispatch. Runtime contracts, error propagation, allocation, and underlying pointer validity remain distinct concerns. Check the specific release, safety configuration, calling convention, and fault conventions at a foreign boundary. [C3-CONTRACTS] [C3-METHODS]

<a id="language-roc"></a>
## Roc

This document uses the official **new-compiler tutorial**, not older syntax interchangeably. It documents `Try(T,E)` with `Ok`/`Err`, postfix `?` propagation, `match`, structural forms, nominal declarations, and pure `->` versus effectful `=>` function types. [ROC-TUTORIAL]

Those function arrows do not by themselves establish a Koka-style algebraic effect-handler system. Likewise, semantic immutability is not a promise that all values have a C-like inline layout or that execution performs no allocation. The tutorial describes a nonblocking compilation workflow: code may run despite diagnostics with a nonzero exit status. A production gate should reject such diagnostics before execution rather than treating a test run as proof that the program passed type checking. Pin the compiler and use its matching tutorial and examples. [ROC-TUTORIAL] [KOKA]

## Additional reference languages and libraries

Haskell/GHC provides particularly direct examples of GADTs, higher-kinded abstraction, and optics; Idris provides dependent indices, erasure, and quantitative usage; Koka provides effect rows and handlers; OCaml provides parameterized modules; Julia demonstrates multiple dispatch; Cats provides accumulated validation. They are included where they distinguish a mechanism unavailable or materially different in the main systems languages. Their presence is explanatory, not a blanket recommendation for a target's runtime or qualification needs. [GHC-GADT] [HASKELL-FUNCTOR] [HASKELL-LENS] [IDRIS-LINEAR] [KOKA] [OCAML-MODULES] [JULIA-METHODS] [CATS-VALIDATED]

<!-- REFERENCE DEFINITIONS -->
[C3-CONTRACTS]: https://c3-lang.org/language-common/contracts/ "C3 documentation: contracts"
[C3-GENERICS]: https://c3-lang.org/generic-programming/generics/ "C3 documentation: generics"
[C3-INTERFACES]: https://c3-lang.org/generic-programming/anyinterfaces/ "C3 documentation: interfaces and any"
[C3-METHODS]: https://c3-lang.org/language-fundamentals/functions/ "C3 documentation: functions and methods"
[C3-OPTIONALS]: https://c3-lang.org/language-common/optionals-essential/ "C3 documentation: fallible optionals"
[C3-SPEC]: https://c3-lang.org/implementation-details/specification/ "C3 language specification"
[CATS-VALIDATED]: https://typelevel.org/cats/datatypes/validated.html "Cats: Validated and accumulated validation"
[CPP-CONSTEXPR]: https://eel.is/c++draft/expr.const "C++ working draft: constant expressions"
[CPP-CONSTRAINTS]: https://eel.is/c++draft/temp.constr "C++ working draft: template constraints"
[CPP-CONTRACTS]: https://eel.is/c++draft/basic.contract "C++ working draft: contracts"
[CPP-EXEC]: https://eel.is/c++draft/exec "C++ working draft: execution control"
[CPP-EXPECTED]: https://eel.is/c++draft/expected "C++ working draft: expected"
[CPP-FUNCTION]: https://eel.is/c++draft/dcl.fct "C++ working draft: function declarations and explicit object parameters"
[CPP-FUNCTIONREF]: https://eel.is/c++draft/func.wrap.ref "C++ working draft: function_ref"
[CPP-GUIDELINES]: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines "C++ Core Guidelines"
[CPP-HIVE]: https://eel.is/c++draft/hive "C++ working draft: hive"
[CPP-INDIRECT]: https://eel.is/c++draft/indirect "C++ working draft: indirect"
[CPP-INPLACE]: https://eel.is/c++draft/inplace.vector "C++ working draft: inplace_vector"
[CPP-MDSPAN]: https://eel.is/c++draft/views.multidim "C++ working draft: multidimensional views"
[CPP-MOVEFUNCTION]: https://eel.is/c++draft/func.wrap.move "C++ working draft: move_only_function"
[CPP-PMR]: https://eel.is/c++draft/mem.res "C++ working draft: memory resources"
[CPP-POLYMORPHIC]: https://eel.is/c++draft/polymorphic "C++ working draft: polymorphic"
[CPP-REFLECTION]: https://eel.is/c++draft/meta "C++ working draft: reflection"
[CPP-SIMD]: https://eel.is/c++draft/simd "C++ working draft: data-parallel types"
[CPP-SPAN]: https://eel.is/c++draft/views.span "C++ working draft: span"
[GHC-GADT]: https://ghc.gitlab.haskell.org/ghc/doc/users_guide/exts/gadt.html "GHC User's Guide: generalized algebraic data types"
[GO-DEFER]: https://go.dev/blog/defer-panic-and-recover "Go: defer, panic, and recover"
[GO-DICTIONARIES]: https://raw.githubusercontent.com/golang/proposal/master/design/generics-implementation-dictionaries-go1.18.md "Go compiler design: generic dictionaries and gcshape sharing"
[GO-FAQ]: https://go.dev/doc/faq "Go frequently asked questions"
[GO-GC]: https://go.dev/doc/gc-guide "Go garbage collector guide"
[GO-IO]: https://pkg.go.dev/io "Go standard library: io"
[GO-SLICES]: https://go.dev/blog/slices-intro "Go slices: usage and internals"
[GO-SPEC]: https://go.dev/ref/spec "Go language specification"
[HASKELL-FUNCTOR]: https://www.haskell.org/onlinereport/haskell2010/haskellch6.html "Haskell 2010 report: predefined types and classes"
[HASKELL-LENS]: https://hackage.haskell.org/package/lens "lens package: lenses, folds, and traversals"
[IDRIS-LINEAR]: https://idris2.readthedocs.io/en/latest/tutorial/multiplicities.html "Idris 2 tutorial: multiplicities and erasure"
[JAI-DEMO]: https://youtu.be/UTqZNujQOlA "Original Jai demonstration: Base language, compile-time execution; historical evidence only"
[JULIA-METHODS]: https://docs.julialang.org/en/v1/manual/methods/ "Julia manual: methods and multiple dispatch"
[KOKA]: https://koka-lang.github.io/koka/doc/book.html "Koka language book"
[NIX-DERIVATIONS]: https://nix.dev/manual/nix/2.34/language/derivations "Nix 2.34 reference manual: derivations"
[NIX-LANGUAGE]: https://nix.dev/manual/nix/2.34/language/ "Nix 2.34 reference manual: language"
[OCAML-MODULES]: https://ocaml.org/manual/5.3/moduleexamples.html "OCaml 5.3 manual: modules and functors"
[ODIN]: https://odin-lang.org/docs/overview/ "Odin language overview"
[ROC-TUTORIAL]: https://raw.githubusercontent.com/roc-lang/roc/main/docs/mini-tutorial-new-compiler.md "Roc official new-compiler tutorial"
[RUST-BORROW]: https://doc.rust-lang.org/book/ch04-02-references-and-borrowing.html "The Rust Book: references and borrowing"
[RUST-CELL]: https://doc.rust-lang.org/std/cell/index.html "Rust standard library: interior mutability"
[RUST-FORGET]: https://doc.rust-lang.org/std/mem/fn.forget.html "Rust standard library: mem::forget"
[RUST-LAYOUT]: https://doc.rust-lang.org/reference/type-layout.html "Rust Reference: type layout"
[RUST-MAYBEUNINIT]: https://doc.rust-lang.org/std/mem/union.MaybeUninit.html "Rust standard library: MaybeUninit"
[RUST-OWNERSHIP]: https://doc.rust-lang.org/book/ch04-01-what-is-ownership.html "The Rust Book: ownership"
[RUST-PIN]: https://doc.rust-lang.org/std/pin/index.html "Rust standard library: pinning"
[RUST-UNSAFE]: https://doc.rust-lang.org/reference/unsafety.html "Rust Reference: unsafety"
[SWIFT-ACTORS]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0306-actors.md "Swift Evolution SE-0306: actors"
[SWIFT-ARRAY]: https://raw.githubusercontent.com/swiftlang/swift/main/stdlib/public/core/Array.swift "Swift standard library: Array implementation and documentation"
[SWIFT-CONCURRENCY]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Concurrency.md "The Swift Programming Language: concurrency"
[SWIFT-LAYOUT]: https://raw.githubusercontent.com/swiftlang/swift/main/docs/ABI/TypeLayout.rst "Swift ABI: type layout"
[SWIFT-NONCOPY]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0390-noncopyable-structs-and-enums.md "Swift Evolution SE-0390: noncopyable structs and enums"
[SWIFT-OPAQUE]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/OpaqueTypes.md "The Swift Programming Language: opaque and boxed protocol types"
[SWIFT-PACKS]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0393-parameter-packs.md "Swift Evolution SE-0393: parameter packs"
[SWIFT-RELEASE]: https://www.swift.org/blog/swift-6.2-released/ "Swift 6.2 release: systems programming facilities"
[SWIFT-SENDABLE]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0302-concurrent-value-and-concurrent-closures.md "Swift Evolution SE-0302: Sendable"
[SWIFT-THROWS]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0413-typed-throws.md "Swift Evolution SE-0413: typed throws"
[V-INTERFACES]: https://docs.vlang.io/type-declarations.html#interfaces "V documentation: interfaces"
[V-MEMORY]: https://docs.vlang.io/memory-management.html "V documentation: memory management"
[V-OPTIONS]: https://docs.vlang.io/type-declarations.html "V documentation: options, results, sum types, and generics"
[ZIG]: https://ziglang.org/documentation/master/ "Zig language reference, master snapshot"
[ZIG-ALLOCATOR]: https://raw.githubusercontent.com/ziglang/zig/master/lib/std/mem/Allocator.zig "Zig standard library: Allocator source"
[ZIG-ARENA]: https://raw.githubusercontent.com/ziglang/zig/master/lib/std/heap/arena_allocator.zig "Zig standard library: arena allocator source"
