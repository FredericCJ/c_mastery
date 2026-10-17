# I. Memory Management & Data Layout Views

This category separates storage ownership, access permissions, object lifetime, address stability, allocation policy, and physical arrangement. They are independent design axes: a view is not an owner, an owner need not allocate, an arena is not a borrow checker, and a stable handle does not imply a stable address.

<a id="m01"></a>
## M01. Ownership-bearing resource wrappers and deterministic destruction

**1. Modern approach & concrete entities.** Represent acquisition and release as the construction and destruction of an owning value. Examples include C++ `std::unique_ptr<T, D>`, `std::unique_lock<M>`, and custom file-descriptor classes; Rust `Box<T>`, owned I/O handles, and `Drop`; Swift noncopyable resource wrappers with `deinit`. The resource may be memory, a lock, a subscription, a peripheral reservation, or a transaction. [CPP-UNIQUE] [RUST-DROP] [SWIFT-NONCOPY]

**2. Classical antecedent / historical anti-pattern.** Unmatched `malloc/free`, `open/close`, and `lock/unlock`; ownership inferred from naming; duplicated cleanup on every return; assigning a second raw pointer without deciding whether it owns anything. RAII is itself an older technique, not a C++20 invention.

**3. Underlying mechanism & memory layout.** The wrapper stores a resource representation and whatever cleanup state it requires. Control-flow lowering inserts destruction at applicable lifetime endpoints. A stateless C++ deleter commonly adds no storage; a stateful deleter can enlarge the wrapper. Rust drop glue destroys owned fields and may need conditional bookkeeping when initialization or moves depend on branches. Cleanup code is ordinary executable code, not a garbage collector.

**4. Engineering trade-offs & footprint.** Destruction may deallocate a graph, block, or acquire another lock; deterministic ordering does not establish constant latency. Fallible `close`, `flush`, and commit operations often require explicit methods, with destruction only as fallback. Aborts and leaks remain possible; Rust deliberately permits forgetting a value. Resource-safe APIs therefore establish safety even when cleanup never happens, rather than relying on every destructor eventually running. [RUST-FORGET]

<a id="m02"></a>
## M02. Affine ownership, noncopyable values, and consuming operations

**1. Modern approach & concrete entities.** Rust non-`Copy` values and by-value arguments; Swift `~Copyable`, `borrowing`, and `consuming`; C++ deleted copy operations and move-only classes. Idris 2 multiplicity-`1` arguments illustrate genuinely linear usage constraints. [RUST-OWNERSHIP] [SWIFT-NONCOPY] [IDRIS-LINEAR]

**2. Classical antecedent / historical anti-pattern.** Duplicating resource-owning bits, accidentally closing a handle twice, copying synchronization objects, or documenting “the caller relinquishes ownership” without a checkable interface.

**3. Underlying mechanism & memory layout.** Ownership transfer changes which program binding may use a resource. An affine discipline permits at most one consumption; a linear discipline requires exactly one use under its usage rules. Rust rejects use of a moved binding until it is validly reinitialized. The runtime may merely move bytes, pass registers, or eliminate the transfer. No universal “ownership flag” has to be stored beside every value.

**4. Engineering trade-offs & footprint.** A large inline value can still be expensive to move. A C++ `std::move(x)` only casts `x` into an expression category that can select move operations; it neither moves by itself nor disables future use of `x`. C++ moved-from values still exist and remain subject to their type's contract. Noncopyability also does not guarantee eventual release: affine ownership, process termination, deliberate leaking, and true linear resource protocols must be distinguished. APIs must decide how ownership is returned when a consuming operation fails.

<a id="m03"></a>
## M03. Borrowing and compile-time lifetime relationships

**1. Modern approach & concrete entities.** Rust `&'a T`, `&'a mut T`, lifetime bounds, reborrowing, and non-lexical borrow analysis; Swift lifetime-dependent non-owning `Span` APIs. C++ references and views provide access syntax without the same general lifetime-checking guarantee. [RUST-BORROW] [SWIFT-SPAN]

**2. Classical antecedent / historical anti-pattern.** Returning a local address, retaining a reference after its owner is destroyed, using a container element after reallocation, and allowing a callback to escape while borrowing temporary state.

**3. Underlying mechanism & memory layout.** The compiler relates the region in which a borrow is usable to the lifetime of its source and to conflicting accesses. Lifetime parameters generally erase: a reference does not carry an expiration timestamp or a runtime reference count. Rust's exclusive mutable borrow constrains simultaneous access through aliases; it is not just a non-null pointer annotation. A lifetime parameter expresses a relationship, not an instruction to extend storage duration.

**4. Engineering trade-offs & footprint.** Graphs, self-referential structures, caches, and asynchronous captures can require a different representation or narrowly audited unsafe code. Static analysis may reject a safe algorithm whose disjointness it cannot derive. Borrow checking does not establish valid protocol values, absence of deadlock, bounded execution, or synchronization with DMA. The benefit is eliminating classes of lifetime/aliasing errors from safe code, not eliminating every runtime check or every pointer.

<a id="m04"></a>
## M04. Borrow splitting and disjoint mutable views

**1. Modern approach & concrete entities.** Rust slice `split_at_mut`, `chunks_mut`, and `iter_mut`; field-sensitive borrowing; domain APIs returning non-overlapping subregions. C++ `span::subspan` can express corresponding regions but does not prove their mutual disjointness. [RUST-SLICE] [CPP-SPAN]

**2. Classical antecedent / historical anti-pattern.** Hand-computed overlapping pointer ranges, disabling alias analysis globally, or sharing one mutable buffer between workers with only comments describing their assigned segments.

**3. Underlying mechanism & memory layout.** A checked split validates the split point, then creates two descriptors whose index intervals cannot overlap. A small unsafe implementation can establish this invariant once and expose a safe API. The elements remain in the original allocation. Iterators can progressively remove yielded elements from the remaining region, allowing multiple live mutable element references without aliasing the same element.

**4. Engineering trade-offs & footprint.** The boundary check may remain unless proved redundant. Disjoint bytes are not automatically independent logical objects: two records may reference the same heap allocation or device. Splitting storage also does not establish cache-line separation, so independent writers can still suffer false sharing. Irregular graphs and scatter/gather mappings require stronger representations than two contiguous intervals. This technique is particularly valuable when partitioning numerical kernels or worker tasks because the API carries a useful invariant that raw offsets do not.

<a id="m05"></a>
## M05. Contiguous slices, spans, and bounded access descriptors

**1. Modern approach & concrete entities.** C++20 `std::span<T, Extent>`; Rust `&[T]` and `&mut [T]`; Go `[]T`; Zig and Odin `[]T`; C3 `T[]`. These expose a sequence without requiring a particular owning container. [CPP-SPAN] [GO-SLICES] [ZIG] [ODIN] [C3-SPEC]

**2. Classical antecedent / historical anti-pattern.** Independent pointer/count parameters, array-to-pointer decay that loses extent, and APIs accepting an owning vector merely to read contiguous elements.

**3. Underlying mechanism & memory layout.** A dynamic descriptor is commonly pointer plus length. Go additionally carries capacity. A fixed-extent C++ span can encode the length in the type rather than storing it. Descriptor copying aliases backing storage; it does not duplicate elements. Slice construction and indexing checks depend on the language, operation, and safety mode.

**4. Engineering trade-offs & footprint.** C++ spans can dangle and do not freeze the owner. A small Go slice can retain a large backing allocation. Bounds metadata does not validate a fabricated pointer or prove lifetime. A slice of owning references still involves indirection per element. Different ABI shapes matter at foreign boundaries. Distinguish `span<T>` from `span<const T>`: constness of the descriptor itself is not the same as read-only element access. C++26 hardening facilities are not a substitute for ownership tracking. [CPP-SPAN]

<a id="m06"></a>
## M06. Encoding-aware text and binary views

**1. Modern approach & concrete entities.** Rust `&str`/`String` for UTF-8, `&[u8]` for arbitrary bytes, and `CStr`/`CString` for C-string invariants; C++ `std::string_view`, `std::u8string_view`, and `std::span<const std::byte>`. [RUST-STR] [RUST-CSTR] [CPP-STRINGVIEW]

**2. Classical antecedent / historical anti-pattern.** Treating every `char*` as both binary storage and a null-terminated string; assuming byte offsets are character indices; silently truncating at embedded NUL.

**3. Underlying mechanism & memory layout.** A view contains access metadata rather than text ownership. UTF-8 validation can establish an invariant once at construction; later operations can rely on valid encoding while still checking character boundaries. Rust `&str` and `&[u8]` have the same reference layout, but different validity requirements. C++ string-view character type does not by itself prove that the underlying sequence is valid Unicode. [RUST-LAYOUT]

**4. Engineering trade-offs & footprint.** Validating encoding scans data; transcoding allocates or writes output somewhere. Unicode scalar values, grapheme clusters, display columns, and code units remain different units of measurement. A valid UTF-8 string can still contain prohibited application characters or misleading normalization variants. Borrowed text inherits storage-lifetime restrictions. Crossing C APIs may require a terminator or conversion rather than passing `.data()` blindly. Stronger text types eliminate representation ambiguity, not every semantic text-validation requirement.

<a id="m07"></a>
## M07. Multidimensional, strided, and accessor-parametric views

**1. Modern approach & concrete entities.** C++23 `std::mdspan<T, Extents, Layout, Accessor>`, `std::extents`, `layout_left`, `layout_right`, and `layout_stride`. A custom accessor can represent access semantics distinct from ordinary pointer dereference. [CPP-MDSPAN]

**2. Classical antecedent / historical anti-pattern.** Treating `T**` as a contiguous matrix, duplicating shape/stride conventions across functions, and hard-coding row-major indexing macros into every numerical algorithm.

**3. Underlying mechanism & memory layout.** The descriptor combines a data handle, extents, an index mapping, and an accessor. A simple strided mapping computes `offset = sum(index[k] * stride[k])`. Static extents or strides can erase metadata and fold arithmetic. The matrix data need not move or be nested behind row pointers. Changing a view's logical shape does not necessarily change the physical layout.

**4. Engineering trade-offs & footprint.** Dynamic mappings retain metadata and address arithmetic. A transposed view is cheap to construct but may produce an expensive traversal. Correctness still requires valid backing storage and applicable mapping preconditions. A const view object does not necessarily mean immutable elements. Algorithms should constrain the mapping properties they require, such as uniqueness or contiguous access, rather than silently assuming them. Custom accessors can introduce additional work, so an `mdspan` abstraction is not universally equivalent to a raw pointer plus a multiplication.

<a id="m08"></a>
## M08. Separating storage owners from access interfaces

**1. Modern approach & concrete entities.** Rust `Vec<T>` versus slices, `String` versus `str`, and `PathBuf` versus `Path`; C++ `vector<T>` versus `span<T>`, and `string` versus `string_view`. Accept a borrow for observation; accept an owner only when the operation actually needs ownership or growth. [RUST-VEC] [RUST-PATH] [CPP-VECTOR]

**2. Classical antecedent / historical anti-pattern.** Container-specific APIs with unnecessary copying, functions that may secretly retain any pointer they receive, and capacity-changing operations exposed to consumers that only need to inspect data.

**3. Underlying mechanism & memory layout.** An owning growable buffer tracks allocation and initialized length, usually with capacity metadata. A borrowed descriptor exposes only the relevant region. Constructing that descriptor generally copies a few words. Returning ownership can transfer an allocation rather than copying all elements; returning a borrow ties the result to existing storage.

**4. Engineering trade-offs & footprint.** The API must choose whether retention is allowed; a borrowed parameter cannot be stored indefinitely without changing the contract. Generic views can obscure required allocation or contiguity properties unless constrained. A “read-only” API may still trigger work through lazy internals or reference-semantic elements. Separating ownership from access reduces coupling, but does not eliminate capacity management, allocator failure, or expensive conversion between genuinely different representations. The correct signature follows the operation's required authority, not a universal preference for passing everything by reference.

<a id="m09"></a>
## M09. Value semantics, copy-on-write, and value-oriented indirection

**1. Modern approach & concrete entities.** Swift `Array` value semantics with copy-on-write storage; Rust `Cow<'a, B>`; C++26 `std::indirect<T>` and `std::polymorphic<T>` for owning indirection with value-oriented copying. These are related techniques, not interchangeable implementations. [SWIFT-ARRAY] [RUST-COW] [CPP-INDIRECT] [CPP-POLYMORPHIC]

**2. Classical antecedent / historical anti-pattern.** Copies that unexpectedly alias mutable logical state, mandatory reference identity for ordinary data, and handwritten virtual `clone()` hierarchies.

**3. Underlying mechanism & memory layout.** Logical copies can share backing storage until mutation requires uniqueness. Swift array storage may perform a uniqueness test before mutation. Rust `Cow` is a borrowed-or-owned sum; it does not itself imply shared reference-counted ownership. Value-oriented indirection stores an owned object out of line while making copy operations duplicate the logical value, including its concrete dynamic type where applicable.

**4. Engineering trade-offs & footprint.** A cheap copy can defer a large allocation/copy to the first write, harming latency predictability. Owning indirection adds pointer chasing and typically allocation. A value-semantic collection of reference-semantic elements does not deep-copy every referenced object. Uniqueness checks, allocator behavior, and retained snapshots affect the cost model. C++ value wrappers also retain moved-from states required by their object model; “value semantics” does not mean immutable, pointer-free, allocation-free, or a single physical representation.

<a id="m10"></a>
## M10. Explicit shared ownership and weak observation

**1. Modern approach & concrete entities.** Rust `Rc<T>`, `Arc<T>`, and `Weak<T>`; C++ `shared_ptr<T>` and `weak_ptr<T>`; Swift ARC and weak references. [RUST-RC] [RUST-ARC] [CPP-SHARED]

**2. Classical antecedent / historical anti-pattern.** Informal last-user cleanup conventions, manually synchronized reference counts, or callbacks holding dangling pointers after an owner disappears.

**3. Underlying mechanism & memory layout.** An allocation or associated control structure maintains ownership counts. Copies and releases update these counts; destruction occurs when strong ownership ends. Weak references can test or acquire strong ownership without keeping the value alive themselves. Some APIs can combine object and control allocation, while others use separate storage. Pointer-sized appearance at one call site does not reveal total allocation footprint.

**4. Engineering trade-offs & footprint.** Atomic reference-count updates can bounce cache lines. Non-atomic counting is cheaper but cannot be freely shared between threads. Cycles of strong references can leak. Releasing the last owner may recursively destroy a large graph. `Arc<T>` does not synchronize arbitrary mutation of `T`; `shared_ptr<T>` does not make the pointee thread-safe. Use shared ownership where independently extending lifetime is a real requirement, rather than as a substitute for deciding whether a reference should merely borrow. Weak observation can fail and must be represented accordingly.

<a id="m11"></a>
## M11. Tracing garbage collection and escape-driven placement

**1. Modern approach & concrete entities.** Go's tracing collector and compiler escape analysis; V's documented default garbage collection. V documents `-autofree` as an evolving mechanism, not a blanket equivalent of statically checked ownership. [GO-GC] [V-MEMORY]

**2. Classical antecedent / historical anti-pattern.** Manually establishing the last use of every allocation in a shared graph, freeing cyclic graphs by hand, and forcing all potentially escaping objects into explicitly managed storage.

**3. Underlying mechanism & memory layout.** Escape analysis can keep suitable objects in shorter-lived storage. The collector identifies reachable objects through roots and runtime metadata, and reclaims unreachable allocations. Allocation paths, scanning, write barriers, and concurrent collector work become part of execution. Reachability is not identical to semantic usefulness: a stale reference can keep otherwise unnecessary data alive.

**4. Engineering trade-offs & footprint.** Memory headroom, throughput, scheduling interference, and tail latency must be budgeted. Stack placement is an optimization, not an invariant to infer from local-variable syntax. GC does not provide prompt release of file descriptors, locks, or device reservations. Reference cycles are handled differently from reference counting, but a live accidental reference still causes retention. This is an important modern safety approach, yet it should not be classified as an erased, universally zero-runtime-cost abstraction or assumed appropriate for every bounded-latency system.

<a id="m12"></a>
## M12. Explicit allocator injection and allocation-policy composition

**1. Modern approach & concrete entities.** C++ `std::pmr::memory_resource`, `polymorphic_allocator`, and allocator-aware containers; Zig `std.mem.Allocator`; Odin `context.allocator` and explicit allocator arguments. [CPP-PMR] [ZIG-ALLOCATOR] [ODIN]

**2. Classical antecedent / historical anti-pattern.** Hidden global `malloc`, container implementations inseparable from their storage policy, and test code unable to inject allocation failure or account for a subsystem's memory consumption.

**3. Underlying mechanism & memory layout.** The consumer carries or receives an allocator handle. Allocations dispatch through a statically known policy, function table, or virtual resource. Storage can come from a region, pool, fixed buffer, diagnostic allocator, or upstream heap. Allocation policy changes without changing the logical element type or algorithm.

**4. Engineering trade-offs & footprint.** Type-erased allocators can add dispatch and metadata. Allocator lifetime must cover users of its allocations. Copy, move, swap, and deallocation compatibility need explicit rules. Passing an allocator does not ensure all transitive callees use it. Implicit context is convenient but makes allocation authority less visible than a direct parameter. For hard limits, disable or control fallback rather than merely giving an arena an initial buffer. Allocation-failure behavior belongs in the operation's error and exception policy.

<a id="m13"></a>
## M13. Regions, arenas, and bulk lifetime management

**1. Modern approach & concrete entities.** C++ `std::pmr::monotonic_buffer_resource`; Zig arena allocators; region-scoped parser or request storage; Rust arena libraries behind lifetime-aware APIs. [CPP-ARENA] [ZIG-ARENA] [BUMPALO]

**2. Classical antecedent / historical anti-pattern.** One general-purpose allocation/free pair per short-lived node, fragmentation from interleaved object lifetimes, and complicated individual ownership when everything dies at a known phase boundary.

**3. Underlying mechanism & memory layout.** A region obtains larger blocks and suballocates aligned chunks, frequently by bumping an offset. Individual deallocation may be a no-op; resetting or destroying the region releases storage together. Locality can improve because allocations made near each other are placed near each other, though object-access order remains decisive.

**4. Engineering trade-offs & footprint.** Early-dead values consume capacity until region reclamation. Releasing storage is not automatically the same as running every object's destructor. A C++ container must not retain pointers into a released resource. Rust arena APIs need sound lifetime relationships or suitable handle indirection. Unbounded fallback destroys a claimed fixed-memory budget. Region selection should follow actual lifetime cohorts; using one process-wide arena merely converts forgotten frees into deliberate permanent retention. Account for alignment waste, block headers, retained capacity, and the cost of destruction before reset.

<a id="m14"></a>
## M14. Fixed-size and fixed-capacity inline containers

**1. Modern approach & concrete entities.** C++ `std::array<T,N>` and C++26 `std::inplace_vector<T,N>`; Rust `[T; N]` and `heapless::Vec<T,N>`; Swift `InlineArray`. Fixed size and bounded variable length are different contracts. [CPP-INPLACE] [HEAPLESS] [SWIFT-INLINE]

**2. Classical antecedent / historical anti-pattern.** Heap allocation for a known maximum, a raw array with a separately drifting logical count, and capacity growth inside a hard real-time execution path.

**3. Underlying mechanism & memory layout.** Capacity resides inside the enclosing object. A bounded variable-length container additionally tracks how many elements are initialized. A first approximation is `N * sizeof(T) + count + padding`, but exact layout depends on the implementation. Inline storage resides wherever the owner resides: stack, static memory, heap allocation, or another aggregate.

**4. Engineering trade-offs & footprint.** Unused capacity still consumes memory and can enlarge every task frame or state-machine alternative. Large copies/moves may be expensive. Capacity exhaustion needs a checked return, exception, or documented precondition; silently overwriting is not an acceptable substitute. The container's storage can be allocation-free while its elements allocate. Bounded capacity does not make insertion time constant if shifting elements is required. Choose capacity from an explicit workload/resource bound, not from an average-case measurement alone.

<a id="m15"></a>
## M15. Small-buffer optimization and adaptive inline storage

**1. Modern approach & concrete entities.** LLVM `SmallVector<T,N>`; Rust `smallvec::SmallVec`; implementation-specific short-string optimization; inline capture storage in some owning callable wrappers. [LLVM-CONTAINERS] [SMALLVEC]

**2. Classical antecedent / historical anti-pattern.** A heap allocation for every tiny string, short list, or callback environment, regardless of how often the common case fits in a few words.

**3. Underlying mechanism & memory layout.** The object embeds a small buffer and discriminates between inline and out-of-line storage. Growth beyond inline capacity allocates and moves or copies elements to a larger block. The representation contains enough state to distinguish modes and release the appropriate storage.

**4. Engineering trade-offs & footprint.** Unlike a fixed-capacity vector, this does not promise absence of heap allocation. Larger inline capacity increases the size of every object even when empty. Crossing the threshold produces a latency cliff, and moves of inline values can be more expensive than transferring a heap pointer. Pointer stability differs between modes and operations. Do not assume a particular short-string capacity across libraries or ABIs. Evaluate the distribution of sizes, cache density, move frequency, and threshold crossings; “avoids allocations in the common case” is a workload claim, not a universal guarantee.

<a id="m16"></a>
## M16. Structure of arrays, AoSoA, and hot/cold data separation

**1. Modern approach & concrete entities.** SoA and AoSoA storage; Odin `#soa[N]T` and `#soa[]T`; separate compact hot fields and rarely used cold metadata; archetype-style component columns. [ODIN] [FLECS-DESIGN]

**2. Classical antecedent / historical anti-pattern.** A heap object and vptr per entity in a bulk numerical loop, following pointer chains to fetch one field, or loading a large record when the kernel only needs two scalars.

**3. Underlying mechanism & memory layout.** AoS stores complete records consecutively. SoA stores each field in its own stream. AoSoA groups short SoA blocks to balance vectorized access and entity-local operations. Hot/cold splitting removes rarely accessed data from frequently traversed records. These transformations change which useful bytes arrive in each cache line; they do not depend on banning methods or encapsulation.

**4. Engineering trade-offs & footprint.** Field-wise kernels can gain unit-stride loads, while operations consuming one complete entity may need multiple streams. An apparent element reference can be a proxy rather than `T&`. Insertions/removals must update parallel columns consistently. SIMD width, cache lines, prefetch behavior, and access distribution influence the best blocking. Benefits shrink or reverse when the workload changes. Preserve a logical API over the representation so layout changes remain localized rather than propagating raw field arrays throughout the codebase.

<a id="m17"></a>
## M17. Portable SIMD values and explicit lane masks

**1. Modern approach & concrete entities.** C++26 working-draft `<simd>` facilities, including `std::simd::vec` and mask types; Zig vector types such as `@Vector(N,T)`; typed vector and mask APIs rather than unstructured intrinsic sequences. [CPP-SIMD] [ZIG]

**2. Classical antecedent / historical anti-pattern.** Duplicated scalar and target-specific kernels, unsafe lane reinterpretation, and scattering ISA-specific register types through domain interfaces.

**3. Underlying mechanism & memory layout.** A vector value represents multiple homogeneous lanes, and a mask identifies participating lanes. The compiler lowers operations to native vector instructions, scalar sequences, or combinations appropriate to the target and ABI. Masks may be vector registers, compact predicate registers, or another representation; an array of booleans is not a universal model.

**4. Engineering trade-offs & footprint.** Unsupported widths may split operations. Tail handling, reductions, alignment, gather/scatter, and masking semantics influence generated work. A masked computation must not be assumed to suppress every invalid memory access unless its particular API guarantees that. Floating-point reassociation can change results. Wide SIMD can alter frequency and register pressure on relevant targets. Portable syntax improves maintainability but does not replace disassembly, numerical validation, or target-specific benchmarking; it also does not make an irregular memory-access pattern contiguous.

<a id="m18"></a>
## M18. Pinning and address-sensitive values

**1. Modern approach & concrete entities.** Rust `Pin<P>`, `Pin<&mut T>`, `Pin<Box<T>>`, and `Unpin`; pin-projection libraries for composites. [RUST-PIN]

**2. Classical antecedent / historical anti-pattern.** Moving a self-referential object after its internal pointer is established, relocating a registered intrusive node, and letting an asynchronous operation retain the address of movable storage.

**3. Underlying mechanism & memory layout.** Pinning constrains moving or invalidating the pointee after the pin contract takes effect. It need not add a runtime tag. A pinned owning pointer can itself move while its heap allocation stays at the same address. The useful guarantee depends on the pointee not opting out through `Unpin` and on correct unsafe implementations/projections.

**4. Engineering trade-offs & footprint.** Pinning is not a runtime operation that physically locks bytes in memory, nor is it OS page pinning for DMA. Stack pinning is possible, so heap allocation is not inherent. Projection and destruction invariants are subtle. Pinning does not automatically establish a valid self-reference or extend a borrow. Deleting C++ move constructors can enforce part of an address-stability discipline, but is not a full counterpart of Rust's pinning abstraction. Model whether stability is required for a value, an allocation, a slot, or a hardware mapping; these are different obligations.

<a id="m19"></a>
## M19. Explicit partial initialization and initialization-state tracking

**1. Modern approach & concrete entities.** Rust `MaybeUninit<T>`, checked initialized-length bookkeeping, and `Vec::spare_capacity_mut`; C++ `std::construct_at`, `std::destroy_at`, and uninitialized-memory algorithms. [RUST-MAYBEUNINIT] [CPP-SPECIALMEM]

**2. Classical antecedent / historical anti-pattern.** Treating `malloc` storage as live objects, using `memset(0)` for every type, setting a vector's length before constructing its elements, and leaking partially constructed buffers on failure.

**3. Underlying mechanism & memory layout.** Raw storage and a valid value are represented separately. `MaybeUninit<T>` allows storage without claiming that its bits constitute a valid `T`; construction advances an explicit initialized frontier. A guard destroys only the successfully initialized prefix during failure. C++ similarly distinguishes storage duration from object lifetime and constructor execution.

**4. Engineering trade-offs & footprint.** Converting to `T` before establishing its invariants is unsafe, even when the memory has the right size. Zero bits are invalid for some references, enums, and other types. `MaybeUninit` can inhibit niche optimization in enclosing structures. The bookkeeping and cleanup proof are additional complexity, but permit efficient bulk construction without redundant default initialization. This is an advanced implementation technique to encapsulate behind safe interfaces, not a reason to expose partially initialized objects to ordinary callers.

<a id="m20"></a>
## M20. Interior mutability with explicit enforcement policy

**1. Modern approach & concrete entities.** Rust `Cell<T>`, `RefCell<T>`, `UnsafeCell<T>`, and synchronization wrappers. The shared-access type states that mutation is mediated by a specific mechanism rather than prohibited absolutely. [RUST-CELL]

**2. Classical antecedent / historical anti-pattern.** Casting away constness, modifying shared state through undocumented aliases, and assuming a `const` or shared-reference interface guarantees deep immutability.

**3. Underlying mechanism & memory layout.** `Cell` supports controlled replacement without handing out ordinary simultaneous references to mutable contents. `RefCell` stores borrow state and checks access dynamically. `UnsafeCell` is the primitive allowing mutation behind shared references; safe wrappers establish the missing invariants. These choices differ from locks, which additionally coordinate concurrent access.

**4. Engineering trade-offs & footprint.** Runtime borrowing adds checks and can fail or panic when borrowing rules are violated. Reentrancy through callbacks can trigger conflicts unexpectedly. `UnsafeCell` alone provides no synchronization. Encapsulated caches can preserve logical constness while mutating physical representation, but the distinction must be documented. Choosing interior mutability can simplify graph ergonomics at the price of moving some errors from compilation to execution; it should be a conscious enforcement decision rather than a blanket escape hatch from ownership design.

<a id="m21"></a>
## M21. Thread-transfer and shared-access constraints

**1. Modern approach & concrete entities.** Rust `Send` and `Sync`, `Mutex<T>`, `RwLock<T>`, and atomic types; Swift `Sendable` and isolation-aware concurrency. [RUST-SEND] [RUST-SYNC] [RUST-MUTEX] [SWIFT-SENDABLE]

**2. Classical antecedent / historical anti-pattern.** Passing arbitrary pointers to worker threads, sharing a reference-counted object and assuming its state is synchronized, or relying on a code review to identify every cross-thread capture.

**3. Underlying mechanism & memory layout.** Marker constraints govern whether ownership or shared access may cross thread/isolation boundaries. For Rust, `T: Sync` corresponds to shared references to `T` being transferable safely. Marker traits generally need no runtime fields, while locks and atomics supply actual runtime ordering and coordination. Derivation follows the relevant constituents and language rules; unsafe implementations are part of the trusted boundary.

**4. Engineering trade-offs & footprint.** Locks can block or deadlock, and atomics can contend. Type-level data-race prevention does not establish logical atomicity, a transaction across multiple fields, correct publication order, or bounded blocking. Negative cases such as `Rc<T>` deliberately constrain sharing. A wrapper can make a resource transferable without making every operation safe from every execution context, particularly for GUI, interrupt, device, and affinity-bound APIs. Separate memory safety from scheduling and protocol correctness.

<a id="m22"></a>
## M22. Generational handles and stable logical identity

**1. Modern approach & concrete entities.** Slot maps, generational indices, typed entity keys, and Rust `slotmap::SlotMap`. A handle typically identifies a slot and an incarnation of that slot. [SLOTMAP]

**2. Classical antecedent / historical anti-pattern.** Retaining raw pointers into relocated storage, reusing array indices without detecting stale users, and equating object identity with the current physical address.

**3. Underlying mechanism & memory layout.** Lookup validates `handle.generation == slot.generation` and that the slot is occupied. Reusing a slot increments its generation. Keys can remain meaningful while other storage is compacted or reorganized through an appropriate indirection table. A branded key type can additionally distinguish separate pools.

**4. Engineering trade-offs & footprint.** Lookups require indexing, validation, and sometimes an additional indirection. Slot metadata and free lists occupy memory. Generation counters are finite, so wraparound/reuse policies matter; stale detection is not an unconditional eternal guarantee. Logical identity does not establish address stability or keep an object alive. Handles should not be forged or persisted across restarts without a defined namespace/incarnation policy. This is especially useful for scene graphs, ASTs, device registries, and ECS entities whose identity must outlive temporary references.

<a id="m23"></a>
## M23. Deferred concurrent reclamation: epochs and hazard protection

**1. Modern approach & concrete entities.** Epoch-based reclamation, hazard-pointer schemes, and Rust `crossbeam_epoch::{Atomic, Owned, Shared, Guard}`. These separate removing an object from a concurrent data structure from freeing its storage. [CROSSBEAM-EPOCH] [HAZARD-PAPER]

**2. Classical antecedent / historical anti-pattern.** Calling `free` immediately after an atomic unlink while another reader may still dereference the node, or adding reference counting without examining contention and acquisition races.

**3. Underlying mechanism & memory layout.** Readers announce protected access through an epoch/guard or publish hazardous pointers. Removed nodes enter a retired set. Reclamation waits until the scheme establishes that relevant readers can no longer access them. Per-thread records, retired lists, epochs, and ordering operations are part of the representation.

**4. Engineering trade-offs & footprint.** Slow or stalled participants can delay reclamation, increasing memory consumption. Hazard scanning and epoch advancement add work; algorithmic progress and reclamation progress are separate properties. A guard's scope can become a performance and memory-retention boundary. The data structure still needs a correct publication protocol and an ABA strategy where applicable. An atomic pointer is not a lifetime guarantee. These techniques are specialized alternatives for concurrent structures, not default replacements for simple exclusive ownership and ordinary locks.

<a id="m24"></a>
## M24. Safe interfaces around unsafe layout and provenance operations

**1. Modern approach & concrete entities.** Rust `unsafe` blocks/functions, raw-pointer APIs, `NonNull<T>`, and strict-provenance operations; C++ narrow low-level modules exposing ownership/view types rather than raw representation access. [RUST-UNSAFE] [RUST-PTR] [RUST-NONNULL]

**2. Classical antecedent / historical anti-pattern.** Pervasive pointer arithmetic, integer-to-pointer fabrication, unchecked representation casts, and invariants that every caller must reconstruct independently.

**3. Underlying mechanism & memory layout.** A small trusted implementation establishes alignment, initialization, lifetime, aliasing, and provenance obligations before constructing safe access types. Provenance concerns which allocation/access rights a pointer derives from, not merely its numeric address. An unsafe block does not disable the compiler's semantic assumptions; its author must satisfy them.

**4. Engineering trade-offs & footprint.** The safe wrapper can inline completely, but its proof and test burden remain. `NonNull<T>` only excludes null; it does not prove dereferenceability or initialized storage. Correctness must cover panics, reentrancy, leaks, and malicious implementations of any traits the API trusts. Reinterpretation rules differ across languages and types. Concentrating unsafety makes audits tractable, but a sound-looking signature is not evidence that the implementation has met its obligations. Include sanitizers, fuzzing, and model-appropriate tools around this boundary.

<a id="m25"></a>
## M25. Zero-sized markers and empty-state layout optimization

**1. Modern approach & concrete entities.** Rust unit/empty marker types and `PhantomData`; C++ empty-base optimization and C++20 `[[no_unique_address]]`. [RUST-LAYOUT] [RUST-PHANTOM] [CPP-NOUNIQUE]

**2. Classical antecedent / historical anti-pattern.** Storing runtime booleans or integer tags for a policy known entirely at compile time, or paying per-object storage for stateless strategies solely because they are modeled as objects.

**3. Underlying mechanism & memory layout.** A zero-sized marker participates in type identity and analysis without requiring payload bytes. C++ permits potentially overlapping storage for appropriately marked members; a standalone empty C++ object is not generally zero-sized. Empty deleters or policies can therefore be embedded without necessarily increasing the containing object's size.

**4. Engineering trade-offs & footprint.** Alignment, distinct-address requirements, member combinations, and ABI details still matter. `[[no_unique_address]]` is permission for layout optimization, not a portable formula for every object size. Markers can increase specialization and diagnostic complexity even when runtime storage disappears. In Rust, `PhantomData` influences variance, auto traits, and drop analysis, so changing the marker can alter soundness properties without changing measured size. Treat zero-size as a representation property, not evidence that a type-level design is cognitively free.

<a id="m26"></a>
## M26. Stable-address segmented storage instead of per-node allocation

**1. Modern approach & concrete entities.** C++26 `std::hive<T>` and suitable pool/block containers; stable-address storage with reusable erased slots. This differs from a generational handle, which stabilizes identity rather than necessarily the address. [CPP-HIVE]

**2. Classical antecedent / historical anti-pattern.** Allocating every object separately to avoid vector relocation, or selecting a linked list solely because insertion must not invalidate existing object addresses.

**3. Underlying mechanism & memory layout.** Objects occupy slots in allocated blocks, with metadata to skip holes and reuse erased storage. Block allocation amortizes allocator interactions, while avoiding wholesale relocation on ordinary insertion. Traversal is segmented rather than one globally contiguous array. Invalidation guarantees are operation-specific and must be taken from the container contract.

**4. Engineering trade-offs & footprint.** Holes, block metadata, and unused capacity consume space. The structure is not a random-access contiguous range, and block changes affect traversal predictability. Erased elements cease to exist even if their address is later reused; a raw pointer cannot distinguish incarnations. Compaction or special capacity-management operations may have different guarantees from insertion/erasure. Choose this representation when physical stability and iteration matter together; choose a dense vector or SoA when maximal compactness and bulk scanning dominate instead.

<!-- REFERENCE DEFINITIONS -->
[BUMPALO]: https://docs.rs/bumpalo/latest/bumpalo/ "bumpalo crate documentation"
[C3-SPEC]: https://c3-lang.org/implementation-details/specification/ "C3 language specification"
[CPP-ARENA]: https://eel.is/c++draft/mem.res.monotonic.buffer "C++ working draft: monotonic_buffer_resource"
[CPP-HIVE]: https://eel.is/c++draft/hive "C++ working draft: hive"
[CPP-INDIRECT]: https://eel.is/c++draft/indirect "C++ working draft: indirect"
[CPP-INPLACE]: https://eel.is/c++draft/inplace.vector "C++ working draft: inplace_vector"
[CPP-MDSPAN]: https://eel.is/c++draft/views.multidim "C++ working draft: multidimensional views"
[CPP-NOUNIQUE]: https://eel.is/c++draft/dcl.attr.nouniqueaddr "C++ working draft: no_unique_address"
[CPP-PMR]: https://eel.is/c++draft/mem.res "C++ working draft: memory resources"
[CPP-POLYMORPHIC]: https://eel.is/c++draft/polymorphic "C++ working draft: polymorphic"
[CPP-SHARED]: https://eel.is/c++draft/util.smartptr.shared "C++ working draft: shared_ptr"
[CPP-SIMD]: https://eel.is/c++draft/simd "C++ working draft: data-parallel types"
[CPP-SPAN]: https://eel.is/c++draft/views.span "C++ working draft: span"
[CPP-SPECIALMEM]: https://eel.is/c++draft/specialized.algorithms "C++ working draft: specialized memory algorithms"
[CPP-STRINGVIEW]: https://eel.is/c++draft/string.view "C++ working draft: string_view"
[CPP-UNIQUE]: https://eel.is/c++draft/unique.ptr "C++ working draft: unique_ptr"
[CPP-VECTOR]: https://eel.is/c++draft/vector "C++ working draft: vector"
[CROSSBEAM-EPOCH]: https://docs.rs/crossbeam-epoch/latest/crossbeam_epoch/ "Crossbeam epoch-based reclamation documentation"
[FLECS-DESIGN]: https://raw.githubusercontent.com/SanderMertens/flecs/master/docs/DesignWithFlecs.md "Flecs official design guide"
[GO-GC]: https://go.dev/doc/gc-guide "Go garbage collector guide"
[GO-SLICES]: https://go.dev/blog/slices-intro "Go slices: usage and internals"
[HAZARD-PAPER]: https://research.ibm.com/publications/hazard-pointers-safe-memory-reclamation-for-lock-free-objects "Maged M. Michael: Hazard Pointers, IEEE TPDS, 2004; IBM Research record"
[HEAPLESS]: https://docs.rs/heapless/latest/heapless/ "heapless crate documentation"
[IDRIS-LINEAR]: https://idris2.readthedocs.io/en/latest/tutorial/multiplicities.html "Idris 2 tutorial: multiplicities and erasure"
[LLVM-CONTAINERS]: https://llvm.org/docs/ProgrammersManual.html "LLVM Programmer's Manual: container selection"
[ODIN]: https://odin-lang.org/docs/overview/ "Odin language overview"
[RUST-ARC]: https://doc.rust-lang.org/std/sync/struct.Arc.html "Rust standard library: Arc"
[RUST-BORROW]: https://doc.rust-lang.org/book/ch04-02-references-and-borrowing.html "The Rust Book: references and borrowing"
[RUST-CELL]: https://doc.rust-lang.org/std/cell/index.html "Rust standard library: interior mutability"
[RUST-COW]: https://doc.rust-lang.org/std/borrow/enum.Cow.html "Rust standard library: Cow"
[RUST-CSTR]: https://doc.rust-lang.org/std/ffi/struct.CStr.html "Rust standard library: CStr"
[RUST-DROP]: https://doc.rust-lang.org/reference/destructors.html "Rust Reference: destructors"
[RUST-FORGET]: https://doc.rust-lang.org/std/mem/fn.forget.html "Rust standard library: mem::forget"
[RUST-LAYOUT]: https://doc.rust-lang.org/reference/type-layout.html "Rust Reference: type layout"
[RUST-MAYBEUNINIT]: https://doc.rust-lang.org/std/mem/union.MaybeUninit.html "Rust standard library: MaybeUninit"
[RUST-MUTEX]: https://doc.rust-lang.org/std/sync/struct.Mutex.html "Rust standard library: Mutex"
[RUST-NONNULL]: https://doc.rust-lang.org/std/ptr/struct.NonNull.html "Rust standard library: NonNull"
[RUST-OWNERSHIP]: https://doc.rust-lang.org/book/ch04-01-what-is-ownership.html "The Rust Book: ownership"
[RUST-PATH]: https://doc.rust-lang.org/std/path/index.html "Rust standard library: paths"
[RUST-PHANTOM]: https://doc.rust-lang.org/std/marker/struct.PhantomData.html "Rust standard library: PhantomData"
[RUST-PIN]: https://doc.rust-lang.org/std/pin/index.html "Rust standard library: pinning"
[RUST-PTR]: https://doc.rust-lang.org/std/ptr/index.html "Rust standard library: pointers and provenance"
[RUST-RC]: https://doc.rust-lang.org/std/rc/index.html "Rust standard library: Rc"
[RUST-SEND]: https://doc.rust-lang.org/std/marker/trait.Send.html "Rust standard library: Send"
[RUST-SLICE]: https://doc.rust-lang.org/std/primitive.slice.html "Rust standard library: slices"
[RUST-STR]: https://doc.rust-lang.org/std/str/index.html "Rust standard library: string slices"
[RUST-SYNC]: https://doc.rust-lang.org/std/marker/trait.Sync.html "Rust standard library: Sync"
[RUST-UNSAFE]: https://doc.rust-lang.org/reference/unsafety.html "Rust Reference: unsafety"
[RUST-VEC]: https://doc.rust-lang.org/std/vec/struct.Vec.html "Rust standard library: Vec"
[SLOTMAP]: https://docs.rs/slotmap/latest/slotmap/ "slotmap crate documentation"
[SMALLVEC]: https://docs.rs/smallvec/latest/smallvec/ "smallvec crate documentation"
[SWIFT-ARRAY]: https://raw.githubusercontent.com/swiftlang/swift/main/stdlib/public/core/Array.swift "Swift standard library: Array implementation and documentation"
[SWIFT-INLINE]: https://developer.apple.com/documentation/swift/inlinearray "Swift standard library: InlineArray"
[SWIFT-NONCOPY]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0390-noncopyable-structs-and-enums.md "Swift Evolution SE-0390: noncopyable structs and enums"
[SWIFT-SENDABLE]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0302-concurrent-value-and-concurrent-closures.md "Swift Evolution SE-0302: Sendable"
[SWIFT-SPAN]: https://www.swift.org/blog/swift-6.2-released/ "Swift 6.2 release: Span and lifetime safety"
[V-MEMORY]: https://docs.vlang.io/memory-management.html "V documentation: memory management"
[ZIG]: https://ziglang.org/documentation/master/ "Zig language reference, master snapshot"
[ZIG-ALLOCATOR]: https://raw.githubusercontent.com/ziglang/zig/master/lib/std/mem/Allocator.zig "Zig standard library: Allocator source"
[ZIG-ARENA]: https://raw.githubusercontent.com/ziglang/zig/master/lib/std/heap/arena_allocator.zig "Zig standard library: arena allocator source"
