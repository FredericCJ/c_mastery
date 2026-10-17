# III. Polymorphism, Genericity & Type Systems

Genericity, conformance, dispatch, ownership, and separate compilation are different axes. A trait can support static or dynamic dispatch. An interface can be structural without being allocation-free. A generic implementation can specialize, share code with dictionaries, or combine both strategies.

<a id="p01"></a>
## P01. Constraint-based generic programming

**1. Modern approach & concrete entities.** C++20 concepts and `requires`; Rust trait bounds and `where`; Swift protocol constraints; Go generic constraint interfaces with type sets; C3 constrained generic modules. [CPP-CONSTRAINTS] [RUST-BOUNDS] [GO-SPEC] [C3-GENERICS]

**2. Classical antecedent / historical anti-pattern.** Unconstrained templates failing deep in implementation details, `void*` containers with unchecked casts, and inheritance introduced solely to make an algorithm accept several unrelated types.

**3. Underlying mechanism & memory layout.** The compiler checks that a substitution satisfies required expressions, types, constants, or relationships. The constraint itself needs no object field or vptr. Actual machine code depends on instantiation and dispatch strategy. Constraints can participate in overload selection, allowing a more specialized algorithm when stronger properties are available.

**4. Engineering trade-offs & footprint.** Syntactic satisfaction is not proof of semantic laws: a callable comparator can violate strict weak ordering. Constraint normalization, overload resolution, and recursive obligations add compile-time work. Excessively detailed constraints can expose implementation choices and make errors worse rather than better. Go type-set unions constrain admissible types; they are not runtime tagged unions. Design constraints around stable algorithm requirements, and document non-checkable laws separately. Measure code generation rather than inferring zero overhead from the mere presence of a concept or trait bound.

<a id="p02"></a>
## P02. Monomorphization and static specialization

**1. Modern approach & concrete entities.** C++ template instantiation; Rust monomorphization; Zig generic functions driven by compile-time type/value arguments. [CPP-INSTANTIATION] [RUST-MONOMORPH] [ZIG]

**2. Classical antecedent / historical anti-pattern.** A universal `void*` implementation with runtime size/operation callbacks, or a virtual base interface used for behavior whose concrete implementation is known at every call site.

**3. Underlying mechanism & memory layout.** The compiler generates or selects concrete instances for relevant substitutions. Known sizes, alignments, operations, and constants enable inlining, constant propagation, dead-branch elimination, and specialized layout. A generic type's fields are laid out using its actual type arguments. Linkers and optimizers may merge or discard duplicate code, but this is not guaranteed for every distinct instantiation.

**4. Engineering trade-offs & footprint.** Many type/policy combinations can inflate compilation time, debug information, object files, and instruction-cache footprint. More specialization can reduce data-path work while increasing front-end fetch pressure. Cross-module optimization may require visible definitions or link-time optimization. Large generic functions can benefit from a small specialized wrapper around a shared non-generic core. Static dispatch is attractive in tight loops, but a dynamically dispatched boundary outside the loop can be smaller and equally adequate for the workload.

<a id="p03"></a>
## P03. Dictionary passing, witness tables, and representation sharing

**1. Modern approach & concrete entities.** Go's documented shape-sharing/dictionary compiler design; Swift unspecialized generic calls carrying metadata and protocol witnesses; type-class dictionaries in functional-language implementations. [GO-DICTIONARIES] [SWIFT-CALLING]

**2. Classical antecedent / historical anti-pattern.** The false dichotomy that generic code must either be a fully duplicated template instance or a class hierarchy with one virtual method table per object.

**3. Underlying mechanism & memory layout.** A shared implementation receives evidence describing type-dependent operations and representation. A dictionary or witness table supplies methods or metadata that cannot be fixed in the shared body. Shape-based compilation shares code among suitable representation classes while specializing other aspects. Optimization can propagate known evidence and remove some indirect calls.

**4. Engineering trade-offs & footprint.** Shared code reduces duplication but can retain indirect dispatch, metadata loads, and abstraction barriers. Dictionaries themselves occupy code/data space, though not necessarily in every user object. Implementation designs evolve; the Go design document is evidence of a compiler strategy, not a permanent language ABI guarantee. Swift resilient generic code and whole-module-specialized code can have different costs despite the same source signature. Inspect the actual calling convention and generated implementation for the target build before classifying an API as static or dynamic in performance terms.

<a id="p04"></a>
## P04. Traits, protocols, and coherent external conformance

**1. Modern approach & concrete entities.** Rust `trait`/`impl`, blanket implementations, supertraits, and coherence/orphan rules; Swift protocol conformance and protocol extensions; Haskell type classes as a related model. [RUST-TRAITS] [RUST-IMPLS] [SWIFT-PROTOCOLS]

**2. Classical antecedent / historical anti-pattern.** Requiring a type's storage declaration to inherit from every behavioral interface, adapter subclasses merely to add a capability, and globally conflicting implementations of the same abstraction.

**3. Underlying mechanism & memory layout.** Behavioral conformance can be declared separately from the record's stored fields. Static calls resolve an implementation; erased calls use appropriate runtime evidence. Coherence rules restrict overlapping implementations so an operation has a sufficiently unambiguous interpretation. Supertraits describe additional obligations, not inherited object storage.

**4. Engineering trade-offs & footprint.** External-conformance restrictions can force a newtype adapter around a foreign type. Blanket implementations can block later specializations or create compatibility constraints. Adding default methods or conformances can alter lookup/dispatch behavior. Traits and protocols do not automatically enforce behavioral substitutability, thread safety, or mathematical laws. The useful replacement for inheritance here is separating data representation from declared capabilities, while retaining deliberate rules about who may define an implementation and how it is selected.

<a id="p05"></a>
## P05. Structural conformance versus nominal type identity

**1. Modern approach & concrete entities.** Go method-set-based interface satisfaction; V interfaces; Roc structural records/tag unions and nominal declarations; nominal Rust traits/types and Swift declared protocol conformance. [GO-INTERFACES] [V-INTERFACES] [ROC-TUTORIAL] [RUST-TRAITS]

**2. Classical antecedent / historical anti-pattern.** A mandatory inheritance declaration for simple compatibility, or runtime duck typing that discovers a missing operation only after the program reaches that path.

**3. Underlying mechanism & memory layout.** Structural conformance compares available operations or shape; nominal identity depends on declared type identity or conformance. Structural record extension and row-like constraints can abstract over selected fields while allowing others. Neither mechanism by itself requires objects to have the same physical layout or dispatch representation.

**4. Engineering trade-offs & footprint.** Structural matching decouples producers from interfaces but can create accidental compatibility between semantically different operations. Nominal distinctions require adapters and declarations but prevent some category mistakes. Method sets, pointer/value receivers, and visibility rules matter in Go. Nix attribute-set parameter destructuring is dynamically checked and is not a static structural type system. Choose nominal wrappers when identity carries domain meaning, and structural capability matching when independent types legitimately provide the same operation contract.

<a id="p06"></a>
## P06. Policy composition and statically injected strategies

**1. Modern approach & concrete entities.** C++ policy classes, constrained function objects, and template strategy parameters; Rust generic strategy types; Odin composition/member exposure with `using`. [CPP-CONSTRAINTS] [CPP-NOUNIQUE] [ODIN]

**2. Classical antecedent / historical anti-pattern.** Deep inheritance combinations for logging, allocation, locking, encoding, or error policy; global hooks replaced at runtime despite every build using one fixed strategy.

**3. Underlying mechanism & memory layout.** A composed object contains stateful policies as fields and represents stateless policies through type parameters or empty members. Calls resolve directly where the concrete strategy is known. Different concerns can be selected independently instead of creating a superclass for each combination.

**4. Engineering trade-offs & footprint.** Every combination can produce a new type and specialization. Public policy parameters increase API surface and error-message complexity. Stateful policies enlarge the object; stateless ones can still create code bloat. Runtime replacement becomes difficult without adding erasure or a sum. Some policies interact semantically—for example, a non-thread-safe allocator and a cross-thread ownership policy—so mere composability is not compatibility. Use static strategies when selection is genuinely static; preserve dynamic dispatch at authentic plugin or runtime configuration boundaries.

<a id="p07"></a>
## P07. Explicit receiver parameters and cv/ref-generic methods

**1. Modern approach & concrete entities.** C++23 explicit object parameters, including `this auto&& self`; Rust's explicit ownership-oriented receivers such as `self`, `&self`, and `&mut self`; receiver-aware generic helper functions. [CPP-FUNCTION] [RUST-ASSOCIATED]

**2. Classical antecedent / historical anti-pattern.** Duplicating four const/lvalue/rvalue-qualified accessors, using CRTP solely to recover the derived receiver type, and returning references from temporary owners accidentally.

**3. Underlying mechanism & memory layout.** The receiver becomes an explicit parameter whose type/category participates in deduction. One definition can select forwarding behavior for different receiver qualifications. The syntax need not add a vptr or change object layout. In Rust, receiver form also communicates borrowing or consumption requirements that participate in ownership checking.

**4. Engineering trade-offs & footprint.** Perfect forwarding can propagate a dangerous reference from a temporary if the API does not restrict its lifetime. Generic receiver methods may generate several instances and complicate overload resolution. Explicit object parameters replace some CRTP uses, not every mixin, customization, or recursive static-polymorphism pattern. C++ receiver syntax does not add Rust-style ownership enforcement. Decide whether an accessor should return a borrow, a copied value, or a consumed member for each receiver category rather than assuming universal forwarding is always correct.

<a id="p08"></a>
## P08. Existential packaging and explicit dynamic dispatch

**1. Modern approach & concrete entities.** Rust `&dyn Trait` and `Box<dyn Trait>`; Swift `any P`; Go interface values; C3 `any` and dynamic interfaces. [RUST-DYN] [SWIFT-LAYOUT] [GO-REFLECTION] [C3-INTERFACES]

**2. Classical antecedent / historical anti-pattern.** Exposing a class hierarchy's storage/inheritance decisions merely to call a few operations, or passing `void*` and reconstructing its type through unchecked casts.

**3. Underlying mechanism & memory layout.** An existential hides a concrete type while packaging the operations/evidence needed to use it. Rust trait-object pointers pair access to data with vtable metadata. Swift general opaque existentials use value storage plus metadata and protocol witnesses, with out-of-line storage when needed. Go interfaces carry dynamic type and value information. C3's documented lookup mechanism must not simply be assigned Rust's vtable layout.

**4. Engineering trade-offs & footprint.** Calls can involve indirect branches and inhibit inlining. Inline existential buffers enlarge containers; boxed values allocate and add pointer chasing. Borrowed trait objects can avoid allocation. Dynamic dispatch supports heterogeneous collections, runtime extensions, and smaller shared code. Object-safety/dyn-compatibility restrictions determine which traits can be erased. The relevant question is where to place an erased boundary, not whether a modern program can eliminate every vtable without losing useful extensibility.

<a id="p09"></a>
## P09. Opaque concrete return types

**1. Modern approach & concrete entities.** Rust return-position `impl Trait`; Swift `some P`; C++ deduced `auto` return types as a related but different facility. [RUST-IMPLTRAIT] [SWIFT-OPAQUE]

**2. Classical antecedent / historical anti-pattern.** Publishing enormous iterator/closure types in interfaces, boxing a value solely to hide its spelling, or introducing an abstract base for one statically fixed implementation.

**3. Underlying mechanism & memory layout.** The producer chooses a concrete type while clients use a constrained abstract view of its interface. The concrete identity remains fixed for the applicable declaration/instantiation and can support specialization and inline storage. This differs from an existential value that may contain different concrete types at runtime.

**4. Engineering trade-offs & footprint.** Multiple return branches generally need one concrete representation, an explicit sum, or erasure. Hidden spelling does not imply stable binary layout or an ABI boundary. Generic and lifetime captures can constrain callers even when the underlying type is unnamed. C++ return-type deduction often requires definition visibility and does not reproduce every Swift opaque-type guarantee. Opaque types are effective for implementation-heavy pipelines whose identity is fixed; they are not substitutes for runtime plugin selection.

<a id="p10"></a>
## P10. Owning and borrowing type-erased callables

**1. Modern approach & concrete entities.** C++ `std::function`, C++23 `std::move_only_function`, and C++26 `std::function_ref`; Rust borrowed or boxed `dyn Fn`-family objects; application-specific erased operation tables. [CPP-FUNCTION-ERASURE] [CPP-MOVEFUNCTION] [CPP-FUNCTIONREF]

**2. Classical antecedent / historical anti-pattern.** A raw callback plus `void*` whose retention policy is undocumented, handwritten trampoline subclasses, and forcing every callback environment to be copyable.

**3. Underlying mechanism & memory layout.** Erasure combines callable state with an invocation thunk and, for owning wrappers, appropriate movement/copy/destruction operations. Small captures may fit inline; larger ones can allocate. A borrowing callable reference stores access to someone else's callable and an invocation mechanism rather than owning its environment.

**4. Engineering trade-offs & footprint.** Dynamic invocation can prevent inlining. Owning wrappers may allocate and enlarge objects; borrowing wrappers can dangle. Move-only erasure accommodates uniquely owned captures without making them copyable. Signature qualifiers, `noexcept`, empty-state behavior, and reference returns affect correctness. A callback accepted for immediate invocation is a different contract from a callback retained asynchronously. Select the wrapper accordingly instead of using an owning general-purpose function object everywhere or treating a non-owning wrapper as a free drop-in replacement.

<a id="p11"></a>
## P11. Closures with explicit capture and invocation capabilities

**1. Modern approach & concrete entities.** C++ lambdas with value/reference/init captures; Rust closure traits `Fn`, `FnMut`, and `FnOnce`; Swift capture lists and non-escaping/escaping closure distinctions. [RUST-CLOSURES] [CPP-LAMBDA] [SWIFT-CLOSURES]

**2. Classical antecedent / historical anti-pattern.** Global variables supplying callback context, unsafe cast-based environment records, and callback APIs unable to express that invoking an operation consumes its captured resource.

**3. Underlying mechanism & memory layout.** A closure is conceptually generated code plus a capture environment. A statically known closure has a concrete compiler-generated type, and direct calls can inline. Rust's call traits distinguish invocation through shared access, mutable access, or consumption; the actual body determines which capabilities it supports. Capture-by-value is not necessarily a deep copy of referenced objects.

**4. Engineering trade-offs & footprint.** Capturing a large object increases closure/frame size. Capturing a shared owner incurs its lifetime/accounting cost, while capturing a reference introduces an outlives obligation. Converting a closure to an erased wrapper adds a separate layer of cost. A Rust `move` closure is not automatically only `FnOnce`; what the body does with captured values matters. Avoid accidental retention through broad capture defaults, especially when callbacks are registered with long-lived services or asynchronous runtimes.

<a id="p12"></a>
## P12. Associated types and generic associated type families

**1. Modern approach & concrete entities.** Rust `Iterator::Item` and generic associated types such as `type Item<'a>`; Swift `associatedtype`; C++ dependent nested types and constrained type relationships. [RUST-ASSOCIATED] [SWIFT-ASSOCIATED]

**2. Classical antecedent / historical anti-pattern.** Returning a universal base type and downcasting each element, or passing several unrelated generic parameters that can be combined inconsistently.

**3. Underlying mechanism & memory layout.** An implementation chooses types logically associated with its capability. A type family can additionally depend on a lifetime or other parameter. A lending iterator may return an item borrowing from its current state, expressed as a family indexed by the borrow. Static specialization resolves these projections without storing a runtime type object in every value.

**4. Engineering trade-offs & footprint.** Associated-type equalities and lifetime families increase constraint complexity. Lending APIs can restrict retaining one item while requesting the next. Existential use may require fixing associated types or may be unavailable for some designs. Generic associated types do not amount to unrestricted higher-kinded polymorphism. Use an associated type when the implementation chooses one coherent result family; use a separate generic parameter when callers legitimately choose among independent types. That distinction improves both API meaning and inference.

<a id="p13"></a>
## P13. Higher-ranked bounds, variance, and safe substitution

**1. Modern approach & concrete entities.** Rust `for<'a>` higher-ranked trait bounds, lifetime subtyping, and covariance/contravariance/invariance; analogous rank-polymorphic interfaces in functional languages. [RUST-BOUNDS] [RUST-VARIANCE]

**2. Classical antecedent / historical anti-pattern.** Assuming that a callback valid for one borrowed lifetime is valid for every lifetime, or allowing writable containers to substitute covariantly and thereby accept invalid writes.

**3. Underlying mechanism & memory layout.** A higher-ranked bound quantifies over lifetimes at the use site, for example requiring a callable to accept a borrow with any suitable lifetime. Variance describes how a composite type's substitutability depends on its parameters. Mutable reference relationships are constrained to prevent writing a shorter-lived value into storage expected to remain valid longer. These rules generally affect compilation, not object fields.

**4. Engineering trade-offs & footprint.** The notation is demanding and diagnostics can be abstract. Incorrect variance in unsafe wrapper design can make a safe-looking API unsound. An HRTB does not mean a captured environment lives forever; it expresses the callable's accepted relationships. Overly invariant types can unnecessarily restrict ergonomics. These mechanisms are particularly relevant when designing callback registries, scoped resource APIs, and lifetime-branded handles, rather than something every ordinary business function should expose directly.

<a id="p14"></a>
## P14. Values in types: const generics and static shapes

**1. Modern approach & concrete entities.** Rust const generics and `[T;N]`; C++ non-type template parameters, static `span` extents, and `std::array`; Swift `InlineArray`; Zig `comptime` parameters. [RUST-GENERICS] [CPP-TEMPLATE-PARAMETERS] [SWIFT-INLINE]

**2. Classical antecedent / historical anti-pattern.** Passing dimensions separately, discovering incompatible shapes after deployment, and using preprocessor-generated families of fixed-size functions.

**3. Underlying mechanism & memory layout.** Compile-time values participate in type identity and constraint selection. Statically known sizes can eliminate stored dimensions and enable unrolling, fixed-offset addressing, or specialized algorithms. Different dimensions may denote different types even when their runtime payloads have similar roles.

**4. Engineering trade-offs & footprint.** Each parameterization can create more code and debug/type information. A runtime-selected dimension cannot be made compile-time merely by wrapping it in a generic API; a bridge to dynamic shapes or finite dispatch is required. Const generics do not imply arbitrary dependent proofs about every dimension expression. Large static storage can inflate coroutine frames or stack usage. Use type-level values where incompatibility should prevent construction and where specialization pays for itself, not for every incidental configuration integer.

<a id="p15"></a>
## P15. Typed parameter packs and heterogeneous variadics

**1. Modern approach & concrete entities.** C++ parameter packs, pack expansion, and fold expressions; Swift parameter packs with `each` and `repeat`; fixed-arity generic families generated through typed metaprogramming. [CPP-PACKS] [SWIFT-PACKS]

**2. Classical antecedent / historical anti-pattern.** C varargs losing type information, sentinel-terminated argument lists, duplicated overloads for many arities, and unchecked runtime interpretation of heterogeneous argument buffers.

**3. Underlying mechanism & memory layout.** The compiler retains the type of each argument and expands operations over the pack. A pack is a compile-time sequence, not automatically a heap container or runtime array of variants. A tuple can store its elements inline with their individual alignment requirements; a fold combines expressions according to its defined operator structure.

**4. Engineering trade-offs & footprint.** Large packs can produce long instantiation chains, diagnostic explosions, and large generated functions. Evaluation order and forwarding/lifetime rules require attention. Static arity is not a replacement for genuinely runtime-sized heterogeneous input. Rust formatting macros preserve type checking through expansion, but should not be described as a general variadic-generics facility. Constrain pack relationships explicitly, including equal lengths or compatible element operations, rather than relying on obscure errors during expansion.

<a id="p16"></a>
## P16. Compile-time execution and staged computation

**1. Modern approach & concrete entities.** C++ `constexpr`, C++20 `consteval`, and C++23 `if consteval`; Rust `const fn`; Zig `comptime`. Jai is considered in the evidence-limited language profile rather than assigned an unverified current compiler API here. [CPP-CONSTEXPR] [RUST-CONST] [ZIG]

**2. Classical antecedent / historical anti-pattern.** Runtime generation of fixed lookup tables, preprocessor arithmetic, and template-recursion machinery used only to compute ordinary constants.

**3. Underlying mechanism & memory layout.** The compiler evaluates permitted computation before runtime and materializes constants or specialized code. Type/value staging can select algorithms and reject impossible configurations. C++ `constexpr` permits constant evaluation in eligible contexts, while an immediate `consteval` invocation must satisfy its stricter rules. A computed table still occupies storage if retained in the binary.

**4. Engineering trade-offs & footprint.** Runtime work becomes compiler work, potentially increasing memory consumption and build latency. Precomputation can trade CPU cycles for ROM or instruction-cache pressure. Host/target distinctions matter for code generators that access the build environment. Constant evaluation has language-specific restrictions and compiler resource limits. Staging is most valuable when inputs are truly fixed and the produced artifacts remain inspectable; it does not automatically establish reproducible builds or better runtime locality.

<a id="p17"></a>
## P17. Static reflection and structure-derived implementation

**1. Modern approach & concrete entities.** C++26 `std::meta::info`, reflection expressions `^^T`, and splicing; Zig `@typeInfo`; compiler-supported structural metadata used to generate serializers, visitors, registries, and bindings. [CPP-REFLECTION] [ZIG]

**2. Classical antecedent / historical anti-pattern.** Parallel field lists, handwritten registration tables, X-macros maintained separately from the actual type, and runtime introspection for structure already fixed at compilation.

**3. Underlying mechanism & memory layout.** The metaprogram examines compiler-known entities and generates ordinary declarations or expressions. If reflection is used only during translation, its metadata need not survive in the runtime binary. Generated code can still contain dispatch tables, names, and field descriptors when the requested functionality needs them.

**4. Engineering trade-offs & footprint.** Reflection reduces duplicated descriptions but increases build complexity and can expose private structure unintentionally. Automatic serialization can accidentally bind a wire schema to field order, spelling, or an unstable layout. Compiler reflection and Rust procedural macros differ: token transformation is not a general semantic type-reflection API. C++26 support must be checked feature by feature. Review generated behavior, compatibility policy, and diagnostics; replacing hand-written code with generated code does not reduce the obligations of the generated implementation.

<a id="p18"></a>
## P18. Hygienic syntax transformation and procedural macros

**1. Modern approach & concrete entities.** Rust `macro_rules!`, derive macros, attribute macros, and function-like procedural macros; Swift attached/freestanding macros. [RUST-MACROS] [RUST-PROCMACROS] [SWIFT-MACROS]

**2. Classical antecedent / historical anti-pattern.** Textual C macros that evaluate expressions unexpectedly, capture names accidentally, or generate syntax whose errors are detached from the user's source.

**3. Underlying mechanism & memory layout.** Syntax-aware expansion transforms structured tokens or syntax into ordinary code. Hygiene rules govern name resolution and capture. Rust declarative macros have mixed-site hygiene; procedural macros are not universally hygienic and must manage generated paths/names carefully. Expansion itself need not have runtime representation, while the emitted code has its normal cost.

**4. Engineering trade-offs & footprint.** Macro execution can lengthen builds, worsen diagnostics, and expand the trusted build surface. Procedural macros can execute host-side code with relevant environmental capabilities. Expansion can still duplicate a side-effecting argument if the macro is written that way. Generated source is harder to review when hidden behind a terse annotation. Prefer functions/generics for ordinary value computation, and macros when syntax generation, derivation, or language-like notation provides a concrete benefit that those mechanisms cannot express cleanly.

<a id="p19"></a>
## P19. Derived operations with explicit semantic laws

**1. Modern approach & concrete entities.** C++20 defaulted equality and three-way comparison; Rust `#[derive]` for `Eq`, `PartialEq`, `Ord`, `Hash`, `Clone`, and other applicable traits; Swift synthesized conformances. [CPP-COMPARE] [RUST-DERIVE] [RUST-EQ] [RUST-HASH]

**2. Classical antecedent / historical anti-pattern.** Handwritten member-by-member comparison that forgets a new field, inconsistent equality/hash behavior, and inheritance hierarchies that compare only base state.

**3. Underlying mechanism & memory layout.** The compiler or derive implementation generates operations from the fields and their available operations. Comparison becomes field comparisons with defined short-circuit/order behavior, not necessarily `memcmp`; padding and semantic equivalence make raw-byte equality unsuitable in general. Derivation adds methods/code, not necessarily object metadata.

**4. Engineering trade-offs & footprint.** Structural derivation may not match domain identity: a cache field should perhaps not affect equality. Floating-point values complicate total-order and equivalence assumptions. `Clone` can recursively allocate or increment reference counts; it is not synonymous with a cheap bit copy. Hashing must remain consistent with equality and with the key's mutability policy. Derivation removes repetitive code but cannot decide the correct semantics for a model. Record those laws and test them with property-based techniques where appropriate.

<a id="p20"></a>
## P20. Modules, visibility boundaries, and representation hiding

**1. Modern approach & concrete entities.** C++20 modules and exported declarations; Rust modules with restricted visibility such as `pub(crate)`; Go packages; C3 modules; abstract module signatures in ML-family languages. [CPP-MODULES] [RUST-PRIVACY] [GO-PACKAGES]

**2. Classical antecedent / historical anti-pattern.** Textual inclusion exposing every implementation detail, preprocessor namespace pollution, and public record fields that allow callers to break invariants.

**3. Underlying mechanism & memory layout.** Visibility rules restrict which entities clients may name or construct. Module systems organize compilation and dependency structure without requiring a runtime module object. A private representation plus sanctioned constructors makes type-level invariants enforceable at the component boundary.

**4. Engineering trade-offs & footprint.** C++ module build artifacts depend on compiler/build configuration and are not a universal stable distribution ABI. Build graphs and caching become important. Representation hiding can reduce optimization opportunities when bodies/layouts are unavailable; exposing everything can increase recompilation and coupling. A module is not automatically a sandbox or dynamic library. Distinguish source visibility, semantic dependency, binary linkage, and runtime authority. Use each boundary for the obligation it actually enforces rather than expecting one module declaration to solve all four.

<a id="p21"></a>
## P21. Customization-point objects and constrained extension hooks

**1. Modern approach & concrete entities.** C++ ranges customization-point objects such as `std::ranges::begin`, and carefully defined ADL-based customization; Rust trait methods as a related explicit extension mechanism. [CPP-CPO] [CPP-RANGE-ACCESS]

**2. Classical antecedent / historical anti-pattern.** Mandatory virtual hooks for every generic algorithm, accidental overload capture, and unconstrained free functions that silently select unintended conversions.

**3. Underlying mechanism & memory layout.** A customization-point object is a callable interface with constrained resolution rules. It can select member functions, permitted ADL functions, or fallback implementations and then invoke the selected operation. A stateless object can disappear from generated code. The customization protocol determines which extensions are discoverable and what result requirements apply.

**4. Engineering trade-offs & footprint.** Lookup and constraints can be subtle, and overly generic hooks increase coupling between namespaces. A CPO is not an automatic guarantee of direct dispatch: the chosen implementation may itself erase or allocate. `tag_invoke` is a convention used by some libraries, not a universal standard customization mechanism for every C++ facility. Prefer a narrow documented hook with testable requirements over a broad “customize anything” mechanism whose lookup behavior becomes part of every diagnostic.

<a id="p22"></a>
## P22. Heterogeneous compile-time records and type-indexed lookup

**1. Modern approach & concrete entities.** C++ `std::tuple`, `std::get<I>`, `std::apply`, and index sequences; Rust tuples and type-indexed library patterns; generated records keyed by compile-time tags. [CPP-TUPLE] [RUST-TUPLE]

**2. Classical antecedent / historical anti-pattern.** Arrays of `void*`, string-keyed heterogeneous maps for a fixed schema, and repeated casts retrieving statically known services or fields.

**3. Underlying mechanism & memory layout.** Each position/key has a known type. Storage can place values inline, including empty-type optimization where permitted. Lookup by a compile-time index or distinct type becomes a fixed projection rather than a hash lookup or runtime type test. Expansion can apply a function to all elements without losing their individual types.

**4. Engineering trade-offs & footprint.** These structures encode a closed schema and cannot naturally accept arbitrary runtime keys. Alignment and padding may make a tuple larger than the sum of payload sizes. Repeated element types can make type-based lookup ambiguous. Large packs inflate diagnostics and code generation. They are effective for fixed pipelines or statically assembled components, but inappropriate as a blanket replacement for dynamic registries, configuration maps, or stable serialized schemas.

<a id="p23"></a>
## P23. Multiple dispatch and symmetric operation selection

**1. Modern approach & concrete entities.** Julia methods dispatched on the tuple of argument types; multimethod systems; C++ multi-variant visitation as a closed-world alternative. [JULIA-METHODS] [CPP-VARIANT-VISIT]

**2. Classical antecedent / historical anti-pattern.** Double-dispatch visitor hierarchies, nested type-code switches, and choosing one arbitrary operand as the privileged object that owns a fundamentally symmetric operation.

**3. Underlying mechanism & memory layout.** Method selection considers several argument types. An implementation can cache dispatch results and specialize known combinations. Multi-variant visitation instead enumerates a statically closed combination space and lowers it through generated dispatch logic. Neither technique requires the operands to inherit storage from a shared base.

**4. Engineering trade-offs & footprint.** Open multiple dispatch introduces ambiguity and method-table evolution concerns. Julia specialization and dynamic lookup have a different execution model from ahead-of-time C++ templates. Closed multi-visitation can require a Cartesian product of cases and grow code rapidly. This model suits operations such as shape collisions or mixed numeric promotion, but is excessive when one operand or one strategy already provides a clear extension point. Distinguish source expressiveness from the dispatch work that remains at runtime.

<a id="p24"></a>
## P24. Extension methods and non-intrusive behavior attachment

**1. Modern approach & concrete entities.** Swift extensions and protocol extensions; Rust extension traits; C3 methods associated with types; free-function APIs with method-like syntax where the language supports it. [SWIFT-EXTENSIONS] [C3-METHODS] [RUST-TRAITS]

**2. Classical antecedent / historical anti-pattern.** Subclassing solely to add convenience operations, giant utility classes, and changing an upstream type definition whenever a downstream domain needs another operation.

**3. Underlying mechanism & memory layout.** Operations are associated with an existing type without necessarily adding stored instance data. Name lookup and conformance rules determine whether a call is a directly resolved extension, a protocol requirement dispatched through witnesses, or another mechanism. Storage composition is not implied by method-call syntax.

**4. Engineering trade-offs & footprint.** Extension names can conflict, and adding an upstream method can affect resolution. Protocol-extension dispatch distinctions matter in Swift. Extension traits may require imports and can interact with coherence. Convenience methods cannot legitimately bypass representation invariants merely because they read like members. Keep domain-specific extensions scoped to the domain that gives them meaning. This technique modernizes organization and reuse; it does not inherently add memory safety, dynamic extensibility, or faster generated code.

<a id="p25"></a>
## P25. Higher-kinded abstraction over type constructors

**1. Modern approach & concrete entities.** Haskell `Functor`, `Applicative`, and `Monad`; Scala higher-kinded type parameters such as `F[_]`; library encodings and associated-type families in languages without direct support. [HASKELL-FUNCTOR] [CATS-TYPECLASSES]

**2. Classical antecedent / historical anti-pattern.** Duplicating the same structural traversal for every container/effect wrapper, or forcing all containers through one erased base interface to share composition logic.

**3. Underlying mechanism & memory layout.** A generic abstraction ranges over a type constructor such as `F`, not merely one concrete type such as `F<Int>`. Operations relate `F<A>` and `F<B>` while preserving the constructor's structure and laws. Compilation can use specialization, dictionaries, or erasure according to the language; higher kinds do not dictate one memory layout.

**4. Engineering trade-offs & footprint.** Encodings in languages without native support can become verbose and obscure associated lifetime or ownership constraints. Dictionaries and wrappers can retain runtime costs when not specialized away. Laws are often conventional rather than compiler-proved. Rust GATs and C++ template-template parameters cover useful subsets but should not be presented as identical to an unrestricted higher-kinded type system. This is valuable for reusable composition libraries, not necessarily the simplest abstraction for a small fixed embedded pipeline.

<a id="p26"></a>
## P26. Parameterized modules and module-level dependency injection

**1. Modern approach & concrete entities.** OCaml functors and abstract signatures; C3 generic modules; Nix functions taking dependency attribute sets. A module functor and a Haskell `Functor` are different concepts despite the shared word. [OCAML-MODULES] [C3-GENERICS] [NIX-LANGUAGE]

**2. Classical antecedent / historical anti-pattern.** Global service lookup, preprocessor-selected module variants, and assembling an object-inheritance graph merely to supply a coherent family of related types and operations.

**3. Underlying mechanism & memory layout.** A module parameter supplies several related declarations together. An ML functor maps a module satisfying a signature to another module, preserving abstract type relationships. C3 generic modules instantiate declarations with parameters. A Nix function produces an attribute set/value from supplied dependencies through lazy evaluation, rather than compiled static type checking.

**4. Engineering trade-offs & footprint.** Static module instantiation can duplicate code; runtime module values or Nix closures can retain environments. Abstract/generative type identity can surprise consumers and requires clear sharing constraints. Nix dependency injection is compositional but dynamically typed. Passing an entire service set can conceal dependencies unless the required subset is explicit. The engineering advantage is configuring a coherent component family at a boundary, not claiming every module mechanism is a zero-cost static template.

<!-- REFERENCE DEFINITIONS -->
[C3-GENERICS]: https://c3-lang.org/generic-programming/generics/ "C3 documentation: generics"
[C3-INTERFACES]: https://c3-lang.org/generic-programming/anyinterfaces/ "C3 documentation: interfaces and any"
[C3-METHODS]: https://c3-lang.org/language-fundamentals/functions/ "C3 documentation: functions and methods"
[CATS-TYPECLASSES]: https://typelevel.org/cats/typeclasses.html "Cats: type classes"
[CPP-COMPARE]: https://eel.is/c++draft/class.compare.default "C++ working draft: defaulted comparisons"
[CPP-CONSTEXPR]: https://eel.is/c++draft/expr.const "C++ working draft: constant expressions"
[CPP-CONSTRAINTS]: https://eel.is/c++draft/temp.constr "C++ working draft: template constraints"
[CPP-CPO]: https://eel.is/c++draft/customization.point.object "C++ working draft: customization point objects"
[CPP-FUNCTION]: https://eel.is/c++draft/dcl.fct "C++ working draft: function declarations and explicit object parameters"
[CPP-FUNCTION-ERASURE]: https://eel.is/c++draft/func.wrap.func "C++ working draft: function"
[CPP-FUNCTIONREF]: https://eel.is/c++draft/func.wrap.ref "C++ working draft: function_ref"
[CPP-INSTANTIATION]: https://eel.is/c++draft/temp.inst "C++ working draft: template instantiation"
[CPP-LAMBDA]: https://eel.is/c++draft/expr.prim.lambda "C++ working draft: lambda expressions"
[CPP-MODULES]: https://eel.is/c++draft/module "C++ working draft: modules"
[CPP-MOVEFUNCTION]: https://eel.is/c++draft/func.wrap.move "C++ working draft: move_only_function"
[CPP-NOUNIQUE]: https://eel.is/c++draft/dcl.attr.nouniqueaddr "C++ working draft: no_unique_address"
[CPP-PACKS]: https://eel.is/c++draft/temp.variadic "C++ working draft: variadic templates"
[CPP-RANGE-ACCESS]: https://eel.is/c++draft/range.access "C++ working draft: range access"
[CPP-REFLECTION]: https://eel.is/c++draft/meta "C++ working draft: reflection"
[CPP-TEMPLATE-PARAMETERS]: https://eel.is/c++draft/temp.param "C++ working draft: template parameters"
[CPP-TUPLE]: https://eel.is/c++draft/tuple "C++ working draft: tuples"
[CPP-VARIANT-VISIT]: https://eel.is/c++draft/variant.visit "C++ working draft: variant visitation"
[GO-DICTIONARIES]: https://raw.githubusercontent.com/golang/proposal/master/design/generics-implementation-dictionaries-go1.18.md "Go compiler design: generic dictionaries and gcshape sharing"
[GO-INTERFACES]: https://go.dev/ref/spec#Interface_types "Go specification: interface types"
[GO-PACKAGES]: https://go.dev/doc/code "Go: code organization and packages"
[GO-REFLECTION]: https://go.dev/blog/laws-of-reflection "Go: the laws of reflection"
[GO-SPEC]: https://go.dev/ref/spec "Go language specification"
[HASKELL-FUNCTOR]: https://www.haskell.org/onlinereport/haskell2010/haskellch6.html "Haskell 2010 report: predefined types and classes"
[JULIA-METHODS]: https://docs.julialang.org/en/v1/manual/methods/ "Julia manual: methods and multiple dispatch"
[NIX-LANGUAGE]: https://nix.dev/manual/nix/2.34/language/ "Nix 2.34 reference manual: language"
[OCAML-MODULES]: https://ocaml.org/manual/5.3/moduleexamples.html "OCaml 5.3 manual: modules and functors"
[ODIN]: https://odin-lang.org/docs/overview/ "Odin language overview"
[ROC-TUTORIAL]: https://raw.githubusercontent.com/roc-lang/roc/main/docs/mini-tutorial-new-compiler.md "Roc official new-compiler tutorial"
[RUST-ASSOCIATED]: https://doc.rust-lang.org/reference/items/associated-items.html "Rust Reference: associated items"
[RUST-BOUNDS]: https://doc.rust-lang.org/reference/trait-bounds.html "Rust Reference: trait and lifetime bounds"
[RUST-CLOSURES]: https://doc.rust-lang.org/reference/types/closure.html "Rust Reference: closure types"
[RUST-CONST]: https://doc.rust-lang.org/reference/const_eval.html "Rust Reference: constant evaluation"
[RUST-DERIVE]: https://doc.rust-lang.org/reference/attributes/derive.html "Rust Reference: derive attributes"
[RUST-DYN]: https://doc.rust-lang.org/reference/types/trait-object.html "Rust Reference: trait objects"
[RUST-EQ]: https://doc.rust-lang.org/std/cmp/trait.Eq.html "Rust standard library: Eq laws"
[RUST-GENERICS]: https://doc.rust-lang.org/reference/items/generics.html "Rust Reference: generic parameters"
[RUST-HASH]: https://doc.rust-lang.org/std/hash/trait.Hash.html "Rust standard library: Hash laws"
[RUST-IMPLS]: https://doc.rust-lang.org/reference/items/implementations.html "Rust Reference: implementations and coherence"
[RUST-IMPLTRAIT]: https://doc.rust-lang.org/reference/types/impl-trait.html "Rust Reference: impl Trait"
[RUST-MACROS]: https://doc.rust-lang.org/reference/macros-by-example.html "Rust Reference: declarative macros"
[RUST-MONOMORPH]: https://rustc-dev-guide.rust-lang.org/backend/monomorph.html "Rust Compiler Development Guide: monomorphization"
[RUST-PRIVACY]: https://doc.rust-lang.org/reference/visibility-and-privacy.html "Rust Reference: visibility and privacy"
[RUST-PROCMACROS]: https://doc.rust-lang.org/reference/procedural-macros.html "Rust Reference: procedural macros"
[RUST-TRAITS]: https://doc.rust-lang.org/reference/items/traits.html "Rust Reference: traits"
[RUST-TUPLE]: https://doc.rust-lang.org/reference/types/tuple.html "Rust Reference: tuple types"
[RUST-VARIANCE]: https://doc.rust-lang.org/reference/subtyping.html "Rust Reference: subtyping and variance"
[SWIFT-ASSOCIATED]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Generics.md "The Swift Programming Language: generics and associated types"
[SWIFT-CALLING]: https://raw.githubusercontent.com/swiftlang/swift/main/docs/ABI/CallingConvention.rst "Swift ABI: calling conventions"
[SWIFT-CLOSURES]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Closures.md "The Swift Programming Language: closures"
[SWIFT-EXTENSIONS]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Extensions.md "The Swift Programming Language: extensions"
[SWIFT-INLINE]: https://developer.apple.com/documentation/swift/inlinearray "Swift standard library: InlineArray"
[SWIFT-LAYOUT]: https://raw.githubusercontent.com/swiftlang/swift/main/docs/ABI/TypeLayout.rst "Swift ABI: type layout"
[SWIFT-MACROS]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Macros.md "The Swift Programming Language: macros"
[SWIFT-OPAQUE]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/OpaqueTypes.md "The Swift Programming Language: opaque and boxed protocol types"
[SWIFT-PACKS]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0393-parameter-packs.md "Swift Evolution SE-0393: parameter packs"
[SWIFT-PROTOCOLS]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Protocols.md "The Swift Programming Language: protocols"
[V-INTERFACES]: https://docs.vlang.io/type-declarations.html#interfaces "V documentation: interfaces"
[ZIG]: https://ziglang.org/documentation/master/ "Zig language reference, master snapshot"
