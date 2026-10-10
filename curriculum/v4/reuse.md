# Reuse source, contracts and evidence deliberately

The artifact registry identifies actual components. An import records a contract
version and immutable source snapshot, not merely the name of a previous project.
Export the library portion of the record tool; its command wrapper remains an
application. An ELF inspection, waveform or timing trace is evidence, not a
library that every later program must link.

## Handoff procedure

1. Before opening the old source, reconstruct the component's invariant and the
   assumptions the new consumer depends on.
2. Select its source revision and contract. Check allocation, blocking, callbacks,
   reentrancy, ISR use, retention, alignment, capacity and failure semantics.
3. Import the declared CMake target and actual source/include directories. Include
   the recursive pinned dependency closure so the handoff remains relocatable.
4. Write the adapter. Exercise partial progress, failure, exhaustion, cancellation
   or reset according to this new boundary; preserve the component regressions.
5. State which claims survive. Revalidate changed assumptions, then export only
   the claims and target scope that have evidence.

Do not rewrite the ring, codec or allocator because it is inconvenient to import.
A redesign is legitimate when a changed contract requires it; preserve the prior
revision and explain the migration. Never substitute assessor implementations
into ordinary learner imports. Assessor mode is explicit author validation.

## Example: one ring over several boundaries

First implement a caller-owned FIFO, including capacity 1, non-power-of-two wrap,
full/empty disambiguation and failure-preserving operations. A later threaded
consumer adds predicate-based blocking and shutdown. An SPSC variant changes the
publication argument. A DMA consumer introduces device completion and cache-line
ownership; an AMP consumer introduces peer epochs and independent reset.

FIFO ordering can be reused. Single-thread evidence does not establish atomic
publication, interrupt safety, cache visibility or DMA quiescence. Each handoff
names these separate claims so a new assumption cannot hide behind old tests.

## Compatibility boundaries

Preserve CVM1, the existing store format and kernel v1/v2 interfaces. A store
adapter must satisfy caller serialization, complete-page I/O and explicit
durability even when its underlying service is asynchronous. HTTP timeouts use
monotonic milliseconds; execution timers use their bounded modular tick contract.
Conversions must declare units and wrap limits. No universal runtime ABI is added.

Strict freestanding images still audit the entire link closure: no libc, libgcc,
compiler-rt, libatomic, HAL or existing OS implementation can supply the studied
original mechanism. Standard type headers and already-reviewed intrinsics retain
their existing policy. Hosted Linux and pinned FreeRTOS comparisons stay separate.

An accepted host-scoped component can be reused while its physical obligations
remain open. Editing source or a contract invalidates affected exported claims;
it does not erase historical learning or unrelated components.
