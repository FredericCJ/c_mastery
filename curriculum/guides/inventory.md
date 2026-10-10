# Required implementation inventory

Build a reusable component library gradually. Implement the studied mechanisms in C rather than forwarding to an equivalent library routine. Caller-provided storage or an explicit allocator interface makes the portable core usable in both hosted and firmware harnesses. Apply profiles C/B as appropriate and audit the actual generated dependencies.

| Family | Required core mechanisms | Ownership and acceptance evidence | Principal modules |
|---|---|---|---|
| Sequences | Fixed array, bounded/growable vector, stack, FIFO ring and linked list | Capacity arithmetic, empty/full/wrap states, iterator invalidation and allocation failure | T14, T18 |
| Associative structures | Linear-probing hash table, binary search tree, one balanced-tree family and trie/prefix index | Collision/deletion/tombstone behavior, ordering, prefix and balancing invariants, adversarial capacities | T14, T15 |
| Priority and graph structures | Binary heap/priority queue, bitset/sparse set, disjoint set, graph representation and traversal | Heap order, union/find invariants, disconnected/cyclic graph handling, explicit frontier capacity | T14, T15 |
| Algorithms | Search/partition, insertion sort, quicksort, heap sort, stable merge, counting/radix method, selection, graph traversal/ordering/path, one greedy and one dynamic-programming problem | Stability where promised, comparator contract, overflow-safe indexing, time/space argument | T15 |
| Byte and text processing | Byte copy/move/search, bounded string operations, UTF-8 validation, numeric parse/format, framing, incremental parser and small lexer/grammar processor | Overlap policy, embedded zeroes, truncation, malformed sequences, endian-independent serialization and explicit grammar | T12, T15, P3 |
| Allocation | Arena with marks, fixed-slot pool, stack discipline, bounded variable-size allocator with splitting/coalescing, segregated-class/slab comparison mechanism | Alignment, overflow, fragmentation, exhaustion, lifetime/rollback and metadata corruption tests | T16, P1 |
| Numerical mechanisms | Stable accumulation, fixed-point saturating kernel, matrix layout, interpolation, FIR filter and a small linear solve/iteration | Error contract, residual, conditioning, exact/reference comparison and bounded storage | T17, P6 |
| Abstractions | Callback/context, iterator, tagged union, opaque handle, allocator/backend interface and state machine | Signature compatibility, ownership, invalid states, reentrancy and replacement backend | T18, T19, T20 |
| Runtime and binary | Own entry adapter, minimal memory helpers, linker placement and Cortex-M reset/vector/data/bss initialization | Symbol/dependency closure, ABI alignment, section/segment map and return/fault policy | T08–T11, T27, P2/P4 |
| Systems services | Partial-I/O adapters, event loop, bounded work queue, shutdown/backpressure and portable protocol reuse | Error injection, readiness states, race/lifetime reasoning and resource ceilings | T21, T23–T25, P5 |
| Firmware services | GPIO/UART/timer/SPI/I2C and supported ADC/DMA driver milestones, ISR communication and cooperative scheduling | Register contracts, timeout/recovery, DMA ownership, overflow, timing limits and physical evidence | T28–T31, P4 |
| Verification tooling | Contract tests, property/differential cases, failure injector and reproducible benchmark | Independent oracle, minimized regressions, model assumptions and measurement variability | T32–T35 |

The default balanced-tree choice is AVL; a red-black tree is an acceptable substitution when its invariant and deletion contract are implemented and reviewed. Implementing both is elective. A project may offer a smaller preliminary representation, but it must culminate in the mechanism named in its core contract. If the supplied module chooses another balanced family, use that module's gate and record the substitution in the dossier.

## Component interfaces before code

Define element ownership, copy/move policy, comparator ordering and invalidation rules before choosing representation. A generic byte container cannot infer the lifetime or destruction needs of pointer members. Allocator callbacks need alignment, failure and free/reset contracts. A queue's capacity convention must be explicit: storage of size N is not always usable capacity N.

Core allocators manage a declared backing region. Avoid ordering unrelated allocated-object pointers as if ISO C guaranteed a global numeric address order. Store validated offsets into one backing array when that is the design. Historical allocator readings are critique material; their assumptions are not silently inherited.

Specify the backing region's object type as well as its alignment. For C17, distinguish genuine allocated storage, actual typed pool elements and declared byte arrays. Byte-array bookkeeping and byte payload operations do not grant unrestricted typed access to that storage. Use a representation-safe byte interface or an explicitly documented target contract; retain the T03 effective-type argument in the allocator dossier.

## Worked boundary reasoning

For a vector with `count` elements, require `element_size > 0` before dividing by it. Then prove `count <= SIZE_MAX / element_size` before multiplication. Growing capacity is another checked operation. If growth fails, the original buffer, count and ownership must still satisfy the invariant. A nominal successful append cannot validate that failure rule.

For a circular queue with capacity four and separate count, enqueueing four values fills it; the fifth returns `FULL` without modifying indices, data or count. After two dequeues and two enqueues, iteration follows logical order across the physical wrap. The integration fixtures test this representation-independent contract rather than prescribing your internal index strategy.

For a numerical kernel, choose an error measure before optimizing. Near a zero exact result, a relative-only tolerance is ill-defined or misleading. Use an absolute bound as well and explain conditioning. SIMD and compiler transformations must preserve the chosen contract, including tail elements and exceptional inputs.

One explicit comparison rule is

$$
|y_{\mathrm{computed}}-y_{\mathrm{reference}}|
\le \tau_{\mathrm{abs}}+\tau_{\mathrm{rel}}|y_{\mathrm{reference}}|.
$$

Choose both nonnegative tolerances from the numerical contract, rather than adjusting them after a failed run. The absolute tolerance has the output's units; the relative tolerance is dimensionless. With an exact reference of zero, an output of `1e-8` passes an absolute tolerance of `1e-7`. With reference `1000`, output `1000.002`, absolute tolerance `1e-7` and relative tolerance `1e-6`, the error `0.002` exceeds the allowed `0.0010001`. Account separately for uncertainty in a reference approximation. This rule checks a declared error criterion; it does not establish conditioning or stability by itself.

## Extensions

Additional tree/allocator families, lock-free reclamation, bytecode interpreter, shell, ELF loading, storage engine, RTOS context switching, kernel driver, signal transforms and specialized numerical kernels remain optional. Each extension needs its own inspected evidence, dependencies and gate. Awareness of their tradeoffs is retained even when their implementation is omitted.

<!-- performance-debugging guide extension -->

## Investigation adapters on existing components

Add exact work/lifetime counters, bounded event/timestamp adapters, a tri-state reducer, bounded compiler hooks and fault-dossier replay only where the new project asks for them. Reuse the earlier benchmark runner, parser, allocator, algorithm, event loop and driver. The new implementation is the experimental adapter or repair, not a duplicate mechanism library.

Defend cost units and model bounds, identify the first invalid state, preserve cleanup and ownership, and show regression plus delayed transfer. The project directory in {ref}`guide-performance-debugging` maps these additional milestones and their owning budgets.
