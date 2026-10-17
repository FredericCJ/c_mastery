# VII. Engineering selection and migration guide

## Select a representation from the actual obligation

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

## A cost-review sheet for each selected abstraction

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

## A practical migration sequence

First, inventory existing obligations without changing the architecture: raw owners, retained views, invalidation events, sentinel meanings, implicit phases, external effects, and ABI boundaries. Record which are currently enforced by code, review discipline, tests, or not at all.

Next, replace ambiguous vocabulary at narrow interfaces. An owning handle, a bounded view, a nominal quantity, and a structured error often improve correctness before a broader rewrite. Keep the established C or device boundary intact where required and introduce checked adapters around it. Avoid simultaneously changing storage layout, dispatch, error policy, and protocol semantics unless a staged compatibility test exists.

Then model admissible states and transitions. Decide which distinctions are static, which depend on runtime data, and which represent uncertainty. Validate untrusted inputs before creating invariant-carrying values. Preserve the original resource on failure where the contract permits it, and represent recovery-required states where it does not.

Finally, optimize measured bottlenecks. Introduce SoA, specialization, arenas, or in-place storage only after observing the actual workload and defining the needed bounds. Repeat binary-size, stack, allocation, latency, and failure-path checks after each representation change. This sequence is an engineering recommendation derived from the independent failure modes cataloged above, not a claim that one migration order is universally optimal.

## Misconceptions to reject during review

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

## Evidence limitations

This census is not an ABI certification, language conformance suite, WCET analysis, or performance benchmark. Local example results cover only the listed compiler/runtime configurations. Most source URLs are live documentation rather than immutable snapshots; record commit hashes and toolchain manifests when turning the census into a project standard. Jai's current implementation details remain deliberately evidence-limited. Experimental or working-draft facilities should be qualified independently before production adoption.

<!-- REFERENCE DEFINITIONS -->
[CPP-ARENA]: https://eel.is/c++draft/mem.res.monotonic.buffer "C++ working draft: monotonic_buffer_resource"
[CPP-COROUTINE]: https://eel.is/c++draft/dcl.fct.def.coroutine "C++ working draft: coroutines"
[CPP-EXPECTED]: https://eel.is/c++draft/expected "C++ working draft: expected"
[CPP-PMR]: https://eel.is/c++draft/mem.res "C++ working draft: memory resources"
[CPP-SPAN]: https://eel.is/c++draft/views.span "C++ working draft: span"
[GO-DICTIONARIES]: https://raw.githubusercontent.com/golang/proposal/master/design/generics-implementation-dictionaries-go1.18.md "Go compiler design: generic dictionaries and gcshape sharing"
[RUST-ARC]: https://doc.rust-lang.org/std/sync/struct.Arc.html "Rust standard library: Arc"
[RUST-OPTION]: https://doc.rust-lang.org/std/option/index.html "Rust standard library: Option and representation guarantees"
[RUST-OWNERSHIP]: https://doc.rust-lang.org/book/ch04-01-what-is-ownership.html "The Rust Book: ownership"
[SWIFT-ARRAY]: https://raw.githubusercontent.com/swiftlang/swift/main/stdlib/public/core/Array.swift "Swift standard library: Array implementation and documentation"
[TOKIO-CHANNELS]: https://tokio.rs/tokio/tutorial/channels "Tokio tutorial: channels"
[TOKIO-SELECT]: https://docs.rs/tokio/latest/tokio/macro.select.html "Tokio: select and cancellation safety"
