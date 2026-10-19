# Service image resource observations

`service_image_costs.json` records the exact executed image identities, raw
`size -A` output and compiler `.su` records for all eight compiler/optimization/
CPU profiles. The two/four-CPU runs use the same compiled profile; changing CPU
count changes guest execution, not the source's table capacities.

The observed kernel `.text` bytes were49,438 (GCC O0),38,321 (GCC O2),53,873
(Clang O0) and57,549 (Clang O2). Optimization did not universally shrink code.
ELF file length includes debug sections and is not loaded instruction size.
The `.bss` region is approximately1,006KB and includes retained static kernel
tables/stacks, not just the newly added services. Attribute costs using the map
and representation before claiming the service adapter alone uses that amount.

The largest reported single-function static frame is `load_task`:2560/2512
bytes for GCC O0/O2 and2552/2472 for Clang O0/O2. Its512-entry acquisition log
and ELF descriptor explain the scale. These are compiler frame estimates, not
whole-call-chain maxima: add called-function live frames, saved interrupt state,
alignment and the admitted nesting contract before defending a stack bound.
The source's serialization premise permits one static saved task descriptor;
removing that premise requires a different lifetime/reentrancy design.

Eight queues with two32-byte payload slots reserve512 payload bytes before
metadata, capabilities and waiter copies. Each blocked send adds one separately
owned pending payload in its fixed thread slot. File data copies are bounded by
the actual admitted read length (at most256), while seek/dup only update checked
metadata. The immutable archive backing is retained for guest lifetime. Two
mutable32-byte versions trade bounded storage for explicit FULL when readers
pin both versions. These representation costs explain rejected operations;
they do not establish universal overhead or latency advantages.

All full kernel/user images have no undefined symbols or dynamic dependencies.
Clang O0's emitted aggregate-initialization `memset` exposed a real missing
closure dependency; the builder now links original byte-loop helpers. Merely
using freestanding flags would not have established that closure. No physical
latency, WCET, cache-miss count or independent learner performance was observed.
