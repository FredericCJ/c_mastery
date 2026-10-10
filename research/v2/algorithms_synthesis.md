# Source-to-mechanism synthesis

- CLRS fourth edition, inspected insertion/heap/open-address/BFS/string sections:
  strict unsigned stable insertion, moving heap violation, bounded tombstone
  search, hop-distance oracle and length-delimited naive matching. The original C
  code makes capacities and error outputs explicit.
- Hanson first edition, inspected interface and arena scopes: caller contracts,
  bounded byte allocation, mark/rewind and typed-pool lifetime. Historical code is
  neither copied nor treated as a modern C safety oracle.
- Morin 0.1f linear probing: preserve lookup reachability across tombstones.
  The fixed test hash is deliberately collision-heavy; no expected constant-time
  result follows from it.
- Pinned AMD64 draft 0.99.6 and Arm AAPCS32 2025Q1 scalar scopes: integer/pointer
  argument placement, preserved registers, nested call and stack alignment probes.
  Windows x64, aggregate, variadic and floating-point PCS are outside scope.
- Cortex-M4 DUI0553B inspected instruction scopes: unsigned conditional branches,
  byte-addressed element scaling, loads/stores, calls and returns. QEMU exercises
  architectural behavior, not STM32 bus timing or electrical effects.

AVL, union-find, Dijkstra, compensated summation and cache-shadow mechanisms are
original bounded implementations connected to existing project contracts. Their
tests are executable evidence, not substituted reading certificates. Additional
mathematical proof and device-specific experiments remain learner obligations.
