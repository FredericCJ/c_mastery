# Modern Programming Techniques

## A Systems-Oriented Census of 100 Techniques

**Research snapshot: 13 October 2026.**

A reference on ownership, representation, control flow, genericity, algebraic modeling, and protocol safety. Every technique follows the requested four-point analytical grid.

## Navigation

- [Reading kernel](#reading-kernel)
- [I. Memory Management & Data Layout Views](#section-1)
- [II. Control Flow, Error Handling & Vocabulary Types](#section-2)
- [III. Polymorphism, Genericity & Type Systems](#section-3)
- [IV. Data Modeling & State Machines](#section-4)
- [V. Language crosswalk and maturity boundaries](#section-5)
- [VI. Layout models and worked examples](#section-6)
- [VII. Engineering selection and migration guide](#section-7)
- [VIII. Terminology and non-equivalences](#section-8)
- [Primary-source registry](#primary-sources)

### Technique index

#### I. Memory Management & Data Layout Views

- [M01 — Ownership-bearing resource wrappers and deterministic destruction](#m01)
- [M02 — Affine ownership, noncopyable values, and consuming operations](#m02)
- [M03 — Borrowing and compile-time lifetime relationships](#m03)
- [M04 — Borrow splitting and disjoint mutable views](#m04)
- [M05 — Contiguous slices, spans, and bounded access descriptors](#m05)
- [M06 — Encoding-aware text and binary views](#m06)
- [M07 — Multidimensional, strided, and accessor-parametric views](#m07)
- [M08 — Separating storage owners from access interfaces](#m08)
- [M09 — Value semantics, copy-on-write, and value-oriented indirection](#m09)
- [M10 — Explicit shared ownership and weak observation](#m10)
- [M11 — Tracing garbage collection and escape-driven placement](#m11)
- [M12 — Explicit allocator injection and allocation-policy composition](#m12)
- [M13 — Regions, arenas, and bulk lifetime management](#m13)
- [M14 — Fixed-size and fixed-capacity inline containers](#m14)
- [M15 — Small-buffer optimization and adaptive inline storage](#m15)
- [M16 — Structure of arrays, AoSoA, and hot/cold data separation](#m16)
- [M17 — Portable SIMD values and explicit lane masks](#m17)
- [M18 — Pinning and address-sensitive values](#m18)
- [M19 — Explicit partial initialization and initialization-state tracking](#m19)
- [M20 — Interior mutability with explicit enforcement policy](#m20)
- [M21 — Thread-transfer and shared-access constraints](#m21)
- [M22 — Generational handles and stable logical identity](#m22)
- [M23 — Deferred concurrent reclamation: epochs and hazard protection](#m23)
- [M24 — Safe interfaces around unsafe layout and provenance operations](#m24)
- [M25 — Zero-sized markers and empty-state layout optimization](#m25)
- [M26 — Stable-address segmented storage instead of per-node allocation](#m26)

#### II. Control Flow, Error Handling & Vocabulary Types

- [C01 — Explicit absence with optional values](#c01)
- [C02 — Disjoint success/failure results](#c02)
- [C03 — Product returns for partial progress and independent outcomes](#c03)
- [C04 — Structured early-return propagation](#c04)
- [C05 — Typed throwing and explicit exception boundaries](#c05)
- [C06 — Monadic mapping, flattening, and error transformation](#c06)
- [C07 — Applicative validation and error accumulation](#c07)
- [C08 — Unit, bottom, and impossible outcome types](#c08)
- [C09 — Lexically registered cleanup and error-only cleanup](#c09)
- [C10 — Transactional updates and commit/rollback guards](#c10)
- [C11 — Explicit arithmetic and conversion failure policy](#c11)
- [C12 — Expression-oriented control flow and guarded bindings](#c12)
- [C13 — Lazy ranges, iterator pipelines, and loop fusion](#c13)
- [C14 — Typed early termination in higher-order algorithms](#c14)
- [C15 — Type-checked formatting and output construction](#c15)
- [C16 — Must-use results and deliberate discard](#c16)
- [C17 — Coroutines and compiler-generated asynchronous state machines](#c17)
- [C18 — Structured concurrency and scoped task lifetimes](#c18)
- [C19 — Explicit cancellation, deadlines, and bounded backpressure](#c19)
- [C20 — Typed asynchronous composition with senders and receivers](#c20)
- [C21 — Actor isolation and serialized access to mutable state](#c21)
- [C22 — Effect tracking and algebraic effect handlers](#c22)
- [C23 — Executable interface contracts](#c23)
- [C24 — One-time initialization and synchronized lazy values](#c24)

#### III. Polymorphism, Genericity & Type Systems

- [P01 — Constraint-based generic programming](#p01)
- [P02 — Monomorphization and static specialization](#p02)
- [P03 — Dictionary passing, witness tables, and representation sharing](#p03)
- [P04 — Traits, protocols, and coherent external conformance](#p04)
- [P05 — Structural conformance versus nominal type identity](#p05)
- [P06 — Policy composition and statically injected strategies](#p06)
- [P07 — Explicit receiver parameters and cv/ref-generic methods](#p07)
- [P08 — Existential packaging and explicit dynamic dispatch](#p08)
- [P09 — Opaque concrete return types](#p09)
- [P10 — Owning and borrowing type-erased callables](#p10)
- [P11 — Closures with explicit capture and invocation capabilities](#p11)
- [P12 — Associated types and generic associated type families](#p12)
- [P13 — Higher-ranked bounds, variance, and safe substitution](#p13)
- [P14 — Values in types: const generics and static shapes](#p14)
- [P15 — Typed parameter packs and heterogeneous variadics](#p15)
- [P16 — Compile-time execution and staged computation](#p16)
- [P17 — Static reflection and structure-derived implementation](#p17)
- [P18 — Hygienic syntax transformation and procedural macros](#p18)
- [P19 — Derived operations with explicit semantic laws](#p19)
- [P20 — Modules, visibility boundaries, and representation hiding](#p20)
- [P21 — Customization-point objects and constrained extension hooks](#p21)
- [P22 — Heterogeneous compile-time records and type-indexed lookup](#p22)
- [P23 — Multiple dispatch and symmetric operation selection](#p23)
- [P24 — Extension methods and non-intrusive behavior attachment](#p24)
- [P25 — Higher-kinded abstraction over type constructors](#p25)
- [P26 — Parameterized modules and module-level dependency injection](#p26)

#### IV. Data Modeling & State Machines

- [D01 — Product types and named records](#d01)
- [D02 — Sum types and payload-bearing alternatives](#d02)
- [D03 — Exhaustive pattern matching and checked decomposition](#d03)
- [D04 — Newtypes and strong nominal identifiers](#d04)
- [D05 — Dimensional quantities, units, and scale-aware values](#d05)
- [D06 — Smart constructors and invariant-carrying values](#d06)
- [D07 — Refinement types and solver-checked predicates](#d07)
- [D08 — Typestate and consuming protocol transitions](#d08)
- [D09 — Staged builders and compile-time completeness](#d09)
- [D10 — Phantom brands and provenance distinctions](#d10)
- [D11 — Indexed data families, GADTs, and dependent shapes](#d11)
- [D12 — Session-typed communication protocols](#d12)
- [D13 — Capability-oriented authority and narrow dependency interfaces](#d13)
- [D14 — Functional cores, reducers, and explicit command emission](#d14)
- [D15 — Persistent immutable collections and structural sharing](#d15)
- [D16 — Optics: lenses, prisms, and composable traversal](#d16)
- [D17 — Entity-component modeling and data-driven composition](#d17)
- [D18 — Recursive data through arenas, indices, and explicit identity](#d18)
- [D19 — Typed embedded DSLs and parser combinators](#d19)
- [D20 — Boundary types, explicit representation, and checked zero-copy parsing](#d20)
- [D21 — Declarative configuration and dependency-graph construction](#d21)
- [D22 — Evolution-aware sums and explicit unknown values](#d22)
- [D23 — Typed flag sets versus exclusive state alternatives](#d23)
- [D24 — Hierarchical state machines and orthogonal state composition](#d24)


---

<a id="reading-kernel"></a>
## Reading kernel: guarantees, representations, and cost

**Research snapshot:** 13 October 2026. **Scope:** 100 distinct techniques in four categories, with language comparisons and engineering appendices. This is a broad architectural census, not a claim that every programming-language technique has been enumerated or that all listed implementations provide equivalent guarantees.

### What “modern” means here

The organizing direction is to move implicit contracts into explicit representations: ownership into resource types, absence and failure into vocabulary types, valid alternatives into sums, behavioral requirements into constraints, and protocol sequencing into state-indexed interfaces. Many foundations—RAII, algebraic data types, regions, parametric polymorphism, statecharts, and persistent collections—are older than their current implementations. Contemporary uptake is not a historical invention date.

The antecedent in each entry identifies a specific mismatch or risk. It does not declare that inheritance, raw pointers, C ABI interfaces, exceptions, mutable records, or integer flags are universally inappropriate. An open plugin boundary may require dynamic dispatch; a hardware register demands explicit low-level access; a C ABI can be the most stable integration boundary. The objective is to confine the obligations of those mechanisms and make their contracts reviewable.

### The four-point entry contract

Every numbered technique contains the same four fields:

1. **Modern approach & concrete entities:** terminology and actual language or library primitives.
2. **Classical antecedent / historical anti-pattern:** the specific design weakness or risk being addressed.
3. **Underlying mechanism & memory layout:** representation, lowering, dispatch, lifetime, and storage behavior.
4. **Engineering trade-offs & footprint:** runtime, compile-time, binary-size, predictability, and maintenance consequences.

Stable identifiers are `M01–M26`, `C01–C24`, `P01–P26`, and `D01–D24`. These identifiers refer to techniques, not languages or chronological generations. Neighboring entries deliberately separate mechanisms often conflated in discussions: a slice versus a borrow, a result versus propagation syntax, a constraint versus a code-generation strategy, or typestate versus proof of physical device state.

### Classify the guarantee before selecting the feature

| Guarantee class | What it establishes | What remains outside it |
|---|---|---|
| Static language rule | Admissibility of a program under the language's typing, ownership, or effect rules | Assumptions at unsafe/foreign boundaries; external behavior; resource budgets |
| Library invariant | Properties established by restricted constructors and invariant-preserving methods | Bugs in trusted implementation; bypasses; undocumented mutation paths |
| Runtime enforcement | A check, lock, counter, collector, or scheduler enforces a condition while executing | Associated instruction, memory, latency, and failure costs |
| Optimization opportunity | A compiler may erase, inline, specialize, reuse, or elide work | No unconditional cost guarantee without stronger specification or measurement |
| Documented layout/ABI guarantee | A specified representation relationship or calling convention | Other targets, representation modes, language versions, and wire semantics |
| Working-draft or evolving facility | A documented design available for evaluation | Uniform compiler support, stable APIs, and production qualification |

Rust's lifetime checks normally do not add runtime lifetime metadata, but bounds checks and allocations are separate matters. C++ `span` packages access without establishing the same lifetime proof. The Rust layout reference explicitly distinguishes guarantees from current implementation observations; this census follows that distinction rather than turning an observed fat-pointer size into a universal ABI rule. [RUST-BORROW] [RUST-LAYOUT] [CPP-SPAN]

### Treat ownership, access, storage, identity, and effects independently

For any value, record five independent answers. Who destroys it? Who may read or mutate it, and for how long? Where is its storage, and can that storage move? What identifies it across time? Which effects can operations perform? A heap object can have value semantics. An inline object can borrow memory owned elsewhere. A logical handle can remain stable while an allocation moves. A pure function can still need substantial storage. A noncopyable object can still be leaked.

Similarly, distinguish a **closed** alternative set from an **open** implementation set. A sum is effective when the domain cases are intentionally enumerated. An existential interface is effective when new implementations must be added without changing the consumer. Neither dimension alone specifies allocation or dispatch: a sum may contain pointers; a borrowed trait object may avoid allocation; a concrete generic may still carry runtime metadata. [RUST-DYN] [CPP-VARIANT] [SWIFT-CALLING]

### Evaluate zero-cost claims against an equivalent contract

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

### Source and maturity policy

Language facts are linked to specifications, official manuals, implementation documentation, project-authored library documentation, or original research. The engineering analyses are architectural deductions rather than benchmark claims. Source labels such as `[RUST-PIN]` are stable reference identifiers and clickable links in rendered Markdown. The complete source registry is included in the package.

C++26 entries refer to facilities documented in the current working draft, not to a promise that every compiler or standard-library combination implements them. Rolling `main`, `master`, and `latest` documentation is identified by the research snapshot; URLs can subsequently change. V's memory-management modes, Roc's new-compiler syntax, and Jai's evidence limitations receive explicit treatment in the language crosswalk. No all-language conformance or performance test was performed. Only the narrowly identified C++ and Go examples in the validation appendix were compiled and run.

---

<a id="section-1"></a>
## I. Memory Management & Data Layout Views

This category separates storage ownership, access permissions, object lifetime, address stability, allocation policy, and physical arrangement. They are independent design axes: a view is not an owner, an owner need not allocate, an arena is not a borrow checker, and a stable handle does not imply a stable address.

<a id="m01"></a>
### M01. Ownership-bearing resource wrappers and deterministic destruction

**1. Modern approach & concrete entities.** Represent acquisition and release as the construction and destruction of an owning value. Examples include C++ `std::unique_ptr<T, D>`, `std::unique_lock<M>`, and custom file-descriptor classes; Rust `Box<T>`, owned I/O handles, and `Drop`; Swift noncopyable resource wrappers with `deinit`. The resource may be memory, a lock, a subscription, a peripheral reservation, or a transaction. [CPP-UNIQUE] [RUST-DROP] [SWIFT-NONCOPY]

**2. Classical antecedent / historical anti-pattern.** Unmatched `malloc/free`, `open/close`, and `lock/unlock`; ownership inferred from naming; duplicated cleanup on every return; assigning a second raw pointer without deciding whether it owns anything. RAII is itself an older technique, not a C++20 invention.

**3. Underlying mechanism & memory layout.** The wrapper stores a resource representation and whatever cleanup state it requires. Control-flow lowering inserts destruction at applicable lifetime endpoints. A stateless C++ deleter commonly adds no storage; a stateful deleter can enlarge the wrapper. Rust drop glue destroys owned fields and may need conditional bookkeeping when initialization or moves depend on branches. Cleanup code is ordinary executable code, not a garbage collector.

**4. Engineering trade-offs & footprint.** Destruction may deallocate a graph, block, or acquire another lock; deterministic ordering does not establish constant latency. Fallible `close`, `flush`, and commit operations often require explicit methods, with destruction only as fallback. Aborts and leaks remain possible; Rust deliberately permits forgetting a value. Resource-safe APIs therefore establish safety even when cleanup never happens, rather than relying on every destructor eventually running. [RUST-FORGET]

<a id="m02"></a>
### M02. Affine ownership, noncopyable values, and consuming operations

**1. Modern approach & concrete entities.** Rust non-`Copy` values and by-value arguments; Swift `~Copyable`, `borrowing`, and `consuming`; C++ deleted copy operations and move-only classes. Idris 2 multiplicity-`1` arguments illustrate genuinely linear usage constraints. [RUST-OWNERSHIP] [SWIFT-NONCOPY] [IDRIS-LINEAR]

**2. Classical antecedent / historical anti-pattern.** Duplicating resource-owning bits, accidentally closing a handle twice, copying synchronization objects, or documenting “the caller relinquishes ownership” without a checkable interface.

**3. Underlying mechanism & memory layout.** Ownership transfer changes which program binding may use a resource. An affine discipline permits at most one consumption; a linear discipline requires exactly one use under its usage rules. Rust rejects use of a moved binding until it is validly reinitialized. The runtime may merely move bytes, pass registers, or eliminate the transfer. No universal “ownership flag” has to be stored beside every value.

**4. Engineering trade-offs & footprint.** A large inline value can still be expensive to move. A C++ `std::move(x)` only casts `x` into an expression category that can select move operations; it neither moves by itself nor disables future use of `x`. C++ moved-from values still exist and remain subject to their type's contract. Noncopyability also does not guarantee eventual release: affine ownership, process termination, deliberate leaking, and true linear resource protocols must be distinguished. APIs must decide how ownership is returned when a consuming operation fails.

<a id="m03"></a>
### M03. Borrowing and compile-time lifetime relationships

**1. Modern approach & concrete entities.** Rust `&'a T`, `&'a mut T`, lifetime bounds, reborrowing, and non-lexical borrow analysis; Swift lifetime-dependent non-owning `Span` APIs. C++ references and views provide access syntax without the same general lifetime-checking guarantee. [RUST-BORROW] [SWIFT-SPAN]

**2. Classical antecedent / historical anti-pattern.** Returning a local address, retaining a reference after its owner is destroyed, using a container element after reallocation, and allowing a callback to escape while borrowing temporary state.

**3. Underlying mechanism & memory layout.** The compiler relates the region in which a borrow is usable to the lifetime of its source and to conflicting accesses. Lifetime parameters generally erase: a reference does not carry an expiration timestamp or a runtime reference count. Rust's exclusive mutable borrow constrains simultaneous access through aliases; it is not just a non-null pointer annotation. A lifetime parameter expresses a relationship, not an instruction to extend storage duration.

**4. Engineering trade-offs & footprint.** Graphs, self-referential structures, caches, and asynchronous captures can require a different representation or narrowly audited unsafe code. Static analysis may reject a safe algorithm whose disjointness it cannot derive. Borrow checking does not establish valid protocol values, absence of deadlock, bounded execution, or synchronization with DMA. The benefit is eliminating classes of lifetime/aliasing errors from safe code, not eliminating every runtime check or every pointer.

<a id="m04"></a>
### M04. Borrow splitting and disjoint mutable views

**1. Modern approach & concrete entities.** Rust slice `split_at_mut`, `chunks_mut`, and `iter_mut`; field-sensitive borrowing; domain APIs returning non-overlapping subregions. C++ `span::subspan` can express corresponding regions but does not prove their mutual disjointness. [RUST-SLICE] [CPP-SPAN]

**2. Classical antecedent / historical anti-pattern.** Hand-computed overlapping pointer ranges, disabling alias analysis globally, or sharing one mutable buffer between workers with only comments describing their assigned segments.

**3. Underlying mechanism & memory layout.** A checked split validates the split point, then creates two descriptors whose index intervals cannot overlap. A small unsafe implementation can establish this invariant once and expose a safe API. The elements remain in the original allocation. Iterators can progressively remove yielded elements from the remaining region, allowing multiple live mutable element references without aliasing the same element.

**4. Engineering trade-offs & footprint.** The boundary check may remain unless proved redundant. Disjoint bytes are not automatically independent logical objects: two records may reference the same heap allocation or device. Splitting storage also does not establish cache-line separation, so independent writers can still suffer false sharing. Irregular graphs and scatter/gather mappings require stronger representations than two contiguous intervals. This technique is particularly valuable when partitioning numerical kernels or worker tasks because the API carries a useful invariant that raw offsets do not.

<a id="m05"></a>
### M05. Contiguous slices, spans, and bounded access descriptors

**1. Modern approach & concrete entities.** C++20 `std::span<T, Extent>`; Rust `&[T]` and `&mut [T]`; Go `[]T`; Zig and Odin `[]T`; C3 `T[]`. These expose a sequence without requiring a particular owning container. [CPP-SPAN] [GO-SLICES] [ZIG] [ODIN] [C3-SPEC]

**2. Classical antecedent / historical anti-pattern.** Independent pointer/count parameters, array-to-pointer decay that loses extent, and APIs accepting an owning vector merely to read contiguous elements.

**3. Underlying mechanism & memory layout.** A dynamic descriptor is commonly pointer plus length. Go additionally carries capacity. A fixed-extent C++ span can encode the length in the type rather than storing it. Descriptor copying aliases backing storage; it does not duplicate elements. Slice construction and indexing checks depend on the language, operation, and safety mode.

**4. Engineering trade-offs & footprint.** C++ spans can dangle and do not freeze the owner. A small Go slice can retain a large backing allocation. Bounds metadata does not validate a fabricated pointer or prove lifetime. A slice of owning references still involves indirection per element. Different ABI shapes matter at foreign boundaries. Distinguish `span<T>` from `span<const T>`: constness of the descriptor itself is not the same as read-only element access. C++26 hardening facilities are not a substitute for ownership tracking. [CPP-SPAN]

<a id="m06"></a>
### M06. Encoding-aware text and binary views

**1. Modern approach & concrete entities.** Rust `&str`/`String` for UTF-8, `&[u8]` for arbitrary bytes, and `CStr`/`CString` for C-string invariants; C++ `std::string_view`, `std::u8string_view`, and `std::span<const std::byte>`. [RUST-STR] [RUST-CSTR] [CPP-STRINGVIEW]

**2. Classical antecedent / historical anti-pattern.** Treating every `char*` as both binary storage and a null-terminated string; assuming byte offsets are character indices; silently truncating at embedded NUL.

**3. Underlying mechanism & memory layout.** A view contains access metadata rather than text ownership. UTF-8 validation can establish an invariant once at construction; later operations can rely on valid encoding while still checking character boundaries. Rust `&str` and `&[u8]` have the same reference layout, but different validity requirements. C++ string-view character type does not by itself prove that the underlying sequence is valid Unicode. [RUST-LAYOUT]

**4. Engineering trade-offs & footprint.** Validating encoding scans data; transcoding allocates or writes output somewhere. Unicode scalar values, grapheme clusters, display columns, and code units remain different units of measurement. A valid UTF-8 string can still contain prohibited application characters or misleading normalization variants. Borrowed text inherits storage-lifetime restrictions. Crossing C APIs may require a terminator or conversion rather than passing `.data()` blindly. Stronger text types eliminate representation ambiguity, not every semantic text-validation requirement.

<a id="m07"></a>
### M07. Multidimensional, strided, and accessor-parametric views

**1. Modern approach & concrete entities.** C++23 `std::mdspan<T, Extents, Layout, Accessor>`, `std::extents`, `layout_left`, `layout_right`, and `layout_stride`. A custom accessor can represent access semantics distinct from ordinary pointer dereference. [CPP-MDSPAN]

**2. Classical antecedent / historical anti-pattern.** Treating `T**` as a contiguous matrix, duplicating shape/stride conventions across functions, and hard-coding row-major indexing macros into every numerical algorithm.

**3. Underlying mechanism & memory layout.** The descriptor combines a data handle, extents, an index mapping, and an accessor. A simple strided mapping computes `offset = sum(index[k] * stride[k])`. Static extents or strides can erase metadata and fold arithmetic. The matrix data need not move or be nested behind row pointers. Changing a view's logical shape does not necessarily change the physical layout.

**4. Engineering trade-offs & footprint.** Dynamic mappings retain metadata and address arithmetic. A transposed view is cheap to construct but may produce an expensive traversal. Correctness still requires valid backing storage and applicable mapping preconditions. A const view object does not necessarily mean immutable elements. Algorithms should constrain the mapping properties they require, such as uniqueness or contiguous access, rather than silently assuming them. Custom accessors can introduce additional work, so an `mdspan` abstraction is not universally equivalent to a raw pointer plus a multiplication.

<a id="m08"></a>
### M08. Separating storage owners from access interfaces

**1. Modern approach & concrete entities.** Rust `Vec<T>` versus slices, `String` versus `str`, and `PathBuf` versus `Path`; C++ `vector<T>` versus `span<T>`, and `string` versus `string_view`. Accept a borrow for observation; accept an owner only when the operation actually needs ownership or growth. [RUST-VEC] [RUST-PATH] [CPP-VECTOR]

**2. Classical antecedent / historical anti-pattern.** Container-specific APIs with unnecessary copying, functions that may secretly retain any pointer they receive, and capacity-changing operations exposed to consumers that only need to inspect data.

**3. Underlying mechanism & memory layout.** An owning growable buffer tracks allocation and initialized length, usually with capacity metadata. A borrowed descriptor exposes only the relevant region. Constructing that descriptor generally copies a few words. Returning ownership can transfer an allocation rather than copying all elements; returning a borrow ties the result to existing storage.

**4. Engineering trade-offs & footprint.** The API must choose whether retention is allowed; a borrowed parameter cannot be stored indefinitely without changing the contract. Generic views can obscure required allocation or contiguity properties unless constrained. A “read-only” API may still trigger work through lazy internals or reference-semantic elements. Separating ownership from access reduces coupling, but does not eliminate capacity management, allocator failure, or expensive conversion between genuinely different representations. The correct signature follows the operation's required authority, not a universal preference for passing everything by reference.

<a id="m09"></a>
### M09. Value semantics, copy-on-write, and value-oriented indirection

**1. Modern approach & concrete entities.** Swift `Array` value semantics with copy-on-write storage; Rust `Cow<'a, B>`; C++26 `std::indirect<T>` and `std::polymorphic<T>` for owning indirection with value-oriented copying. These are related techniques, not interchangeable implementations. [SWIFT-ARRAY] [RUST-COW] [CPP-INDIRECT] [CPP-POLYMORPHIC]

**2. Classical antecedent / historical anti-pattern.** Copies that unexpectedly alias mutable logical state, mandatory reference identity for ordinary data, and handwritten virtual `clone()` hierarchies.

**3. Underlying mechanism & memory layout.** Logical copies can share backing storage until mutation requires uniqueness. Swift array storage may perform a uniqueness test before mutation. Rust `Cow` is a borrowed-or-owned sum; it does not itself imply shared reference-counted ownership. Value-oriented indirection stores an owned object out of line while making copy operations duplicate the logical value, including its concrete dynamic type where applicable.

**4. Engineering trade-offs & footprint.** A cheap copy can defer a large allocation/copy to the first write, harming latency predictability. Owning indirection adds pointer chasing and typically allocation. A value-semantic collection of reference-semantic elements does not deep-copy every referenced object. Uniqueness checks, allocator behavior, and retained snapshots affect the cost model. C++ value wrappers also retain moved-from states required by their object model; “value semantics” does not mean immutable, pointer-free, allocation-free, or a single physical representation.

<a id="m10"></a>
### M10. Explicit shared ownership and weak observation

**1. Modern approach & concrete entities.** Rust `Rc<T>`, `Arc<T>`, and `Weak<T>`; C++ `shared_ptr<T>` and `weak_ptr<T>`; Swift ARC and weak references. [RUST-RC] [RUST-ARC] [CPP-SHARED]

**2. Classical antecedent / historical anti-pattern.** Informal last-user cleanup conventions, manually synchronized reference counts, or callbacks holding dangling pointers after an owner disappears.

**3. Underlying mechanism & memory layout.** An allocation or associated control structure maintains ownership counts. Copies and releases update these counts; destruction occurs when strong ownership ends. Weak references can test or acquire strong ownership without keeping the value alive themselves. Some APIs can combine object and control allocation, while others use separate storage. Pointer-sized appearance at one call site does not reveal total allocation footprint.

**4. Engineering trade-offs & footprint.** Atomic reference-count updates can bounce cache lines. Non-atomic counting is cheaper but cannot be freely shared between threads. Cycles of strong references can leak. Releasing the last owner may recursively destroy a large graph. `Arc<T>` does not synchronize arbitrary mutation of `T`; `shared_ptr<T>` does not make the pointee thread-safe. Use shared ownership where independently extending lifetime is a real requirement, rather than as a substitute for deciding whether a reference should merely borrow. Weak observation can fail and must be represented accordingly.

<a id="m11"></a>
### M11. Tracing garbage collection and escape-driven placement

**1. Modern approach & concrete entities.** Go's tracing collector and compiler escape analysis; V's documented default garbage collection. V documents `-autofree` as an evolving mechanism, not a blanket equivalent of statically checked ownership. [GO-GC] [V-MEMORY]

**2. Classical antecedent / historical anti-pattern.** Manually establishing the last use of every allocation in a shared graph, freeing cyclic graphs by hand, and forcing all potentially escaping objects into explicitly managed storage.

**3. Underlying mechanism & memory layout.** Escape analysis can keep suitable objects in shorter-lived storage. The collector identifies reachable objects through roots and runtime metadata, and reclaims unreachable allocations. Allocation paths, scanning, write barriers, and concurrent collector work become part of execution. Reachability is not identical to semantic usefulness: a stale reference can keep otherwise unnecessary data alive.

**4. Engineering trade-offs & footprint.** Memory headroom, throughput, scheduling interference, and tail latency must be budgeted. Stack placement is an optimization, not an invariant to infer from local-variable syntax. GC does not provide prompt release of file descriptors, locks, or device reservations. Reference cycles are handled differently from reference counting, but a live accidental reference still causes retention. This is an important modern safety approach, yet it should not be classified as an erased, universally zero-runtime-cost abstraction or assumed appropriate for every bounded-latency system.

<a id="m12"></a>
### M12. Explicit allocator injection and allocation-policy composition

**1. Modern approach & concrete entities.** C++ `std::pmr::memory_resource`, `polymorphic_allocator`, and allocator-aware containers; Zig `std.mem.Allocator`; Odin `context.allocator` and explicit allocator arguments. [CPP-PMR] [ZIG-ALLOCATOR] [ODIN]

**2. Classical antecedent / historical anti-pattern.** Hidden global `malloc`, container implementations inseparable from their storage policy, and test code unable to inject allocation failure or account for a subsystem's memory consumption.

**3. Underlying mechanism & memory layout.** The consumer carries or receives an allocator handle. Allocations dispatch through a statically known policy, function table, or virtual resource. Storage can come from a region, pool, fixed buffer, diagnostic allocator, or upstream heap. Allocation policy changes without changing the logical element type or algorithm.

**4. Engineering trade-offs & footprint.** Type-erased allocators can add dispatch and metadata. Allocator lifetime must cover users of its allocations. Copy, move, swap, and deallocation compatibility need explicit rules. Passing an allocator does not ensure all transitive callees use it. Implicit context is convenient but makes allocation authority less visible than a direct parameter. For hard limits, disable or control fallback rather than merely giving an arena an initial buffer. Allocation-failure behavior belongs in the operation's error and exception policy.

<a id="m13"></a>
### M13. Regions, arenas, and bulk lifetime management

**1. Modern approach & concrete entities.** C++ `std::pmr::monotonic_buffer_resource`; Zig arena allocators; region-scoped parser or request storage; Rust arena libraries behind lifetime-aware APIs. [CPP-ARENA] [ZIG-ARENA] [BUMPALO]

**2. Classical antecedent / historical anti-pattern.** One general-purpose allocation/free pair per short-lived node, fragmentation from interleaved object lifetimes, and complicated individual ownership when everything dies at a known phase boundary.

**3. Underlying mechanism & memory layout.** A region obtains larger blocks and suballocates aligned chunks, frequently by bumping an offset. Individual deallocation may be a no-op; resetting or destroying the region releases storage together. Locality can improve because allocations made near each other are placed near each other, though object-access order remains decisive.

**4. Engineering trade-offs & footprint.** Early-dead values consume capacity until region reclamation. Releasing storage is not automatically the same as running every object's destructor. A C++ container must not retain pointers into a released resource. Rust arena APIs need sound lifetime relationships or suitable handle indirection. Unbounded fallback destroys a claimed fixed-memory budget. Region selection should follow actual lifetime cohorts; using one process-wide arena merely converts forgotten frees into deliberate permanent retention. Account for alignment waste, block headers, retained capacity, and the cost of destruction before reset.

<a id="m14"></a>
### M14. Fixed-size and fixed-capacity inline containers

**1. Modern approach & concrete entities.** C++ `std::array<T,N>` and C++26 `std::inplace_vector<T,N>`; Rust `[T; N]` and `heapless::Vec<T,N>`; Swift `InlineArray`. Fixed size and bounded variable length are different contracts. [CPP-INPLACE] [HEAPLESS] [SWIFT-INLINE]

**2. Classical antecedent / historical anti-pattern.** Heap allocation for a known maximum, a raw array with a separately drifting logical count, and capacity growth inside a hard real-time execution path.

**3. Underlying mechanism & memory layout.** Capacity resides inside the enclosing object. A bounded variable-length container additionally tracks how many elements are initialized. A first approximation is `N * sizeof(T) + count + padding`, but exact layout depends on the implementation. Inline storage resides wherever the owner resides: stack, static memory, heap allocation, or another aggregate.

**4. Engineering trade-offs & footprint.** Unused capacity still consumes memory and can enlarge every task frame or state-machine alternative. Large copies/moves may be expensive. Capacity exhaustion needs a checked return, exception, or documented precondition; silently overwriting is not an acceptable substitute. The container's storage can be allocation-free while its elements allocate. Bounded capacity does not make insertion time constant if shifting elements is required. Choose capacity from an explicit workload/resource bound, not from an average-case measurement alone.

<a id="m15"></a>
### M15. Small-buffer optimization and adaptive inline storage

**1. Modern approach & concrete entities.** LLVM `SmallVector<T,N>`; Rust `smallvec::SmallVec`; implementation-specific short-string optimization; inline capture storage in some owning callable wrappers. [LLVM-CONTAINERS] [SMALLVEC]

**2. Classical antecedent / historical anti-pattern.** A heap allocation for every tiny string, short list, or callback environment, regardless of how often the common case fits in a few words.

**3. Underlying mechanism & memory layout.** The object embeds a small buffer and discriminates between inline and out-of-line storage. Growth beyond inline capacity allocates and moves or copies elements to a larger block. The representation contains enough state to distinguish modes and release the appropriate storage.

**4. Engineering trade-offs & footprint.** Unlike a fixed-capacity vector, this does not promise absence of heap allocation. Larger inline capacity increases the size of every object even when empty. Crossing the threshold produces a latency cliff, and moves of inline values can be more expensive than transferring a heap pointer. Pointer stability differs between modes and operations. Do not assume a particular short-string capacity across libraries or ABIs. Evaluate the distribution of sizes, cache density, move frequency, and threshold crossings; “avoids allocations in the common case” is a workload claim, not a universal guarantee.

<a id="m16"></a>
### M16. Structure of arrays, AoSoA, and hot/cold data separation

**1. Modern approach & concrete entities.** SoA and AoSoA storage; Odin `#soa[N]T` and `#soa[]T`; separate compact hot fields and rarely used cold metadata; archetype-style component columns. [ODIN] [FLECS-DESIGN]

**2. Classical antecedent / historical anti-pattern.** A heap object and vptr per entity in a bulk numerical loop, following pointer chains to fetch one field, or loading a large record when the kernel only needs two scalars.

**3. Underlying mechanism & memory layout.** AoS stores complete records consecutively. SoA stores each field in its own stream. AoSoA groups short SoA blocks to balance vectorized access and entity-local operations. Hot/cold splitting removes rarely accessed data from frequently traversed records. These transformations change which useful bytes arrive in each cache line; they do not depend on banning methods or encapsulation.

**4. Engineering trade-offs & footprint.** Field-wise kernels can gain unit-stride loads, while operations consuming one complete entity may need multiple streams. An apparent element reference can be a proxy rather than `T&`. Insertions/removals must update parallel columns consistently. SIMD width, cache lines, prefetch behavior, and access distribution influence the best blocking. Benefits shrink or reverse when the workload changes. Preserve a logical API over the representation so layout changes remain localized rather than propagating raw field arrays throughout the codebase.

<a id="m17"></a>
### M17. Portable SIMD values and explicit lane masks

**1. Modern approach & concrete entities.** C++26 working-draft `<simd>` facilities, including `std::simd::vec` and mask types; Zig vector types such as `@Vector(N,T)`; typed vector and mask APIs rather than unstructured intrinsic sequences. [CPP-SIMD] [ZIG]

**2. Classical antecedent / historical anti-pattern.** Duplicated scalar and target-specific kernels, unsafe lane reinterpretation, and scattering ISA-specific register types through domain interfaces.

**3. Underlying mechanism & memory layout.** A vector value represents multiple homogeneous lanes, and a mask identifies participating lanes. The compiler lowers operations to native vector instructions, scalar sequences, or combinations appropriate to the target and ABI. Masks may be vector registers, compact predicate registers, or another representation; an array of booleans is not a universal model.

**4. Engineering trade-offs & footprint.** Unsupported widths may split operations. Tail handling, reductions, alignment, gather/scatter, and masking semantics influence generated work. A masked computation must not be assumed to suppress every invalid memory access unless its particular API guarantees that. Floating-point reassociation can change results. Wide SIMD can alter frequency and register pressure on relevant targets. Portable syntax improves maintainability but does not replace disassembly, numerical validation, or target-specific benchmarking; it also does not make an irregular memory-access pattern contiguous.

<a id="m18"></a>
### M18. Pinning and address-sensitive values

**1. Modern approach & concrete entities.** Rust `Pin<P>`, `Pin<&mut T>`, `Pin<Box<T>>`, and `Unpin`; pin-projection libraries for composites. [RUST-PIN]

**2. Classical antecedent / historical anti-pattern.** Moving a self-referential object after its internal pointer is established, relocating a registered intrusive node, and letting an asynchronous operation retain the address of movable storage.

**3. Underlying mechanism & memory layout.** Pinning constrains moving or invalidating the pointee after the pin contract takes effect. It need not add a runtime tag. A pinned owning pointer can itself move while its heap allocation stays at the same address. The useful guarantee depends on the pointee not opting out through `Unpin` and on correct unsafe implementations/projections.

**4. Engineering trade-offs & footprint.** Pinning is not a runtime operation that physically locks bytes in memory, nor is it OS page pinning for DMA. Stack pinning is possible, so heap allocation is not inherent. Projection and destruction invariants are subtle. Pinning does not automatically establish a valid self-reference or extend a borrow. Deleting C++ move constructors can enforce part of an address-stability discipline, but is not a full counterpart of Rust's pinning abstraction. Model whether stability is required for a value, an allocation, a slot, or a hardware mapping; these are different obligations.

<a id="m19"></a>
### M19. Explicit partial initialization and initialization-state tracking

**1. Modern approach & concrete entities.** Rust `MaybeUninit<T>`, checked initialized-length bookkeeping, and `Vec::spare_capacity_mut`; C++ `std::construct_at`, `std::destroy_at`, and uninitialized-memory algorithms. [RUST-MAYBEUNINIT] [CPP-SPECIALMEM]

**2. Classical antecedent / historical anti-pattern.** Treating `malloc` storage as live objects, using `memset(0)` for every type, setting a vector's length before constructing its elements, and leaking partially constructed buffers on failure.

**3. Underlying mechanism & memory layout.** Raw storage and a valid value are represented separately. `MaybeUninit<T>` allows storage without claiming that its bits constitute a valid `T`; construction advances an explicit initialized frontier. A guard destroys only the successfully initialized prefix during failure. C++ similarly distinguishes storage duration from object lifetime and constructor execution.

**4. Engineering trade-offs & footprint.** Converting to `T` before establishing its invariants is unsafe, even when the memory has the right size. Zero bits are invalid for some references, enums, and other types. `MaybeUninit` can inhibit niche optimization in enclosing structures. The bookkeeping and cleanup proof are additional complexity, but permit efficient bulk construction without redundant default initialization. This is an advanced implementation technique to encapsulate behind safe interfaces, not a reason to expose partially initialized objects to ordinary callers.

<a id="m20"></a>
### M20. Interior mutability with explicit enforcement policy

**1. Modern approach & concrete entities.** Rust `Cell<T>`, `RefCell<T>`, `UnsafeCell<T>`, and synchronization wrappers. The shared-access type states that mutation is mediated by a specific mechanism rather than prohibited absolutely. [RUST-CELL]

**2. Classical antecedent / historical anti-pattern.** Casting away constness, modifying shared state through undocumented aliases, and assuming a `const` or shared-reference interface guarantees deep immutability.

**3. Underlying mechanism & memory layout.** `Cell` supports controlled replacement without handing out ordinary simultaneous references to mutable contents. `RefCell` stores borrow state and checks access dynamically. `UnsafeCell` is the primitive allowing mutation behind shared references; safe wrappers establish the missing invariants. These choices differ from locks, which additionally coordinate concurrent access.

**4. Engineering trade-offs & footprint.** Runtime borrowing adds checks and can fail or panic when borrowing rules are violated. Reentrancy through callbacks can trigger conflicts unexpectedly. `UnsafeCell` alone provides no synchronization. Encapsulated caches can preserve logical constness while mutating physical representation, but the distinction must be documented. Choosing interior mutability can simplify graph ergonomics at the price of moving some errors from compilation to execution; it should be a conscious enforcement decision rather than a blanket escape hatch from ownership design.

<a id="m21"></a>
### M21. Thread-transfer and shared-access constraints

**1. Modern approach & concrete entities.** Rust `Send` and `Sync`, `Mutex<T>`, `RwLock<T>`, and atomic types; Swift `Sendable` and isolation-aware concurrency. [RUST-SEND] [RUST-SYNC] [RUST-MUTEX] [SWIFT-SENDABLE]

**2. Classical antecedent / historical anti-pattern.** Passing arbitrary pointers to worker threads, sharing a reference-counted object and assuming its state is synchronized, or relying on a code review to identify every cross-thread capture.

**3. Underlying mechanism & memory layout.** Marker constraints govern whether ownership or shared access may cross thread/isolation boundaries. For Rust, `T: Sync` corresponds to shared references to `T` being transferable safely. Marker traits generally need no runtime fields, while locks and atomics supply actual runtime ordering and coordination. Derivation follows the relevant constituents and language rules; unsafe implementations are part of the trusted boundary.

**4. Engineering trade-offs & footprint.** Locks can block or deadlock, and atomics can contend. Type-level data-race prevention does not establish logical atomicity, a transaction across multiple fields, correct publication order, or bounded blocking. Negative cases such as `Rc<T>` deliberately constrain sharing. A wrapper can make a resource transferable without making every operation safe from every execution context, particularly for GUI, interrupt, device, and affinity-bound APIs. Separate memory safety from scheduling and protocol correctness.

<a id="m22"></a>
### M22. Generational handles and stable logical identity

**1. Modern approach & concrete entities.** Slot maps, generational indices, typed entity keys, and Rust `slotmap::SlotMap`. A handle typically identifies a slot and an incarnation of that slot. [SLOTMAP]

**2. Classical antecedent / historical anti-pattern.** Retaining raw pointers into relocated storage, reusing array indices without detecting stale users, and equating object identity with the current physical address.

**3. Underlying mechanism & memory layout.** Lookup validates `handle.generation == slot.generation` and that the slot is occupied. Reusing a slot increments its generation. Keys can remain meaningful while other storage is compacted or reorganized through an appropriate indirection table. A branded key type can additionally distinguish separate pools.

**4. Engineering trade-offs & footprint.** Lookups require indexing, validation, and sometimes an additional indirection. Slot metadata and free lists occupy memory. Generation counters are finite, so wraparound/reuse policies matter; stale detection is not an unconditional eternal guarantee. Logical identity does not establish address stability or keep an object alive. Handles should not be forged or persisted across restarts without a defined namespace/incarnation policy. This is especially useful for scene graphs, ASTs, device registries, and ECS entities whose identity must outlive temporary references.

<a id="m23"></a>
### M23. Deferred concurrent reclamation: epochs and hazard protection

**1. Modern approach & concrete entities.** Epoch-based reclamation, hazard-pointer schemes, and Rust `crossbeam_epoch::{Atomic, Owned, Shared, Guard}`. These separate removing an object from a concurrent data structure from freeing its storage. [CROSSBEAM-EPOCH] [HAZARD-PAPER]

**2. Classical antecedent / historical anti-pattern.** Calling `free` immediately after an atomic unlink while another reader may still dereference the node, or adding reference counting without examining contention and acquisition races.

**3. Underlying mechanism & memory layout.** Readers announce protected access through an epoch/guard or publish hazardous pointers. Removed nodes enter a retired set. Reclamation waits until the scheme establishes that relevant readers can no longer access them. Per-thread records, retired lists, epochs, and ordering operations are part of the representation.

**4. Engineering trade-offs & footprint.** Slow or stalled participants can delay reclamation, increasing memory consumption. Hazard scanning and epoch advancement add work; algorithmic progress and reclamation progress are separate properties. A guard's scope can become a performance and memory-retention boundary. The data structure still needs a correct publication protocol and an ABA strategy where applicable. An atomic pointer is not a lifetime guarantee. These techniques are specialized alternatives for concurrent structures, not default replacements for simple exclusive ownership and ordinary locks.

<a id="m24"></a>
### M24. Safe interfaces around unsafe layout and provenance operations

**1. Modern approach & concrete entities.** Rust `unsafe` blocks/functions, raw-pointer APIs, `NonNull<T>`, and strict-provenance operations; C++ narrow low-level modules exposing ownership/view types rather than raw representation access. [RUST-UNSAFE] [RUST-PTR] [RUST-NONNULL]

**2. Classical antecedent / historical anti-pattern.** Pervasive pointer arithmetic, integer-to-pointer fabrication, unchecked representation casts, and invariants that every caller must reconstruct independently.

**3. Underlying mechanism & memory layout.** A small trusted implementation establishes alignment, initialization, lifetime, aliasing, and provenance obligations before constructing safe access types. Provenance concerns which allocation/access rights a pointer derives from, not merely its numeric address. An unsafe block does not disable the compiler's semantic assumptions; its author must satisfy them.

**4. Engineering trade-offs & footprint.** The safe wrapper can inline completely, but its proof and test burden remain. `NonNull<T>` only excludes null; it does not prove dereferenceability or initialized storage. Correctness must cover panics, reentrancy, leaks, and malicious implementations of any traits the API trusts. Reinterpretation rules differ across languages and types. Concentrating unsafety makes audits tractable, but a sound-looking signature is not evidence that the implementation has met its obligations. Include sanitizers, fuzzing, and model-appropriate tools around this boundary.

<a id="m25"></a>
### M25. Zero-sized markers and empty-state layout optimization

**1. Modern approach & concrete entities.** Rust unit/empty marker types and `PhantomData`; C++ empty-base optimization and C++20 `[[no_unique_address]]`. [RUST-LAYOUT] [RUST-PHANTOM] [CPP-NOUNIQUE]

**2. Classical antecedent / historical anti-pattern.** Storing runtime booleans or integer tags for a policy known entirely at compile time, or paying per-object storage for stateless strategies solely because they are modeled as objects.

**3. Underlying mechanism & memory layout.** A zero-sized marker participates in type identity and analysis without requiring payload bytes. C++ permits potentially overlapping storage for appropriately marked members; a standalone empty C++ object is not generally zero-sized. Empty deleters or policies can therefore be embedded without necessarily increasing the containing object's size.

**4. Engineering trade-offs & footprint.** Alignment, distinct-address requirements, member combinations, and ABI details still matter. `[[no_unique_address]]` is permission for layout optimization, not a portable formula for every object size. Markers can increase specialization and diagnostic complexity even when runtime storage disappears. In Rust, `PhantomData` influences variance, auto traits, and drop analysis, so changing the marker can alter soundness properties without changing measured size. Treat zero-size as a representation property, not evidence that a type-level design is cognitively free.

<a id="m26"></a>
### M26. Stable-address segmented storage instead of per-node allocation

**1. Modern approach & concrete entities.** C++26 `std::hive<T>` and suitable pool/block containers; stable-address storage with reusable erased slots. This differs from a generational handle, which stabilizes identity rather than necessarily the address. [CPP-HIVE]

**2. Classical antecedent / historical anti-pattern.** Allocating every object separately to avoid vector relocation, or selecting a linked list solely because insertion must not invalidate existing object addresses.

**3. Underlying mechanism & memory layout.** Objects occupy slots in allocated blocks, with metadata to skip holes and reuse erased storage. Block allocation amortizes allocator interactions, while avoiding wholesale relocation on ordinary insertion. Traversal is segmented rather than one globally contiguous array. Invalidation guarantees are operation-specific and must be taken from the container contract.

**4. Engineering trade-offs & footprint.** Holes, block metadata, and unused capacity consume space. The structure is not a random-access contiguous range, and block changes affect traversal predictability. Erased elements cease to exist even if their address is later reused; a raw pointer cannot distinguish incarnations. Compaction or special capacity-management operations may have different guarantees from insertion/erasure. Choose this representation when physical stability and iteration matter together; choose a dense vector or SoA when maximal compactness and bulk scanning dominate instead.

---

<a id="section-2"></a>
## II. Control Flow, Error Handling & Vocabulary Types

This category distinguishes absence, recoverable failure, partial progress, cancellation, contract violation, and non-returning execution. Collapsing all of them into a null pointer, exception, or boolean discards information that downstream code needs.

<a id="c01"></a>
### C01. Explicit absence with optional values

**1. Modern approach & concrete entities.** Rust `Option<T>`; C++ `std::optional<T>`; Swift `T?`; Zig `?T`; V option types. Optionality belongs to the type rather than to an undocumented subset of payload values. [RUST-OPTION] [CPP-OPTIONAL] [V-OPTIONS]

**2. Classical antecedent / historical anti-pattern.** `-1` for a missing index, zero for an absent measurement, an empty string for “not provided,” and a separate validity flag that can disagree with its value.

**3. Underlying mechanism & memory layout.** Semantically, `Option<T>` is `None | Some(T)`. A conventional representation stores payload space and an engagement tag. A niche can encode absence using a representation unavailable to valid `T` values. Rust documents specific null-pointer/nonzero optimizations; the guarantee is not universal for every `T`. C++ optional holds its value within its own storage rather than requiring a separate allocation. [RUST-OPTION] [CPP-OPTIONAL]

**4. Engineering trade-offs & footprint.** Optional payloads can increase size and alignment. The abstraction may still use a physical sentinel internally; what improves is its encapsulated, type-checked meaning. Nested optionals can represent different absence reasons but often deserve named alternatives instead. An optional reference does not own the referent. A present pointer payload can itself be null when the type permits it. Unchecked unwrapping merely relocates the failure risk; absence must be eliminated through a branch, default, propagation, or a proved invariant.

<a id="c02"></a>
### C02. Disjoint success/failure results

**1. Modern approach & concrete entities.** Rust `Result<T,E>`; C++23 `std::expected<T,E>`; Swift `Result<Success,Failure>`; Zig `E!T`; V `!T`; Roc `Try(T,E)` in its current new-compiler tutorial. C3 `T?` represents an optional result carrying a fault rather than simply a nullable payload. [RUST-RESULT] [CPP-EXPECTED] [ROC-TUTORIAL] [C3-OPTIONALS]

**2. Classical antecedent / historical anti-pattern.** Magical status integers, `errno` disconnected from the returned value, output parameters usable after failure, and partially initialized “result” structs without a defined state model.

**3. Underlying mechanism & memory layout.** A disjoint result contains either the success payload or error payload. It normally uses inline union-like storage with a discriminant or available niche. Branching on the outcome selects the valid alternative. Error payloads can be compact enums or richer structures; those structures may themselves own allocations or references.

**4. Engineering trade-offs & footprint.** A large error type enlarges the result even on successful paths. Returning the value may require copying/moving, hidden return storage, or additional registers depending on ABI. Converting lower-level errors can lose detail or allocate context. An explicit result does not prevent unrelated panics, exceptions, or process termination. C++ `expected` does not have `variant`'s `valueless_by_exception` state, but operations can still throw according to the contained types and operation contracts. [CPP-EXPECTED]

<a id="c03"></a>
### C03. Product returns for partial progress and independent outcomes

**1. Modern approach & concrete entities.** Go `(T,error)` and `io.Reader.Read` returning `(n int, err error)`; named structs such as `ReadOutcome { bytes, status }`; Rust or C++ domain structs returning partial data alongside a terminal condition. [GO-IO]

**2. Classical antecedent / historical anti-pattern.** Assuming every operation is all-or-nothing, discarding bytes obtained before EOF, and using one integer both as a byte count and a negative error code without a precise contract.

**3. Underlying mechanism & memory layout.** A product stores both components simultaneously rather than choosing between them. This can intentionally represent `n > 0` together with an error. The return convention may place fields in registers or caller-provided storage. Unlike a disjoint union, a plain product does not exclude meaningless combinations unless construction and validation establish that restriction.

**4. Engineering trade-offs & footprint.** The caller must process the success component before interpreting the terminal condition when the API requires it. Go's typed-nil interface behavior can surprise error-producing code: an interface containing a typed nil pointer is not necessarily a nil interface. Product results can be clearer for streaming and incremental operations, whereas disjoint results better express truly exclusive outcomes. Selecting `Result` merely because it looks more modern can make a partial-progress protocol less accurate. Model actual outcome cardinality before choosing representation. [GO-FAQ]

<a id="c04"></a>
### C04. Structured early-return propagation

**1. Modern approach & concrete entities.** Rust `?`; Zig `try` and `catch`; V result propagation with `!`; C3 postfix `!`; Roc `?`. These are language-specific propagation mechanisms, not identical operators with identical error models. [RUST-TRY] [ZIG] [V-OPTIONS] [C3-OPTIONALS] [ROC-TUTORIAL]

**2. Classical antecedent / historical anti-pattern.** Repeated `if (status != OK) return status;`, deeply nested success branches, and forgetting to release resources on one newly introduced error path.

**3. Underlying mechanism & memory layout.** Propagation commonly lowers to examining an outcome, extracting its success value, or returning the error through the enclosing function's return convention. Language-specific conversions may transform the error. Live local owners are destroyed according to the language's scope rules. No extra persistent result object is inherently required beyond the intermediate values that survive optimization.

**4. Engineering trade-offs & footprint.** Propagation is not recovery, logging, retry, rollback, or compensation. A convenient operator can make a function exit at many points, which matters for state mutation and resource transactions. Error conversion can discard category information. Rust's underlying generalized propagation traits have stability constraints distinct from everyday stable `?` usage; do not assume every customization is portable stable Rust. C++ `expected` supplies a result representation but does not itself add a language-level `?` operator. Make ownership and partially performed effects visible around propagation sites.

<a id="c05"></a>
### C05. Typed throwing and explicit exception boundaries

**1. Modern approach & concrete entities.** Swift `throws(E)` and `throws(Never)`; C++ `noexcept` and explicit exception-handling boundaries; result-returning adapters at FFI or subsystem edges. Typed throwing constrains the error effect rather than requiring the caller to manipulate a result object. [SWIFT-THROWS] [CPP-EXCEPT]

**2. Classical antecedent / historical anti-pattern.** Undocumented exceptions escaping arbitrary layers, treating every failure as an untyped string, and allowing foreign callers to encounter unwinding they cannot handle.

**3. Underlying mechanism & memory layout.** A typed effect records which error type a function can throw. Lowering and ABI are language-specific: Swift error propagation is not mechanically equivalent to C++ exception unwinding. Typical C++ table-driven exception ABIs use unwind metadata and cold-path runtime machinery, but the language does not prescribe a universal implementation. `noexcept` restricts propagation behavior; it is not proof that the body contains no throwing operation.

**4. Engineering trade-offs & footprint.** Unwind tables, exception objects, cleanup paths, and optimizer constraints can affect code size and latency. A failure to respect a non-throwing boundary can terminate execution instead of recovering. Typed errors make handling more precise but can couple layers to large error taxonomies. Exceptions can remain reasonable for rare non-local failures; explicit results can suit expected operational failures. Compare equivalent failure behavior rather than contrasting checked results with unchecked integer returns and calling the extra branch “overhead.”

<a id="c06"></a>
### C06. Monadic mapping, flattening, and error transformation

**1. Modern approach & concrete entities.** Rust `map`, `map_err`, `and_then`, `or_else`, and `transpose`; C++23 optional/expected monadic operations such as `and_then`, `transform`, and `transform_error`; Swift `map` and `flatMap`. [RUST-RESULT-METHODS] [CPP-EXPECTED-MONADIC] [CPP-OPTIONAL-MONADIC]

**2. Classical antecedent / historical anti-pattern.** Repeated unwrap/check/rewrap code, nested `Result<Result<T,E>,E>` values, and branching that hides the actual data transformation.

**3. Underlying mechanism & memory layout.** `map` changes a success value using `T -> U`; `and_then` composes `T -> Result<U,E>` and flattens one result layer. Error-mapping operates on the other alternative. A compiler can inline these short functions into the same branch sequence as explicit control flow. `transpose` can exchange nesting such as `Option<Result<T,E>>` and `Result<Option<T>,E>` without erasing their distinct semantics.

**4. Engineering trade-offs & footprint.** Closures can capture owners, borrow data, allocate, or invoke indirect calls. Fluent syntax does not establish purity or costlessness. Large chains increase type complexity and may make debugger stepping less clear. Short-circuit composition evaluates dependent stages sequentially and normally stops at the first failure; it is not error accumulation. Name intermediate domain operations when a chain hides important side effects or ownership transitions. “Monadic” describes a compositional shape and laws, not necessarily a full higher-kinded type system.

<a id="c07"></a>
### C07. Applicative validation and error accumulation

**1. Modern approach & concrete entities.** Scala Cats `Validated`/`ValidatedNel`, applicative combination of independent validations, or explicit `Validation<T, Errors>` libraries in other languages. This is a library-level technique, not a built-in feature shared by every surveyed language. [CATS-VALIDATED]

**2. Classical antecedent / historical anti-pattern.** Rejecting a configuration after only its first bad field, mutating one shared error list from every validator, or chaining dependent results when the checks are actually independent.

**3. Underlying mechanism & memory layout.** Independent computations each produce a valid value or errors. A combination operation constructs the aggregate when all succeed, otherwise combines errors using a defined associative operation. Unlike monadic sequencing, later validation does not require the successful value of an earlier stage. The error collection can be a fixed-capacity buffer, linked non-empty collection, vector, or another representation.

**4. Engineering trade-offs & footprint.** All relevant checks run rather than stopping early, and error accumulation consumes storage. Validation dependencies limit parallel or applicative composition. Ordering and duplicate handling must be specified. Bounded systems should define what happens when the diagnostic capacity is exhausted. Accumulation is appropriate for forms, schemas, and configurations; it is often inappropriate when a failing operation mutates a device or invalidates assumptions needed by subsequent checks. Keep validation separate from committing external effects.

<a id="c08"></a>
### C08. Unit, bottom, and impossible outcome types

**1. Modern approach & concrete entities.** Rust `()`, `!`, and `Infallible`; Swift `Void` and `Never`; Zig `void` and `noreturn`; C++ `expected<void,E>` and `std::monostate`. [RUST-INFALLIBLE] [RUST-NEVER] [CPP-EXPECTED]

**2. Classical antecedent / historical anti-pattern.** Dummy success integers, fake payload fields in commands, and keeping an error branch in an API even when it cannot be constructed through valid code.

**3. Underlying mechanism & memory layout.** A unit type has one logical value; an uninhabited type has none. `Result<T,Infallible>` cannot produce its error alternative through valid safe construction. A diverging expression can fit control-flow branches without supplying a normal result. Compilers can remove impossible alternatives and their dead code where the language guarantees permit it.

**4. Engineering trade-offs & footprint.** An erased unit value in Rust does not imply every empty C++ object has size zero. Availability of the Rust never type in all possible type positions must be distinguished from established non-returning return-type usage. Marking a branch unreachable is not a substitute for establishing that it cannot occur. Foreign input, unsafe values, or invalid discriminants can violate the assumptions behind the proof. Use impossible types for genuinely impossible outcomes, not failures that are merely rare or currently unimplemented.

<a id="c09"></a>
### C09. Lexically registered cleanup and error-only cleanup

**1. Modern approach & concrete entities.** Go, Swift, Odin, and C3 `defer`; Zig `defer` and `errdefer`; C++ scope-guard libraries or an explicitly implemented guard. Do not assume `std::scope_exit` is universally part of the standard library merely because similarly named experimental/library facilities exist. [GO-DEFER] [ZIG] [SCOPEGUARD]

**2. Classical antecedent / historical anti-pattern.** Scattered cleanup labels, acquisition far from release logic, and duplicated rollback calls on every early return.

**3. Underlying mechanism & memory layout.** A cleanup action is registered and invoked at its defined exit point. Compilers can lower fixed lexical cleanup to direct calls; dynamic registrations may require bookkeeping. Go evaluates deferred call arguments when registering the call and executes defers at function exit. Zig `errdefer` applies to error exit from its scope, not ordinary success.

**4. Engineering trade-offs & footprint.** Scope rules vary across languages. A Go defer inside a long loop can retain resources until the enclosing function returns. Captured variables or pointer references can refer to changed or expired state if the API permits escape. Deferred cleanup is not ownership checking; double cleanup is still possible in a permissive language. Error-only cleanup must be disabled or transferred correctly when ownership is handed off. Check release ordering, nested scopes, and whether cleanup itself can fail or block.

<a id="c10"></a>
### C10. Transactional updates and commit/rollback guards

**1. Modern approach & concrete entities.** Build-then-commit, copy-and-swap where applicable, RAII rollback guards, consuming transaction handles, and domain-specific `commit() -> Result`. C++ exception-safety guarantees provide useful vocabulary: no-throw, strong, and basic guarantees. [CPP-GUIDELINES] [SCOPEGUARD]

**2. Classical antecedent / historical anti-pattern.** Mutating shared state incrementally and leaving it half-updated when allocation, validation, or I/O fails; assuming destructor execution automatically undoes prior effects.

**3. Underlying mechanism & memory layout.** Work occurs in temporary storage or under a rollback record. A successful final step publishes the new state, often through a non-throwing swap or pointer replacement. Before commit, failure destroys provisional resources and preserves the previous visible state. An undo log stores inverse operations when a full copy is unsuitable.

**4. Engineering trade-offs & footprint.** Temporary copies, logs, and retained old state increase peak memory. A commit is only atomic relative to an explicitly defined observer/concurrency model. External effects may not be reversible: sending a message, starting DMA, or writing a register cannot generally be undone by restoring a C++ object. Filesystem or database transactions need their own guarantees. Strong exception safety is therefore a local contract to establish, not a consequence of using `Result`, exceptions, or RAII alone.

<a id="c11"></a>
### C11. Explicit arithmetic and conversion failure policy

**1. Modern approach & concrete entities.** Rust `checked_add`, `wrapping_add`, `saturating_add`, `overflowing_add`, and `TryFrom`; Zig wrapping/saturating arithmetic; domain wrappers distinguishing representable, bounded, and scaled values. [RUST-U32] [RUST-TRYFROM] [ZIG]

**2. Classical antecedent / historical anti-pattern.** Assuming signed overflow wraps, silent narrowing casts, conflating arithmetic overflow with sensor saturation, and performing size multiplication before checking it.

**3. Underlying mechanism & memory layout.** Checked arithmetic returns an optional/result or overflow flag; saturating arithmetic clamps; wrapping arithmetic computes modulo the integer range. Lowering can use carry/overflow flags, comparisons, wide intermediates, or dedicated instructions. Fallible conversion establishes representability in the destination type.

**4. Engineering trade-offs & footprint.** Checks may introduce branches or additional instructions, while saturation may hide invalid upstream calculations. A representable conversion does not validate units or physical meaning. Multiplication/addition order matters for overflow checks, especially for buffer sizes and offsets. Floating-point operations require a different model involving rounding, NaNs, infinities, and precision rather than integer overflow alone. Select the policy per operation; globally choosing wrap or saturate is rarely an adequate domain specification. Compiler build modes must not silently change a critical numeric contract.

<a id="c12"></a>
### C12. Expression-oriented control flow and guarded bindings

**1. Modern approach & concrete entities.** Rust `if`/`match` expressions, `if let`, and `let ... else`; Swift `guard let`; Zig conditional and switch expressions; Nix `if ... then ... else`. [RUST-IF] [RUST-LET] [SWIFT-CONTROL] [NIX-LANGUAGE]

**2. Classical antecedent / historical anti-pattern.** Declaring an uninitialized variable and assigning it in branches, keeping invalid values in scope, and deeply nesting normal execution under defensive checks.

**3. Underlying mechanism & memory layout.** A branch expression computes one value and joins through a compiler-level control-flow merge, commonly represented by an SSA phi or equivalent. A guarded binding narrows the usable state: the success value exists only after the pattern has been established, while the failure branch exits or handles the alternative.

**4. Engineering trade-offs & footprint.** Expression syntax does not remove branch instructions. All producing branches need compatible types, and moving values into one branch can constrain later use. A guard can improve readability but may hide too many early exits in a long function. Dynamically typed Nix performs its checks during evaluation; expression orientation alone is not compile-time type safety. Avoid default-initializing a semantically invalid value merely to satisfy syntax when a value-producing branch can express the actual construction directly.

<a id="c13"></a>
### C13. Lazy ranges, iterator pipelines, and loop fusion

**1. Modern approach & concrete entities.** C++20 ranges/views; Rust `Iterator`; Swift lazy sequences; Go `iter.Seq`/`Seq2` and range-over-function iteration. [CPP-RANGES] [RUST-ITERATOR] [GO-ITER]

**2. Classical antecedent / historical anti-pattern.** Allocating an intermediate collection for every filter/map, duplicated traversal code, and hand-maintained indices that obscure the algorithm's data transformation.

**3. Underlying mechanism & memory layout.** Adaptors contain traversal state and callable environments. Pull iterators produce the next element on demand; Go's sequence interface uses a yield callback. Specialization and inlining can fuse several stages into one loop. A lazy view need not materialize its output, although a terminal `collect` or equivalent does.

**4. Engineering trade-offs & footprint.** Laziness defers work and failure, and repeated traversal may repeat computation. Captured references can dangle in languages without appropriate lifetime checking. Not every C++ view is non-owning, and `borrowed_range` is not a borrow checker. Iterator-category requirements affect algorithm complexity. Closure state and nested adaptor types can increase compilation time and code size. Fusion is an optimization opportunity, not a contractual guarantee that every pipeline beats a direct loop. Preserve a measurable relationship to the expected access pattern.

<a id="c14"></a>
### C14. Typed early termination in higher-order algorithms

**1. Modern approach & concrete entities.** Rust `ControlFlow<Break,Continue>`, `Iterator::try_fold`, and `try_for_each`; explicit stop/continue enums in callback-driven traversals. [RUST-CONTROLFLOW] [RUST-ITERATOR]

**2. Classical antecedent / historical anti-pattern.** Returning `false` to mean either success, failure, or “stop searching”; throwing an exception solely to escape a callback; and storing a discovered value in an outer mutable variable.

**3. Underlying mechanism & memory layout.** The callback returns a typed control result, optionally carrying the break value. The traversal inspects the discriminant and either continues or returns the payload. Static generic callbacks can inline this into the same control flow as an ordinary loop with `break value` behavior.

**4. Engineering trade-offs & footprint.** The traversal and callback must agree whether stopping is normal completion, a found result, cancellation, or failure. A break payload can enlarge temporary values. Short-circuit behavior changes which side effects execute and how much input is consumed, which is important for streams and parsers. This technique avoids conflating algorithm control with exceptional failure, but it does not establish that a partially consumed iterator can safely be retried or reused without an explicit contract.

<a id="c15"></a>
### C15. Type-checked formatting and output construction

**1. Modern approach & concrete entities.** C++ `std::format_string<Args...>`, `format`, and `format_to`; Rust `format_args!`, `write!`, and formatting traits. [CPP-FORMAT] [RUST-FORMAT]

**2. Classical antecedent / historical anti-pattern.** `printf` argument/format mismatches, buffer overflow through unbounded formatting, and converting every intermediate value to a temporary string before writing it.

**3. Underlying mechanism & memory layout.** Static format strings can be checked against argument types during translation. A formatting argument package can borrow values and route them to a sink, avoiding construction of one combined intermediate string. The formatter still executes numeric conversion, padding, escaping, or user-defined formatting logic at runtime.

**4. Engineering trade-offs & footprint.** Producing an owning result string can allocate. `format_to` is not automatically bounded: the output iterator/sink must enforce capacity when necessary. Runtime format strings need the API's runtime-validation path. User-defined formatters can perform expensive work or fail. Borrowed argument objects have lifetime restrictions. Binary size may grow from instantiated formatters or Unicode/locale support. Type checking solves argument consistency; bounded memory use, log truncation policy, and blocking I/O remain separate requirements.

<a id="c16"></a>
### C16. Must-use results and deliberate discard

**1. Modern approach & concrete entities.** Rust `#[must_use]`, the must-use behavior of result/future types, and C++ `[[nodiscard]]` with diagnostic messages. [RUST-DIAGNOSTICS] [CPP-NODISCARD]

**2. Classical antecedent / historical anti-pattern.** Ignoring return codes accidentally, constructing a lazy future and never polling it, and discarding a guard immediately while believing it protects a later region.

**3. Underlying mechanism & memory layout.** The compiler diagnoses certain discarded expressions according to language and diagnostic rules. No per-value runtime tracking is necessary. Projects can promote relevant warnings to errors while allowing explicit, reviewable discard where appropriate.

**4. Engineering trade-offs & footprint.** Diagnostics are not a proof of meaningful handling: assigning to an unused variable or explicitly discarding can satisfy the mechanism. Warning policies differ, and not all ways of losing a value are diagnosed. Excessive annotation can create noise that users suppress indiscriminately. The goal is to make accidental loss conspicuous, especially for failure-bearing values and lazy operations. Pair this with useful error types and review rules; a must-use integer still lacks a clear domain contract, and a nodiscard view can still dangle.

<a id="c17"></a>
### C17. Coroutines and compiler-generated asynchronous state machines

**1. Modern approach & concrete entities.** C++20 `co_await`, `co_yield`, promise types, and `coroutine_handle`; Rust `async`, `Future`, and `Poll`; Swift `async`/`await`. [CPP-COROUTINE] [RUST-FUTURE] [SWIFT-CONCURRENCY]

**2. Classical antecedent / historical anti-pattern.** Callback pyramids, manually numbered resume states, and allocating an OS thread for every pending I/O operation.

**3. Underlying mechanism & memory layout.** Suspension points become states, and locals that remain live across suspension become fields in a frame/future. Resumption dispatches from that state. Nested Rust futures can be stored inline; some address-sensitive futures need pinning before polling. C++ supplies frame allocation machinery and permits allocation elision in specified circumstances. Executors, not the `await` keyword alone, determine scheduling.

**4. Engineering trade-offs & footprint.** Large live locals enlarge the frame; spawning or boxing may allocate. A borrowed object must remain valid across suspension. Awaiting does not necessarily yield, and blocking inside asynchronous code can stall its executor. Destroying a suspended computation must follow its language/library lifecycle rules. Cancellation is not rollback, and a dropped future may have already initiated external work. State-machine syntax simplifies control structure while making frame size, capture ownership, and suspension boundaries central design concerns.

<a id="c18"></a>
### C18. Structured concurrency and scoped task lifetimes

**1. Modern approach & concrete entities.** Swift `async let` and task groups; Rust `std::thread::scope` for scoped threads; libraries that bind child tasks to a parent scope. [SWIFT-CONCURRENCY] [RUST-SCOPE]

**2. Classical antecedent / historical anti-pattern.** Detached workers outliving borrowed inputs, untracked background tasks, and shutdown implemented through sleeps and global flags.

**3. Underlying mechanism & memory layout.** The enclosing scope owns or coordinates child execution and does not finish in the normal structured fashion until required child completion is resolved. Scoped threads can borrow suitable local data because the scope bounds their lifetime. Task-group state tracks children, completions, and cancellation relationships; the exact representation depends on the runtime.

**4. Engineering trade-offs & footprint.** Joining children can delay exit indefinitely if they do not terminate. Cancellation is commonly cooperative and must be observed by the work. A task scope establishes lifetime structure, not hard deadlines, transactional behavior, or deterministic scheduling. Scoped OS threads still have stacks and kernel/runtime costs. A cancellation token alone does not make detached task launching structured. Identify the scope that owns each task, its shutdown responsibility, and how errors from multiple children are collected or prioritized.

<a id="c19"></a>
### C19. Explicit cancellation, deadlines, and bounded backpressure

**1. Modern approach & concrete entities.** Go `context.Context`; cancellation-safe asynchronous operations; bounded channels such as Tokio `mpsc`; C++ stop-token facilities. [GO-CONTEXT] [TOKIO-CHANNELS] [TOKIO-SELECT] [CPP-STOP]

**2. Classical antecedent / historical anti-pattern.** Global “stop” booleans, infinite queues absorbing overload, timeouts checked only after work completes, and abandoning an operation while it still accesses freed memory.

**3. Underlying mechanism & memory layout.** Cancellation state is shared through a token/context and observed at defined points. Deadlines connect time to that state. A bounded queue limits in-flight messages; producers wait, fail, or apply an explicit drop policy when capacity is exhausted. Cancellation safety describes whether interrupting and retrying an operation loses or duplicates progress.

**4. Engineering trade-offs & footprint.** Tokens, callbacks, timers, waiters, and queues consume storage and synchronization work. A deadline notification does not prove the operation has ceased. Selecting between futures can drop losing branches, so their cancellation contracts matter. Backpressure can propagate latency or deadlock if a producer holds a resource the consumer needs. Distinguish queue capacity, task count, payload ownership, and external in-flight I/O; bounding only one does not bound the entire subsystem.

<a id="c20"></a>
### C20. Typed asynchronous composition with senders and receivers

**1. Modern approach & concrete entities.** C++26 `std::execution` sender/receiver facilities: `just`, `then`, `when_all`, `connect`, `start`, schedulers, and completion channels `set_value`, `set_error`, and `set_stopped`. [CPP-EXEC]

**2. Classical antecedent / historical anti-pattern.** Untyped callback/context registries, opaque scheduler placement, and one virtual interface or allocation per asynchronous composition stage.

**3. Underlying mechanism & memory layout.** A sender describes a computation. Connecting it to a receiver constructs an operation state; starting initiates execution. Completion signatures carry possible value/error/stopped outcomes. Template composition can embed child operation states directly and resolve many connections statically, rather than requiring every stage to be separately heap-allocated.

**4. Engineering trade-offs & footprint.** Runtime schedulers and particular adaptors can still allocate or dispatch dynamically. Operation-state lifetime must cover the asynchronous work that uses it. Large composed types increase compilation and diagnostic complexity. Stopped completion is not the same as an application error or successful rollback. Execution placement, cancellation forwarding, and completion ordering remain explicit design questions. This is a standardization direction represented in the C++26 draft; verify implementation support rather than assuming a C++26 language switch enables the complete library on every toolchain.

<a id="c21"></a>
### C21. Actor isolation and serialized access to mutable state

**1. Modern approach & concrete entities.** Swift `actor`, `@MainActor`, isolated parameters, and `Sendable` boundaries; actor-style services implemented with owned state plus typed mailboxes in other languages. [SWIFT-ACTORS] [SWIFT-SENDABLE] [TOKIO-CHANNELS]

**2. Classical antecedent / historical anti-pattern.** Shared mutable objects with locks added individually, arbitrary callbacks accessing a service's internals, and assuming that all methods of a class run on one thread.

**3. Underlying mechanism & memory layout.** Actor-isolated state is accessed through an execution domain that serializes relevant work. Cross-boundary interactions may enqueue jobs and suspend callers. State resides with the actor; messages/jobs carry values or permitted references. Isolation is a semantic restriction, not necessarily a permanent dedicated OS thread.

**4. Engineering trade-offs & footprint.** Mailboxes, scheduling, and synchronization introduce overhead. Swift actor methods can be reentrant across `await`: another operation may change state before the suspended method resumes. Serialization of individual segments is not atomicity of an entire multi-await protocol. Slow work can create queueing delays, and unbounded mailboxes can exhaust memory. Invariant checks often need repeating after suspension. Actors reduce accidental shared mutation but do not eliminate deadlock-like dependency cycles, starvation, or application-level races.

<a id="c22"></a>
### C22. Effect tracking and algebraic effect handlers

**1. Modern approach & concrete entities.** Koka effect rows and handlers; Roc's documented distinction between pure `->` and effectful `=>` function arrows. Roc's distinction should not be presented as equivalent to a general algebraic effect-handler implementation. [KOKA] [ROC-TUTORIAL]

**2. Classical antecedent / historical anti-pattern.** Signatures that conceal I/O, mutation, suspension, or failure; implicit global dependencies; and using exceptions as the only extensible non-local control mechanism.

**3. Underlying mechanism & memory layout.** The type system tracks which operations a computation may perform. Effect-polymorphic code can preserve the effects of a supplied computation. A handler provides an interpretation and may resume a continuation. Implementations can use evidence passing, continuation transformations, specialization, or runtime continuation objects; annotation erasure does not imply all handling operations erase.

**4. Engineering trade-offs & footprint.** Continuations may allocate or retain substantial state, and multi-shot handling complicates resource ownership. Effect inference and handler nesting can make diagnostics difficult. The tracked set is only as expressive as the language's effect model. A function can be effectful and still return an explicit `Result`: operational effects and failure representation are separate dimensions. Effect systems can expose hidden behavior, but they do not automatically prove termination, bounded resource usage, or correct interaction with foreign operations.

<a id="c23"></a>
### C23. Executable interface contracts

**1. Modern approach & concrete entities.** C++26 `pre`, `post`, and `contract_assert`; C3 contract annotations; runtime assertions tied to precise interface predicates. [CPP-CONTRACTS] [C3-CONTRACTS]

**2. Classical antecedent / historical anti-pattern.** Preconditions buried in comments, duplicated assertions with unclear responsibility, and implicit assumptions about valid indices, shapes, or initialized state.

**3. Underlying mechanism & memory layout.** Predicates are associated with calls, returns, or implementation points. Depending on the applicable evaluation semantics, execution can check them and invoke violation handling. Contracts need not add per-object storage; they add evaluation and failure-handling paths when enabled. They are distinct from refinement types and machine-checked proofs.

**4. Engineering trade-offs & footprint.** Predicates can be expensive and may run in different modes. An ignored or non-terminating check does not establish that the condition holds for subsequent execution. Keep predicates free from essential side effects. Malformed external input normally requires ordinary rejection, not reliance on a contract-violation mode. Postconditions involving external state can race or become stale. Contracts make obligations explicit and tool-visible; whether they enforce, diagnose, or merely document a condition must be resolved for the selected language and build configuration.

<a id="c24"></a>
### C24. One-time initialization and synchronized lazy values

**1. Modern approach & concrete entities.** Rust `OnceLock<T>` and `LazyLock<T>`; C++ function-local static initialization and `std::call_once`; Go `sync.Once` and related helpers. [RUST-ONCE] [CPP-ONCE] [GO-ONCE]

**2. Classical antecedent / historical anti-pattern.** Unsynchronized “if not initialized” flags, unsafe double-checked locking, global initialization-order coupling, and repeated expensive computation of an immutable shared value.

**3. Underlying mechanism & memory layout.** A state machine tracks uninitialized, initializing, and initialized conditions. Synchronization publishes the fully initialized value before other threads obtain it. Fast paths typically inspect completion state; losing initializers wait or observe the winner. Failure/retry and poisoning behavior differ among APIs.

**4. Engineering trade-offs & footprint.** First use can allocate, block, or perform expensive initialization, creating a latency spike. Recursive initialization may deadlock or otherwise fail according to the API. The first accessing thread can inherit unexpected work or affinity requirements. A cached value can retain memory for the process lifetime and may be unsuitable when configuration must change. One-time initialization establishes publication safety, not perpetual validity of external facts captured during initialization. Prefer explicit startup initialization where predictability matters more than deferred work.

---

<a id="section-3"></a>
## III. Polymorphism, Genericity & Type Systems

Genericity, conformance, dispatch, ownership, and separate compilation are different axes. A trait can support static or dynamic dispatch. An interface can be structural without being allocation-free. A generic implementation can specialize, share code with dictionaries, or combine both strategies.

<a id="p01"></a>
### P01. Constraint-based generic programming

**1. Modern approach & concrete entities.** C++20 concepts and `requires`; Rust trait bounds and `where`; Swift protocol constraints; Go generic constraint interfaces with type sets; C3 constrained generic modules. [CPP-CONSTRAINTS] [RUST-BOUNDS] [GO-SPEC] [C3-GENERICS]

**2. Classical antecedent / historical anti-pattern.** Unconstrained templates failing deep in implementation details, `void*` containers with unchecked casts, and inheritance introduced solely to make an algorithm accept several unrelated types.

**3. Underlying mechanism & memory layout.** The compiler checks that a substitution satisfies required expressions, types, constants, or relationships. The constraint itself needs no object field or vptr. Actual machine code depends on instantiation and dispatch strategy. Constraints can participate in overload selection, allowing a more specialized algorithm when stronger properties are available.

**4. Engineering trade-offs & footprint.** Syntactic satisfaction is not proof of semantic laws: a callable comparator can violate strict weak ordering. Constraint normalization, overload resolution, and recursive obligations add compile-time work. Excessively detailed constraints can expose implementation choices and make errors worse rather than better. Go type-set unions constrain admissible types; they are not runtime tagged unions. Design constraints around stable algorithm requirements, and document non-checkable laws separately. Measure code generation rather than inferring zero overhead from the mere presence of a concept or trait bound.

<a id="p02"></a>
### P02. Monomorphization and static specialization

**1. Modern approach & concrete entities.** C++ template instantiation; Rust monomorphization; Zig generic functions driven by compile-time type/value arguments. [CPP-INSTANTIATION] [RUST-MONOMORPH] [ZIG]

**2. Classical antecedent / historical anti-pattern.** A universal `void*` implementation with runtime size/operation callbacks, or a virtual base interface used for behavior whose concrete implementation is known at every call site.

**3. Underlying mechanism & memory layout.** The compiler generates or selects concrete instances for relevant substitutions. Known sizes, alignments, operations, and constants enable inlining, constant propagation, dead-branch elimination, and specialized layout. A generic type's fields are laid out using its actual type arguments. Linkers and optimizers may merge or discard duplicate code, but this is not guaranteed for every distinct instantiation.

**4. Engineering trade-offs & footprint.** Many type/policy combinations can inflate compilation time, debug information, object files, and instruction-cache footprint. More specialization can reduce data-path work while increasing front-end fetch pressure. Cross-module optimization may require visible definitions or link-time optimization. Large generic functions can benefit from a small specialized wrapper around a shared non-generic core. Static dispatch is attractive in tight loops, but a dynamically dispatched boundary outside the loop can be smaller and equally adequate for the workload.

<a id="p03"></a>
### P03. Dictionary passing, witness tables, and representation sharing

**1. Modern approach & concrete entities.** Go's documented shape-sharing/dictionary compiler design; Swift unspecialized generic calls carrying metadata and protocol witnesses; type-class dictionaries in functional-language implementations. [GO-DICTIONARIES] [SWIFT-CALLING]

**2. Classical antecedent / historical anti-pattern.** The false dichotomy that generic code must either be a fully duplicated template instance or a class hierarchy with one virtual method table per object.

**3. Underlying mechanism & memory layout.** A shared implementation receives evidence describing type-dependent operations and representation. A dictionary or witness table supplies methods or metadata that cannot be fixed in the shared body. Shape-based compilation shares code among suitable representation classes while specializing other aspects. Optimization can propagate known evidence and remove some indirect calls.

**4. Engineering trade-offs & footprint.** Shared code reduces duplication but can retain indirect dispatch, metadata loads, and abstraction barriers. Dictionaries themselves occupy code/data space, though not necessarily in every user object. Implementation designs evolve; the Go design document is evidence of a compiler strategy, not a permanent language ABI guarantee. Swift resilient generic code and whole-module-specialized code can have different costs despite the same source signature. Inspect the actual calling convention and generated implementation for the target build before classifying an API as static or dynamic in performance terms.

<a id="p04"></a>
### P04. Traits, protocols, and coherent external conformance

**1. Modern approach & concrete entities.** Rust `trait`/`impl`, blanket implementations, supertraits, and coherence/orphan rules; Swift protocol conformance and protocol extensions; Haskell type classes as a related model. [RUST-TRAITS] [RUST-IMPLS] [SWIFT-PROTOCOLS]

**2. Classical antecedent / historical anti-pattern.** Requiring a type's storage declaration to inherit from every behavioral interface, adapter subclasses merely to add a capability, and globally conflicting implementations of the same abstraction.

**3. Underlying mechanism & memory layout.** Behavioral conformance can be declared separately from the record's stored fields. Static calls resolve an implementation; erased calls use appropriate runtime evidence. Coherence rules restrict overlapping implementations so an operation has a sufficiently unambiguous interpretation. Supertraits describe additional obligations, not inherited object storage.

**4. Engineering trade-offs & footprint.** External-conformance restrictions can force a newtype adapter around a foreign type. Blanket implementations can block later specializations or create compatibility constraints. Adding default methods or conformances can alter lookup/dispatch behavior. Traits and protocols do not automatically enforce behavioral substitutability, thread safety, or mathematical laws. The useful replacement for inheritance here is separating data representation from declared capabilities, while retaining deliberate rules about who may define an implementation and how it is selected.

<a id="p05"></a>
### P05. Structural conformance versus nominal type identity

**1. Modern approach & concrete entities.** Go method-set-based interface satisfaction; V interfaces; Roc structural records/tag unions and nominal declarations; nominal Rust traits/types and Swift declared protocol conformance. [GO-INTERFACES] [V-INTERFACES] [ROC-TUTORIAL] [RUST-TRAITS]

**2. Classical antecedent / historical anti-pattern.** A mandatory inheritance declaration for simple compatibility, or runtime duck typing that discovers a missing operation only after the program reaches that path.

**3. Underlying mechanism & memory layout.** Structural conformance compares available operations or shape; nominal identity depends on declared type identity or conformance. Structural record extension and row-like constraints can abstract over selected fields while allowing others. Neither mechanism by itself requires objects to have the same physical layout or dispatch representation.

**4. Engineering trade-offs & footprint.** Structural matching decouples producers from interfaces but can create accidental compatibility between semantically different operations. Nominal distinctions require adapters and declarations but prevent some category mistakes. Method sets, pointer/value receivers, and visibility rules matter in Go. Nix attribute-set parameter destructuring is dynamically checked and is not a static structural type system. Choose nominal wrappers when identity carries domain meaning, and structural capability matching when independent types legitimately provide the same operation contract.

<a id="p06"></a>
### P06. Policy composition and statically injected strategies

**1. Modern approach & concrete entities.** C++ policy classes, constrained function objects, and template strategy parameters; Rust generic strategy types; Odin composition/member exposure with `using`. [CPP-CONSTRAINTS] [CPP-NOUNIQUE] [ODIN]

**2. Classical antecedent / historical anti-pattern.** Deep inheritance combinations for logging, allocation, locking, encoding, or error policy; global hooks replaced at runtime despite every build using one fixed strategy.

**3. Underlying mechanism & memory layout.** A composed object contains stateful policies as fields and represents stateless policies through type parameters or empty members. Calls resolve directly where the concrete strategy is known. Different concerns can be selected independently instead of creating a superclass for each combination.

**4. Engineering trade-offs & footprint.** Every combination can produce a new type and specialization. Public policy parameters increase API surface and error-message complexity. Stateful policies enlarge the object; stateless ones can still create code bloat. Runtime replacement becomes difficult without adding erasure or a sum. Some policies interact semantically—for example, a non-thread-safe allocator and a cross-thread ownership policy—so mere composability is not compatibility. Use static strategies when selection is genuinely static; preserve dynamic dispatch at authentic plugin or runtime configuration boundaries.

<a id="p07"></a>
### P07. Explicit receiver parameters and cv/ref-generic methods

**1. Modern approach & concrete entities.** C++23 explicit object parameters, including `this auto&& self`; Rust's explicit ownership-oriented receivers such as `self`, `&self`, and `&mut self`; receiver-aware generic helper functions. [CPP-FUNCTION] [RUST-ASSOCIATED]

**2. Classical antecedent / historical anti-pattern.** Duplicating four const/lvalue/rvalue-qualified accessors, using CRTP solely to recover the derived receiver type, and returning references from temporary owners accidentally.

**3. Underlying mechanism & memory layout.** The receiver becomes an explicit parameter whose type/category participates in deduction. One definition can select forwarding behavior for different receiver qualifications. The syntax need not add a vptr or change object layout. In Rust, receiver form also communicates borrowing or consumption requirements that participate in ownership checking.

**4. Engineering trade-offs & footprint.** Perfect forwarding can propagate a dangerous reference from a temporary if the API does not restrict its lifetime. Generic receiver methods may generate several instances and complicate overload resolution. Explicit object parameters replace some CRTP uses, not every mixin, customization, or recursive static-polymorphism pattern. C++ receiver syntax does not add Rust-style ownership enforcement. Decide whether an accessor should return a borrow, a copied value, or a consumed member for each receiver category rather than assuming universal forwarding is always correct.

<a id="p08"></a>
### P08. Existential packaging and explicit dynamic dispatch

**1. Modern approach & concrete entities.** Rust `&dyn Trait` and `Box<dyn Trait>`; Swift `any P`; Go interface values; C3 `any` and dynamic interfaces. [RUST-DYN] [SWIFT-LAYOUT] [GO-REFLECTION] [C3-INTERFACES]

**2. Classical antecedent / historical anti-pattern.** Exposing a class hierarchy's storage/inheritance decisions merely to call a few operations, or passing `void*` and reconstructing its type through unchecked casts.

**3. Underlying mechanism & memory layout.** An existential hides a concrete type while packaging the operations/evidence needed to use it. Rust trait-object pointers pair access to data with vtable metadata. Swift general opaque existentials use value storage plus metadata and protocol witnesses, with out-of-line storage when needed. Go interfaces carry dynamic type and value information. C3's documented lookup mechanism must not simply be assigned Rust's vtable layout.

**4. Engineering trade-offs & footprint.** Calls can involve indirect branches and inhibit inlining. Inline existential buffers enlarge containers; boxed values allocate and add pointer chasing. Borrowed trait objects can avoid allocation. Dynamic dispatch supports heterogeneous collections, runtime extensions, and smaller shared code. Object-safety/dyn-compatibility restrictions determine which traits can be erased. The relevant question is where to place an erased boundary, not whether a modern program can eliminate every vtable without losing useful extensibility.

<a id="p09"></a>
### P09. Opaque concrete return types

**1. Modern approach & concrete entities.** Rust return-position `impl Trait`; Swift `some P`; C++ deduced `auto` return types as a related but different facility. [RUST-IMPLTRAIT] [SWIFT-OPAQUE]

**2. Classical antecedent / historical anti-pattern.** Publishing enormous iterator/closure types in interfaces, boxing a value solely to hide its spelling, or introducing an abstract base for one statically fixed implementation.

**3. Underlying mechanism & memory layout.** The producer chooses a concrete type while clients use a constrained abstract view of its interface. The concrete identity remains fixed for the applicable declaration/instantiation and can support specialization and inline storage. This differs from an existential value that may contain different concrete types at runtime.

**4. Engineering trade-offs & footprint.** Multiple return branches generally need one concrete representation, an explicit sum, or erasure. Hidden spelling does not imply stable binary layout or an ABI boundary. Generic and lifetime captures can constrain callers even when the underlying type is unnamed. C++ return-type deduction often requires definition visibility and does not reproduce every Swift opaque-type guarantee. Opaque types are effective for implementation-heavy pipelines whose identity is fixed; they are not substitutes for runtime plugin selection.

<a id="p10"></a>
### P10. Owning and borrowing type-erased callables

**1. Modern approach & concrete entities.** C++ `std::function`, C++23 `std::move_only_function`, and C++26 `std::function_ref`; Rust borrowed or boxed `dyn Fn`-family objects; application-specific erased operation tables. [CPP-FUNCTION-ERASURE] [CPP-MOVEFUNCTION] [CPP-FUNCTIONREF]

**2. Classical antecedent / historical anti-pattern.** A raw callback plus `void*` whose retention policy is undocumented, handwritten trampoline subclasses, and forcing every callback environment to be copyable.

**3. Underlying mechanism & memory layout.** Erasure combines callable state with an invocation thunk and, for owning wrappers, appropriate movement/copy/destruction operations. Small captures may fit inline; larger ones can allocate. A borrowing callable reference stores access to someone else's callable and an invocation mechanism rather than owning its environment.

**4. Engineering trade-offs & footprint.** Dynamic invocation can prevent inlining. Owning wrappers may allocate and enlarge objects; borrowing wrappers can dangle. Move-only erasure accommodates uniquely owned captures without making them copyable. Signature qualifiers, `noexcept`, empty-state behavior, and reference returns affect correctness. A callback accepted for immediate invocation is a different contract from a callback retained asynchronously. Select the wrapper accordingly instead of using an owning general-purpose function object everywhere or treating a non-owning wrapper as a free drop-in replacement.

<a id="p11"></a>
### P11. Closures with explicit capture and invocation capabilities

**1. Modern approach & concrete entities.** C++ lambdas with value/reference/init captures; Rust closure traits `Fn`, `FnMut`, and `FnOnce`; Swift capture lists and non-escaping/escaping closure distinctions. [RUST-CLOSURES] [CPP-LAMBDA] [SWIFT-CLOSURES]

**2. Classical antecedent / historical anti-pattern.** Global variables supplying callback context, unsafe cast-based environment records, and callback APIs unable to express that invoking an operation consumes its captured resource.

**3. Underlying mechanism & memory layout.** A closure is conceptually generated code plus a capture environment. A statically known closure has a concrete compiler-generated type, and direct calls can inline. Rust's call traits distinguish invocation through shared access, mutable access, or consumption; the actual body determines which capabilities it supports. Capture-by-value is not necessarily a deep copy of referenced objects.

**4. Engineering trade-offs & footprint.** Capturing a large object increases closure/frame size. Capturing a shared owner incurs its lifetime/accounting cost, while capturing a reference introduces an outlives obligation. Converting a closure to an erased wrapper adds a separate layer of cost. A Rust `move` closure is not automatically only `FnOnce`; what the body does with captured values matters. Avoid accidental retention through broad capture defaults, especially when callbacks are registered with long-lived services or asynchronous runtimes.

<a id="p12"></a>
### P12. Associated types and generic associated type families

**1. Modern approach & concrete entities.** Rust `Iterator::Item` and generic associated types such as `type Item<'a>`; Swift `associatedtype`; C++ dependent nested types and constrained type relationships. [RUST-ASSOCIATED] [SWIFT-ASSOCIATED]

**2. Classical antecedent / historical anti-pattern.** Returning a universal base type and downcasting each element, or passing several unrelated generic parameters that can be combined inconsistently.

**3. Underlying mechanism & memory layout.** An implementation chooses types logically associated with its capability. A type family can additionally depend on a lifetime or other parameter. A lending iterator may return an item borrowing from its current state, expressed as a family indexed by the borrow. Static specialization resolves these projections without storing a runtime type object in every value.

**4. Engineering trade-offs & footprint.** Associated-type equalities and lifetime families increase constraint complexity. Lending APIs can restrict retaining one item while requesting the next. Existential use may require fixing associated types or may be unavailable for some designs. Generic associated types do not amount to unrestricted higher-kinded polymorphism. Use an associated type when the implementation chooses one coherent result family; use a separate generic parameter when callers legitimately choose among independent types. That distinction improves both API meaning and inference.

<a id="p13"></a>
### P13. Higher-ranked bounds, variance, and safe substitution

**1. Modern approach & concrete entities.** Rust `for<'a>` higher-ranked trait bounds, lifetime subtyping, and covariance/contravariance/invariance; analogous rank-polymorphic interfaces in functional languages. [RUST-BOUNDS] [RUST-VARIANCE]

**2. Classical antecedent / historical anti-pattern.** Assuming that a callback valid for one borrowed lifetime is valid for every lifetime, or allowing writable containers to substitute covariantly and thereby accept invalid writes.

**3. Underlying mechanism & memory layout.** A higher-ranked bound quantifies over lifetimes at the use site, for example requiring a callable to accept a borrow with any suitable lifetime. Variance describes how a composite type's substitutability depends on its parameters. Mutable reference relationships are constrained to prevent writing a shorter-lived value into storage expected to remain valid longer. These rules generally affect compilation, not object fields.

**4. Engineering trade-offs & footprint.** The notation is demanding and diagnostics can be abstract. Incorrect variance in unsafe wrapper design can make a safe-looking API unsound. An HRTB does not mean a captured environment lives forever; it expresses the callable's accepted relationships. Overly invariant types can unnecessarily restrict ergonomics. These mechanisms are particularly relevant when designing callback registries, scoped resource APIs, and lifetime-branded handles, rather than something every ordinary business function should expose directly.

<a id="p14"></a>
### P14. Values in types: const generics and static shapes

**1. Modern approach & concrete entities.** Rust const generics and `[T;N]`; C++ non-type template parameters, static `span` extents, and `std::array`; Swift `InlineArray`; Zig `comptime` parameters. [RUST-GENERICS] [CPP-TEMPLATE-PARAMETERS] [SWIFT-INLINE]

**2. Classical antecedent / historical anti-pattern.** Passing dimensions separately, discovering incompatible shapes after deployment, and using preprocessor-generated families of fixed-size functions.

**3. Underlying mechanism & memory layout.** Compile-time values participate in type identity and constraint selection. Statically known sizes can eliminate stored dimensions and enable unrolling, fixed-offset addressing, or specialized algorithms. Different dimensions may denote different types even when their runtime payloads have similar roles.

**4. Engineering trade-offs & footprint.** Each parameterization can create more code and debug/type information. A runtime-selected dimension cannot be made compile-time merely by wrapping it in a generic API; a bridge to dynamic shapes or finite dispatch is required. Const generics do not imply arbitrary dependent proofs about every dimension expression. Large static storage can inflate coroutine frames or stack usage. Use type-level values where incompatibility should prevent construction and where specialization pays for itself, not for every incidental configuration integer.

<a id="p15"></a>
### P15. Typed parameter packs and heterogeneous variadics

**1. Modern approach & concrete entities.** C++ parameter packs, pack expansion, and fold expressions; Swift parameter packs with `each` and `repeat`; fixed-arity generic families generated through typed metaprogramming. [CPP-PACKS] [SWIFT-PACKS]

**2. Classical antecedent / historical anti-pattern.** C varargs losing type information, sentinel-terminated argument lists, duplicated overloads for many arities, and unchecked runtime interpretation of heterogeneous argument buffers.

**3. Underlying mechanism & memory layout.** The compiler retains the type of each argument and expands operations over the pack. A pack is a compile-time sequence, not automatically a heap container or runtime array of variants. A tuple can store its elements inline with their individual alignment requirements; a fold combines expressions according to its defined operator structure.

**4. Engineering trade-offs & footprint.** Large packs can produce long instantiation chains, diagnostic explosions, and large generated functions. Evaluation order and forwarding/lifetime rules require attention. Static arity is not a replacement for genuinely runtime-sized heterogeneous input. Rust formatting macros preserve type checking through expansion, but should not be described as a general variadic-generics facility. Constrain pack relationships explicitly, including equal lengths or compatible element operations, rather than relying on obscure errors during expansion.

<a id="p16"></a>
### P16. Compile-time execution and staged computation

**1. Modern approach & concrete entities.** C++ `constexpr`, C++20 `consteval`, and C++23 `if consteval`; Rust `const fn`; Zig `comptime`. Jai is considered in the evidence-limited language profile rather than assigned an unverified current compiler API here. [CPP-CONSTEXPR] [RUST-CONST] [ZIG]

**2. Classical antecedent / historical anti-pattern.** Runtime generation of fixed lookup tables, preprocessor arithmetic, and template-recursion machinery used only to compute ordinary constants.

**3. Underlying mechanism & memory layout.** The compiler evaluates permitted computation before runtime and materializes constants or specialized code. Type/value staging can select algorithms and reject impossible configurations. C++ `constexpr` permits constant evaluation in eligible contexts, while an immediate `consteval` invocation must satisfy its stricter rules. A computed table still occupies storage if retained in the binary.

**4. Engineering trade-offs & footprint.** Runtime work becomes compiler work, potentially increasing memory consumption and build latency. Precomputation can trade CPU cycles for ROM or instruction-cache pressure. Host/target distinctions matter for code generators that access the build environment. Constant evaluation has language-specific restrictions and compiler resource limits. Staging is most valuable when inputs are truly fixed and the produced artifacts remain inspectable; it does not automatically establish reproducible builds or better runtime locality.

<a id="p17"></a>
### P17. Static reflection and structure-derived implementation

**1. Modern approach & concrete entities.** C++26 `std::meta::info`, reflection expressions `^^T`, and splicing; Zig `@typeInfo`; compiler-supported structural metadata used to generate serializers, visitors, registries, and bindings. [CPP-REFLECTION] [ZIG]

**2. Classical antecedent / historical anti-pattern.** Parallel field lists, handwritten registration tables, X-macros maintained separately from the actual type, and runtime introspection for structure already fixed at compilation.

**3. Underlying mechanism & memory layout.** The metaprogram examines compiler-known entities and generates ordinary declarations or expressions. If reflection is used only during translation, its metadata need not survive in the runtime binary. Generated code can still contain dispatch tables, names, and field descriptors when the requested functionality needs them.

**4. Engineering trade-offs & footprint.** Reflection reduces duplicated descriptions but increases build complexity and can expose private structure unintentionally. Automatic serialization can accidentally bind a wire schema to field order, spelling, or an unstable layout. Compiler reflection and Rust procedural macros differ: token transformation is not a general semantic type-reflection API. C++26 support must be checked feature by feature. Review generated behavior, compatibility policy, and diagnostics; replacing hand-written code with generated code does not reduce the obligations of the generated implementation.

<a id="p18"></a>
### P18. Hygienic syntax transformation and procedural macros

**1. Modern approach & concrete entities.** Rust `macro_rules!`, derive macros, attribute macros, and function-like procedural macros; Swift attached/freestanding macros. [RUST-MACROS] [RUST-PROCMACROS] [SWIFT-MACROS]

**2. Classical antecedent / historical anti-pattern.** Textual C macros that evaluate expressions unexpectedly, capture names accidentally, or generate syntax whose errors are detached from the user's source.

**3. Underlying mechanism & memory layout.** Syntax-aware expansion transforms structured tokens or syntax into ordinary code. Hygiene rules govern name resolution and capture. Rust declarative macros have mixed-site hygiene; procedural macros are not universally hygienic and must manage generated paths/names carefully. Expansion itself need not have runtime representation, while the emitted code has its normal cost.

**4. Engineering trade-offs & footprint.** Macro execution can lengthen builds, worsen diagnostics, and expand the trusted build surface. Procedural macros can execute host-side code with relevant environmental capabilities. Expansion can still duplicate a side-effecting argument if the macro is written that way. Generated source is harder to review when hidden behind a terse annotation. Prefer functions/generics for ordinary value computation, and macros when syntax generation, derivation, or language-like notation provides a concrete benefit that those mechanisms cannot express cleanly.

<a id="p19"></a>
### P19. Derived operations with explicit semantic laws

**1. Modern approach & concrete entities.** C++20 defaulted equality and three-way comparison; Rust `#[derive]` for `Eq`, `PartialEq`, `Ord`, `Hash`, `Clone`, and other applicable traits; Swift synthesized conformances. [CPP-COMPARE] [RUST-DERIVE] [RUST-EQ] [RUST-HASH]

**2. Classical antecedent / historical anti-pattern.** Handwritten member-by-member comparison that forgets a new field, inconsistent equality/hash behavior, and inheritance hierarchies that compare only base state.

**3. Underlying mechanism & memory layout.** The compiler or derive implementation generates operations from the fields and their available operations. Comparison becomes field comparisons with defined short-circuit/order behavior, not necessarily `memcmp`; padding and semantic equivalence make raw-byte equality unsuitable in general. Derivation adds methods/code, not necessarily object metadata.

**4. Engineering trade-offs & footprint.** Structural derivation may not match domain identity: a cache field should perhaps not affect equality. Floating-point values complicate total-order and equivalence assumptions. `Clone` can recursively allocate or increment reference counts; it is not synonymous with a cheap bit copy. Hashing must remain consistent with equality and with the key's mutability policy. Derivation removes repetitive code but cannot decide the correct semantics for a model. Record those laws and test them with property-based techniques where appropriate.

<a id="p20"></a>
### P20. Modules, visibility boundaries, and representation hiding

**1. Modern approach & concrete entities.** C++20 modules and exported declarations; Rust modules with restricted visibility such as `pub(crate)`; Go packages; C3 modules; abstract module signatures in ML-family languages. [CPP-MODULES] [RUST-PRIVACY] [GO-PACKAGES]

**2. Classical antecedent / historical anti-pattern.** Textual inclusion exposing every implementation detail, preprocessor namespace pollution, and public record fields that allow callers to break invariants.

**3. Underlying mechanism & memory layout.** Visibility rules restrict which entities clients may name or construct. Module systems organize compilation and dependency structure without requiring a runtime module object. A private representation plus sanctioned constructors makes type-level invariants enforceable at the component boundary.

**4. Engineering trade-offs & footprint.** C++ module build artifacts depend on compiler/build configuration and are not a universal stable distribution ABI. Build graphs and caching become important. Representation hiding can reduce optimization opportunities when bodies/layouts are unavailable; exposing everything can increase recompilation and coupling. A module is not automatically a sandbox or dynamic library. Distinguish source visibility, semantic dependency, binary linkage, and runtime authority. Use each boundary for the obligation it actually enforces rather than expecting one module declaration to solve all four.

<a id="p21"></a>
### P21. Customization-point objects and constrained extension hooks

**1. Modern approach & concrete entities.** C++ ranges customization-point objects such as `std::ranges::begin`, and carefully defined ADL-based customization; Rust trait methods as a related explicit extension mechanism. [CPP-CPO] [CPP-RANGE-ACCESS]

**2. Classical antecedent / historical anti-pattern.** Mandatory virtual hooks for every generic algorithm, accidental overload capture, and unconstrained free functions that silently select unintended conversions.

**3. Underlying mechanism & memory layout.** A customization-point object is a callable interface with constrained resolution rules. It can select member functions, permitted ADL functions, or fallback implementations and then invoke the selected operation. A stateless object can disappear from generated code. The customization protocol determines which extensions are discoverable and what result requirements apply.

**4. Engineering trade-offs & footprint.** Lookup and constraints can be subtle, and overly generic hooks increase coupling between namespaces. A CPO is not an automatic guarantee of direct dispatch: the chosen implementation may itself erase or allocate. `tag_invoke` is a convention used by some libraries, not a universal standard customization mechanism for every C++ facility. Prefer a narrow documented hook with testable requirements over a broad “customize anything” mechanism whose lookup behavior becomes part of every diagnostic.

<a id="p22"></a>
### P22. Heterogeneous compile-time records and type-indexed lookup

**1. Modern approach & concrete entities.** C++ `std::tuple`, `std::get<I>`, `std::apply`, and index sequences; Rust tuples and type-indexed library patterns; generated records keyed by compile-time tags. [CPP-TUPLE] [RUST-TUPLE]

**2. Classical antecedent / historical anti-pattern.** Arrays of `void*`, string-keyed heterogeneous maps for a fixed schema, and repeated casts retrieving statically known services or fields.

**3. Underlying mechanism & memory layout.** Each position/key has a known type. Storage can place values inline, including empty-type optimization where permitted. Lookup by a compile-time index or distinct type becomes a fixed projection rather than a hash lookup or runtime type test. Expansion can apply a function to all elements without losing their individual types.

**4. Engineering trade-offs & footprint.** These structures encode a closed schema and cannot naturally accept arbitrary runtime keys. Alignment and padding may make a tuple larger than the sum of payload sizes. Repeated element types can make type-based lookup ambiguous. Large packs inflate diagnostics and code generation. They are effective for fixed pipelines or statically assembled components, but inappropriate as a blanket replacement for dynamic registries, configuration maps, or stable serialized schemas.

<a id="p23"></a>
### P23. Multiple dispatch and symmetric operation selection

**1. Modern approach & concrete entities.** Julia methods dispatched on the tuple of argument types; multimethod systems; C++ multi-variant visitation as a closed-world alternative. [JULIA-METHODS] [CPP-VARIANT-VISIT]

**2. Classical antecedent / historical anti-pattern.** Double-dispatch visitor hierarchies, nested type-code switches, and choosing one arbitrary operand as the privileged object that owns a fundamentally symmetric operation.

**3. Underlying mechanism & memory layout.** Method selection considers several argument types. An implementation can cache dispatch results and specialize known combinations. Multi-variant visitation instead enumerates a statically closed combination space and lowers it through generated dispatch logic. Neither technique requires the operands to inherit storage from a shared base.

**4. Engineering trade-offs & footprint.** Open multiple dispatch introduces ambiguity and method-table evolution concerns. Julia specialization and dynamic lookup have a different execution model from ahead-of-time C++ templates. Closed multi-visitation can require a Cartesian product of cases and grow code rapidly. This model suits operations such as shape collisions or mixed numeric promotion, but is excessive when one operand or one strategy already provides a clear extension point. Distinguish source expressiveness from the dispatch work that remains at runtime.

<a id="p24"></a>
### P24. Extension methods and non-intrusive behavior attachment

**1. Modern approach & concrete entities.** Swift extensions and protocol extensions; Rust extension traits; C3 methods associated with types; free-function APIs with method-like syntax where the language supports it. [SWIFT-EXTENSIONS] [C3-METHODS] [RUST-TRAITS]

**2. Classical antecedent / historical anti-pattern.** Subclassing solely to add convenience operations, giant utility classes, and changing an upstream type definition whenever a downstream domain needs another operation.

**3. Underlying mechanism & memory layout.** Operations are associated with an existing type without necessarily adding stored instance data. Name lookup and conformance rules determine whether a call is a directly resolved extension, a protocol requirement dispatched through witnesses, or another mechanism. Storage composition is not implied by method-call syntax.

**4. Engineering trade-offs & footprint.** Extension names can conflict, and adding an upstream method can affect resolution. Protocol-extension dispatch distinctions matter in Swift. Extension traits may require imports and can interact with coherence. Convenience methods cannot legitimately bypass representation invariants merely because they read like members. Keep domain-specific extensions scoped to the domain that gives them meaning. This technique modernizes organization and reuse; it does not inherently add memory safety, dynamic extensibility, or faster generated code.

<a id="p25"></a>
### P25. Higher-kinded abstraction over type constructors

**1. Modern approach & concrete entities.** Haskell `Functor`, `Applicative`, and `Monad`; Scala higher-kinded type parameters such as `F[_]`; library encodings and associated-type families in languages without direct support. [HASKELL-FUNCTOR] [CATS-TYPECLASSES]

**2. Classical antecedent / historical anti-pattern.** Duplicating the same structural traversal for every container/effect wrapper, or forcing all containers through one erased base interface to share composition logic.

**3. Underlying mechanism & memory layout.** A generic abstraction ranges over a type constructor such as `F`, not merely one concrete type such as `F<Int>`. Operations relate `F<A>` and `F<B>` while preserving the constructor's structure and laws. Compilation can use specialization, dictionaries, or erasure according to the language; higher kinds do not dictate one memory layout.

**4. Engineering trade-offs & footprint.** Encodings in languages without native support can become verbose and obscure associated lifetime or ownership constraints. Dictionaries and wrappers can retain runtime costs when not specialized away. Laws are often conventional rather than compiler-proved. Rust GATs and C++ template-template parameters cover useful subsets but should not be presented as identical to an unrestricted higher-kinded type system. This is valuable for reusable composition libraries, not necessarily the simplest abstraction for a small fixed embedded pipeline.

<a id="p26"></a>
### P26. Parameterized modules and module-level dependency injection

**1. Modern approach & concrete entities.** OCaml functors and abstract signatures; C3 generic modules; Nix functions taking dependency attribute sets. A module functor and a Haskell `Functor` are different concepts despite the shared word. [OCAML-MODULES] [C3-GENERICS] [NIX-LANGUAGE]

**2. Classical antecedent / historical anti-pattern.** Global service lookup, preprocessor-selected module variants, and assembling an object-inheritance graph merely to supply a coherent family of related types and operations.

**3. Underlying mechanism & memory layout.** A module parameter supplies several related declarations together. An ML functor maps a module satisfying a signature to another module, preserving abstract type relationships. C3 generic modules instantiate declarations with parameters. A Nix function produces an attribute set/value from supplied dependencies through lazy evaluation, rather than compiled static type checking.

**4. Engineering trade-offs & footprint.** Static module instantiation can duplicate code; runtime module values or Nix closures can retain environments. Abstract/generative type identity can surprise consumers and requires clear sharing constraints. Nix dependency injection is compositional but dynamically typed. Passing an entire service set can conceal dependencies unless the required subset is explicit. The engineering advantage is configuring a coherent component family at a boundary, not claiming every module mechanism is a zero-cost static template.

---

<a id="section-4"></a>
## IV. Data Modeling & State Machines

The objective is to represent the admissible domain, not merely allocate enough bits to store it. Products express simultaneous facts, sums express alternatives, nominal wrappers distinguish meanings, and indexed types express relationships. Runtime evidence remains necessary wherever the environment, input, or protocol is not controlled by the type checker.

<a id="d01"></a>
### D01. Product types and named records

**1. Modern approach & concrete entities.** Rust structs and tuples, Swift structs, C++ aggregates and `std::tuple`, Go structs, and Roc records represent products. Named result records such as `ReadProgress { bytes_read, terminal_status }` preserve the meaning of multiple simultaneous outcomes. [RUST-TUPLE] [CPP-TUPLE] [GO-SPEC] [ROC-TUTORIAL]

**2. Classical antecedent / historical anti-pattern.** Parallel arrays with implicit correspondence, positional output parameters, universal dictionaries with string keys, and a class hierarchy introduced for a fixed collection of fields. Product modeling itself is older than modern languages; the contemporary improvement is systematic use with explicit invariants and composition.

**3. Underlying mechanism & memory layout.** A product contains all its components. If finite component domains have cardinalities `a` and `b`, the unconstrained product has `a × b` combinations. A typical native record stores fields inline with alignment padding, but field order, ABI classification, and empty-field treatment depend on the language and representation attributes. A product of references stores access paths, not copies of the referents.

**4. Engineering trade-offs & footprint.** The Cartesian product can include invalid combinations. A record `{connected: bool, socket: Optional<Socket>}` admits states that may have no domain meaning. Adding a constructor helps only if later mutation preserves the relation. Product representation is appropriate when all components can coexist, including intentional partial-progress/error combinations. Named fields improve maintenance but do not prove units, lifetime validity, or cross-field consistency. [RUST-LAYOUT]

<a id="d02"></a>
### D02. Sum types and payload-bearing alternatives

**1. Modern approach & concrete entities.** Rust enums, Swift associated-value enums, C++ `std::variant`, Zig `union(enum)`, Odin unions, V sum types, and Roc tag unions. A state can be `Disconnected | Connecting(Attempt) | Connected(Session)` instead of a bag of optional members. [RUST-ENUM] [CPP-VARIANT] [ZIG] [V-OPTIONS]

**2. Classical antecedent / historical anti-pattern.** Untagged unions, integer tags that disagree with payload fields, subclass hierarchies for a closed set of alternatives, and boolean combinations representing mutually exclusive states.

**3. Underlying mechanism & memory layout.** A sum contains one alternative. Its semantic cardinality is the sum of the alternatives' cardinalities. Conventional native layout reserves space for the largest payload plus a discriminator and padding; niche encoding can reuse impossible payload representations. A compiler may optimize a locally unobservable sum into control flow. A recursive sum needs finite indirection, indices, or another bounded representation.

**4. Engineering trade-offs & footprint.** A rare large payload enlarges every inline instance; boxing only that case can reduce average storage but adds allocation and indirection. Closed sums make new operations straightforward but require updates when cases are added. `std::variant` can become `valueless_by_exception`, unlike a simplistic always-one-case model. Neither a native enum nor its discriminant automatically defines a stable wire format. Keep large-case layout and evolution policy explicit.

<a id="d03"></a>
### D03. Exhaustive pattern matching and checked decomposition

**1. Modern approach & concrete entities.** Rust `match`, `if let`, and `let ... else`; Swift `switch`; Zig `switch`; Roc `match`; C++ `std::visit` for variant alternatives. Matching selects a case and exposes only the payload appropriate to that case. [RUST-MATCH] [SWIFT-CONTROL] [CPP-VARIANT-VISIT]

**2. Classical antecedent / historical anti-pattern.** Switching on a type code and casting the wrong payload, forgetting an enum case after a change, repeated `dynamic_cast`, and independent tag tests followed by unsafe member access.

**3. Underlying mechanism & memory layout.** Coverage checking operates over the alternatives and applicable pattern space. Lowering can use comparisons, decision trees, jump tables, or optimized tests of niches. Bindings may borrow, copy, or move payloads according to the language and pattern. The match itself need not allocate. Guards add conditions; their presence does not automatically establish complete coverage.

**4. Engineering trade-offs & footprint.** A catch-all branch trades explicit future-case review for forward compatibility. Cartesian matching over several sums can generate many combinations. C++ multi-variant visitation can increase instantiation and dispatch-table size. Exhaustiveness proves coverage of modeled values, not that the model includes every real-world event. Publicly non-exhaustive types and unknown wire values require a deliberate fallback. C++ visitation is a library facility, not a claim that C++26 has a general native pattern-matching syntax. [RUST-NONEXHAUSTIVE]

<a id="d04"></a>
### D04. Newtypes and strong nominal identifiers

**1. Modern approach & concrete entities.** Rust `struct UserId(u64)`, Go `type UserID uint64`, Odin `distinct`, C++ wrapper classes, and Swift wrapper structs. Distinguish `SensorId`, `BusId`, `ByteCount`, and `SampleCount` even when they use identical machine integers. [GO-SPEC] [ODIN] [RUST-LAYOUT]

**2. Classical antecedent / historical anti-pattern.** Passing unrelated integers interchangeably, aliases that only rename a primitive, universal `void*` handles, and parameter ordering bugs in APIs with many same-typed arguments.

**3. Underlying mechanism & memory layout.** Nominal type identity makes accidental substitution ill-typed. A wrapper can have the same runtime payload as its underlying value. Rust `#[repr(transparent)]` provides representation guarantees when its conditions hold; otherwise equal observed size is not itself an ABI contract. Operations and conversions are explicitly granted rather than inherited indiscriminately from the primitive.

**4. Engineering trade-offs & footprint.** Boilerplate, adapters, serialization, formatting, and operator definitions increase surface area. Aggressive implicit conversion destroys the distinction. Strong IDs prevent category errors but not stale-object access; combine them with generations or validated lookups where needed. A global `UserId` does not identify which database issued it. Transparent representation also does not make every integer a valid value of a wrapper with additional invariants. The type boundary must remain meaningful through FFI and deserialization.

<a id="d05"></a>
### D05. Dimensional quantities, units, and scale-aware values

**1. Modern approach & concrete entities.** C++ `std::chrono::duration<Rep, Period>` for time; domain quantity types parameterized by dimension and unit; Rust or Swift nominal quantity wrappers. Distinguish angle, angular velocity, time, voltage, fixed-point scale, and physical reference frame where the domain requires them. [CPP-DURATION]

**2. Classical antecedent / historical anti-pattern.** Unadorned `double` arguments, hidden milliseconds-to-seconds conventions, mixing degrees and radians, and using raw fixed-point integers without tracking their scaling law.

**3. Underlying mechanism & memory layout.** Dimensions and units can be compile-time metadata while the object stores only its representation. Type-level algebra governs permissible multiplication, division, addition, and comparison. Conversion between units can become a constant scale operation; some conversions erase, while others multiply, divide, round, or check bounds. A point on a scale and a difference between points may need distinct types.

**4. Engineering trade-offs & footprint.** Dimension safety does not establish numerical conditioning, adequate range, sensor calibration, or absence of overflow. Two angular values can share dimensions while referring to different frames or conventions. Template arithmetic and diagnostics become more complex. Integer conversions can truncate; floating conversions introduce rounding. A strongly typed physical interface should state representation, scale, saturation policy, and invalid-measurement handling instead of assuming units solve all numeric correctness.

<a id="d06"></a>
### D06. Smart constructors and invariant-carrying values

**1. Modern approach & concrete entities.** Private representations with validated factories; Rust `NonZero<T>`, `TryFrom`, and domain wrappers; C++ factories returning `std::expected`; Swift failable initializers. A successful constructor can establish a bounded range, validated encoding, normalized identifier, or nonempty collection. [RUST-NONZERO] [RUST-TRYFROM] [CPP-EXPECTED]

**2. Classical antecedent / historical anti-pattern.** Rechecking the same primitive at every function, partially initialized objects, public fields that permit invalid combinations, and boolean “validated” flags independent from the data they supposedly validate.

**3. Underlying mechanism & memory layout.** Validate at the boundary, then expose only invariant-preserving operations. The accepted value need not contain a runtime certificate: restricted construction makes possession of its type evidence of the invariant. Unused representations can additionally support niche optimization. Validation may transform data into a canonical representation, reducing downstream cases.

**4. Engineering trade-offs & footprint.** Evidence is only as strong as all construction and mutation paths. Unsafe conversion, unchecked deserialization, and mutable references to the representation can break it. An external fact—permission, device readiness, file existence—can change after validation; its evidence may need a lifetime, version, lease, or fresh check. `NonNull<T>` proves non-nullness, not dereferenceability. Use domain-specific names that state the actual invariant rather than a misleading universal `Safe<T>`. [RUST-NONNULL]

<a id="d07"></a>
### D07. Refinement types and solver-checked predicates

**1. Modern approach & concrete entities.** Liquid Haskell refinements such as an integer constrained by a predicate; indexed and proof-oriented APIs in dependent languages. These go beyond an ordinary wrapper by making selected value predicates available to a verification system. [LIQUIDHASKELL]

**2. Classical antecedent / historical anti-pattern.** Preconditions written only in comments, repeatedly testing index bounds that follow from known facts, and confusing runtime assertions with proofs that every call satisfies an interface.

**3. Underlying mechanism & memory layout.** The checker generates logical verification conditions from signatures and program operations. An SMT solver or proof term discharges obligations in the supported logic. Refinement evidence often erases, leaving the base representation and ordinary executable code. Runtime input still needs a checked introduction path unless its property is established from other evidence.

**4. Engineering trade-offs & footprint.** Solver support, decidability, annotations, and trusted assumptions limit what can be established automatically. Machine overflow, mathematical integers, floating-point behavior, and foreign calls must be modeled consistently. A proof of array bounds does not imply deadline satisfaction. Changes to an algorithm can break proofs even when the intended behavior is unchanged. C++ contracts are not automatically refinement typing: runtime predicate checking and static logical discharge are different enforcement mechanisms with different costs. [CPP-CONTRACTS]

<a id="d08"></a>
### D08. Typestate and consuming protocol transitions

**1. Modern approach & concrete entities.** `Socket<Connected>`, `Dma<Idle>`, `Dma<Running>`, and consuming state-specific methods; Rust ownership and phantom state parameters; Swift consuming noncopyable wrappers; C++ state-specialized wrappers. [RUST-OWNERSHIP] [RUST-PHANTOM] [SWIFT-NONCOPY]

**2. Classical antecedent / historical anti-pattern.** Every operation available on every object, with `initialized`, `running`, and `closed` booleans checked inconsistently at runtime.

**3. Underlying mechanism & memory layout.** A transition accepts an old-state capability and produces a new-state capability. A compile-time phase marker may occupy no storage; the device handle is transferred unchanged. The type checker restricts which methods are available. Runtime-selected phases require a sum or other dynamic representation rather than pretending all state is known statically.

**4. Engineering trade-offs & footprint.** Failed transitions must specify resource recovery. `Result<Running, (Idle, Error)>` is sound only if failure leaves the device idle; partial activation may require an `Unknown` or `RecoveryRequired` state. Hardware, interrupts, remote peers, and other processes can violate assumptions not modeled by the API. Types establish software sequencing under those assumptions, not physical truth. C++ moving from a wrapper does not itself forbid later calls on it. Typestate is most effective for a manageable number of meaningful phases rather than every transient bit combination.

<a id="d09"></a>
### D09. Staged builders and compile-time completeness

**1. Modern approach & concrete entities.** Builders parameterized by whether required fields are present, such as `RequestBuilder<HasEndpoint, MissingCredentials>`; Rust or C++ consuming setters returning new specializations; Swift staged wrapper types. This combines type-level state with construction rather than ongoing resource operation. [RUST-GENERICS] [CPP-TEMPLATE-PARAMETERS]

**2. Classical antecedent / historical anti-pattern.** A long constructor full of positional primitives, a mutable property bag whose `build()` discovers omissions late, and default zero values that accidentally count as configuration.

**3. Underlying mechanism & memory layout.** Each setter changes a type-level presence state. Only complete states expose `build()`. The runtime representation can remain a record of the provided fields; static markers need not add flags. Builders may use typestate to distinguish mutually exclusive choices as well as required fields.

**4. Engineering trade-offs & footprint.** With many independent requirements, the conceptual state space can approach `2^n`, although only instantiated states generate code. Error messages and inferred types grow. Runtime configuration files still need dynamic validation. Copying a large builder at every stage can be expensive unless movement, borrowing, or optimization avoids it. A staged builder proves field presence, not compatibility among their values; cross-field constraints still need checking. Reserve the technique for high-value invariants rather than making trivial option setters a type-level maze.

<a id="d10"></a>
### D10. Phantom brands and provenance distinctions

**1. Modern approach & concrete entities.** Rust `PhantomData<T>` and lifetime brands; IDs parameterized by pool, transaction, unit, or reference-frame tags; C++ template tag parameters and empty marker fields. [RUST-PHANTOM] [CPP-NOUNIQUE]

**2. Classical antecedent / historical anti-pattern.** Applying an index from one pool to another, mixing coordinate frames with the same numerical representation, and giving borrowed handles no relationship to their issuer.

**3. Underlying mechanism & memory layout.** A phantom parameter influences type checking without storing a value of that parameter. A carefully designed generative scope can create a brand that external code cannot manufacture. Rust marker choices also affect variance, auto traits, and drop analysis; they are not merely decorative documentation.

**4. Engineering trade-offs & footprint.** A shared tag type distinguishes categories, not necessarily separate runtime instances. Two pools both parameterized by `GlobalPool` still share a brand. Constructors and lifetime quantification must establish the intended uniqueness. Brands do not prevent stale reuse within one issuer; generation checks may still be required. Incorrect variance or marker choice can impose unnecessary restrictions or encode the wrong ownership/thread-safety relationship. The marker's runtime size may be zero while the API's conceptual complexity is substantial. Test misuse cases by confirming that forbidden substitutions fail to type-check.

<a id="d11"></a>
### D11. Indexed data families, GADTs, and dependent shapes

**1. Modern approach & concrete entities.** Haskell generalized algebraic data types for typed expression trees; Idris `Vect n a`, `Fin n`, and erased evidence. A constructor can establish that an expression produces a particular type or that a collection has a particular length. [GHC-GADT] [IDRIS-LINEAR]

**2. Classical antecedent / historical anti-pattern.** Untyped AST nodes requiring downcasts, vector lengths tracked only in comments, and interpreters containing “impossible” runtime type-error cases created by their own internal representation.

**3. Underlying mechanism & memory layout.** Constructors constrain type indices, and matching refines the facts available in a branch. A typed expression interpreter can return the type indexed by the expression. A bounded index provides evidence that indexing a matching vector is valid. Some type indices and proofs erase; executable values and any runtime evidence needed by operations remain.

**4. Engineering trade-offs & footprint.** Type inference and pattern matching become more intricate. Proof maintenance can exceed the cost of ordinary implementation. Runtime data must be packaged existentially or validated before its dependent relation is known. Erasing a proof does not imply zero cost for computing evidence or building the underlying structure. Ordinary Rust const generics and C++ template parameters support useful fragments of this style, but they are not unrestricted dependent type systems. Foreign data and unsafe constructors remain trusted boundaries.

<a id="d12"></a>
### D12. Session-typed communication protocols

**1. Modern approach & concrete entities.** Session-typed channels such as the Rust `sesh` library's `Send<T, S>`, `Recv<T, S>`, and `End`, with dual endpoint protocols. A sequence can encode receiving a request, sending a response, and closing. [SESH]

**2. Classical antecedent / historical anti-pattern.** Sending a message that is invalid in the current phase, pairing two receivers, and maintaining protocol sequencing only in diagrams and integer state variables.

**3. Underlying mechanism & memory layout.** Each communication operation consumes an endpoint and yields its continuation endpoint. Peer endpoints carry dual types: one side's send corresponds to the other's receive. The protocol type may erase while the implementation retains channel endpoints, queues, synchronization state, and message storage.

**4. Engineering trade-offs & footprint.** Well-typed sequencing is not equivalent to successful communication. Cancellation, timeouts, crashes, transport corruption, and version negotiation still need representation. Deadlock-freedom claims depend on the library's composition discipline and modeled communication topology; arbitrary external locks or channels can invalidate stronger conclusions. Dynamic protocol choices require typed branching, recursion, or existential packaging. Session types are especially useful at well-defined internal boundaries; a network peer written outside the type system still requires runtime enforcement of the protocol.

<a id="d13"></a>
### D13. Capability-oriented authority and narrow dependency interfaces

**1. Modern approach & concrete entities.** Authority-bearing values such as `cap_std::fs::Dir`; APIs accepting a particular clock, allocator, peripheral, or transport capability instead of consulting ambient global facilities. A capability combines access with authority to perform a bounded operation set. [CAP-STD]

**2. Classical antecedent / historical anti-pattern.** Global service locators, unrestricted filesystem paths, hidden process-wide configuration, and components that can access every device merely because a global namespace exposes them.

**3. Underlying mechanism & memory layout.** A value holds an operating-system handle, bounded reference, or service interface. Static types restrict which operations are exposed; runtime enforcement can constrain the resource domain. A borrowed capability may be just a pointer, while an owning one has release semantics. Explicit parameters make the dependency graph visible.

**4. Engineering trade-offs & footprint.** Threading capabilities through call chains increases signature and lifetime complexity. Dynamic capabilities may add dispatch and reference counting. A convention of passing dependencies is not itself a security sandbox if code can reacquire ambient authority or bypass the wrapper. Authority attenuation and revocation require concrete mechanisms. In embedded software, a typed register-block handle can prevent accidental access to the wrong peripheral without proving MPU configuration, exclusive hardware ownership, or freedom from privileged bypass.

<a id="d14"></a>
### D14. Functional cores, reducers, and explicit command emission

**1. Modern approach & concrete entities.** Pure transition functions of the form `transition(State, Event) -> (State, Commands)`; reducers over ADTs; immutable records; Roc pure functions and Nix expressions as examples of separating calculations from effects. [ROC-TUTORIAL] [NIX-LANGUAGE]

**2. Classical antecedent / historical anti-pattern.** Business logic spread across mutating methods and callbacks, observers recursively triggering more observers, and state transitions inseparable from device access or network I/O.

**3. Underlying mechanism & memory layout.** The core computes a new logical state and an explicit description of requested effects. An imperative shell interprets commands and feeds subsequent observations back as events. The implementation may use copies, unique ownership with storage reuse, or persistent sharing; purity does not mandate one physical representation.

**4. Engineering trade-offs & footprint.** Large state copies and command allocations can be expensive unless representation is chosen deliberately. Describing an effect does not guarantee its execution, ordering, or atomicity. Commands need correlation IDs and completion events where partial failure matters. A pure model simplifies replay and property testing, but clocks, randomness, and external nondeterminism must be explicit inputs. Deterministic replay also requires controlled numerical and ordering behavior; the word “functional” does not supply those guarantees automatically.

<a id="d15"></a>
### D15. Persistent immutable collections and structural sharing

**1. Modern approach & concrete entities.** Persistent vectors, hash-array-mapped tries, and immutable maps; Rust's `im` collection library and analogous functional-language libraries. Persistence means older versions remain accessible after an update, not persistence to disk. [IM-COLLECTIONS]

**2. Classical antecedent / historical anti-pattern.** Deep-copying an entire structure for each snapshot, sharing one mutable graph among readers, and manual undo logs for values that naturally form a version history.

**3. Underlying mechanism & memory layout.** An update constructs a new path or small set of nodes and reuses unchanged substructure. A wide branching factor can keep tree depth small. Node ownership may use reference counting, garbage collection, or explicit regions. Uniqueness-aware implementations can mutate unshared nodes internally while preserving the external value semantics.

**4. Engineering trade-offs & footprint.** Updates allocate or manage node ownership; traversal adds indirection compared with a flat array. Old versions retain shared storage, potentially keeping much more memory alive than expected. Reference-counted nodes add atomic or non-atomic bookkeeping according to the implementation. Sharing can improve snapshot cost while hurting locality in hot loops. Persistent collection semantics and copy-on-write arrays are related but different implementation families. Evaluate version lifetime, branching factor, traversal pattern, and reclamation cost rather than assuming immutability is free.

<a id="d16"></a>
### D16. Optics: lenses, prisms, and composable traversal

**1. Modern approach & concrete entities.** Lenses focusing on product fields, prisms selecting sum alternatives, and traversals addressing zero or more targets; Haskell's `lens` package provides concrete composable abstractions. [HASKELL-LENS]

**2. Classical antecedent / historical anti-pattern.** Repeated nested copy-update code, long chains of getters/setters, string-based property paths, and manually rebuilding enclosing records whenever a deeply nested value changes.

**3. Underlying mechanism & memory layout.** An optic packages or encodes a lawful access/update relationship. Composition constructs a path through nested structures without hard-coding every whole-object update. Depending on representation and specialization, the abstraction can inline into direct projections and reconstruction; a reified dynamic optic may retain callable objects and dispatch.

**4. Engineering trade-offs & footprint.** Lens laws are semantic obligations, not guaranteed merely by using the expected function shape. Persistent updates still have their allocation or copying costs. Highly polymorphic optic types can produce difficult errors and substantial compilation work. A setter that silently normalizes, rejects, or aliases data may violate ordinary lens expectations and require a different abstraction. The technique is most useful when nested immutable transformations recur; it can be disproportionate for a shallow systems record with a few explicit fields.

<a id="d17"></a>
### D17. Entity-component modeling and data-driven composition

**1. Modern approach & concrete entities.** Entity-component systems such as Flecs, typed component sets, and systems that query required component combinations. Entities represent identity; components represent data; systems implement behavior over matching data. [FLECS-DESIGN]

**2. Classical antecedent / historical anti-pattern.** Deep `Entity -> Vehicle -> FlyingVehicle -> ArmedFlyingVehicle` inheritance, duplicated behavior across branches, and one virtual call per small object operation in bulk simulation.

**3. Underlying mechanism & memory layout.** An entity is typically an identifier. Archetype storage groups entities by component combination and places corresponding columns together; sparse-set designs organize components separately. Queries locate matching storage and iterate values in batches. Composition changes through component membership rather than class ancestry.

**4. Engineering trade-offs & footprint.** Structural changes can migrate entities, invalidate references, and incur deferred-update bookkeeping. Queries and entity lookup add indirection; too many archetypes fragment storage. An ECS does not automatically provide a formal domain model, safe concurrent scheduling, or faster execution for every workload. Relationships, ownership, and rare large components need explicit design. Typed component APIs reduce category errors but do not eliminate the need for generation checks and lifecycle rules. Use this for populations with repeated bulk behavior, not as a mandatory replacement for every object.

<a id="d18"></a>
### D18. Recursive data through arenas, indices, and explicit identity

**1. Modern approach & concrete entities.** Arena-backed ASTs and graphs, typed `NodeId` indices, sum-typed node payloads, and generational references when deletion/reuse is required. Rust or C++ vectors can hold nodes while edges store indices rather than owning pointers. [RUST-VEC] [SLOTMAP]

**2. Classical antecedent / historical anti-pattern.** One allocation per AST node, recursive ownership with expensive destruction, raw back-pointers, and shared-pointer cycles used merely to represent graph connectivity.

**3. Underlying mechanism & memory layout.** An arena owns nodes collectively. Edges encode logical references into arena storage, decoupling connectivity from physical addresses. Moving the backing allocation does not invalidate plain indices, provided indexing semantics remain stable. Typed indices can distinguish node classes or arenas; generations detect selected stale-reuse cases.

**4. Engineering trade-offs & footprint.** Lookup introduces indexing and potentially a validity check. Deletion, compaction, and cross-arena references require policy; an integer index does not track ownership by itself. Large sum payloads inflate each slot unless split by node kind or boxed selectively. Bulk reclamation simplifies lifetime management but retains unreachable nodes until reset. Recursive algorithms can still exhaust the call stack even when allocation is arena-based. The representation solves storage and identity questions, not automatically traversal complexity or semantic validity of edges.

<a id="d19"></a>
### D19. Typed embedded DSLs and parser combinators

**1. Modern approach & concrete entities.** Typed expression builders, query DSLs, and parser combinators such as Rust `nom`. A parser's output type describes the value established by successful recognition; typed AST constructors can distinguish expressions, statements, dimensions, and effects. [NOM] [GHC-GADT]

**2. Classical antecedent / historical anti-pattern.** Stringly typed query assembly, unchecked text substitution, handwritten parse routines with inconsistent cursor/error conventions, and generic AST nodes whose operands require runtime downcasts.

**3. Underlying mechanism & memory layout.** Combinators compose recognizers, cursor advancement, and output construction. Statically known combinators may specialize into direct control flow; a runtime parser graph may allocate and dispatch. Zero-copy parsers return views into input when its lifetime permits. Typed DSLs can build explicit syntax trees or interpret construction directly, depending on the design.

**4. Engineering trade-offs & footprint.** Backtracking, ambiguity, recursion depth, input retention, and diagnostic construction remain runtime costs. A parser API's use of `Result` does not imply linear-time behavior. Incomplete streaming input must be distinguished from invalid input. An AST that is well-typed internally may still be unauthorized or unsafe to execute. Heavy combinator types can inflate compile time, while aggressive erasure trades that cost for dynamic calls. Specify grammar, resource limits, lifetime, and failure policy separately.

<a id="d20"></a>
### D20. Boundary types, explicit representation, and checked zero-copy parsing

**1. Modern approach & concrete entities.** Rust `#[repr(C)]`, `#[repr(transparent)]`, and `extern "C"`; Zig `extern struct`; C++ boundary records and C linkage; Rust `zerocopy` traits including `TryFromBytes`, `KnownLayout`, and `Unaligned`. [RUST-LAYOUT] [ZIG] [ZEROCOPY]

**2. Classical antecedent / historical anti-pattern.** Casting a byte buffer to an arbitrary native struct, dumping object memory as a protocol, and exporting native strings, trait objects, or variants across an uncontrolled compiler ABI.

**3. Underlying mechanism & memory layout.** Boundary representations explicitly address size, offsets, alignment, endianness, and admissible bit patterns. A checked view validates applicable conditions before exposing typed access; a decoder may instead construct independent domain values. C linkage controls function linkage/calling conventions as specified, not the validity of arbitrary C++ object layouts.

**4. Engineering trade-offs & footprint.** Zero-copy parsing still incurs checks and ties results to backing-storage lifetime and mutability. Packed fields can require unaligned access sequences; padding can leak uninitialized bytes in naive serialization. A native layout attribute does not define endianness or protocol versioning. C interfaces remain useful interoperability boundaries: modernization consists in narrowing and wrapping their obligations, not pretending an ABI can be removed. Validate the received domain before converting wire integers into stronger internal types.

<a id="d21"></a>
### D21. Declarative configuration and dependency-graph construction

**1. Modern approach & concrete entities.** Nix expressions, immutable attribute sets, parameterized functions, and derivations; configurations represented as values rather than imperative sequences of shell mutations. This is an architectural companion to systems programming, not a claim about zero-cost native object layout. [NIX-LANGUAGE] [NIX-DERIVATIONS]

**2. Classical antecedent / historical anti-pattern.** Provisioning scripts whose result depends on execution history, hidden environment variables, globally installed tools, and copied configuration fragments with untracked dependency relationships.

**3. Underlying mechanism & memory layout.** Evaluation constructs values describing dependencies and build actions. Derivations specify inputs and builder information that the build system uses to organize work and outputs. Lazy evaluation can defer expression work until demanded. The evaluator retains runtime values and thunks; it does not erase into a borrow-checked native interface.

**4. Engineering trade-offs & footprint.** Nix is dynamically typed, so many shape and value errors appear during evaluation rather than a separate static type check. Declarative description, pure evaluation, sandboxed builds, content addressing, and reproducible outputs are distinct properties. An undeclared environmental dependency or nondeterministic builder can defeat reproducibility. Laziness complicates error locality and memory-retention reasoning. Pin inputs and specify build assumptions, while keeping domain validation explicit rather than treating an attribute set as a statically verified schema.

<a id="d22"></a>
### D22. Evolution-aware sums and explicit unknown values

**1. Modern approach & concrete entities.** Rust `#[non_exhaustive]`, wire-level `Unknown(raw_code)` alternatives, versioned schemas, and Protocol Buffers field-number and unknown-field evolution rules. [RUST-NONEXHAUSTIVE] [PROTOBUF]

**2. Classical antecedent / historical anti-pattern.** Assuming every future enum value is already known, reusing removed field identifiers, discarding unrecognized data during forwarding, and relying on native discriminant ordinals as protocol identifiers.

**3. Underlying mechanism & memory layout.** A public type can require external consumers to retain a fallback path. A wire decoder can preserve unknown discriminants and raw payloads rather than inventing a known semantic value. Versioned schemas separate stable external identifiers from implementation layout. Forwarding may retain extra buffers or unknown-field storage.

**4. Engineering trade-offs & footprint.** Tolerating unknown values is not equivalent to safely acting on them. A safety-relevant command may require rejection while a telemetry relay should preserve it. Unknown-field retention consumes memory and may retain maliciously large payloads unless bounded. Compatibility depends on reader/writer direction and changes to meaning, not just whether parsing succeeds. A non-exhaustive source enum provides an evolution mechanism at that API boundary; it does not automatically establish binary or wire compatibility.

<a id="d23"></a>
### D23. Typed flag sets versus exclusive state alternatives

**1. Modern approach & concrete entities.** Rust `bitflags` types, Odin `bit_set`, C++ scoped enums with deliberate flag operators, and finite-set wrappers. Use these for independent memberships or capabilities; use sums for mutually exclusive phases. [BITFLAGS] [ODIN]

**2. Classical antecedent / historical anti-pattern.** Arbitrary integer masks, mixing unrelated flag domains, using booleans to represent exactly-one-of-N states, and accepting reserved bits without a defined policy.

**3. Underlying mechanism & memory layout.** A typed flag set stores a bit vector. Set algebra becomes bitwise operations while nominal type identity distinguishes domains. For `n` independent flags, up to `2^n` combinations are meaningful; an exclusive `n`-case state requires only `n` alternatives. Constructors can reject or preserve unknown bits according to the boundary contract.

**4. Engineering trade-offs & footprint.** A flag type does not prove that every combination is valid. Mutually dependent flags still need validation or a better sum/product decomposition. Reserved bits matter during forward-compatible decoding and hardware register access. Atomic updates require a supported atomic representation or synchronization; individually setting bits is not automatically race-free. Bit packing improves storage density but can introduce read-modify-write traffic and false sharing. The right replacement for boolean soup is sometimes a product of independent states, not one enormous enum.

<a id="d24"></a>
### D24. Hierarchical state machines and orthogonal state composition

**1. Modern approach & concrete entities.** Statecharts with hierarchical states, orthogonal regions, guards, and entry/exit actions; XState provides a concrete modern implementation. Typed events and sum/product state models can be used beneath or alongside the statechart notation. [STATECHARTS]

**2. Classical antecedent / historical anti-pattern.** Large switches duplicating common transitions, boolean state combinations with no transition semantics, and flattening every combination of independent subsystem states into a manually enumerated global state.

**3. Underlying mechanism & memory layout.** Hierarchy factors common transitions and behavior; orthogonal regions represent simultaneously active submachines. Generated or interpreted execution tracks active configuration and dispatches events under specified transition-priority rules. A product of local states can represent global configuration without storing a separate object for every possible combination.

**4. Engineering trade-offs & footprint.** Hierarchy and concurrency reduce textual duplication but not necessarily the reachable-state space. Event queues, deferred events, history states, and run-to-completion semantics require explicit bounds and ordering. A statechart engine may allocate or dispatch indirectly; generated tables and direct code have different footprints. Guards and actions can introduce effects not represented by state labels. Statecharts are historically established, not newly invented; their contemporary value is integrating explicit protocol models with typed events, verification, code generation, and test coverage.

---

<a id="section-5"></a>
## V. Language crosswalk and maturity boundaries

This crosswalk maps the census to concrete language families. It is not a feature-count ranking. A language can emphasize explicit data layout without a borrow checker, static algebraic modeling with runtime memory management, or declarative dependency construction outside the native execution domain.

<a id="language-cpp"></a>
### C++20, C++23, and C++26

C++ combines value-oriented resource wrappers, static generic programming, explicit low-level access, and optional dynamic polymorphism. RAII, moves, and standard containers predate the requested editions. Modern library vocabulary improves signatures, but raw-pointer and reference lifetimes are not generally checked as Rust borrow relationships. Concepts constrain generic interfaces; they do not prove semantic laws such as comparator ordering. [CPP-GUIDELINES] [CPP-CONSTRAINTS]

| Edition grouping | Census-relevant facilities | Important boundary |
|---|---|---|
| C++20 | `span`, concepts and `requires`, ranges, coroutines, modules, `consteval`, `[[no_unique_address]]` | Views are not a borrow checker; modules are not ABI stability; coroutines are not a scheduler |
| C++23 | `expected`, optional/expected monadic operations, `mdspan`, explicit object parameters, `if consteval`, `move_only_function` | Value operations may throw or allocate; explicit object parameters do not make moves consuming in Rust's sense |
| C++26 working-draft facilities | Contracts, static reflection, execution/senders, `inplace_vector`, `hive`, `indirect`, `polymorphic`, `function_ref`, data-parallel `<simd>` | Support is compiler/library-specific; consult the cited draft sections and feature tests |

The facilities and their semantics are documented in the linked draft sections rather than inferred from edition names. Some referenced sections contain later evolution of an older facility; the edition grouping identifies introduction of the named core facility, not every overload visible in a rolling draft. [CPP-SPAN] [CPP-EXPECTED] [CPP-MDSPAN] [CPP-FUNCTION] [CPP-CONSTEXPR] [CPP-MOVEFUNCTION] [CPP-CONTRACTS] [CPP-REFLECTION] [CPP-EXEC] [CPP-INPLACE] [CPP-HIVE] [CPP-INDIRECT] [CPP-POLYMORPHIC] [CPP-FUNCTIONREF] [CPP-SIMD]

For bounded embedded use, inspect the actual element types, allocator fallback, exception policy, callback representation, and object/frame size. `std::pmr` selects allocation policy through a runtime resource interface; it does not mean allocation-free. `std::inplace_vector<T,N>` embeds capacity, but `T` may allocate. `expected<T,E>` avoids a success/error out-parameter protocol, not every possible exception in its payload operations. [CPP-PMR] [CPP-INPLACE] [CPP-EXPECTED]

<a id="language-rust"></a>
### Rust

The characteristic combination is affine ownership, borrow/lifetime checking, ADTs, constrained generics, and explicit unsafe boundaries. Relevant entities include `&T`, `&mut T`, `Box`, `Vec`, `Option`, `Result`, trait bounds, `dyn Trait`, `impl Trait`, `Pin`, `MaybeUninit`, `Send`, and `Sync`. General resource lifetime and aliasing checks are compile-time rules; runtime policies remain available through `Rc`, `Arc`, `RefCell`, locks, and allocation. [RUST-OWNERSHIP] [RUST-BORROW] [RUST-UNSAFE] [RUST-CELL]

A Rust design review should distinguish ownership of a handle from authority over its referent, memory safety from deadlock freedom, and pinning from physical memory constraints. `Pin` is not DMA page locking. Lifetimes do not extend the owner's storage. A safe abstraction may rely on a small unsafe implementation whose invariants must survive panic, cancellation, forgetting, and partial initialization. Public Rust type layout is not automatically a stable FFI contract. [RUST-PIN] [RUST-FORGET] [RUST-MAYBEUNINIT] [RUST-LAYOUT]

<a id="language-go"></a>
### Go

Go emphasizes garbage-collected storage, structural interface conformance, ordinary value records, slices, explicit product-return errors, and lightweight concurrency. `[]T` is a descriptor over shared backing storage, not a borrow-checked unique region. `(T, error)` permits both components to carry information; `io.Reader` makes this useful for partial progress. `defer` is function-scoped. [GO-SLICES] [GO-IO] [GO-DEFER]

Generic type-set constraints should not be confused with first-class runtime sums. The documented compiler design combines representation-shape sharing with dictionaries; the source language does not require one fully specialized machine-code body per type. An interface containing a typed nil pointer can itself be non-nil. The GC manages memory reachability, not timely release of device handles or files. Go also does not statically enforce Rust-like ownership transfer between goroutines. [GO-SPEC] [GO-DICTIONARIES] [GO-FAQ] [GO-GC]

<a id="language-swift"></a>
### Swift

Swift combines value-semantic structs/enums, reference types managed by ARC, protocol abstraction, and increasingly explicit ownership and isolation controls. `some P` preserves an opaque concrete type; `any P` packages an existential. `~Copyable`, borrowing/consuming operations, `throws(E)`, parameter packs, actors, and `Sendable` expose different static obligations. [SWIFT-OPAQUE] [SWIFT-NONCOPY] [SWIFT-THROWS] [SWIFT-PACKS] [SWIFT-ACTORS] [SWIFT-SENDABLE]

Swift 6.2 documents `InlineArray` and lifetime-safe `Span`. Value semantics does not imply eager copying: `Array` can share backing storage until mutation. Existential storage can use inline buffers or out-of-line representation depending on the value and ABI rules. Actor isolation does not make an entire method atomic across `await`; invariants must be re-established after suspension. ARC traffic, copy-on-write separation, and task scheduling must be budgeted separately from static checking. [SWIFT-RELEASE] [SWIFT-ARRAY] [SWIFT-LAYOUT] [SWIFT-CONCURRENCY]

<a id="language-zig"></a>
### Zig

Zig's relevant tools include explicit allocator passing, slices, optionals, error unions, `try`/`catch`, `defer`/`errdefer`, `comptime`, reflection through builtins, and explicit external/packed representations. These support deliberate low-level interfaces and compile-time specialization without requiring inheritance-heavy modeling. [ZIG] [ZIG-ALLOCATOR] [ZIG-ARENA]

A slice does not carry Rust's general lifetime proof, and `defer` does not prevent a freed allocation from remaining reachable through an escaped pointer. Runtime safety settings affect which checks execute. The language-reference and allocator links use rolling sources: method names and APIs should be pinned to the actual compiler release before adopting an example. Explicit allocation is a visibility mechanism, not evidence that an arena or allocator has a bounded latency or no upstream heap dependency. [ZIG]

<a id="language-jai"></a>
### Jai: a metaprogramming reference with narrower evidence

Jai belongs in the comparison principally as a reference point for compile-time execution and programmer-controlled code generation. An original language demonstration explicitly concerns base-language and compile-time execution. It is historical evidence, not a current language specification or ABI contract. [JAI-DEMO]

A sufficiently current, public primary specification was not established for this document. Consequently the census does not present contemporary Jai allocator details, exact reflection APIs, ownership guarantees, generics ABI, or compiler distribution status as verified facts. The corresponding techniques are fully analyzed under staged programming, reflection, allocator composition, and data-oriented layout using independently documented implementations. A Jai implementation assessment should first obtain the exact compiler build and its bundled documentation, then inspect generated code, safety modes, metaprogram permissions, and reproducibility. This limits language-specific certainty; it does not limit the general technique census.

<a id="language-v"></a>
### V

V's documentation distinguishes optional `?T` from fallible `!T`, and describes structural interfaces, sum types, and generics. Its C-backend documentation describes inline option/result payload storage, while error objects and the payloads themselves can introduce allocation. This is more precise than assuming that concise failure syntax is allocation-free throughout an operation. [V-OPTIONS] [V-INTERFACES]

The memory-management documentation identifies default garbage collection and marks `-autofree` as work in progress. These should not be relabeled as Rust-style compile-time lifetime enforcement or unconditional automatic resource safety. Assess the chosen compiler/backend/memory mode explicitly, including foreign calls and generated C when applicable. Generic, interface, and sum-type syntax alone does not establish dispatch cost, stable ABI, or mature toolchain qualification. [V-MEMORY]

<a id="language-nix"></a>
### Nix

Nix is useful here for immutable declarative values, parameterized dependency graphs, and effect separation at the build/configuration level. Its language is lazy and dynamically typed; a function accepting an attribute set does not imply a statically checked row-polymorphic record. A derivation describes a build rather than directly representing a native object. [NIX-LANGUAGE] [NIX-DERIVATIONS]

It is therefore inappropriate to compare Nix attribute sets against C++ records using only `sizeof` or native dispatch overhead. Relevant costs include evaluator memory, deferred work, retained values, dependency closure, and rebuild scope. Explicit dependencies improve auditability, but evaluation purity, sandbox policy, fixed inputs, and deterministic builders remain separate questions. This is complementary tooling for systems software, not a substitute for a low-level ownership type system.

<a id="language-odin"></a>
### Odin

Odin's documented facilities include slices and dynamic arrays, explicit context allocators, tagged unions, distinct types, `bit_set`, structure-of-arrays declarations, and composition through `using`. These are concrete ways to make data shape and storage policy more visible than a hierarchy of heap-allocated objects. [ODIN]

The implicit context is convenient policy plumbing, not equivalent to strict capability confinement or statically proven lifetime ownership. A slice still depends on valid backing storage. `using` is composition/member promotion rather than a mandate for virtual dispatch. SoA helps selected bulk access patterns but changes element-reference semantics and may be worse for record-oriented access. Allocator context, container mutation, and temporary-storage lifetime deserve explicit review.

<a id="language-c3"></a>
### C3

C3 documents fallible optionals, generics, slices, methods, contracts, and dynamic interfaces. Its `T?` failure model should not be translated mechanically into a nullable pointer, and its interface mechanism should not be assigned another language's vtable layout merely because both offer method dispatch. [C3-OPTIONALS] [C3-GENERICS] [C3-INTERFACES] [C3-SPEC]

The useful architectural direction is a C-adjacent interface style with stronger vocabulary and compile-time facilities. It is not a general claim of borrow-checked memory or erased dynamic dispatch. Runtime contracts, error propagation, allocation, and underlying pointer validity remain distinct concerns. Check the specific release, safety configuration, calling convention, and fault conventions at a foreign boundary. [C3-CONTRACTS] [C3-METHODS]

<a id="language-roc"></a>
### Roc

This document uses the official **new-compiler tutorial**, not older syntax interchangeably. It documents `Try(T,E)` with `Ok`/`Err`, postfix `?` propagation, `match`, structural forms, nominal declarations, and pure `->` versus effectful `=>` function types. [ROC-TUTORIAL]

Those function arrows do not by themselves establish a Koka-style algebraic effect-handler system. Likewise, semantic immutability is not a promise that all values have a C-like inline layout or that execution performs no allocation. The tutorial describes a nonblocking compilation workflow: code may run despite diagnostics with a nonzero exit status. A production gate should reject such diagnostics before execution rather than treating a test run as proof that the program passed type checking. Pin the compiler and use its matching tutorial and examples. [ROC-TUTORIAL] [KOKA]

### Additional reference languages and libraries

Haskell/GHC provides particularly direct examples of GADTs, higher-kinded abstraction, and optics; Idris provides dependent indices, erasure, and quantitative usage; Koka provides effect rows and handlers; OCaml provides parameterized modules; Julia demonstrates multiple dispatch; Cats provides accumulated validation. They are included where they distinguish a mechanism unavailable or materially different in the main systems languages. Their presence is explanatory, not a blanket recommendation for a target's runtime or qualification needs. [GHC-GADT] [HASKELL-FUNCTOR] [HASKELL-LENS] [IDRIS-LINEAR] [KOKA] [OCAML-MODULES] [JULIA-METHODS] [CATS-VALIDATED]

---

<a id="section-6"></a>
## VI. Layout models and worked examples

### A. Representation models: guarantees versus useful approximations

For conventional inline storage, a useful first approximation for a sum is the size of its largest alternative plus discriminator and padding. This is not a universal formula: niches can remove a separate discriminator, over-alignment changes padding, and implementation strategies vary. For a product, every field needs representation unless an applicable optimization or language rule permits elision. [RUST-LAYOUT] [CPP-VARIANT]

```text
Illustrative contiguous view:
    descriptor -> first element
    length     = number of accessible elements
    owner      = elsewhere; not encoded by the descriptor alone

Illustrative erased borrowed interface:
    data path       -> concrete object
    operation table -> operations for its hidden concrete type
    lifetime        = a separate contract or static relationship

Illustrative owning resizable sequence:
    data pointer + initialized length + allocation capacity
    element storage and allocator metadata are additional memory

Illustrative closed sum:
    active alternative + suitably aligned alternative storage
    the tag may instead be encoded in an unused payload representation
```

The word “fat pointer” describes pointer-plus-metadata representations, not a single cross-language ABI. Slice length and trait-object dispatch metadata serve different purposes. Go slices additionally expose capacity. Rust's reference documentation distinguishes guaranteed layout facts from current observations about dynamically sized pointees. [GO-SLICES] [RUST-LAYOUT]

An optimized `Option` can still use a sentinel physically: the improvement is that absence is explicit in the type and ordinary clients cannot confuse it with a valid payload. Rust documents no-extra-storage cases for selected payload types. General C++ `optional<T>` and arbitrary Rust `Option<T>` must not be assigned the underlying type's size without checking their specific contracts. [RUST-OPTION] [CPP-OPTIONAL]

### B. C++23: parse bytes into a borrowed algebraic value

The example below uses a deliberately small protocol: one kind byte, a big-endian two-byte payload length, then exactly that many payload bytes. Kind `1` carries data; kind `2` is an acknowledgment with no payload. This is an illustrative protocol designed for the example, not an existing transport specification.

The parser combines M05, C02, D02, D06, and D20. It checks envelope structure before constructing a stronger representation. It performs no payload copy, and its result types require no dynamic storage for these particular alternatives. It does not validate any application-specific meaning inside the payload. Its returned span still borrows the caller's bytes; C++ does not verify that relationship at every later use. [CPP-SPAN] [CPP-EXPECTED] [CPP-VARIANT]

```cpp
/** @file packet_view.cpp
 *  @brief Checked decoding into a borrowed sum type; C++23 example.
 *  @details The frame does not own its input. Its backing storage must outlive
 *           every use of the returned view and remain valid and suitably stable.
 */
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <variant>

/** @brief Recoverable protocol-validation failures. */
enum class ParseError { truncated_header, unknown_kind, length_mismatch,
                        acknowledgment_has_payload };
/** @brief Borrowed bytes from a validated data-frame envelope. */
struct DataFrame { std::span<const std::byte> payload; };
/** @brief An acknowledgment has no payload in this example protocol. */
struct Acknowledgment {};
/** @brief The two admissible frame alternatives. */
using Frame = std::variant<DataFrame, Acknowledgment>;

/** @brief Parse [kind:u8][length:u16 big-endian][payload:length bytes].
 *  @param input A readable byte range; returned data views borrow this range.
 *  @return One frame, or an explicit validation failure.
 *  @details Exactly one complete frame is accepted; trailing data is rejected.
 */
[[nodiscard]] std::expected<Frame, ParseError>
parse_frame(std::span<const std::byte> input) noexcept {
    constexpr std::size_t header_size = 3;
    if (input.size() < header_size) {
        return std::unexpected(ParseError::truncated_header);
    }
    const unsigned int kind = std::to_integer<unsigned int>(input[0]);
    if (kind != 1U && kind != 2U) {
        return std::unexpected(ParseError::unknown_kind);
    }
    const std::size_t length =
        (std::to_integer<std::size_t>(input[1]) << 8U) |
         std::to_integer<std::size_t>(input[2]);
    if (length != input.size() - header_size) {
        return std::unexpected(ParseError::length_mismatch);
    }
    if (kind == 2U) {
        if (length != 0U) {
            return std::unexpected(ParseError::acknowledgment_has_payload);
        }
        return Frame{Acknowledgment{}};
    }
    return Frame{DataFrame{input.subspan(header_size, length)}};
}

/** @brief Run focused behavioral checks and print local ABI observations. */
int main() {
    constexpr std::array data{std::byte{1}, std::byte{0}, std::byte{2},
                              std::byte{0xAA}, std::byte{0xBB}};
    constexpr std::array ack{std::byte{2}, std::byte{0}, std::byte{0}};
    constexpr std::array unknown{std::byte{3}, std::byte{0}, std::byte{0}};
    constexpr std::array bad_length{std::byte{1}, std::byte{0}, std::byte{2}};
    constexpr std::array bad_ack{std::byte{2}, std::byte{0}, std::byte{1},
                                std::byte{0}};
    const auto decoded = parse_frame(data);
    assert(decoded.has_value());
    assert(std::get<DataFrame>(*decoded).payload.data() == data.data() + 3);
    assert(std::get<DataFrame>(*decoded).payload.size() == 2);
    assert(std::holds_alternative<Acknowledgment>(*parse_frame(ack)));
    assert(parse_frame({}).error() == ParseError::truncated_header);
    assert(parse_frame(unknown).error() == ParseError::unknown_kind);
    assert(parse_frame(bad_length).error() == ParseError::length_mismatch);
    assert(parse_frame(bad_ack).error() == ParseError::acknowledgment_has_payload);
    std::cout << "Behavioral checks: passed\n"
              << "sizeof(void*)=" << sizeof(void*) << '\n'
              << "sizeof(span<const byte>)=" << sizeof(std::span<const std::byte>) << '\n'
              << "sizeof(span<const byte,16>)=" << sizeof(std::span<const std::byte,16>) << '\n'
              << "sizeof(unique_ptr<int>)=" << sizeof(std::unique_ptr<int>) << '\n'
              << "sizeof(shared_ptr<int>)=" << sizeof(std::shared_ptr<int>) << '\n'
              << "sizeof(optional<int>)=" << sizeof(std::optional<int>) << '\n'
              << "sizeof(Frame)=" << sizeof(Frame) << '\n'
              << "sizeof(expected<Frame,ParseError>)="
              << sizeof(std::expected<Frame, ParseError>) << '\n';
}
```

The length check subtracts the already-validated header size before comparison rather than adding an untrusted length to a pointer. Envelope validation rejects trailing bytes, making the contract “one exact frame” rather than silently accepting an arbitrary prefix. The `Acknowledgment` alternative cannot carry bytes, so that invariant need not be rechecked by every consumer.

Local compilation and execution produced:

```text
g++ (Debian 14.2.0-19) 14.2.0
x86_64

Command: g++ -std=c++23 -O2 -Wall -Wextra -Wpedantic -Werror examples/packet_view.cpp
Behavioral checks: passed
sizeof(void*)=8
sizeof(span<const byte>)=16
sizeof(span<const byte,16>)=8
sizeof(unique_ptr<int>)=8
sizeof(shared_ptr<int>)=16
sizeof(optional<int>)=8
sizeof(Frame)=24
sizeof(expected<Frame,ParseError>)=32
```

These sizes apply only to the compiler, standard library, target, and options shown. `sizeof(shared_ptr<int>)` excludes the managed allocation and control block. `sizeof(span)` excludes the viewed bytes. `sizeof(Frame)` includes space for the largest alternative and implementation bookkeeping. The test establishes the listed behavior and local observations; it does not establish equivalent assembly to a hand-written baseline, sanitizer coverage, all edge cases, or behavior on an embedded ABI.

### C. Go: a descriptor is not ownership, and typed nil is not a nil interface

The next program demonstrates two distinct issues: a slice shares backing storage, and an interface containing a typed nil pointer still has a dynamic type. The use of `unsafe.Sizeof` is observational; the code does not reinterpret or dereference invalid memory. [GO-SLICES] [GO-FAQ]

```go
// Package main demonstrates descriptor size and typed-nil interface semantics.
package main

import (
    "fmt"
    "unsafe"
)

// Fault is an example concrete error type with a pointer receiver.
type Fault struct{}

// Error implements error, including a receiver that may be nil.
func (*Fault) Error() string { return "example fault" }

func main() {
    var ptr *Fault
    var err error = ptr
    var values []byte
    fmt.Printf("pointer is nil: %t\n", ptr == nil)
    fmt.Printf("interface is nil: %t\n", err == nil)
    fmt.Printf("slice descriptor bytes: %d\n", unsafe.Sizeof(values))
    backing := []int{10, 20, 30}
    view := backing[:2]
    view[0] = 99
    if backing[0] != 99 || err == nil {
        panic("representation check failed")
    }
    fmt.Printf("write through slice changes backing value: %d\n", backing[0])
}
```

Local compilation and execution produced:

```text
go version go1.23.2 linux/amd64

Command: go run examples/go_representation.go
pointer is nil: true
interface is nil: false
slice descriptor bytes: 24
write through slice changes backing value: 99
```

The 24-byte slice descriptor excludes backing-array storage and collector metadata. Slice assignment copies a descriptor, not an independent element array. Consequently a “value was copied” statement must identify whether it means the descriptor, owned contents, or the entire reachable object graph.

### D. Typestate for an external device: model uncertainty instead of hiding it

The following is type-signature pseudocode, not executable code:

```text
configure:
    Unconfigured × ValidatedConfiguration
    -> Result<Idle, (Unconfigured, ConfigurationError)>

start:
    Idle
    -> Result<Starting, (Idle, RejectedBeforeActivation)>

observe_start:
    Starting × HardwareObservation
    -> Running | Idle | RecoveryRequired

request_stop:
    Running
    -> Stopping

observe_stop:
    Stopping × HardwareObservation
    -> Idle | RecoveryRequired
```

The important design decision is not the spelling of the wrappers. An error that occurs before issuing an activation command can safely return `Idle`; an ambiguous observation after activation cannot. A `RecoveryRequired` state prevents ordinary operations until recovery resolves the uncertainty. This is architectural reasoning about the example, not a claim that a type checker independently proves hardware state.

A borrowed DMA buffer adds another dimension: it must remain allocated, sufficiently aligned, mapped, and unavailable for conflicting CPU access for the complete transfer interval. CPU-language pinning addresses selected movement invariants; it does not configure an IOMMU, maintain cache coherency, or lock physical pages. The low-level driver contract must establish those environmental obligations. [RUST-PIN]

### E. Closed sums versus open implementations

Consider either a `variant<Circle, Box>` processed by visitation or a borrowed `dyn Shape`/existential interface. A sum makes the alternative set explicit and can store values inline; an existential makes the operation set explicit and can accept implementations not known by the consumer. Allocation depends on how the values are owned, not only on which dispatch model is chosen. [CPP-VARIANT] [CPP-VARIANT-VISIT] [RUST-DYN]

A practical split is to keep a small closed representation in a hot data-processing core, while using a dynamic interface at a plugin or backend boundary. Converting once at the boundary can be less expensive and easier to verify than carrying erased polymorphism through every inner-loop operation. Conversely, generating a specialized inner loop for every combination of policy types can worsen instruction-cache behavior. Measure both code size and hot-path execution rather than selecting dispatch based on terminology alone.

---

<a id="section-7"></a>
## VII. Engineering selection and migration guide

### Select a representation from the actual obligation

The table is an architectural routing guide, not a universal performance ranking. Start with the invariant or boundary and then select its enforcement mechanism.

| Existing risk or requirement | Candidate techniques | Critical question before adoption |
|---|---|---|
| Ambiguous release responsibility | M01–M03, M08 | Which participant owns destruction, and can access outlive it? |
| Pointer/count disagreement | M05–M07, D20 | Who establishes bounds, lifetime, alignment, and encoding? |
| Shared mutable object graphs | M07, M10–M13, M20–M23, D15 | Is sharing necessary, and how is reclamation synchronized? |
| Frequent transient allocations | M12–M15, D18 | Can lifetimes be grouped or capacity bounded without excessive retention? |
| Poor bulk-data locality | M16–M17, D17 | Which fields are consumed together, and how often does structure change? |
| Address-sensitive asynchronous records | M18, C17–C20 | Which addresses escape, and when is it safe to destroy the state? |
| Sentinel/error-code confusion | C01–C08, D06 | Is the domain absence, exclusive failure, or partial progress plus status? |
| Duplicated cleanup | M01, C09–C10 | What happens on failure, cancellation, exception, and deliberate leak? |
| Callback or vtable overhead | P01–P11, C20 | Is the implementation set open, and is erasure actually on the hot path? |
| Unconstrained generic APIs | P01, P04, P12–P15 | Are required operations sufficient, or do semantic laws need verification? |
| Boolean state soup | D01–D03, D08–D12, D23–D24 | Are states exclusive, independent, hierarchical, or partially observed? |
| Interchangeable primitive parameters | D04–D07, D10 | Which distinctions deserve nominal identity or validated construction? |
| Serialization and FFI fragility | D20–D22 | Which representation and evolution guarantees are explicit? |
| Hidden global services or effects | C22, P26, D13–D14, D21 | What authority and effects does each operation actually require? |
| Boilerplate metadata and adapters | P16–P19 | Is generation tied to stable semantics or accidentally to physical layout? |

### A cost-review sheet for each selected abstraction

Record a concrete answer, evidence, and failure policy for each row. “The compiler optimizes it” is not evidence unless the claim follows from a guarantee or inspected output for the supported configurations.

| Dimension | Review question | Evidence to collect |
|---|---|---|
| Object representation | What does the handle itself store? | Size/alignment, layout documentation, target-specific probes |
| Reachable storage | What allocations and metadata are outside the handle? | Allocation graph, control blocks, arenas, retained backing buffers |
| Lifetime | When does validity begin and end? | Ownership diagram, static relationships, invalidation rules |
| Mutation and aliasing | Which simultaneous access patterns are permitted? | Borrow/synchronization contracts and negative tests |
| Allocation | Which operations can allocate, including error paths? | Allocator instrumentation and explicit no-fallback configuration |
| Movement/copying | What is copied on assignment, call, capture, and growth? | Generated code and large-payload tests |
| Destruction | Can cleanup block, recurse, or release a large graph? | Worst-case ownership population and destruction traces |
| Dispatch | Which calls remain indirect? | IR/assembly and specialization reports |
| Concurrency | What publication, cancellation, and reclamation ordering exists? | Memory-order reasoning, race testing, progress assumptions |
| Failure | What is valid after a failed or interrupted operation? | State transition contract and fault injection |
| Compilation | How does specialization/generation affect builds? | Clean/incremental timing and diagnostic quality |
| Binary and stack | What grows with type combinations or suspension points? | Link map, text/rodata size, stack/frame inspection |
| Boundary | What survives FFI, process, or version changes? | ABI specification, schema tests, foreign-code audit |

For an embedded control path, replace average-only measurements with explicit peak bounds. A function may avoid dynamic allocation in normal operation yet allocate while formatting an error. An arena may be fast until it falls back to an upstream resource. A bounded queue imposes a policy when full: block, reject, drop, overwrite, or degrade. The chosen behavior belongs in the functional contract, not only in a performance note. [CPP-PMR] [CPP-ARENA] [TOKIO-CHANNELS]

### A practical migration sequence

First, inventory existing obligations without changing the architecture: raw owners, retained views, invalidation events, sentinel meanings, implicit phases, external effects, and ABI boundaries. Record which are currently enforced by code, review discipline, tests, or not at all.

Next, replace ambiguous vocabulary at narrow interfaces. An owning handle, a bounded view, a nominal quantity, and a structured error often improve correctness before a broader rewrite. Keep the established C or device boundary intact where required and introduce checked adapters around it. Avoid simultaneously changing storage layout, dispatch, error policy, and protocol semantics unless a staged compatibility test exists.

Then model admissible states and transitions. Decide which distinctions are static, which depend on runtime data, and which represent uncertainty. Validate untrusted inputs before creating invariant-carrying values. Preserve the original resource on failure where the contract permits it, and represent recovery-required states where it does not.

Finally, optimize measured bottlenecks. Introduce SoA, specialization, arenas, or in-place storage only after observing the actual workload and defining the needed bounds. Repeat binary-size, stack, allocation, latency, and failure-path checks after each representation change. This sequence is an engineering recommendation derived from the independent failure modes cataloged above, not a claim that one migration order is universally optimal.

### Misconceptions to reject during review

| Claim | Necessary correction |
|---|---|
| “A span makes the API memory-safe.” | It packages bounds; lifetime and validity enforcement depend on the language and constructor path. |
| “Move means no bytes are copied.” | Move changes ownership semantics or overload selection; inline storage may still be transferred. |
| “A shared pointer makes the object thread-safe.” | Reference-count safety and pointee synchronization are distinct. |
| “An optional removes sentinels.” | It removes sentinel interpretation from ordinary clients; layout may encode absence using a niche. |
| “A result-returning function cannot throw or panic.” | Payload operations and called code can have additional failure channels. |
| “A generic call is always monomorphized.” | Dictionary passing, shape sharing, and witness-mediated code are also implementation strategies. |
| “Value semantics means deep copying.” | Logical independence can be implemented through copy-on-write or structural sharing. |
| “A coroutine has no allocation.” | Frame representation, ownership, and elision depend on implementation and usage. |
| “Cancel means rollback.” | Cancellation can stop future progress without undoing completed effects. |
| “Typestate proves device state.” | It proves permitted software transitions under the environmental assumptions modeled. |
| “Packed layout is a wire format.” | Endianness, validity, schema evolution, and lifetime still need definition. |
| “Immutable implies cheap or real-time-safe.” | Allocation, sharing, reclamation, and traversal costs remain. |

These corrections follow the guarantees and caveats documented for spans, ownership, reference counting, options/results, generic lowering, coroutines, and cancellation in the relevant primary sources. [CPP-SPAN] [RUST-OWNERSHIP] [RUST-ARC] [RUST-OPTION] [CPP-EXPECTED] [GO-DICTIONARIES] [SWIFT-ARRAY] [CPP-COROUTINE] [TOKIO-SELECT]

### Evidence limitations

This census is not an ABI certification, language conformance suite, WCET analysis, or performance benchmark. Local example results cover only the listed compiler/runtime configurations. Most source URLs are live documentation rather than immutable snapshots; record commit hashes and toolchain manifests when turning the census into a project standard. Jai's current implementation details remain deliberately evidence-limited. Experimental or working-draft facilities should be qualified independently before production adoption.

---

<a id="section-8"></a>
## VIII. Terminology and non-equivalences

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

---

<a id="primary-sources"></a>
## Primary-source registry

Research access date: **13 October 2026**. These references support the language, library, and implementation details. The engineering analyses are original synthesis. Rolling documentation may change after this snapshot. Different labels may refer to different sections or uses of one underlying document.

The Jai demonstration is historical evidence only; it is not treated as a current specification. No third-party manuals or source documents are reproduced in this package.

### CPP

- **CPP-ARENA** — [C++ working draft: monotonic_buffer_resource](https://eel.is/c++draft/mem.res.monotonic.buffer).
- **CPP-COMPARE** — [C++ working draft: defaulted comparisons](https://eel.is/c++draft/class.compare.default).
- **CPP-CONSTEXPR** — [C++ working draft: constant expressions](https://eel.is/c++draft/expr.const).
- **CPP-CONSTRAINTS** — [C++ working draft: template constraints](https://eel.is/c++draft/temp.constr).
- **CPP-CONTRACTS** — [C++ working draft: contracts](https://eel.is/c++draft/basic.contract).
- **CPP-COROUTINE** — [C++ working draft: coroutines](https://eel.is/c++draft/dcl.fct.def.coroutine).
- **CPP-CPO** — [C++ working draft: customization point objects](https://eel.is/c++draft/customization.point.object).
- **CPP-DURATION** — [C++ working draft: duration](https://eel.is/c++draft/time.duration).
- **CPP-EXCEPT** — [C++ working draft: exception handling](https://eel.is/c++draft/except).
- **CPP-EXEC** — [C++ working draft: execution control](https://eel.is/c++draft/exec).
- **CPP-EXPECTED** — [C++ working draft: expected](https://eel.is/c++draft/expected).
- **CPP-EXPECTED-MONADIC** — [C++ working draft: expected monadic operations](https://eel.is/c++draft/expected.object.monadic).
- **CPP-FORMAT** — [C++ working draft: formatting](https://eel.is/c++draft/format).
- **CPP-FUNCTION** — [C++ working draft: function declarations and explicit object parameters](https://eel.is/c++draft/dcl.fct).
- **CPP-FUNCTION-ERASURE** — [C++ working draft: function](https://eel.is/c++draft/func.wrap.func).
- **CPP-FUNCTIONREF** — [C++ working draft: function_ref](https://eel.is/c++draft/func.wrap.ref).
- **CPP-GUIDELINES** — [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines).
- **CPP-HIVE** — [C++ working draft: hive](https://eel.is/c++draft/hive).
- **CPP-INDIRECT** — [C++ working draft: indirect](https://eel.is/c++draft/indirect).
- **CPP-INPLACE** — [C++ working draft: inplace_vector](https://eel.is/c++draft/inplace.vector).
- **CPP-INSTANTIATION** — [C++ working draft: template instantiation](https://eel.is/c++draft/temp.inst).
- **CPP-LAMBDA** — [C++ working draft: lambda expressions](https://eel.is/c++draft/expr.prim.lambda).
- **CPP-MDSPAN** — [C++ working draft: multidimensional views](https://eel.is/c++draft/views.multidim).
- **CPP-MODULES** — [C++ working draft: modules](https://eel.is/c++draft/module).
- **CPP-MOVEFUNCTION** — [C++ working draft: move_only_function](https://eel.is/c++draft/func.wrap.move).
- **CPP-NODISCARD** — [C++ working draft: nodiscard](https://eel.is/c++draft/dcl.attr.nodiscard).
- **CPP-NOUNIQUE** — [C++ working draft: no_unique_address](https://eel.is/c++draft/dcl.attr.nouniqueaddr).
- **CPP-ONCE** — [C++ working draft: call_once](https://eel.is/c++draft/thread.once).
- **CPP-OPTIONAL** — [C++ working draft: optional](https://eel.is/c++draft/optional).
- **CPP-OPTIONAL-MONADIC** — [C++ working draft: optional monadic operations](https://eel.is/c++draft/optional.monadic).
- **CPP-PACKS** — [C++ working draft: variadic templates](https://eel.is/c++draft/temp.variadic).
- **CPP-PMR** — [C++ working draft: memory resources](https://eel.is/c++draft/mem.res).
- **CPP-POLYMORPHIC** — [C++ working draft: polymorphic](https://eel.is/c++draft/polymorphic).
- **CPP-RANGE-ACCESS** — [C++ working draft: range access](https://eel.is/c++draft/range.access).
- **CPP-RANGES** — [C++ working draft: ranges](https://eel.is/c++draft/ranges).
- **CPP-REFLECTION** — [C++ working draft: reflection](https://eel.is/c++draft/meta).
- **CPP-SHARED** — [C++ working draft: shared_ptr](https://eel.is/c++draft/util.smartptr.shared).
- **CPP-SIMD** — [C++ working draft: data-parallel types](https://eel.is/c++draft/simd).
- **CPP-SPAN** — [C++ working draft: span](https://eel.is/c++draft/views.span).
- **CPP-SPECIALMEM** — [C++ working draft: specialized memory algorithms](https://eel.is/c++draft/specialized.algorithms).
- **CPP-STOP** — [C++ working draft: stop tokens](https://eel.is/c++draft/thread.stoptoken).
- **CPP-STRINGVIEW** — [C++ working draft: string_view](https://eel.is/c++draft/string.view).
- **CPP-TEMPLATE-PARAMETERS** — [C++ working draft: template parameters](https://eel.is/c++draft/temp.param).
- **CPP-TUPLE** — [C++ working draft: tuples](https://eel.is/c++draft/tuple).
- **CPP-UNIQUE** — [C++ working draft: unique_ptr](https://eel.is/c++draft/unique.ptr).
- **CPP-VARIANT** — [C++ working draft: variant](https://eel.is/c++draft/variant).
- **CPP-VARIANT-VISIT** — [C++ working draft: variant visitation](https://eel.is/c++draft/variant.visit).
- **CPP-VECTOR** — [C++ working draft: vector](https://eel.is/c++draft/vector).

### RUST

- **RUST-ARC** — [Rust standard library: Arc](https://doc.rust-lang.org/std/sync/struct.Arc.html).
- **RUST-ASSOCIATED** — [Rust Reference: associated items](https://doc.rust-lang.org/reference/items/associated-items.html).
- **RUST-BORROW** — [The Rust Book: references and borrowing](https://doc.rust-lang.org/book/ch04-02-references-and-borrowing.html).
- **RUST-BOUNDS** — [Rust Reference: trait and lifetime bounds](https://doc.rust-lang.org/reference/trait-bounds.html).
- **RUST-CELL** — [Rust standard library: interior mutability](https://doc.rust-lang.org/std/cell/index.html).
- **RUST-CLOSURES** — [Rust Reference: closure types](https://doc.rust-lang.org/reference/types/closure.html).
- **RUST-CONST** — [Rust Reference: constant evaluation](https://doc.rust-lang.org/reference/const_eval.html).
- **RUST-CONTROLFLOW** — [Rust standard library: ControlFlow](https://doc.rust-lang.org/std/ops/enum.ControlFlow.html).
- **RUST-COW** — [Rust standard library: Cow](https://doc.rust-lang.org/std/borrow/enum.Cow.html).
- **RUST-CSTR** — [Rust standard library: CStr](https://doc.rust-lang.org/std/ffi/struct.CStr.html).
- **RUST-DERIVE** — [Rust Reference: derive attributes](https://doc.rust-lang.org/reference/attributes/derive.html).
- **RUST-DIAGNOSTICS** — [Rust Reference: diagnostic attributes](https://doc.rust-lang.org/reference/attributes/diagnostics.html).
- **RUST-DROP** — [Rust Reference: destructors](https://doc.rust-lang.org/reference/destructors.html).
- **RUST-DYN** — [Rust Reference: trait objects](https://doc.rust-lang.org/reference/types/trait-object.html).
- **RUST-ENUM** — [Rust Reference: enumerations](https://doc.rust-lang.org/reference/items/enumerations.html).
- **RUST-EQ** — [Rust standard library: Eq laws](https://doc.rust-lang.org/std/cmp/trait.Eq.html).
- **RUST-FORGET** — [Rust standard library: mem::forget](https://doc.rust-lang.org/std/mem/fn.forget.html).
- **RUST-FORMAT** — [Rust standard library: formatting](https://doc.rust-lang.org/std/fmt/index.html).
- **RUST-FUTURE** — [Rust standard library: Future](https://doc.rust-lang.org/std/future/trait.Future.html).
- **RUST-GENERICS** — [Rust Reference: generic parameters](https://doc.rust-lang.org/reference/items/generics.html).
- **RUST-HASH** — [Rust standard library: Hash laws](https://doc.rust-lang.org/std/hash/trait.Hash.html).
- **RUST-IF** — [Rust Reference: if expressions](https://doc.rust-lang.org/reference/expressions/if-expr.html).
- **RUST-IMPLS** — [Rust Reference: implementations and coherence](https://doc.rust-lang.org/reference/items/implementations.html).
- **RUST-IMPLTRAIT** — [Rust Reference: impl Trait](https://doc.rust-lang.org/reference/types/impl-trait.html).
- **RUST-INFALLIBLE** — [Rust standard library: Infallible](https://doc.rust-lang.org/std/convert/enum.Infallible.html).
- **RUST-ITERATOR** — [Rust standard library: Iterator](https://doc.rust-lang.org/std/iter/trait.Iterator.html).
- **RUST-LAYOUT** — [Rust Reference: type layout](https://doc.rust-lang.org/reference/type-layout.html).
- **RUST-LET** — [Rust Reference: statements and let-else](https://doc.rust-lang.org/reference/statements.html).
- **RUST-MACROS** — [Rust Reference: declarative macros](https://doc.rust-lang.org/reference/macros-by-example.html).
- **RUST-MATCH** — [Rust Reference: match expressions](https://doc.rust-lang.org/reference/expressions/match-expr.html).
- **RUST-MAYBEUNINIT** — [Rust standard library: MaybeUninit](https://doc.rust-lang.org/std/mem/union.MaybeUninit.html).
- **RUST-MONOMORPH** — [Rust Compiler Development Guide: monomorphization](https://rustc-dev-guide.rust-lang.org/backend/monomorph.html).
- **RUST-MUTEX** — [Rust standard library: Mutex](https://doc.rust-lang.org/std/sync/struct.Mutex.html).
- **RUST-NEVER** — [Rust Reference: never type](https://doc.rust-lang.org/reference/types/never.html).
- **RUST-NONEXHAUSTIVE** — [Rust Reference: non_exhaustive](https://doc.rust-lang.org/reference/attributes/type_system.html).
- **RUST-NONNULL** — [Rust standard library: NonNull](https://doc.rust-lang.org/std/ptr/struct.NonNull.html).
- **RUST-NONZERO** — [Rust standard library: NonZero](https://doc.rust-lang.org/std/num/struct.NonZero.html).
- **RUST-ONCE** — [Rust standard library: OnceLock](https://doc.rust-lang.org/std/sync/struct.OnceLock.html).
- **RUST-OPTION** — [Rust standard library: Option and representation guarantees](https://doc.rust-lang.org/std/option/index.html).
- **RUST-OWNERSHIP** — [The Rust Book: ownership](https://doc.rust-lang.org/book/ch04-01-what-is-ownership.html).
- **RUST-PATH** — [Rust standard library: paths](https://doc.rust-lang.org/std/path/index.html).
- **RUST-PHANTOM** — [Rust standard library: PhantomData](https://doc.rust-lang.org/std/marker/struct.PhantomData.html).
- **RUST-PIN** — [Rust standard library: pinning](https://doc.rust-lang.org/std/pin/index.html).
- **RUST-PRIVACY** — [Rust Reference: visibility and privacy](https://doc.rust-lang.org/reference/visibility-and-privacy.html).
- **RUST-PROCMACROS** — [Rust Reference: procedural macros](https://doc.rust-lang.org/reference/procedural-macros.html).
- **RUST-PTR** — [Rust standard library: pointers and provenance](https://doc.rust-lang.org/std/ptr/index.html).
- **RUST-RC** — [Rust standard library: Rc](https://doc.rust-lang.org/std/rc/index.html).
- **RUST-RESULT** — [Rust standard library: Result](https://doc.rust-lang.org/std/result/index.html).
- **RUST-RESULT-METHODS** — [Rust standard library: Result methods](https://doc.rust-lang.org/std/result/enum.Result.html).
- **RUST-SCOPE** — [Rust standard library: scoped threads](https://doc.rust-lang.org/std/thread/fn.scope.html).
- **RUST-SEND** — [Rust standard library: Send](https://doc.rust-lang.org/std/marker/trait.Send.html).
- **RUST-SLICE** — [Rust standard library: slices](https://doc.rust-lang.org/std/primitive.slice.html).
- **RUST-STR** — [Rust standard library: string slices](https://doc.rust-lang.org/std/str/index.html).
- **RUST-SYNC** — [Rust standard library: Sync](https://doc.rust-lang.org/std/marker/trait.Sync.html).
- **RUST-TRAITS** — [Rust Reference: traits](https://doc.rust-lang.org/reference/items/traits.html).
- **RUST-TRY** — [Rust Reference: operator expressions and propagation](https://doc.rust-lang.org/reference/expressions/operator-expr.html).
- **RUST-TRYFROM** — [Rust standard library: TryFrom](https://doc.rust-lang.org/std/convert/trait.TryFrom.html).
- **RUST-TUPLE** — [Rust Reference: tuple types](https://doc.rust-lang.org/reference/types/tuple.html).
- **RUST-U32** — [Rust standard library: integer arithmetic policies](https://doc.rust-lang.org/std/primitive.u32.html).
- **RUST-UNSAFE** — [Rust Reference: unsafety](https://doc.rust-lang.org/reference/unsafety.html).
- **RUST-VARIANCE** — [Rust Reference: subtyping and variance](https://doc.rust-lang.org/reference/subtyping.html).
- **RUST-VEC** — [Rust standard library: Vec](https://doc.rust-lang.org/std/vec/struct.Vec.html).

### SWIFT

- **SWIFT-ACTORS** — [Swift Evolution SE-0306: actors](https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0306-actors.md).
- **SWIFT-ARRAY** — [Swift standard library: Array implementation and documentation](https://raw.githubusercontent.com/swiftlang/swift/main/stdlib/public/core/Array.swift).
- **SWIFT-ASSOCIATED** — [The Swift Programming Language: generics and associated types](https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Generics.md).
- **SWIFT-CALLING** — [Swift ABI: calling conventions](https://raw.githubusercontent.com/swiftlang/swift/main/docs/ABI/CallingConvention.rst).
- **SWIFT-CLOSURES** — [The Swift Programming Language: closures](https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Closures.md).
- **SWIFT-CONCURRENCY** — [The Swift Programming Language: concurrency](https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Concurrency.md).
- **SWIFT-CONTROL** — [The Swift Programming Language: control flow](https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/ControlFlow.md).
- **SWIFT-EXTENSIONS** — [The Swift Programming Language: extensions](https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Extensions.md).
- **SWIFT-INLINE** — [Swift standard library: InlineArray](https://developer.apple.com/documentation/swift/inlinearray).
- **SWIFT-LAYOUT** — [Swift ABI: type layout](https://raw.githubusercontent.com/swiftlang/swift/main/docs/ABI/TypeLayout.rst).
- **SWIFT-MACROS** — [The Swift Programming Language: macros](https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Macros.md).
- **SWIFT-NONCOPY** — [Swift Evolution SE-0390: noncopyable structs and enums](https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0390-noncopyable-structs-and-enums.md).
- **SWIFT-OPAQUE** — [The Swift Programming Language: opaque and boxed protocol types](https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/OpaqueTypes.md).
- **SWIFT-PACKS** — [Swift Evolution SE-0393: parameter packs](https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0393-parameter-packs.md).
- **SWIFT-PROTOCOLS** — [The Swift Programming Language: protocols](https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Protocols.md).
- **SWIFT-RELEASE** — [Swift 6.2 release: systems programming facilities](https://www.swift.org/blog/swift-6.2-released/).
- **SWIFT-SENDABLE** — [Swift Evolution SE-0302: Sendable](https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0302-concurrent-value-and-concurrent-closures.md).
- **SWIFT-SPAN** — [Swift 6.2 release: Span and lifetime safety](https://www.swift.org/blog/swift-6.2-released/).
- **SWIFT-THROWS** — [Swift Evolution SE-0413: typed throws](https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0413-typed-throws.md).

### GO

- **GO-CONTEXT** — [Go standard library: context](https://pkg.go.dev/context).
- **GO-DEFER** — [Go: defer, panic, and recover](https://go.dev/blog/defer-panic-and-recover).
- **GO-DICTIONARIES** — [Go compiler design: generic dictionaries and gcshape sharing](https://raw.githubusercontent.com/golang/proposal/master/design/generics-implementation-dictionaries-go1.18.md).
- **GO-FAQ** — [Go frequently asked questions](https://go.dev/doc/faq).
- **GO-GC** — [Go garbage collector guide](https://go.dev/doc/gc-guide).
- **GO-INTERFACES** — [Go specification: interface types](https://go.dev/ref/spec#Interface_types).
- **GO-IO** — [Go standard library: io](https://pkg.go.dev/io).
- **GO-ITER** — [Go standard library: iter](https://pkg.go.dev/iter).
- **GO-ONCE** — [Go standard library: synchronization and once](https://pkg.go.dev/sync).
- **GO-PACKAGES** — [Go: code organization and packages](https://go.dev/doc/code).
- **GO-REFLECTION** — [Go: the laws of reflection](https://go.dev/blog/laws-of-reflection).
- **GO-SLICES** — [Go slices: usage and internals](https://go.dev/blog/slices-intro).
- **GO-SPEC** — [Go language specification](https://go.dev/ref/spec).

### Other languages, libraries, and research

- **BITFLAGS** — [bitflags crate documentation](https://docs.rs/bitflags/latest/bitflags/).
- **BUMPALO** — [bumpalo crate documentation](https://docs.rs/bumpalo/latest/bumpalo/).
- **C3-CONTRACTS** — [C3 documentation: contracts](https://c3-lang.org/language-common/contracts/).
- **C3-GENERICS** — [C3 documentation: generics](https://c3-lang.org/generic-programming/generics/).
- **C3-INTERFACES** — [C3 documentation: interfaces and any](https://c3-lang.org/generic-programming/anyinterfaces/).
- **C3-METHODS** — [C3 documentation: functions and methods](https://c3-lang.org/language-fundamentals/functions/).
- **C3-OPTIONALS** — [C3 documentation: fallible optionals](https://c3-lang.org/language-common/optionals-essential/).
- **C3-SPEC** — [C3 language specification](https://c3-lang.org/implementation-details/specification/).
- **CAP-STD** — [cap-std capability-oriented standard library](https://docs.rs/cap-std/latest/cap_std/).
- **CATS-TYPECLASSES** — [Cats: type classes](https://typelevel.org/cats/typeclasses.html).
- **CATS-VALIDATED** — [Cats: Validated and accumulated validation](https://typelevel.org/cats/datatypes/validated.html).
- **CROSSBEAM-EPOCH** — [Crossbeam epoch-based reclamation documentation](https://docs.rs/crossbeam-epoch/latest/crossbeam_epoch/).
- **FLECS-DESIGN** — [Flecs official design guide](https://raw.githubusercontent.com/SanderMertens/flecs/master/docs/DesignWithFlecs.md).
- **GHC-GADT** — [GHC User's Guide: generalized algebraic data types](https://ghc.gitlab.haskell.org/ghc/doc/users_guide/exts/gadt.html).
- **HASKELL-FUNCTOR** — [Haskell 2010 report: predefined types and classes](https://www.haskell.org/onlinereport/haskell2010/haskellch6.html).
- **HASKELL-LENS** — [lens package: lenses, folds, and traversals](https://hackage.haskell.org/package/lens).
- **HAZARD-PAPER** — [Maged M. Michael: Hazard Pointers, IEEE TPDS, 2004; IBM Research record](https://research.ibm.com/publications/hazard-pointers-safe-memory-reclamation-for-lock-free-objects).
- **HEAPLESS** — [heapless crate documentation](https://docs.rs/heapless/latest/heapless/).
- **IDRIS-LINEAR** — [Idris 2 tutorial: multiplicities and erasure](https://idris2.readthedocs.io/en/latest/tutorial/multiplicities.html).
- **IM-COLLECTIONS** — [im: persistent immutable collection documentation](https://docs.rs/im/latest/im/).
- **JAI-DEMO** — [Original Jai demonstration: Base language, compile-time execution; historical evidence only](https://youtu.be/UTqZNujQOlA).
- **JULIA-METHODS** — [Julia manual: methods and multiple dispatch](https://docs.julialang.org/en/v1/manual/methods/).
- **KOKA** — [Koka language book](https://koka-lang.github.io/koka/doc/book.html).
- **LIQUIDHASKELL** — [Liquid Haskell tutorial by the project authors](https://ucsd-progsys.github.io/liquidhaskell-tutorial/).
- **LLVM-CONTAINERS** — [LLVM Programmer's Manual: container selection](https://llvm.org/docs/ProgrammersManual.html).
- **NIX-DERIVATIONS** — [Nix 2.34 reference manual: derivations](https://nix.dev/manual/nix/2.34/language/derivations).
- **NIX-LANGUAGE** — [Nix 2.34 reference manual: language](https://nix.dev/manual/nix/2.34/language/).
- **NOM** — [nom parser-combinator documentation](https://docs.rs/nom/latest/nom/).
- **OCAML-MODULES** — [OCaml 5.3 manual: modules and functors](https://ocaml.org/manual/5.3/moduleexamples.html).
- **ODIN** — [Odin language overview](https://odin-lang.org/docs/overview/).
- **PROTOBUF** — [Protocol Buffers official proto3 language guide](https://protobuf.dev/programming-guides/proto3/).
- **ROC-TUTORIAL** — [Roc official new-compiler tutorial](https://raw.githubusercontent.com/roc-lang/roc/main/docs/mini-tutorial-new-compiler.md).
- **SCOPEGUARD** — [scopeguard crate documentation](https://docs.rs/scopeguard/latest/scopeguard/).
- **SESH** — [sesh: session types with safe session composition](https://docs.rs/sesh/latest/sesh/).
- **SLOTMAP** — [slotmap crate documentation](https://docs.rs/slotmap/latest/slotmap/).
- **SMALLVEC** — [smallvec crate documentation](https://docs.rs/smallvec/latest/smallvec/).
- **STATECHARTS** — [Stately/XState documentation: state machines and statecharts](https://stately.ai/docs/state-machines-and-statecharts).
- **TOKIO-CHANNELS** — [Tokio tutorial: channels](https://tokio.rs/tokio/tutorial/channels).
- **TOKIO-SELECT** — [Tokio: select and cancellation safety](https://docs.rs/tokio/latest/tokio/macro.select.html).
- **V-INTERFACES** — [V documentation: interfaces](https://docs.vlang.io/type-declarations.html#interfaces).
- **V-MEMORY** — [V documentation: memory management](https://docs.vlang.io/memory-management.html).
- **V-OPTIONS** — [V documentation: options, results, sum types, and generics](https://docs.vlang.io/type-declarations.html).
- **ZEROCOPY** — [zerocopy: checked byte reinterpretation and layout traits](https://docs.rs/zerocopy/latest/zerocopy/).
- **ZIG** — [Zig language reference, master snapshot](https://ziglang.org/documentation/master/).
- **ZIG-ALLOCATOR** — [Zig standard library: Allocator source](https://raw.githubusercontent.com/ziglang/zig/master/lib/std/mem/Allocator.zig).
- **ZIG-ARENA** — [Zig standard library: arena allocator source](https://raw.githubusercontent.com/ziglang/zig/master/lib/std/heap/arena_allocator.zig).


<!-- REFERENCE DEFINITIONS -->
[BITFLAGS]: https://docs.rs/bitflags/latest/bitflags/ "bitflags crate documentation"
[BUMPALO]: https://docs.rs/bumpalo/latest/bumpalo/ "bumpalo crate documentation"
[C3-CONTRACTS]: https://c3-lang.org/language-common/contracts/ "C3 documentation: contracts"
[C3-GENERICS]: https://c3-lang.org/generic-programming/generics/ "C3 documentation: generics"
[C3-INTERFACES]: https://c3-lang.org/generic-programming/anyinterfaces/ "C3 documentation: interfaces and any"
[C3-METHODS]: https://c3-lang.org/language-fundamentals/functions/ "C3 documentation: functions and methods"
[C3-OPTIONALS]: https://c3-lang.org/language-common/optionals-essential/ "C3 documentation: fallible optionals"
[C3-SPEC]: https://c3-lang.org/implementation-details/specification/ "C3 language specification"
[CAP-STD]: https://docs.rs/cap-std/latest/cap_std/ "cap-std capability-oriented standard library"
[CATS-TYPECLASSES]: https://typelevel.org/cats/typeclasses.html "Cats: type classes"
[CATS-VALIDATED]: https://typelevel.org/cats/datatypes/validated.html "Cats: Validated and accumulated validation"
[CPP-ARENA]: https://eel.is/c++draft/mem.res.monotonic.buffer "C++ working draft: monotonic_buffer_resource"
[CPP-COMPARE]: https://eel.is/c++draft/class.compare.default "C++ working draft: defaulted comparisons"
[CPP-CONSTEXPR]: https://eel.is/c++draft/expr.const "C++ working draft: constant expressions"
[CPP-CONSTRAINTS]: https://eel.is/c++draft/temp.constr "C++ working draft: template constraints"
[CPP-CONTRACTS]: https://eel.is/c++draft/basic.contract "C++ working draft: contracts"
[CPP-COROUTINE]: https://eel.is/c++draft/dcl.fct.def.coroutine "C++ working draft: coroutines"
[CPP-CPO]: https://eel.is/c++draft/customization.point.object "C++ working draft: customization point objects"
[CPP-DURATION]: https://eel.is/c++draft/time.duration "C++ working draft: duration"
[CPP-EXCEPT]: https://eel.is/c++draft/except "C++ working draft: exception handling"
[CPP-EXEC]: https://eel.is/c++draft/exec "C++ working draft: execution control"
[CPP-EXPECTED]: https://eel.is/c++draft/expected "C++ working draft: expected"
[CPP-EXPECTED-MONADIC]: https://eel.is/c++draft/expected.object.monadic "C++ working draft: expected monadic operations"
[CPP-FORMAT]: https://eel.is/c++draft/format "C++ working draft: formatting"
[CPP-FUNCTION]: https://eel.is/c++draft/dcl.fct "C++ working draft: function declarations and explicit object parameters"
[CPP-FUNCTION-ERASURE]: https://eel.is/c++draft/func.wrap.func "C++ working draft: function"
[CPP-FUNCTIONREF]: https://eel.is/c++draft/func.wrap.ref "C++ working draft: function_ref"
[CPP-GUIDELINES]: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines "C++ Core Guidelines"
[CPP-HIVE]: https://eel.is/c++draft/hive "C++ working draft: hive"
[CPP-INDIRECT]: https://eel.is/c++draft/indirect "C++ working draft: indirect"
[CPP-INPLACE]: https://eel.is/c++draft/inplace.vector "C++ working draft: inplace_vector"
[CPP-INSTANTIATION]: https://eel.is/c++draft/temp.inst "C++ working draft: template instantiation"
[CPP-LAMBDA]: https://eel.is/c++draft/expr.prim.lambda "C++ working draft: lambda expressions"
[CPP-MDSPAN]: https://eel.is/c++draft/views.multidim "C++ working draft: multidimensional views"
[CPP-MODULES]: https://eel.is/c++draft/module "C++ working draft: modules"
[CPP-MOVEFUNCTION]: https://eel.is/c++draft/func.wrap.move "C++ working draft: move_only_function"
[CPP-NODISCARD]: https://eel.is/c++draft/dcl.attr.nodiscard "C++ working draft: nodiscard"
[CPP-NOUNIQUE]: https://eel.is/c++draft/dcl.attr.nouniqueaddr "C++ working draft: no_unique_address"
[CPP-ONCE]: https://eel.is/c++draft/thread.once "C++ working draft: call_once"
[CPP-OPTIONAL]: https://eel.is/c++draft/optional "C++ working draft: optional"
[CPP-OPTIONAL-MONADIC]: https://eel.is/c++draft/optional.monadic "C++ working draft: optional monadic operations"
[CPP-PACKS]: https://eel.is/c++draft/temp.variadic "C++ working draft: variadic templates"
[CPP-PMR]: https://eel.is/c++draft/mem.res "C++ working draft: memory resources"
[CPP-POLYMORPHIC]: https://eel.is/c++draft/polymorphic "C++ working draft: polymorphic"
[CPP-RANGE-ACCESS]: https://eel.is/c++draft/range.access "C++ working draft: range access"
[CPP-RANGES]: https://eel.is/c++draft/ranges "C++ working draft: ranges"
[CPP-REFLECTION]: https://eel.is/c++draft/meta "C++ working draft: reflection"
[CPP-SHARED]: https://eel.is/c++draft/util.smartptr.shared "C++ working draft: shared_ptr"
[CPP-SIMD]: https://eel.is/c++draft/simd "C++ working draft: data-parallel types"
[CPP-SPAN]: https://eel.is/c++draft/views.span "C++ working draft: span"
[CPP-SPECIALMEM]: https://eel.is/c++draft/specialized.algorithms "C++ working draft: specialized memory algorithms"
[CPP-STOP]: https://eel.is/c++draft/thread.stoptoken "C++ working draft: stop tokens"
[CPP-STRINGVIEW]: https://eel.is/c++draft/string.view "C++ working draft: string_view"
[CPP-TEMPLATE-PARAMETERS]: https://eel.is/c++draft/temp.param "C++ working draft: template parameters"
[CPP-TUPLE]: https://eel.is/c++draft/tuple "C++ working draft: tuples"
[CPP-UNIQUE]: https://eel.is/c++draft/unique.ptr "C++ working draft: unique_ptr"
[CPP-VARIANT]: https://eel.is/c++draft/variant "C++ working draft: variant"
[CPP-VARIANT-VISIT]: https://eel.is/c++draft/variant.visit "C++ working draft: variant visitation"
[CPP-VECTOR]: https://eel.is/c++draft/vector "C++ working draft: vector"
[CROSSBEAM-EPOCH]: https://docs.rs/crossbeam-epoch/latest/crossbeam_epoch/ "Crossbeam epoch-based reclamation documentation"
[FLECS-DESIGN]: https://raw.githubusercontent.com/SanderMertens/flecs/master/docs/DesignWithFlecs.md "Flecs official design guide"
[GHC-GADT]: https://ghc.gitlab.haskell.org/ghc/doc/users_guide/exts/gadt.html "GHC User's Guide: generalized algebraic data types"
[GO-CONTEXT]: https://pkg.go.dev/context "Go standard library: context"
[GO-DEFER]: https://go.dev/blog/defer-panic-and-recover "Go: defer, panic, and recover"
[GO-DICTIONARIES]: https://raw.githubusercontent.com/golang/proposal/master/design/generics-implementation-dictionaries-go1.18.md "Go compiler design: generic dictionaries and gcshape sharing"
[GO-FAQ]: https://go.dev/doc/faq "Go frequently asked questions"
[GO-GC]: https://go.dev/doc/gc-guide "Go garbage collector guide"
[GO-INTERFACES]: https://go.dev/ref/spec#Interface_types "Go specification: interface types"
[GO-IO]: https://pkg.go.dev/io "Go standard library: io"
[GO-ITER]: https://pkg.go.dev/iter "Go standard library: iter"
[GO-ONCE]: https://pkg.go.dev/sync "Go standard library: synchronization and once"
[GO-PACKAGES]: https://go.dev/doc/code "Go: code organization and packages"
[GO-REFLECTION]: https://go.dev/blog/laws-of-reflection "Go: the laws of reflection"
[GO-SLICES]: https://go.dev/blog/slices-intro "Go slices: usage and internals"
[GO-SPEC]: https://go.dev/ref/spec "Go language specification"
[HASKELL-FUNCTOR]: https://www.haskell.org/onlinereport/haskell2010/haskellch6.html "Haskell 2010 report: predefined types and classes"
[HASKELL-LENS]: https://hackage.haskell.org/package/lens "lens package: lenses, folds, and traversals"
[HAZARD-PAPER]: https://research.ibm.com/publications/hazard-pointers-safe-memory-reclamation-for-lock-free-objects "Maged M. Michael: Hazard Pointers, IEEE TPDS, 2004; IBM Research record"
[HEAPLESS]: https://docs.rs/heapless/latest/heapless/ "heapless crate documentation"
[IDRIS-LINEAR]: https://idris2.readthedocs.io/en/latest/tutorial/multiplicities.html "Idris 2 tutorial: multiplicities and erasure"
[IM-COLLECTIONS]: https://docs.rs/im/latest/im/ "im: persistent immutable collection documentation"
[JAI-DEMO]: https://youtu.be/UTqZNujQOlA "Original Jai demonstration: Base language, compile-time execution; historical evidence only"
[JULIA-METHODS]: https://docs.julialang.org/en/v1/manual/methods/ "Julia manual: methods and multiple dispatch"
[KOKA]: https://koka-lang.github.io/koka/doc/book.html "Koka language book"
[LIQUIDHASKELL]: https://ucsd-progsys.github.io/liquidhaskell-tutorial/ "Liquid Haskell tutorial by the project authors"
[LLVM-CONTAINERS]: https://llvm.org/docs/ProgrammersManual.html "LLVM Programmer's Manual: container selection"
[NIX-DERIVATIONS]: https://nix.dev/manual/nix/2.34/language/derivations "Nix 2.34 reference manual: derivations"
[NIX-LANGUAGE]: https://nix.dev/manual/nix/2.34/language/ "Nix 2.34 reference manual: language"
[NOM]: https://docs.rs/nom/latest/nom/ "nom parser-combinator documentation"
[OCAML-MODULES]: https://ocaml.org/manual/5.3/moduleexamples.html "OCaml 5.3 manual: modules and functors"
[ODIN]: https://odin-lang.org/docs/overview/ "Odin language overview"
[PROTOBUF]: https://protobuf.dev/programming-guides/proto3/ "Protocol Buffers official proto3 language guide"
[ROC-TUTORIAL]: https://raw.githubusercontent.com/roc-lang/roc/main/docs/mini-tutorial-new-compiler.md "Roc official new-compiler tutorial"
[RUST-ARC]: https://doc.rust-lang.org/std/sync/struct.Arc.html "Rust standard library: Arc"
[RUST-ASSOCIATED]: https://doc.rust-lang.org/reference/items/associated-items.html "Rust Reference: associated items"
[RUST-BORROW]: https://doc.rust-lang.org/book/ch04-02-references-and-borrowing.html "The Rust Book: references and borrowing"
[RUST-BOUNDS]: https://doc.rust-lang.org/reference/trait-bounds.html "Rust Reference: trait and lifetime bounds"
[RUST-CELL]: https://doc.rust-lang.org/std/cell/index.html "Rust standard library: interior mutability"
[RUST-CLOSURES]: https://doc.rust-lang.org/reference/types/closure.html "Rust Reference: closure types"
[RUST-CONST]: https://doc.rust-lang.org/reference/const_eval.html "Rust Reference: constant evaluation"
[RUST-CONTROLFLOW]: https://doc.rust-lang.org/std/ops/enum.ControlFlow.html "Rust standard library: ControlFlow"
[RUST-COW]: https://doc.rust-lang.org/std/borrow/enum.Cow.html "Rust standard library: Cow"
[RUST-CSTR]: https://doc.rust-lang.org/std/ffi/struct.CStr.html "Rust standard library: CStr"
[RUST-DERIVE]: https://doc.rust-lang.org/reference/attributes/derive.html "Rust Reference: derive attributes"
[RUST-DIAGNOSTICS]: https://doc.rust-lang.org/reference/attributes/diagnostics.html "Rust Reference: diagnostic attributes"
[RUST-DROP]: https://doc.rust-lang.org/reference/destructors.html "Rust Reference: destructors"
[RUST-DYN]: https://doc.rust-lang.org/reference/types/trait-object.html "Rust Reference: trait objects"
[RUST-ENUM]: https://doc.rust-lang.org/reference/items/enumerations.html "Rust Reference: enumerations"
[RUST-EQ]: https://doc.rust-lang.org/std/cmp/trait.Eq.html "Rust standard library: Eq laws"
[RUST-FORGET]: https://doc.rust-lang.org/std/mem/fn.forget.html "Rust standard library: mem::forget"
[RUST-FORMAT]: https://doc.rust-lang.org/std/fmt/index.html "Rust standard library: formatting"
[RUST-FUTURE]: https://doc.rust-lang.org/std/future/trait.Future.html "Rust standard library: Future"
[RUST-GENERICS]: https://doc.rust-lang.org/reference/items/generics.html "Rust Reference: generic parameters"
[RUST-HASH]: https://doc.rust-lang.org/std/hash/trait.Hash.html "Rust standard library: Hash laws"
[RUST-IF]: https://doc.rust-lang.org/reference/expressions/if-expr.html "Rust Reference: if expressions"
[RUST-IMPLS]: https://doc.rust-lang.org/reference/items/implementations.html "Rust Reference: implementations and coherence"
[RUST-IMPLTRAIT]: https://doc.rust-lang.org/reference/types/impl-trait.html "Rust Reference: impl Trait"
[RUST-INFALLIBLE]: https://doc.rust-lang.org/std/convert/enum.Infallible.html "Rust standard library: Infallible"
[RUST-ITERATOR]: https://doc.rust-lang.org/std/iter/trait.Iterator.html "Rust standard library: Iterator"
[RUST-LAYOUT]: https://doc.rust-lang.org/reference/type-layout.html "Rust Reference: type layout"
[RUST-LET]: https://doc.rust-lang.org/reference/statements.html "Rust Reference: statements and let-else"
[RUST-MACROS]: https://doc.rust-lang.org/reference/macros-by-example.html "Rust Reference: declarative macros"
[RUST-MATCH]: https://doc.rust-lang.org/reference/expressions/match-expr.html "Rust Reference: match expressions"
[RUST-MAYBEUNINIT]: https://doc.rust-lang.org/std/mem/union.MaybeUninit.html "Rust standard library: MaybeUninit"
[RUST-MONOMORPH]: https://rustc-dev-guide.rust-lang.org/backend/monomorph.html "Rust Compiler Development Guide: monomorphization"
[RUST-MUTEX]: https://doc.rust-lang.org/std/sync/struct.Mutex.html "Rust standard library: Mutex"
[RUST-NEVER]: https://doc.rust-lang.org/reference/types/never.html "Rust Reference: never type"
[RUST-NONEXHAUSTIVE]: https://doc.rust-lang.org/reference/attributes/type_system.html "Rust Reference: non_exhaustive"
[RUST-NONNULL]: https://doc.rust-lang.org/std/ptr/struct.NonNull.html "Rust standard library: NonNull"
[RUST-NONZERO]: https://doc.rust-lang.org/std/num/struct.NonZero.html "Rust standard library: NonZero"
[RUST-ONCE]: https://doc.rust-lang.org/std/sync/struct.OnceLock.html "Rust standard library: OnceLock"
[RUST-OPTION]: https://doc.rust-lang.org/std/option/index.html "Rust standard library: Option and representation guarantees"
[RUST-OWNERSHIP]: https://doc.rust-lang.org/book/ch04-01-what-is-ownership.html "The Rust Book: ownership"
[RUST-PATH]: https://doc.rust-lang.org/std/path/index.html "Rust standard library: paths"
[RUST-PHANTOM]: https://doc.rust-lang.org/std/marker/struct.PhantomData.html "Rust standard library: PhantomData"
[RUST-PIN]: https://doc.rust-lang.org/std/pin/index.html "Rust standard library: pinning"
[RUST-PRIVACY]: https://doc.rust-lang.org/reference/visibility-and-privacy.html "Rust Reference: visibility and privacy"
[RUST-PROCMACROS]: https://doc.rust-lang.org/reference/procedural-macros.html "Rust Reference: procedural macros"
[RUST-PTR]: https://doc.rust-lang.org/std/ptr/index.html "Rust standard library: pointers and provenance"
[RUST-RC]: https://doc.rust-lang.org/std/rc/index.html "Rust standard library: Rc"
[RUST-RESULT]: https://doc.rust-lang.org/std/result/index.html "Rust standard library: Result"
[RUST-RESULT-METHODS]: https://doc.rust-lang.org/std/result/enum.Result.html "Rust standard library: Result methods"
[RUST-SCOPE]: https://doc.rust-lang.org/std/thread/fn.scope.html "Rust standard library: scoped threads"
[RUST-SEND]: https://doc.rust-lang.org/std/marker/trait.Send.html "Rust standard library: Send"
[RUST-SLICE]: https://doc.rust-lang.org/std/primitive.slice.html "Rust standard library: slices"
[RUST-STR]: https://doc.rust-lang.org/std/str/index.html "Rust standard library: string slices"
[RUST-SYNC]: https://doc.rust-lang.org/std/marker/trait.Sync.html "Rust standard library: Sync"
[RUST-TRAITS]: https://doc.rust-lang.org/reference/items/traits.html "Rust Reference: traits"
[RUST-TRY]: https://doc.rust-lang.org/reference/expressions/operator-expr.html "Rust Reference: operator expressions and propagation"
[RUST-TRYFROM]: https://doc.rust-lang.org/std/convert/trait.TryFrom.html "Rust standard library: TryFrom"
[RUST-TUPLE]: https://doc.rust-lang.org/reference/types/tuple.html "Rust Reference: tuple types"
[RUST-U32]: https://doc.rust-lang.org/std/primitive.u32.html "Rust standard library: integer arithmetic policies"
[RUST-UNSAFE]: https://doc.rust-lang.org/reference/unsafety.html "Rust Reference: unsafety"
[RUST-VARIANCE]: https://doc.rust-lang.org/reference/subtyping.html "Rust Reference: subtyping and variance"
[RUST-VEC]: https://doc.rust-lang.org/std/vec/struct.Vec.html "Rust standard library: Vec"
[SCOPEGUARD]: https://docs.rs/scopeguard/latest/scopeguard/ "scopeguard crate documentation"
[SESH]: https://docs.rs/sesh/latest/sesh/ "sesh: session types with safe session composition"
[SLOTMAP]: https://docs.rs/slotmap/latest/slotmap/ "slotmap crate documentation"
[SMALLVEC]: https://docs.rs/smallvec/latest/smallvec/ "smallvec crate documentation"
[STATECHARTS]: https://stately.ai/docs/state-machines-and-statecharts "Stately/XState documentation: state machines and statecharts"
[SWIFT-ACTORS]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0306-actors.md "Swift Evolution SE-0306: actors"
[SWIFT-ARRAY]: https://raw.githubusercontent.com/swiftlang/swift/main/stdlib/public/core/Array.swift "Swift standard library: Array implementation and documentation"
[SWIFT-ASSOCIATED]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Generics.md "The Swift Programming Language: generics and associated types"
[SWIFT-CALLING]: https://raw.githubusercontent.com/swiftlang/swift/main/docs/ABI/CallingConvention.rst "Swift ABI: calling conventions"
[SWIFT-CLOSURES]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Closures.md "The Swift Programming Language: closures"
[SWIFT-CONCURRENCY]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Concurrency.md "The Swift Programming Language: concurrency"
[SWIFT-CONTROL]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/ControlFlow.md "The Swift Programming Language: control flow"
[SWIFT-EXTENSIONS]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Extensions.md "The Swift Programming Language: extensions"
[SWIFT-INLINE]: https://developer.apple.com/documentation/swift/inlinearray "Swift standard library: InlineArray"
[SWIFT-LAYOUT]: https://raw.githubusercontent.com/swiftlang/swift/main/docs/ABI/TypeLayout.rst "Swift ABI: type layout"
[SWIFT-MACROS]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Macros.md "The Swift Programming Language: macros"
[SWIFT-NONCOPY]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0390-noncopyable-structs-and-enums.md "Swift Evolution SE-0390: noncopyable structs and enums"
[SWIFT-OPAQUE]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/OpaqueTypes.md "The Swift Programming Language: opaque and boxed protocol types"
[SWIFT-PACKS]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0393-parameter-packs.md "Swift Evolution SE-0393: parameter packs"
[SWIFT-PROTOCOLS]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Protocols.md "The Swift Programming Language: protocols"
[SWIFT-RELEASE]: https://www.swift.org/blog/swift-6.2-released/ "Swift 6.2 release: systems programming facilities"
[SWIFT-SENDABLE]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0302-concurrent-value-and-concurrent-closures.md "Swift Evolution SE-0302: Sendable"
[SWIFT-SPAN]: https://www.swift.org/blog/swift-6.2-released/ "Swift 6.2 release: Span and lifetime safety"
[SWIFT-THROWS]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0413-typed-throws.md "Swift Evolution SE-0413: typed throws"
[TOKIO-CHANNELS]: https://tokio.rs/tokio/tutorial/channels "Tokio tutorial: channels"
[TOKIO-SELECT]: https://docs.rs/tokio/latest/tokio/macro.select.html "Tokio: select and cancellation safety"
[V-INTERFACES]: https://docs.vlang.io/type-declarations.html#interfaces "V documentation: interfaces"
[V-MEMORY]: https://docs.vlang.io/memory-management.html "V documentation: memory management"
[V-OPTIONS]: https://docs.vlang.io/type-declarations.html "V documentation: options, results, sum types, and generics"
[ZEROCOPY]: https://docs.rs/zerocopy/latest/zerocopy/ "zerocopy: checked byte reinterpretation and layout traits"
[ZIG]: https://ziglang.org/documentation/master/ "Zig language reference, master snapshot"
[ZIG-ALLOCATOR]: https://raw.githubusercontent.com/ziglang/zig/master/lib/std/mem/Allocator.zig "Zig standard library: Allocator source"
[ZIG-ARENA]: https://raw.githubusercontent.com/ziglang/zig/master/lib/std/heap/arena_allocator.zig "Zig standard library: arena allocator source"
