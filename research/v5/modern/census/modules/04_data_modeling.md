# IV. Data Modeling & State Machines

The objective is to represent the admissible domain, not merely allocate enough bits to store it. Products express simultaneous facts, sums express alternatives, nominal wrappers distinguish meanings, and indexed types express relationships. Runtime evidence remains necessary wherever the environment, input, or protocol is not controlled by the type checker.

<a id="d01"></a>
## D01. Product types and named records

**1. Modern approach & concrete entities.** Rust structs and tuples, Swift structs, C++ aggregates and `std::tuple`, Go structs, and Roc records represent products. Named result records such as `ReadProgress { bytes_read, terminal_status }` preserve the meaning of multiple simultaneous outcomes. [RUST-TUPLE] [CPP-TUPLE] [GO-SPEC] [ROC-TUTORIAL]

**2. Classical antecedent / historical anti-pattern.** Parallel arrays with implicit correspondence, positional output parameters, universal dictionaries with string keys, and a class hierarchy introduced for a fixed collection of fields. Product modeling itself is older than modern languages; the contemporary improvement is systematic use with explicit invariants and composition.

**3. Underlying mechanism & memory layout.** A product contains all its components. If finite component domains have cardinalities `a` and `b`, the unconstrained product has `a × b` combinations. A typical native record stores fields inline with alignment padding, but field order, ABI classification, and empty-field treatment depend on the language and representation attributes. A product of references stores access paths, not copies of the referents.

**4. Engineering trade-offs & footprint.** The Cartesian product can include invalid combinations. A record `{connected: bool, socket: Optional<Socket>}` admits states that may have no domain meaning. Adding a constructor helps only if later mutation preserves the relation. Product representation is appropriate when all components can coexist, including intentional partial-progress/error combinations. Named fields improve maintenance but do not prove units, lifetime validity, or cross-field consistency. [RUST-LAYOUT]

<a id="d02"></a>
## D02. Sum types and payload-bearing alternatives

**1. Modern approach & concrete entities.** Rust enums, Swift associated-value enums, C++ `std::variant`, Zig `union(enum)`, Odin unions, V sum types, and Roc tag unions. A state can be `Disconnected | Connecting(Attempt) | Connected(Session)` instead of a bag of optional members. [RUST-ENUM] [CPP-VARIANT] [ZIG] [V-OPTIONS]

**2. Classical antecedent / historical anti-pattern.** Untagged unions, integer tags that disagree with payload fields, subclass hierarchies for a closed set of alternatives, and boolean combinations representing mutually exclusive states.

**3. Underlying mechanism & memory layout.** A sum contains one alternative. Its semantic cardinality is the sum of the alternatives' cardinalities. Conventional native layout reserves space for the largest payload plus a discriminator and padding; niche encoding can reuse impossible payload representations. A compiler may optimize a locally unobservable sum into control flow. A recursive sum needs finite indirection, indices, or another bounded representation.

**4. Engineering trade-offs & footprint.** A rare large payload enlarges every inline instance; boxing only that case can reduce average storage but adds allocation and indirection. Closed sums make new operations straightforward but require updates when cases are added. `std::variant` can become `valueless_by_exception`, unlike a simplistic always-one-case model. Neither a native enum nor its discriminant automatically defines a stable wire format. Keep large-case layout and evolution policy explicit.

<a id="d03"></a>
## D03. Exhaustive pattern matching and checked decomposition

**1. Modern approach & concrete entities.** Rust `match`, `if let`, and `let ... else`; Swift `switch`; Zig `switch`; Roc `match`; C++ `std::visit` for variant alternatives. Matching selects a case and exposes only the payload appropriate to that case. [RUST-MATCH] [SWIFT-CONTROL] [CPP-VARIANT-VISIT]

**2. Classical antecedent / historical anti-pattern.** Switching on a type code and casting the wrong payload, forgetting an enum case after a change, repeated `dynamic_cast`, and independent tag tests followed by unsafe member access.

**3. Underlying mechanism & memory layout.** Coverage checking operates over the alternatives and applicable pattern space. Lowering can use comparisons, decision trees, jump tables, or optimized tests of niches. Bindings may borrow, copy, or move payloads according to the language and pattern. The match itself need not allocate. Guards add conditions; their presence does not automatically establish complete coverage.

**4. Engineering trade-offs & footprint.** A catch-all branch trades explicit future-case review for forward compatibility. Cartesian matching over several sums can generate many combinations. C++ multi-variant visitation can increase instantiation and dispatch-table size. Exhaustiveness proves coverage of modeled values, not that the model includes every real-world event. Publicly non-exhaustive types and unknown wire values require a deliberate fallback. C++ visitation is a library facility, not a claim that C++26 has a general native pattern-matching syntax. [RUST-NONEXHAUSTIVE]

<a id="d04"></a>
## D04. Newtypes and strong nominal identifiers

**1. Modern approach & concrete entities.** Rust `struct UserId(u64)`, Go `type UserID uint64`, Odin `distinct`, C++ wrapper classes, and Swift wrapper structs. Distinguish `SensorId`, `BusId`, `ByteCount`, and `SampleCount` even when they use identical machine integers. [GO-SPEC] [ODIN] [RUST-LAYOUT]

**2. Classical antecedent / historical anti-pattern.** Passing unrelated integers interchangeably, aliases that only rename a primitive, universal `void*` handles, and parameter ordering bugs in APIs with many same-typed arguments.

**3. Underlying mechanism & memory layout.** Nominal type identity makes accidental substitution ill-typed. A wrapper can have the same runtime payload as its underlying value. Rust `#[repr(transparent)]` provides representation guarantees when its conditions hold; otherwise equal observed size is not itself an ABI contract. Operations and conversions are explicitly granted rather than inherited indiscriminately from the primitive.

**4. Engineering trade-offs & footprint.** Boilerplate, adapters, serialization, formatting, and operator definitions increase surface area. Aggressive implicit conversion destroys the distinction. Strong IDs prevent category errors but not stale-object access; combine them with generations or validated lookups where needed. A global `UserId` does not identify which database issued it. Transparent representation also does not make every integer a valid value of a wrapper with additional invariants. The type boundary must remain meaningful through FFI and deserialization.

<a id="d05"></a>
## D05. Dimensional quantities, units, and scale-aware values

**1. Modern approach & concrete entities.** C++ `std::chrono::duration<Rep, Period>` for time; domain quantity types parameterized by dimension and unit; Rust or Swift nominal quantity wrappers. Distinguish angle, angular velocity, time, voltage, fixed-point scale, and physical reference frame where the domain requires them. [CPP-DURATION]

**2. Classical antecedent / historical anti-pattern.** Unadorned `double` arguments, hidden milliseconds-to-seconds conventions, mixing degrees and radians, and using raw fixed-point integers without tracking their scaling law.

**3. Underlying mechanism & memory layout.** Dimensions and units can be compile-time metadata while the object stores only its representation. Type-level algebra governs permissible multiplication, division, addition, and comparison. Conversion between units can become a constant scale operation; some conversions erase, while others multiply, divide, round, or check bounds. A point on a scale and a difference between points may need distinct types.

**4. Engineering trade-offs & footprint.** Dimension safety does not establish numerical conditioning, adequate range, sensor calibration, or absence of overflow. Two angular values can share dimensions while referring to different frames or conventions. Template arithmetic and diagnostics become more complex. Integer conversions can truncate; floating conversions introduce rounding. A strongly typed physical interface should state representation, scale, saturation policy, and invalid-measurement handling instead of assuming units solve all numeric correctness.

<a id="d06"></a>
## D06. Smart constructors and invariant-carrying values

**1. Modern approach & concrete entities.** Private representations with validated factories; Rust `NonZero<T>`, `TryFrom`, and domain wrappers; C++ factories returning `std::expected`; Swift failable initializers. A successful constructor can establish a bounded range, validated encoding, normalized identifier, or nonempty collection. [RUST-NONZERO] [RUST-TRYFROM] [CPP-EXPECTED]

**2. Classical antecedent / historical anti-pattern.** Rechecking the same primitive at every function, partially initialized objects, public fields that permit invalid combinations, and boolean “validated” flags independent from the data they supposedly validate.

**3. Underlying mechanism & memory layout.** Validate at the boundary, then expose only invariant-preserving operations. The accepted value need not contain a runtime certificate: restricted construction makes possession of its type evidence of the invariant. Unused representations can additionally support niche optimization. Validation may transform data into a canonical representation, reducing downstream cases.

**4. Engineering trade-offs & footprint.** Evidence is only as strong as all construction and mutation paths. Unsafe conversion, unchecked deserialization, and mutable references to the representation can break it. An external fact—permission, device readiness, file existence—can change after validation; its evidence may need a lifetime, version, lease, or fresh check. `NonNull<T>` proves non-nullness, not dereferenceability. Use domain-specific names that state the actual invariant rather than a misleading universal `Safe<T>`. [RUST-NONNULL]

<a id="d07"></a>
## D07. Refinement types and solver-checked predicates

**1. Modern approach & concrete entities.** Liquid Haskell refinements such as an integer constrained by a predicate; indexed and proof-oriented APIs in dependent languages. These go beyond an ordinary wrapper by making selected value predicates available to a verification system. [LIQUIDHASKELL]

**2. Classical antecedent / historical anti-pattern.** Preconditions written only in comments, repeatedly testing index bounds that follow from known facts, and confusing runtime assertions with proofs that every call satisfies an interface.

**3. Underlying mechanism & memory layout.** The checker generates logical verification conditions from signatures and program operations. An SMT solver or proof term discharges obligations in the supported logic. Refinement evidence often erases, leaving the base representation and ordinary executable code. Runtime input still needs a checked introduction path unless its property is established from other evidence.

**4. Engineering trade-offs & footprint.** Solver support, decidability, annotations, and trusted assumptions limit what can be established automatically. Machine overflow, mathematical integers, floating-point behavior, and foreign calls must be modeled consistently. A proof of array bounds does not imply deadline satisfaction. Changes to an algorithm can break proofs even when the intended behavior is unchanged. C++ contracts are not automatically refinement typing: runtime predicate checking and static logical discharge are different enforcement mechanisms with different costs. [CPP-CONTRACTS]

<a id="d08"></a>
## D08. Typestate and consuming protocol transitions

**1. Modern approach & concrete entities.** `Socket<Connected>`, `Dma<Idle>`, `Dma<Running>`, and consuming state-specific methods; Rust ownership and phantom state parameters; Swift consuming noncopyable wrappers; C++ state-specialized wrappers. [RUST-OWNERSHIP] [RUST-PHANTOM] [SWIFT-NONCOPY]

**2. Classical antecedent / historical anti-pattern.** Every operation available on every object, with `initialized`, `running`, and `closed` booleans checked inconsistently at runtime.

**3. Underlying mechanism & memory layout.** A transition accepts an old-state capability and produces a new-state capability. A compile-time phase marker may occupy no storage; the device handle is transferred unchanged. The type checker restricts which methods are available. Runtime-selected phases require a sum or other dynamic representation rather than pretending all state is known statically.

**4. Engineering trade-offs & footprint.** Failed transitions must specify resource recovery. `Result<Running, (Idle, Error)>` is sound only if failure leaves the device idle; partial activation may require an `Unknown` or `RecoveryRequired` state. Hardware, interrupts, remote peers, and other processes can violate assumptions not modeled by the API. Types establish software sequencing under those assumptions, not physical truth. C++ moving from a wrapper does not itself forbid later calls on it. Typestate is most effective for a manageable number of meaningful phases rather than every transient bit combination.

<a id="d09"></a>
## D09. Staged builders and compile-time completeness

**1. Modern approach & concrete entities.** Builders parameterized by whether required fields are present, such as `RequestBuilder<HasEndpoint, MissingCredentials>`; Rust or C++ consuming setters returning new specializations; Swift staged wrapper types. This combines type-level state with construction rather than ongoing resource operation. [RUST-GENERICS] [CPP-TEMPLATE-PARAMETERS]

**2. Classical antecedent / historical anti-pattern.** A long constructor full of positional primitives, a mutable property bag whose `build()` discovers omissions late, and default zero values that accidentally count as configuration.

**3. Underlying mechanism & memory layout.** Each setter changes a type-level presence state. Only complete states expose `build()`. The runtime representation can remain a record of the provided fields; static markers need not add flags. Builders may use typestate to distinguish mutually exclusive choices as well as required fields.

**4. Engineering trade-offs & footprint.** With many independent requirements, the conceptual state space can approach `2^n`, although only instantiated states generate code. Error messages and inferred types grow. Runtime configuration files still need dynamic validation. Copying a large builder at every stage can be expensive unless movement, borrowing, or optimization avoids it. A staged builder proves field presence, not compatibility among their values; cross-field constraints still need checking. Reserve the technique for high-value invariants rather than making trivial option setters a type-level maze.

<a id="d10"></a>
## D10. Phantom brands and provenance distinctions

**1. Modern approach & concrete entities.** Rust `PhantomData<T>` and lifetime brands; IDs parameterized by pool, transaction, unit, or reference-frame tags; C++ template tag parameters and empty marker fields. [RUST-PHANTOM] [CPP-NOUNIQUE]

**2. Classical antecedent / historical anti-pattern.** Applying an index from one pool to another, mixing coordinate frames with the same numerical representation, and giving borrowed handles no relationship to their issuer.

**3. Underlying mechanism & memory layout.** A phantom parameter influences type checking without storing a value of that parameter. A carefully designed generative scope can create a brand that external code cannot manufacture. Rust marker choices also affect variance, auto traits, and drop analysis; they are not merely decorative documentation.

**4. Engineering trade-offs & footprint.** A shared tag type distinguishes categories, not necessarily separate runtime instances. Two pools both parameterized by `GlobalPool` still share a brand. Constructors and lifetime quantification must establish the intended uniqueness. Brands do not prevent stale reuse within one issuer; generation checks may still be required. Incorrect variance or marker choice can impose unnecessary restrictions or encode the wrong ownership/thread-safety relationship. The marker's runtime size may be zero while the API's conceptual complexity is substantial. Test misuse cases by confirming that forbidden substitutions fail to type-check.

<a id="d11"></a>
## D11. Indexed data families, GADTs, and dependent shapes

**1. Modern approach & concrete entities.** Haskell generalized algebraic data types for typed expression trees; Idris `Vect n a`, `Fin n`, and erased evidence. A constructor can establish that an expression produces a particular type or that a collection has a particular length. [GHC-GADT] [IDRIS-LINEAR]

**2. Classical antecedent / historical anti-pattern.** Untyped AST nodes requiring downcasts, vector lengths tracked only in comments, and interpreters containing “impossible” runtime type-error cases created by their own internal representation.

**3. Underlying mechanism & memory layout.** Constructors constrain type indices, and matching refines the facts available in a branch. A typed expression interpreter can return the type indexed by the expression. A bounded index provides evidence that indexing a matching vector is valid. Some type indices and proofs erase; executable values and any runtime evidence needed by operations remain.

**4. Engineering trade-offs & footprint.** Type inference and pattern matching become more intricate. Proof maintenance can exceed the cost of ordinary implementation. Runtime data must be packaged existentially or validated before its dependent relation is known. Erasing a proof does not imply zero cost for computing evidence or building the underlying structure. Ordinary Rust const generics and C++ template parameters support useful fragments of this style, but they are not unrestricted dependent type systems. Foreign data and unsafe constructors remain trusted boundaries.

<a id="d12"></a>
## D12. Session-typed communication protocols

**1. Modern approach & concrete entities.** Session-typed channels such as the Rust `sesh` library's `Send<T, S>`, `Recv<T, S>`, and `End`, with dual endpoint protocols. A sequence can encode receiving a request, sending a response, and closing. [SESH]

**2. Classical antecedent / historical anti-pattern.** Sending a message that is invalid in the current phase, pairing two receivers, and maintaining protocol sequencing only in diagrams and integer state variables.

**3. Underlying mechanism & memory layout.** Each communication operation consumes an endpoint and yields its continuation endpoint. Peer endpoints carry dual types: one side's send corresponds to the other's receive. The protocol type may erase while the implementation retains channel endpoints, queues, synchronization state, and message storage.

**4. Engineering trade-offs & footprint.** Well-typed sequencing is not equivalent to successful communication. Cancellation, timeouts, crashes, transport corruption, and version negotiation still need representation. Deadlock-freedom claims depend on the library's composition discipline and modeled communication topology; arbitrary external locks or channels can invalidate stronger conclusions. Dynamic protocol choices require typed branching, recursion, or existential packaging. Session types are especially useful at well-defined internal boundaries; a network peer written outside the type system still requires runtime enforcement of the protocol.

<a id="d13"></a>
## D13. Capability-oriented authority and narrow dependency interfaces

**1. Modern approach & concrete entities.** Authority-bearing values such as `cap_std::fs::Dir`; APIs accepting a particular clock, allocator, peripheral, or transport capability instead of consulting ambient global facilities. A capability combines access with authority to perform a bounded operation set. [CAP-STD]

**2. Classical antecedent / historical anti-pattern.** Global service locators, unrestricted filesystem paths, hidden process-wide configuration, and components that can access every device merely because a global namespace exposes them.

**3. Underlying mechanism & memory layout.** A value holds an operating-system handle, bounded reference, or service interface. Static types restrict which operations are exposed; runtime enforcement can constrain the resource domain. A borrowed capability may be just a pointer, while an owning one has release semantics. Explicit parameters make the dependency graph visible.

**4. Engineering trade-offs & footprint.** Threading capabilities through call chains increases signature and lifetime complexity. Dynamic capabilities may add dispatch and reference counting. A convention of passing dependencies is not itself a security sandbox if code can reacquire ambient authority or bypass the wrapper. Authority attenuation and revocation require concrete mechanisms. In embedded software, a typed register-block handle can prevent accidental access to the wrong peripheral without proving MPU configuration, exclusive hardware ownership, or freedom from privileged bypass.

<a id="d14"></a>
## D14. Functional cores, reducers, and explicit command emission

**1. Modern approach & concrete entities.** Pure transition functions of the form `transition(State, Event) -> (State, Commands)`; reducers over ADTs; immutable records; Roc pure functions and Nix expressions as examples of separating calculations from effects. [ROC-TUTORIAL] [NIX-LANGUAGE]

**2. Classical antecedent / historical anti-pattern.** Business logic spread across mutating methods and callbacks, observers recursively triggering more observers, and state transitions inseparable from device access or network I/O.

**3. Underlying mechanism & memory layout.** The core computes a new logical state and an explicit description of requested effects. An imperative shell interprets commands and feeds subsequent observations back as events. The implementation may use copies, unique ownership with storage reuse, or persistent sharing; purity does not mandate one physical representation.

**4. Engineering trade-offs & footprint.** Large state copies and command allocations can be expensive unless representation is chosen deliberately. Describing an effect does not guarantee its execution, ordering, or atomicity. Commands need correlation IDs and completion events where partial failure matters. A pure model simplifies replay and property testing, but clocks, randomness, and external nondeterminism must be explicit inputs. Deterministic replay also requires controlled numerical and ordering behavior; the word “functional” does not supply those guarantees automatically.

<a id="d15"></a>
## D15. Persistent immutable collections and structural sharing

**1. Modern approach & concrete entities.** Persistent vectors, hash-array-mapped tries, and immutable maps; Rust's `im` collection library and analogous functional-language libraries. Persistence means older versions remain accessible after an update, not persistence to disk. [IM-COLLECTIONS]

**2. Classical antecedent / historical anti-pattern.** Deep-copying an entire structure for each snapshot, sharing one mutable graph among readers, and manual undo logs for values that naturally form a version history.

**3. Underlying mechanism & memory layout.** An update constructs a new path or small set of nodes and reuses unchanged substructure. A wide branching factor can keep tree depth small. Node ownership may use reference counting, garbage collection, or explicit regions. Uniqueness-aware implementations can mutate unshared nodes internally while preserving the external value semantics.

**4. Engineering trade-offs & footprint.** Updates allocate or manage node ownership; traversal adds indirection compared with a flat array. Old versions retain shared storage, potentially keeping much more memory alive than expected. Reference-counted nodes add atomic or non-atomic bookkeeping according to the implementation. Sharing can improve snapshot cost while hurting locality in hot loops. Persistent collection semantics and copy-on-write arrays are related but different implementation families. Evaluate version lifetime, branching factor, traversal pattern, and reclamation cost rather than assuming immutability is free.

<a id="d16"></a>
## D16. Optics: lenses, prisms, and composable traversal

**1. Modern approach & concrete entities.** Lenses focusing on product fields, prisms selecting sum alternatives, and traversals addressing zero or more targets; Haskell's `lens` package provides concrete composable abstractions. [HASKELL-LENS]

**2. Classical antecedent / historical anti-pattern.** Repeated nested copy-update code, long chains of getters/setters, string-based property paths, and manually rebuilding enclosing records whenever a deeply nested value changes.

**3. Underlying mechanism & memory layout.** An optic packages or encodes a lawful access/update relationship. Composition constructs a path through nested structures without hard-coding every whole-object update. Depending on representation and specialization, the abstraction can inline into direct projections and reconstruction; a reified dynamic optic may retain callable objects and dispatch.

**4. Engineering trade-offs & footprint.** Lens laws are semantic obligations, not guaranteed merely by using the expected function shape. Persistent updates still have their allocation or copying costs. Highly polymorphic optic types can produce difficult errors and substantial compilation work. A setter that silently normalizes, rejects, or aliases data may violate ordinary lens expectations and require a different abstraction. The technique is most useful when nested immutable transformations recur; it can be disproportionate for a shallow systems record with a few explicit fields.

<a id="d17"></a>
## D17. Entity-component modeling and data-driven composition

**1. Modern approach & concrete entities.** Entity-component systems such as Flecs, typed component sets, and systems that query required component combinations. Entities represent identity; components represent data; systems implement behavior over matching data. [FLECS-DESIGN]

**2. Classical antecedent / historical anti-pattern.** Deep `Entity -> Vehicle -> FlyingVehicle -> ArmedFlyingVehicle` inheritance, duplicated behavior across branches, and one virtual call per small object operation in bulk simulation.

**3. Underlying mechanism & memory layout.** An entity is typically an identifier. Archetype storage groups entities by component combination and places corresponding columns together; sparse-set designs organize components separately. Queries locate matching storage and iterate values in batches. Composition changes through component membership rather than class ancestry.

**4. Engineering trade-offs & footprint.** Structural changes can migrate entities, invalidate references, and incur deferred-update bookkeeping. Queries and entity lookup add indirection; too many archetypes fragment storage. An ECS does not automatically provide a formal domain model, safe concurrent scheduling, or faster execution for every workload. Relationships, ownership, and rare large components need explicit design. Typed component APIs reduce category errors but do not eliminate the need for generation checks and lifecycle rules. Use this for populations with repeated bulk behavior, not as a mandatory replacement for every object.

<a id="d18"></a>
## D18. Recursive data through arenas, indices, and explicit identity

**1. Modern approach & concrete entities.** Arena-backed ASTs and graphs, typed `NodeId` indices, sum-typed node payloads, and generational references when deletion/reuse is required. Rust or C++ vectors can hold nodes while edges store indices rather than owning pointers. [RUST-VEC] [SLOTMAP]

**2. Classical antecedent / historical anti-pattern.** One allocation per AST node, recursive ownership with expensive destruction, raw back-pointers, and shared-pointer cycles used merely to represent graph connectivity.

**3. Underlying mechanism & memory layout.** An arena owns nodes collectively. Edges encode logical references into arena storage, decoupling connectivity from physical addresses. Moving the backing allocation does not invalidate plain indices, provided indexing semantics remain stable. Typed indices can distinguish node classes or arenas; generations detect selected stale-reuse cases.

**4. Engineering trade-offs & footprint.** Lookup introduces indexing and potentially a validity check. Deletion, compaction, and cross-arena references require policy; an integer index does not track ownership by itself. Large sum payloads inflate each slot unless split by node kind or boxed selectively. Bulk reclamation simplifies lifetime management but retains unreachable nodes until reset. Recursive algorithms can still exhaust the call stack even when allocation is arena-based. The representation solves storage and identity questions, not automatically traversal complexity or semantic validity of edges.

<a id="d19"></a>
## D19. Typed embedded DSLs and parser combinators

**1. Modern approach & concrete entities.** Typed expression builders, query DSLs, and parser combinators such as Rust `nom`. A parser's output type describes the value established by successful recognition; typed AST constructors can distinguish expressions, statements, dimensions, and effects. [NOM] [GHC-GADT]

**2. Classical antecedent / historical anti-pattern.** Stringly typed query assembly, unchecked text substitution, handwritten parse routines with inconsistent cursor/error conventions, and generic AST nodes whose operands require runtime downcasts.

**3. Underlying mechanism & memory layout.** Combinators compose recognizers, cursor advancement, and output construction. Statically known combinators may specialize into direct control flow; a runtime parser graph may allocate and dispatch. Zero-copy parsers return views into input when its lifetime permits. Typed DSLs can build explicit syntax trees or interpret construction directly, depending on the design.

**4. Engineering trade-offs & footprint.** Backtracking, ambiguity, recursion depth, input retention, and diagnostic construction remain runtime costs. A parser API's use of `Result` does not imply linear-time behavior. Incomplete streaming input must be distinguished from invalid input. An AST that is well-typed internally may still be unauthorized or unsafe to execute. Heavy combinator types can inflate compile time, while aggressive erasure trades that cost for dynamic calls. Specify grammar, resource limits, lifetime, and failure policy separately.

<a id="d20"></a>
## D20. Boundary types, explicit representation, and checked zero-copy parsing

**1. Modern approach & concrete entities.** Rust `#[repr(C)]`, `#[repr(transparent)]`, and `extern "C"`; Zig `extern struct`; C++ boundary records and C linkage; Rust `zerocopy` traits including `TryFromBytes`, `KnownLayout`, and `Unaligned`. [RUST-LAYOUT] [ZIG] [ZEROCOPY]

**2. Classical antecedent / historical anti-pattern.** Casting a byte buffer to an arbitrary native struct, dumping object memory as a protocol, and exporting native strings, trait objects, or variants across an uncontrolled compiler ABI.

**3. Underlying mechanism & memory layout.** Boundary representations explicitly address size, offsets, alignment, endianness, and admissible bit patterns. A checked view validates applicable conditions before exposing typed access; a decoder may instead construct independent domain values. C linkage controls function linkage/calling conventions as specified, not the validity of arbitrary C++ object layouts.

**4. Engineering trade-offs & footprint.** Zero-copy parsing still incurs checks and ties results to backing-storage lifetime and mutability. Packed fields can require unaligned access sequences; padding can leak uninitialized bytes in naive serialization. A native layout attribute does not define endianness or protocol versioning. C interfaces remain useful interoperability boundaries: modernization consists in narrowing and wrapping their obligations, not pretending an ABI can be removed. Validate the received domain before converting wire integers into stronger internal types.

<a id="d21"></a>
## D21. Declarative configuration and dependency-graph construction

**1. Modern approach & concrete entities.** Nix expressions, immutable attribute sets, parameterized functions, and derivations; configurations represented as values rather than imperative sequences of shell mutations. This is an architectural companion to systems programming, not a claim about zero-cost native object layout. [NIX-LANGUAGE] [NIX-DERIVATIONS]

**2. Classical antecedent / historical anti-pattern.** Provisioning scripts whose result depends on execution history, hidden environment variables, globally installed tools, and copied configuration fragments with untracked dependency relationships.

**3. Underlying mechanism & memory layout.** Evaluation constructs values describing dependencies and build actions. Derivations specify inputs and builder information that the build system uses to organize work and outputs. Lazy evaluation can defer expression work until demanded. The evaluator retains runtime values and thunks; it does not erase into a borrow-checked native interface.

**4. Engineering trade-offs & footprint.** Nix is dynamically typed, so many shape and value errors appear during evaluation rather than a separate static type check. Declarative description, pure evaluation, sandboxed builds, content addressing, and reproducible outputs are distinct properties. An undeclared environmental dependency or nondeterministic builder can defeat reproducibility. Laziness complicates error locality and memory-retention reasoning. Pin inputs and specify build assumptions, while keeping domain validation explicit rather than treating an attribute set as a statically verified schema.

<a id="d22"></a>
## D22. Evolution-aware sums and explicit unknown values

**1. Modern approach & concrete entities.** Rust `#[non_exhaustive]`, wire-level `Unknown(raw_code)` alternatives, versioned schemas, and Protocol Buffers field-number and unknown-field evolution rules. [RUST-NONEXHAUSTIVE] [PROTOBUF]

**2. Classical antecedent / historical anti-pattern.** Assuming every future enum value is already known, reusing removed field identifiers, discarding unrecognized data during forwarding, and relying on native discriminant ordinals as protocol identifiers.

**3. Underlying mechanism & memory layout.** A public type can require external consumers to retain a fallback path. A wire decoder can preserve unknown discriminants and raw payloads rather than inventing a known semantic value. Versioned schemas separate stable external identifiers from implementation layout. Forwarding may retain extra buffers or unknown-field storage.

**4. Engineering trade-offs & footprint.** Tolerating unknown values is not equivalent to safely acting on them. A safety-relevant command may require rejection while a telemetry relay should preserve it. Unknown-field retention consumes memory and may retain maliciously large payloads unless bounded. Compatibility depends on reader/writer direction and changes to meaning, not just whether parsing succeeds. A non-exhaustive source enum provides an evolution mechanism at that API boundary; it does not automatically establish binary or wire compatibility.

<a id="d23"></a>
## D23. Typed flag sets versus exclusive state alternatives

**1. Modern approach & concrete entities.** Rust `bitflags` types, Odin `bit_set`, C++ scoped enums with deliberate flag operators, and finite-set wrappers. Use these for independent memberships or capabilities; use sums for mutually exclusive phases. [BITFLAGS] [ODIN]

**2. Classical antecedent / historical anti-pattern.** Arbitrary integer masks, mixing unrelated flag domains, using booleans to represent exactly-one-of-N states, and accepting reserved bits without a defined policy.

**3. Underlying mechanism & memory layout.** A typed flag set stores a bit vector. Set algebra becomes bitwise operations while nominal type identity distinguishes domains. For `n` independent flags, up to `2^n` combinations are meaningful; an exclusive `n`-case state requires only `n` alternatives. Constructors can reject or preserve unknown bits according to the boundary contract.

**4. Engineering trade-offs & footprint.** A flag type does not prove that every combination is valid. Mutually dependent flags still need validation or a better sum/product decomposition. Reserved bits matter during forward-compatible decoding and hardware register access. Atomic updates require a supported atomic representation or synchronization; individually setting bits is not automatically race-free. Bit packing improves storage density but can introduce read-modify-write traffic and false sharing. The right replacement for boolean soup is sometimes a product of independent states, not one enormous enum.

<a id="d24"></a>
## D24. Hierarchical state machines and orthogonal state composition

**1. Modern approach & concrete entities.** Statecharts with hierarchical states, orthogonal regions, guards, and entry/exit actions; XState provides a concrete modern implementation. Typed events and sum/product state models can be used beneath or alongside the statechart notation. [STATECHARTS]

**2. Classical antecedent / historical anti-pattern.** Large switches duplicating common transitions, boolean state combinations with no transition semantics, and flattening every combination of independent subsystem states into a manually enumerated global state.

**3. Underlying mechanism & memory layout.** Hierarchy factors common transitions and behavior; orthogonal regions represent simultaneously active submachines. Generated or interpreted execution tracks active configuration and dispatches events under specified transition-priority rules. A product of local states can represent global configuration without storing a separate object for every possible combination.

**4. Engineering trade-offs & footprint.** Hierarchy and concurrency reduce textual duplication but not necessarily the reachable-state space. Event queues, deferred events, history states, and run-to-completion semantics require explicit bounds and ordering. A statechart engine may allocate or dispatch indirectly; generated tables and direct code have different footprints. Guards and actions can introduce effects not represented by state labels. Statecharts are historically established, not newly invented; their contemporary value is integrating explicit protocol models with typed events, verification, code generation, and test coverage.

<!-- REFERENCE DEFINITIONS -->
[BITFLAGS]: https://docs.rs/bitflags/latest/bitflags/ "bitflags crate documentation"
[CAP-STD]: https://docs.rs/cap-std/latest/cap_std/ "cap-std capability-oriented standard library"
[CPP-CONTRACTS]: https://eel.is/c++draft/basic.contract "C++ working draft: contracts"
[CPP-DURATION]: https://eel.is/c++draft/time.duration "C++ working draft: duration"
[CPP-EXPECTED]: https://eel.is/c++draft/expected "C++ working draft: expected"
[CPP-NOUNIQUE]: https://eel.is/c++draft/dcl.attr.nouniqueaddr "C++ working draft: no_unique_address"
[CPP-TEMPLATE-PARAMETERS]: https://eel.is/c++draft/temp.param "C++ working draft: template parameters"
[CPP-TUPLE]: https://eel.is/c++draft/tuple "C++ working draft: tuples"
[CPP-VARIANT]: https://eel.is/c++draft/variant "C++ working draft: variant"
[CPP-VARIANT-VISIT]: https://eel.is/c++draft/variant.visit "C++ working draft: variant visitation"
[FLECS-DESIGN]: https://raw.githubusercontent.com/SanderMertens/flecs/master/docs/DesignWithFlecs.md "Flecs official design guide"
[GHC-GADT]: https://ghc.gitlab.haskell.org/ghc/doc/users_guide/exts/gadt.html "GHC User's Guide: generalized algebraic data types"
[GO-SPEC]: https://go.dev/ref/spec "Go language specification"
[HASKELL-LENS]: https://hackage.haskell.org/package/lens "lens package: lenses, folds, and traversals"
[IDRIS-LINEAR]: https://idris2.readthedocs.io/en/latest/tutorial/multiplicities.html "Idris 2 tutorial: multiplicities and erasure"
[IM-COLLECTIONS]: https://docs.rs/im/latest/im/ "im: persistent immutable collection documentation"
[LIQUIDHASKELL]: https://ucsd-progsys.github.io/liquidhaskell-tutorial/ "Liquid Haskell tutorial by the project authors"
[NIX-DERIVATIONS]: https://nix.dev/manual/nix/2.34/language/derivations "Nix 2.34 reference manual: derivations"
[NIX-LANGUAGE]: https://nix.dev/manual/nix/2.34/language/ "Nix 2.34 reference manual: language"
[NOM]: https://docs.rs/nom/latest/nom/ "nom parser-combinator documentation"
[ODIN]: https://odin-lang.org/docs/overview/ "Odin language overview"
[PROTOBUF]: https://protobuf.dev/programming-guides/proto3/ "Protocol Buffers official proto3 language guide"
[ROC-TUTORIAL]: https://raw.githubusercontent.com/roc-lang/roc/main/docs/mini-tutorial-new-compiler.md "Roc official new-compiler tutorial"
[RUST-ENUM]: https://doc.rust-lang.org/reference/items/enumerations.html "Rust Reference: enumerations"
[RUST-GENERICS]: https://doc.rust-lang.org/reference/items/generics.html "Rust Reference: generic parameters"
[RUST-LAYOUT]: https://doc.rust-lang.org/reference/type-layout.html "Rust Reference: type layout"
[RUST-MATCH]: https://doc.rust-lang.org/reference/expressions/match-expr.html "Rust Reference: match expressions"
[RUST-NONEXHAUSTIVE]: https://doc.rust-lang.org/reference/attributes/type_system.html "Rust Reference: non_exhaustive"
[RUST-NONNULL]: https://doc.rust-lang.org/std/ptr/struct.NonNull.html "Rust standard library: NonNull"
[RUST-NONZERO]: https://doc.rust-lang.org/std/num/struct.NonZero.html "Rust standard library: NonZero"
[RUST-OWNERSHIP]: https://doc.rust-lang.org/book/ch04-01-what-is-ownership.html "The Rust Book: ownership"
[RUST-PHANTOM]: https://doc.rust-lang.org/std/marker/struct.PhantomData.html "Rust standard library: PhantomData"
[RUST-TRYFROM]: https://doc.rust-lang.org/std/convert/trait.TryFrom.html "Rust standard library: TryFrom"
[RUST-TUPLE]: https://doc.rust-lang.org/reference/types/tuple.html "Rust Reference: tuple types"
[RUST-VEC]: https://doc.rust-lang.org/std/vec/struct.Vec.html "Rust standard library: Vec"
[SESH]: https://docs.rs/sesh/latest/sesh/ "sesh: session types with safe session composition"
[SLOTMAP]: https://docs.rs/slotmap/latest/slotmap/ "slotmap crate documentation"
[STATECHARTS]: https://stately.ai/docs/state-machines-and-statecharts "Stately/XState documentation: state machines and statecharts"
[SWIFT-CONTROL]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/ControlFlow.md "The Swift Programming Language: control flow"
[SWIFT-NONCOPY]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0390-noncopyable-structs-and-enums.md "Swift Evolution SE-0390: noncopyable structs and enums"
[V-OPTIONS]: https://docs.vlang.io/type-declarations.html "V documentation: options, results, sum types, and generics"
[ZEROCOPY]: https://docs.rs/zerocopy/latest/zerocopy/ "zerocopy: checked byte reinterpretation and layout traits"
[ZIG]: https://ziglang.org/documentation/master/ "Zig language reference, master snapshot"
