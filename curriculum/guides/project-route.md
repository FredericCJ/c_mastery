# Project route and reusable artifacts

Follow this ordered project route after the entry diagnostic. Complete selected objective cards before their dependent experiments; a card does not require completing its entire domain. Early diagnosis and memory experiments recur later at greater depth.

Each accepted artifact carries its source revision, interface, ownership contract and evidence. Reusing it transfers only claims whose assumptions remain true. The directory does not add workload to the owning projects.

| Step | Project | New work | Prerequisite projects | Executable family |
|---|---|---|---|---|
| 1 | {ref}`PR-T01-01` | Trace, invariant and finite-representation diagnostic |  | foundations |
| 2 | {ref}`PR-T01-02` | Predict, reject and repair small C executions | {ref}`PR-T01-01` | foundations |
| 3 | {ref}`PR-T01-03` | Reproducible bounded list tool | {ref}`PR-T01-02` | foundations |
| 4 | {ref}`PR-T02-01` | Declarator and separate-unit interface catalog | {ref}`PR-T01-03` | foundations |
| 5 | {ref}`PR-T02-02` | Typed expressions and a two-pass variadic protocol | {ref}`PR-T01-03`, {ref}`PR-T02-01` | foundations |
| 6 | {ref}`PR-T02-03` | Initialization, lifetime and qualified views | {ref}`PR-T01-03`, {ref}`PR-T02-02` | foundations |
| 7 | {ref}`PR-T02-04` | Header definitions and negative compilation matrix | {ref}`PR-T01-03`, {ref}`PR-T02-03` | foundations |
| 8 | {ref}`PR-T03-01` | Lifetime and aligned-storage atlas | {ref}`PR-T02-04` | foundations |
| 9 | {ref}`PR-T03-02` | Nested array and bounded cursor | {ref}`PR-T02-04`, {ref}`PR-T03-01` | foundations |
| 10 | {ref}`PR-T03-03` | Raw-storage slot and alias contract audit | {ref}`PR-T02-04`, {ref}`PR-T03-02` | See project-specific evidence contract |
| 11 | {ref}`PR-T03-04` | Checked flexible buffer and resize transitions | {ref}`PR-T02-04`, {ref}`PR-T03-03` | foundations |
| 12 | {ref}`PR-T03-05` | Memory defect clinic and container recovery review | {ref}`PR-T02-04`, {ref}`PR-T03-04` | See project-specific evidence contract |
| 13 | {ref}`PR-T04-01` | Checked arithmetic and signed division kernel | {ref}`PR-T02-04` | foundations |
| 14 | {ref}`PR-T04-02` | Bit operations and explicit wire layout | {ref}`PR-T02-04`, {ref}`PR-T04-01` | foundations |
| 15 | {ref}`PR-T04-03` | Fixed-point rounding and saturation | {ref}`PR-T02-04`, {ref}`PR-T04-02` | See project-specific evidence contract |
| 16 | {ref}`PR-T04-04` | Floating contract and boundary laboratory | {ref}`PR-T02-04`, {ref}`PR-T04-03` | See project-specific evidence contract |
| 17 | {ref}`PR-T05-01` | Historical port and authority ledger | {ref}`PR-T02-04` | See project-specific evidence contract |
| 18 | {ref}`PR-T05-02` | C99 and C11 capability migration | {ref}`PR-T02-04`, {ref}`PR-T05-01` | foundations |
| 19 | {ref}`PR-T05-03` | Corrective-edition claim review | {ref}`PR-T02-04`, {ref}`PR-T05-02` | See project-specific evidence contract |
| 20 | {ref}`PR-T05-04` | C23 probes with a portable fallback | {ref}`PR-T02-04`, {ref}`PR-T05-03` | foundations |
| 21 | {ref}`PR-T05-05` | Extension and proposal replacement plan | {ref}`PR-T02-04`, {ref}`PR-T05-04` | See project-specific evidence contract |
| 22 | {ref}`PR-T06-01` | Preprocessor laboratory and safe wrappers | {ref}`PR-T02-04` | foundations |
| 23 | {ref}`PR-T06-02` | Authoritative registry and typed dispatch | {ref}`PR-T02-04`, {ref}`PR-T06-01` | foundations |
| 24 | {ref}`PR-T06-03` | Validated configuration and schema generator | {ref}`PR-T02-04`, {ref}`PR-T06-02` | See project-specific evidence contract |
| 25 | {ref}`PR-T07-01` | Reconstruct the four-stage build | {ref}`PR-T02-04`, {ref}`PR-T03-05` | foundations |
| 26 | {ref}`PR-T44-01` | Evidence-ready warning and logging dossier | {ref}`PR-T03-01`, {ref}`PR-T07-01` | foundations |
| 27 | {ref}`PR-T44-02` | Tri-state minimal reproducer | {ref}`PR-T44-01` | foundations |
| 28 | {ref}`PR-T44-03` | First invalid state with GDB | {ref}`PR-T44-02` | foundations |
| 29 | {ref}`PR-T45-01` | Prediction and controlled observation contract | {ref}`PR-T03-01`, {ref}`PR-T07-01` | foundations |
| 30 | {ref}`PR-T45-02` | Extent, underflow and terminator repair | {ref}`PR-T03-02`, {ref}`PR-T45-01` | foundations |
| 31 | {ref}`PR-T45-03` | Checked sizing and narrowing repair | {ref}`PR-T04-01`, {ref}`PR-T45-02` | foundations |
| 32 | {ref}`PR-T07-02` | Profiles and generated dependency graph | {ref}`PR-T02-04`, {ref}`PR-T03-05`, {ref}`PR-T07-01` | See project-specific evidence contract |
| 33 | {ref}`PR-T07-03` | Install and consume from a second repository | {ref}`PR-T02-04`, {ref}`PR-T03-05`, {ref}`PR-T07-02` | foundations |
| 34 | {ref}`PR-T07-04` | Cross-toolchain manifest | {ref}`PR-T02-04`, {ref}`PR-T03-05`, {ref}`PR-T07-03` | See project-specific evidence contract |
| 35 | {ref}`PR-T07-05` | Offline dependency and CI evidence | {ref}`PR-T02-04`, {ref}`PR-T03-05`, {ref}`PR-T07-04` | See project-specific evidence contract |
| 36 | {ref}`PR-T07-06` | Explain artifact differences | {ref}`PR-T02-04`, {ref}`PR-T03-05`, {ref}`PR-T07-05` | foundations |
| 37 | {ref}`PR-T08-01` | Decode computation and instruction bytes | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06` | See project-specific evidence contract |
| 38 | {ref}`PR-T08-02` | ABI boundary and frame laboratory | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T08-01` | See project-specific evidence contract |
| 39 | {ref}`PR-T08-03` | Assembly routine under a hostile harness | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T08-02` | See project-specific evidence contract |
| 40 | {ref}`PR-T08-04` | Inline contracts and trap boundaries | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T08-03` | See project-specific evidence contract |
| 41 | {ref}`PR-T09-01` | ELF symbols and archive resolution | {ref}`PR-T07-06`, {ref}`PR-T08-04` | See project-specific evidence contract |
| 42 | {ref}`PR-T09-02` | Relocation and position independence | {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T09-01` | See project-specific evidence contract |
| 43 | {ref}`PR-T09-03` | Loader, TLS and versioned exports | {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T09-02` | See project-specific evidence contract |
| 44 | {ref}`PR-T09-04` | Linker layout, retention and budgets | {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T09-03` | See project-specific evidence contract |
| 45 | {ref}`PR-T09-05` | Raw image with a debug companion | {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T09-04` | See project-specific evidence contract |
| 46 | {ref}`PR-T10-01` | Entry stack and startup ownership | {ref}`PR-T09-05` | See project-specific evidence contract |
| 47 | {ref}`PR-T10-02` | Runtime initialization and exit choices | {ref}`PR-T09-05`, {ref}`PR-T10-01` | See project-specific evidence contract |
| 48 | {ref}`PR-T10-03` | Raw syscall transfer adapter | {ref}`PR-T09-05`, {ref}`PR-T10-02` | See project-specific evidence contract |
| 49 | {ref}`PR-T10-04` | Grow and debug a minimal image | {ref}`PR-T09-05`, {ref}`PR-T10-03` | See project-specific evidence contract |
| 50 | {ref}`PR-T11-01` | Dependency-profile option matrix | {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T09-05` | See project-specific evidence contract |
| 51 | {ref}`PR-P2-01` | Custom entry meets the portable core | {ref}`PR-T09-01`, {ref}`PR-T10-01`, {ref}`PR-T11-01` | See project-specific evidence contract |
| 52 | {ref}`PR-T11-02` | Compiler helper census | {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T11-01` | See project-specific evidence contract |
| 53 | {ref}`PR-T11-03` | Byte support without external library calls | {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T11-02` | See project-specific evidence contract |
| 54 | {ref}`PR-T11-04` | Refusing environment adapter | {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T11-03` | See project-specific evidence contract |
| 55 | {ref}`PR-T11-05` | Automated object and image audit | {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T11-04` | See project-specific evidence contract |
| 56 | {ref}`PR-T12-01` | Header, type and bounded-format inventory | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T06-03` | See project-specific evidence contract |
| 57 | {ref}`PR-T12-02` | Byte buffers, locale boundary and chunked text | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T06-03`, {ref}`PR-T12-01` | foundations |
| 58 | {ref}`PR-T12-03` | Allocation adapter and strict conversion parser | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T06-03`, {ref}`PR-T12-02` | foundations |
| 59 | {ref}`PR-T12-04` | Stream state and execution-environment notebook | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T06-03`, {ref}`PR-T12-03` | See project-specific evidence contract |
| 60 | {ref}`PR-T12-05` | Sorting, math and pinned implementation literacy | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T06-03`, {ref}`PR-T12-04` | foundations |
| 61 | {ref}`PR-T13-01` | Effects census with interception blind spots | {ref}`PR-T07-06`, {ref}`PR-T12-05` | See project-specific evidence contract |
| 62 | {ref}`PR-T13-02` | Stack, locks and fresh-process costs | {ref}`PR-T07-06`, {ref}`PR-T12-05`, {ref}`PR-T13-01` | See project-specific evidence contract |
| 63 | {ref}`PR-T13-03` | State and execution-context matrix | {ref}`PR-T07-06`, {ref}`PR-T12-05`, {ref}`PR-T13-02` | See project-specific evidence contract |
| 64 | {ref}`PR-T13-04` | Pinned sort effects and restricted replacement | {ref}`PR-T07-06`, {ref}`PR-T12-05`, {ref}`PR-T13-03` | See project-specific evidence contract |
| 65 | {ref}`PR-T18-01` | Transactional bounded-view API | {ref}`PR-T03-05`, {ref}`PR-T12-05` | foundations |
| 66 | {ref}`PR-T18-02` | Owner, borrowed view and stable handle | {ref}`PR-T03-05`, {ref}`PR-T12-05`, {ref}`PR-T18-01` | foundations |
| 67 | {ref}`PR-T18-03` | Lifecycle and representation alternatives | {ref}`PR-T03-05`, {ref}`PR-T12-05`, {ref}`PR-T18-02` | See project-specific evidence contract |
| 68 | {ref}`PR-T18-04` | Execution-context and compatible API evolution | {ref}`PR-T03-05`, {ref}`PR-T12-05`, {ref}`PR-T18-03` | foundations |
| 69 | {ref}`PR-T14-01` | Caller-owned arrays, views and rings | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T18-04` | algorithms |
| 70 | {ref}`PR-T14-02` | Failure-preserving dynamic array | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T14-01`, {ref}`PR-T18-04` | See project-specific evidence contract |
| 71 | {ref}`PR-T14-03` | Linked ownership and queue ends | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T14-02`, {ref}`PR-T18-04` | See project-specific evidence contract |
| 72 | {ref}`PR-T14-04` | Collision and tombstone tables | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T14-03`, {ref}`PR-T18-04` | algorithms |
| 73 | {ref}`PR-T14-05` | Heap and ordered-tree laboratory | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T14-04`, {ref}`PR-T18-04` | algorithms |
| 74 | {ref}`PR-T14-06` | Complete AVL family | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T14-05`, {ref}`PR-T18-04` | algorithms |
| 75 | {ref}`PR-T14-07` | Byte prefixes and set representations | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T14-06`, {ref}`PR-T18-04` | See project-specific evidence contract |
| 76 | {ref}`PR-T14-08` | Partitions and graph storage | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T14-07`, {ref}`PR-T18-04` | algorithms |
| 77 | {ref}`PR-T14-09` | One advanced structure for a workload | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T18-04` | See project-specific evidence contract |
| 78 | {ref}`PR-T15-01` | Bounds and comparison-sort suite | {ref}`PR-T04-04`, {ref}`PR-T14-08` | algorithms |
| 79 | {ref}`PR-T43-01` | A cost contract before a stopwatch | {ref}`PR-T07-01`, {ref}`PR-T15-01` | foundations |
| 80 | {ref}`PR-T43-02` | Prefix credits for a reused growing table | {ref}`PR-T14-02`, {ref}`PR-T43-01` | See project-specific evidence contract |
| 81 | {ref}`PR-T15-02` | Duplicate-aware quicksort and selection | {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T15-01` | See project-specific evidence contract |
| 82 | {ref}`PR-T15-03` | Counting and radix domains | {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T15-02` | See project-specific evidence contract |
| 83 | {ref}`PR-T15-04` | Bounded byte matching | {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T15-03` | algorithms |
| 84 | {ref}`PR-T15-05` | Traversals, DAGs and weighted paths | {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T15-04` | algorithms |
| 85 | {ref}`PR-T15-06` | Exchange argument and compressed DP | {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T15-05` | See project-specific evidence contract |
| 86 | {ref}`PR-T15-07` | Span kernel and checked integers | {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T15-06` | See project-specific evidence contract |
| 87 | {ref}`PR-T15-08` | Wire records and incremental frames | {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T15-07` | See project-specific evidence contract |
| 88 | {ref}`PR-P3-01` | One wire format for both tracks | {ref}`PR-T04-02`, {ref}`PR-T15-08`, {ref}`PR-T18-01` | See project-specific evidence contract |
| 89 | {ref}`PR-T15-09` | Tokenizer and precedence parser | {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T15-08` | See project-specific evidence contract |
| 90 | {ref}`PR-T15-10` | Specialized algorithm with a reference | {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T15-09` | See project-specific evidence contract |
| 91 | {ref}`PR-T16-01` | Aligned caller-storage arena | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T18-04` | algorithms |
| 92 | {ref}`PR-P1-01` | Bounded library integration | {ref}`PR-T14-01`, {ref}`PR-T14-03`, {ref}`PR-T16-01`, {ref}`PR-T18-02` | See project-specific evidence contract |
| 93 | {ref}`PR-T16-02` | Nested marks and LIFO release | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T16-01`, {ref}`PR-T18-04` | algorithms |
| 94 | {ref}`PR-T16-03` | Generation-counted fixed pool | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T16-02`, {ref}`PR-T18-04` | algorithms |
| 95 | {ref}`PR-T16-04` | Free lists, tags and coalescing | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T16-03`, {ref}`PR-T18-04` | See project-specific evidence contract |
| 96 | {ref}`PR-T16-05` | Size classes and retained slabs | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T16-04`, {ref}`PR-T18-04` | See project-specific evidence contract |
| 97 | {ref}`PR-T16-06` | Buddy policy under bounded storage | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T16-05`, {ref}`PR-T18-04` | See project-specific evidence contract |
| 98 | {ref}`PR-T16-07` | Failure-preserving resize | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T16-06`, {ref}`PR-T18-04` | See project-specific evidence contract |
| 99 | {ref}`PR-T16-08` | Backends and lifetime architecture | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T16-07`, {ref}`PR-T18-04` | See project-specific evidence contract |
| 100 | {ref}`PR-T16-09` | Adversarial allocator evidence | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T16-08`, {ref}`PR-T18-04` | algorithms |
| 101 | {ref}`PR-T17-01` | Accumulation and error ledger | {ref}`PR-T04-04`, {ref}`PR-T14-08` | algorithms |
| 102 | {ref}`PR-T17-02` | Saturating Q7 multiply-accumulate | {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T17-01` | algorithms |
| 103 | {ref}`PR-T17-03` | Strided matrix and layout kernel | {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T17-02` | See project-specific evidence contract |
| 104 | {ref}`PR-T17-04` | Clamped interpolation and approximation | {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T17-03` | See project-specific evidence contract |
| 105 | {ref}`PR-T17-05` | Streaming FIR and first-order IIR | {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T17-04` | See project-specific evidence contract |
| 106 | {ref}`PR-T17-06` | Pivoted 2x2 solver | {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T17-05` | See project-specific evidence contract |
| 107 | {ref}`PR-T17-07` | Euler convergence experiment | {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T17-06` | See project-specific evidence contract |
| 108 | {ref}`PR-T19-01` | Stateful object with operation table and callbacks | {ref}`PR-T06-03`, {ref}`PR-T18-04` | foundations |
| 109 | {ref}`PR-T19-02` | Generic storage and tagged result variants | {ref}`PR-T06-03`, {ref}`PR-T18-04`, {ref}`PR-T19-01` | See project-specific evidence contract |
| 110 | {ref}`PR-T19-03` | Pure state strategies and two traversal styles | {ref}`PR-T06-03`, {ref}`PR-T18-04`, {ref}`PR-T19-02` | See project-specific evidence contract |
| 111 | {ref}`PR-T19-04` | Composable short-write stream and stable handles | {ref}`PR-T06-03`, {ref}`PR-T18-04`, {ref}`PR-T19-03` | foundations |
| 112 | {ref}`PR-T20-01` | Portable record core and three semantic adapters | {ref}`PR-T18-04`, {ref}`PR-T19-04` | foundations |
| 113 | {ref}`PR-T20-02` | Layer responsibility and portable service slice | {ref}`PR-T18-04`, {ref}`PR-T19-04`, {ref}`PR-T20-01` | foundations |
| 114 | {ref}`PR-T20-03` | Bounded execution and cooperating machines | {ref}`PR-T18-04`, {ref}`PR-T19-04`, {ref}`PR-T20-02` | foundations |
| 115 | {ref}`PR-T20-04` | Configuration identity and fault decision review | {ref}`PR-T18-04`, {ref}`PR-T19-04`, {ref}`PR-T20-03` | See project-specific evidence contract |
| 116 | {ref}`PR-T21-01` | Error translation and failure-complete construction | {ref}`PR-T18-04`, {ref}`PR-T20-04` | foundations |
| 117 | {ref}`PR-P1-02` | Exhaustion and dependency closure | {ref}`PR-P1-01`, {ref}`PR-T13-01`, {ref}`PR-T16-09`, {ref}`PR-T21-01` | See project-specific evidence contract |
| 118 | {ref}`PR-T21-02` | Transactional update and cancellation ownership | {ref}`PR-T18-04`, {ref}`PR-T20-04`, {ref}`PR-T21-01` | foundations |
| 119 | {ref}`PR-T21-03` | Bounded observability with independent switches | {ref}`PR-T18-04`, {ref}`PR-T20-04`, {ref}`PR-T21-02` | foundations |
| 120 | {ref}`PR-T21-04` | Correlatable fault record and recovery boundary | {ref}`PR-T18-04`, {ref}`PR-T20-04`, {ref}`PR-T21-03` | foundations |
| 121 | {ref}`PR-T22-01` | Readable repository and executable API documentation | {ref}`PR-T18-04`, {ref}`PR-T21-04` | foundations |
| 122 | {ref}`PR-T22-02` | Traceable package consumed outside the tree | {ref}`PR-T18-04`, {ref}`PR-T21-04`, {ref}`PR-T22-01` | foundations |
| 123 | {ref}`PR-T22-03` | Foreign consumer and dependency provenance | {ref}`PR-T18-04`, {ref}`PR-T21-04`, {ref}`PR-T22-02` | foundations |
| 124 | {ref}`PR-T22-04` | Review, minimized regression and two reproducible releases | {ref}`PR-T18-04`, {ref}`PR-T21-04`, {ref}`PR-T22-03` | See project-specific evidence contract |
| 125 | {ref}`PR-T23-01` | Process launcher and descriptor ledger | {ref}`PR-T07-06`, {ref}`PR-T18-04` | See project-specific evidence contract |
| 126 | {ref}`PR-T23-02` | Robust files, offsets and namespaces | {ref}`PR-T07-06`, {ref}`PR-T18-04`, {ref}`PR-T23-01` | See project-specific evidence contract |
| 127 | {ref}`PR-P2-02` | Short transfer and syscall dossier | {ref}`PR-P2-01`, {ref}`PR-T10-03`, {ref}`PR-T21-01`, {ref}`PR-T23-02` | See project-specific evidence contract |
| 128 | {ref}`PR-T23-03` | Mappings and relocatable shared records | {ref}`PR-T07-06`, {ref}`PR-T18-04`, {ref}`PR-T23-02` | See project-specific evidence contract |
| 129 | {ref}`PR-T23-04` | Pipeline and terminal restoration | {ref}`PR-T07-06`, {ref}`PR-T18-04`, {ref}`PR-T23-03` | See project-specific evidence contract |
| 130 | {ref}`PR-T23-05` | File locking and interrupted updates | {ref}`PR-T07-06`, {ref}`PR-T18-04`, {ref}`PR-T23-04` | See project-specific evidence contract |
| 131 | {ref}`PR-T23-06` | Clock and capability boundaries | {ref}`PR-T07-06`, {ref}`PR-T18-04`, {ref}`PR-T23-05` | See project-specific evidence contract |
| 132 | {ref}`PR-T24-01` | Signals without a lost wakeup | {ref}`PR-T15-09`, {ref}`PR-T23-06` | See project-specific evidence contract |
| 133 | {ref}`PR-T24-02` | Local request/reply ownership | {ref}`PR-T15-09`, {ref}`PR-T23-06`, {ref}`PR-T24-01` | See project-specific evidence contract |
| 134 | {ref}`PR-T24-03` | Streams and datagram policies | {ref}`PR-T15-09`, {ref}`PR-T23-06`, {ref}`PR-T24-02` | See project-specific evidence contract |
| 135 | {ref}`PR-T24-04` | Readiness loop and generation identities | {ref}`PR-T15-09`, {ref}`PR-T23-06`, {ref}`PR-T24-03` | See project-specific evidence contract |
| 136 | {ref}`PR-T24-05` | Deadline and notification counters | {ref}`PR-T15-09`, {ref}`PR-T23-06`, {ref}`PR-T24-04` | See project-specific evidence contract |
| 137 | {ref}`PR-T24-06` | One parser, serial and test backends | {ref}`PR-T15-09`, {ref}`PR-T23-06`, {ref}`PR-T24-05` | See project-specific evidence contract |
| 138 | {ref}`PR-T24-07` | Advanced I/O decision dossier | {ref}`PR-T15-09`, {ref}`PR-T23-06`, {ref}`PR-T24-06` | See project-specific evidence contract |
| 139 | {ref}`PR-T25-01` | Thread-owned work and join obligations | {ref}`PR-T02-04`, {ref}`PR-T03-05`, {ref}`PR-T05-04`, {ref}`PR-T18-04` | See project-specific evidence contract |
| 140 | {ref}`PR-P5-01` | Concurrent ownership and backpressure | {ref}`PR-T18-02`, {ref}`PR-T24-04`, {ref}`PR-T25-01` | See project-specific evidence contract |
| 141 | {ref}`PR-T25-02` | Bounded queue with predicates | {ref}`PR-T02-04`, {ref}`PR-T03-05`, {ref}`PR-T05-04`, {ref}`PR-T18-04`, {ref}`PR-T25-01` | See project-specific evidence contract |
| 142 | {ref}`PR-P5-02` | Shutdown under load | {ref}`PR-P5-01`, {ref}`PR-T21-02`, {ref}`PR-T24-04`, {ref}`PR-T25-02` | See project-specific evidence contract |
| 143 | {ref}`PR-T25-03` | Publication, atomics and CAS traces | {ref}`PR-T02-04`, {ref}`PR-T03-05`, {ref}`PR-T05-04`, {ref}`PR-T18-04`, {ref}`PR-T25-02` | See project-specific evidence contract |
| 144 | {ref}`PR-T25-04` | Progress and locality counterexamples | {ref}`PR-T02-04`, {ref}`PR-T03-05`, {ref}`PR-T05-04`, {ref}`PR-T18-04`, {ref}`PR-T25-03` | See project-specific evidence contract |
| 145 | {ref}`PR-T25-05` | Concurrency argument plus detection | {ref}`PR-T02-04`, {ref}`PR-T03-05`, {ref}`PR-T05-04`, {ref}`PR-T18-04`, {ref}`PR-T25-04` | See project-specific evidence contract |
| 146 | {ref}`PR-T26-01` | Histories, progress and SPSC awareness | {ref}`PR-T14-08`, {ref}`PR-T25-05` | See project-specific evidence contract |
| 147 | {ref}`PR-T26-02` | Identity and reclamation awareness | {ref}`PR-T14-08`, {ref}`PR-T25-05`, {ref}`PR-T26-01` | See project-specific evidence contract |
| 148 | {ref}`PR-T26-03` | Kernel ordering and lifetime boundary | {ref}`PR-T14-08`, {ref}`PR-T25-05`, {ref}`PR-T26-02` | See project-specific evidence contract |
| 149 | {ref}`PR-T26-04` | Optional SPSC component | {ref}`PR-T14-08`, {ref}`PR-T25-05`, {ref}`PR-T26-03` | See project-specific evidence contract |
| 150 | {ref}`PR-T26-05` | Optional advanced object study | {ref}`PR-T14-08`, {ref}`PR-T25-05`, {ref}`PR-T26-04` | See project-specific evidence contract |
| 151 | {ref}`PR-T27-01` | Target and reset evidence ledger | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T11-05` | See project-specific evidence contract |
| 152 | {ref}`PR-T27-02` | Vector and stack image | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T11-05`, {ref}`PR-T27-01` | See project-specific evidence contract |
| 153 | {ref}`PR-T27-03` | Startup memory and runtime hooks | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T11-05`, {ref}`PR-T27-02` | See project-specific evidence contract |
| 154 | {ref}`PR-T27-04` | Minimal startup replacement | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T11-05`, {ref}`PR-T27-03` | See project-specific evidence contract |
| 155 | {ref}`PR-T27-05` | Retained state and standalone boot | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T11-05`, {ref}`PR-T27-04` | See project-specific evidence contract |
| 156 | {ref}`PR-T28-01` | Side-effect MMIO and GPIO | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T20-04`, {ref}`PR-T27-05` | See project-specific evidence contract |
| 157 | {ref}`PR-T28-02` | Clock-aware bounded UART | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T20-04`, {ref}`PR-T27-05`, {ref}`PR-T28-01` | See project-specific evidence contract |
| 158 | {ref}`PR-T28-03` | Timer and PWM configuration | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T20-04`, {ref}`PR-T27-05`, {ref}`PR-T28-02` | See project-specific evidence contract |
| 159 | {ref}`PR-T28-04` | Bounded SPI and one-byte I2C transaction | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T20-04`, {ref}`PR-T27-05`, {ref}`PR-T28-03` | See project-specific evidence contract |
| 160 | {ref}`PR-T28-05` | Single-channel ADC acquisition | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T20-04`, {ref}`PR-T27-05`, {ref}`PR-T28-04` | See project-specific evidence contract |
| 161 | {ref}`PR-T28-06` | DMA buffer and portable service boundary | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T20-04`, {ref}`PR-T27-05`, {ref}`PR-T28-05` | See project-specific evidence contract |
| 162 | {ref}`PR-T28-07` | Elective Ethernet diagnostic service | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T20-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06` | See project-specific evidence contract |
| 163 | {ref}`PR-T29-01` | Exception frame and nesting decoder | {ref}`PR-T27-05`, {ref}`PR-T28-06` | See project-specific evidence contract |
| 164 | {ref}`PR-T29-02` | Bounded ISR and nested masking | {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-01` | See project-specific evidence contract |
| 165 | {ref}`PR-T29-03` | Foreground/ISR bounded queue | {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-02` | See project-specific evidence contract |
| 166 | {ref}`PR-P4-01` | Acquisition to bounded frames | {ref}`PR-T27-04`, {ref}`PR-T28-02`, {ref}`PR-T29-03` | See project-specific evidence contract |
| 167 | {ref}`PR-T29-04` | DMA cancellation and visibility | {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-03` | See project-specific evidence contract |
| 168 | {ref}`PR-T29-05` | Coherent time and delayed-service validation | {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-04` | See project-specific evidence contract |
| 169 | {ref}`PR-T30-01` | Bounded superloop and cooperative release | {ref}`PR-T20-04`, {ref}`PR-T21-04`, {ref}`PR-T29-05` | See project-specific evidence contract |
| 170 | {ref}`PR-T30-02` | RTOS ownership and resource ledger | {ref}`PR-T20-04`, {ref}`PR-T21-04`, {ref}`PR-T29-05`, {ref}`PR-T30-01` | See project-specific evidence contract |
| 171 | {ref}`PR-T30-03` | Rollover and latency ledger | {ref}`PR-T20-04`, {ref}`PR-T21-04`, {ref}`PR-T29-05`, {ref}`PR-T30-02` | See project-specific evidence contract |
| 172 | {ref}`PR-T30-04` | Fixed-priority response analysis | {ref}`PR-T20-04`, {ref}`PR-T21-04`, {ref}`PR-T29-05`, {ref}`PR-T30-03` | See project-specific evidence contract |
| 173 | {ref}`PR-T30-05` | Meaningful watchdog and sleep entry | {ref}`PR-T20-04`, {ref}`PR-T21-04`, {ref}`PR-T29-05`, {ref}`PR-T30-04` | See project-specific evidence contract |
| 174 | {ref}`PR-T31-01` | Debug connection and optimized-state notebook | {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05` | See project-specific evidence contract |
| 175 | {ref}`PR-T31-02` | Fault and stack capture | {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-01` | See project-specific evidence contract |
| 176 | {ref}`PR-T31-03` | Peripheral/DMA diagnosis | {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-02` | See project-specific evidence contract |
| 177 | {ref}`PR-T31-04` | Bounded trace and release deployment | {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-03` | See project-specific evidence contract |
| 178 | {ref}`PR-T31-05` | Interrupted update and bounded recovery | {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-04` | See project-specific evidence contract |
| 179 | {ref}`PR-P4-02` | Timing and recovery boundaries | {ref}`PR-P4-01`, {ref}`PR-T29-05`, {ref}`PR-T30-03`, {ref}`PR-T31-05` | See project-specific evidence contract |
| 180 | {ref}`PR-T32-01` | Contract test architecture and adapters | {ref}`PR-T07-06`, {ref}`PR-T12-05`, {ref}`PR-T18-04` | foundations |
| 181 | {ref}`PR-T32-02` | Model sequences and metamorphic checks | {ref}`PR-T07-06`, {ref}`PR-T12-05`, {ref}`PR-T18-04`, {ref}`PR-T32-01` | foundations |
| 182 | {ref}`PR-T32-03` | Bounded byte fuzz harness | {ref}`PR-T07-06`, {ref}`PR-T12-05`, {ref}`PR-T18-04`, {ref}`PR-T32-02` | See project-specific evidence contract |
| 183 | {ref}`PR-P3-02` | Streaming and malformed-input campaign | {ref}`PR-P3-01`, {ref}`PR-T15-08`, {ref}`PR-T21-02`, {ref}`PR-T32-03` | See project-specific evidence contract |
| 184 | {ref}`PR-T32-04` | Acquisition and partial-progress injection | {ref}`PR-T07-06`, {ref}`PR-T12-05`, {ref}`PR-T18-04`, {ref}`PR-T32-03` | foundations |
| 185 | {ref}`PR-T32-05` | Native instrumentation, Memcheck and static alarms | {ref}`PR-T07-06`, {ref}`PR-T12-05`, {ref}`PR-T18-04`, {ref}`PR-T32-04` | See project-specific evidence contract |
| 186 | {ref}`PR-T32-06` | MC/DC pairs and mutation strength | {ref}`PR-T07-06`, {ref}`PR-T12-05`, {ref}`PR-T18-04`, {ref}`PR-T32-05` | foundations |
| 187 | {ref}`PR-T32-07` | Debugging and environment replay package | {ref}`PR-T07-06`, {ref}`PR-T12-05`, {ref}`PR-T18-04`, {ref}`PR-T32-06` | foundations |
| 188 | {ref}`PR-T33-01` | Prefix invariant and termination | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T18-04`, {ref}`PR-T32-07` | foundations |
| 189 | {ref}`PR-T33-02` | Machine arithmetic and ACSL frame | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T33-01` | foundations |
| 190 | {ref}`PR-T33-03` | CBMC harness and demanding environment | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T33-02` | foundations |
| 191 | {ref}`PR-T33-04` | WP obligations and explicit assumptions | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T33-03` | See project-specific evidence contract |
| 192 | {ref}`PR-T33-05` | Eva precision and composed claims | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T33-04` | See project-specific evidence contract |
| 193 | {ref}`PR-T33-06` | Elective finite publication model | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T14-08`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T33-05` | See project-specific evidence contract |
| 194 | {ref}`PR-T34-01` | Workload gate and traffic hypothesis | {ref}`PR-T08-04`, {ref}`PR-T17-06`, {ref}`PR-T32-07` | foundations |
| 195 | {ref}`PR-P6-01` | Numerical contract before optimization | {ref}`PR-T17-01`, {ref}`PR-T17-05`, {ref}`PR-T34-01` | See project-specific evidence contract |
| 196 | {ref}`PR-T34-02` | Layout and branch experiment | {ref}`PR-T08-04`, {ref}`PR-T17-06`, {ref}`PR-T32-07`, {ref}`PR-T34-01` | See project-specific evidence contract |
| 197 | {ref}`PR-T34-03` | Auto-vectorization and alias evidence | {ref}`PR-T08-04`, {ref}`PR-T17-06`, {ref}`PR-T32-07`, {ref}`PR-T34-02` | See project-specific evidence contract |
| 198 | {ref}`PR-T34-04` | SSE2 kernel and safe dispatch | {ref}`PR-T08-04`, {ref}`PR-T17-06`, {ref}`PR-T32-07`, {ref}`PR-T34-03` | See project-specific evidence contract |
| 199 | {ref}`PR-T34-05` | Benchmark, sampling and counter record | {ref}`PR-T08-04`, {ref}`PR-T17-06`, {ref}`PR-T32-07`, {ref}`PR-T34-04` | foundations |
| 200 | {ref}`PR-P6-02` | Vectorization and reproducible measurement | {ref}`PR-P6-01`, {ref}`PR-T34-03`, {ref}`PR-T34-04`, {ref}`PR-T34-05` | See project-specific evidence contract |
| 201 | {ref}`PR-T34-06` | Whole-program and embedded trade-off | {ref}`PR-T08-04`, {ref}`PR-T17-06`, {ref}`PR-T32-07`, {ref}`PR-T34-05` | See project-specific evidence contract |
| 202 | {ref}`PR-T35-01` | Bounded authorized command decoder | {ref}`PR-T18-04`, {ref}`PR-T21-04`, {ref}`PR-T23-06`, {ref}`PR-T32-07` | foundations |
| 203 | {ref}`PR-CAP-LINUX-01` | Linux architecture and threat model | {ref}`PR-P1-02`, {ref}`PR-P2-02`, {ref}`PR-P3-02`, {ref}`PR-P5-02`, {ref}`PR-P6-02`, {ref}`PR-T20-01`, {ref}`PR-T22-01`, {ref}`PR-T35-01` | See project-specific evidence contract |
| 204 | {ref}`PR-CAP-LINUX-02` | Service assembly and packaging | {ref}`PR-CAP-LINUX-01`, {ref}`PR-P1-02`, {ref}`PR-P2-02`, {ref}`PR-P3-02`, {ref}`PR-P5-02`, {ref}`PR-P6-02`, {ref}`PR-T22-02`, {ref}`PR-T23-02`, {ref}`PR-T24-04` | See project-specific evidence contract |
| 205 | {ref}`PR-CAP-MCU-01` | Firmware architecture and resource contract | {ref}`PR-P1-02`, {ref}`PR-P3-02`, {ref}`PR-P4-02`, {ref}`PR-P6-02`, {ref}`PR-T20-01`, {ref}`PR-T30-02`, {ref}`PR-T35-01` | See project-specific evidence contract |
| 206 | {ref}`PR-CAP-MCU-02` | Image assembly and target acceptance | {ref}`PR-CAP-MCU-01`, {ref}`PR-P1-02`, {ref}`PR-P3-02`, {ref}`PR-P4-02`, {ref}`PR-P6-02`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T31-04` | See project-specific evidence contract |
| 207 | {ref}`PR-T35-02` | Descriptor authority and permitted-effect worker | {ref}`PR-T18-04`, {ref}`PR-T21-04`, {ref}`PR-T23-06`, {ref}`PR-T32-07`, {ref}`PR-T35-01` | See project-specific evidence contract |
| 208 | {ref}`PR-CAP-MCU-03` | Fault containment and update policy | {ref}`PR-CAP-MCU-02`, {ref}`PR-P1-02`, {ref}`PR-P3-02`, {ref}`PR-P4-02`, {ref}`PR-P6-02`, {ref}`PR-T31-05`, {ref}`PR-T35-02` | See project-specific evidence contract |
| 209 | {ref}`PR-T35-03` | Quota and fail-safe recovery service | {ref}`PR-T18-04`, {ref}`PR-T21-04`, {ref}`PR-T23-06`, {ref}`PR-T32-07`, {ref}`PR-T35-02` | foundations |
| 210 | {ref}`PR-CAP-LINUX-03` | Fault and overload campaign | {ref}`PR-CAP-LINUX-02`, {ref}`PR-P1-02`, {ref}`PR-P2-02`, {ref}`PR-P3-02`, {ref}`PR-P5-02`, {ref}`PR-P6-02`, {ref}`PR-T21-01`, {ref}`PR-T32-04`, {ref}`PR-T35-03` | See project-specific evidence contract |
| 211 | {ref}`PR-T35-04` | Actual ELF and runtime hardening audit | {ref}`PR-T18-04`, {ref}`PR-T21-04`, {ref}`PR-T23-06`, {ref}`PR-T32-07`, {ref}`PR-T35-03` | See project-specific evidence contract |
| 212 | {ref}`PR-T35-05` | Synthetic secret lifetime and work leakage | {ref}`PR-T18-04`, {ref}`PR-T21-04`, {ref}`PR-T23-06`, {ref}`PR-T32-07`, {ref}`PR-T35-04` | See project-specific evidence contract |
| 213 | {ref}`PR-T35-06` | Artifact integrity and security review | {ref}`PR-T18-04`, {ref}`PR-T21-04`, {ref}`PR-T23-06`, {ref}`PR-T32-07`, {ref}`PR-T35-05` | See project-specific evidence contract |
| 214 | {ref}`PR-CAP-LINUX-04` | Linux defense and transfer | {ref}`PR-CAP-LINUX-03`, {ref}`PR-P1-02`, {ref}`PR-P2-02`, {ref}`PR-P3-02`, {ref}`PR-P5-02`, {ref}`PR-P6-02`, {ref}`PR-T20-04`, {ref}`PR-T33-05`, {ref}`PR-T35-06` | See project-specific evidence contract |
| 215 | {ref}`PR-CAP-MCU-04` | Firmware defense and transfer | {ref}`PR-CAP-MCU-03`, {ref}`PR-P1-02`, {ref}`PR-P3-02`, {ref}`PR-P4-02`, {ref}`PR-P6-02`, {ref}`PR-T30-04`, {ref}`PR-T33-05`, {ref}`PR-T35-06` | See project-specific evidence contract |
| 216 | {ref}`PR-T36-01` | Invariant and representation ledger | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T05-04`, {ref}`PR-T06-03`, {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04` | algorithms |
| 217 | {ref}`PR-T36-02` | Paired original assembly lower-bound kernels | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T05-04`, {ref}`PR-T06-03`, {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T36-01` | algorithms |
| 218 | {ref}`PR-T36-03` | Tagged stable insertion in assembly | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T05-04`, {ref}`PR-T06-03`, {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T36-02` | algorithms |
| 219 | {ref}`PR-T36-04` | Moving heap violation in original assembly | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T05-04`, {ref}`PR-T06-03`, {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T36-03` | algorithms |
| 220 | {ref}`PR-T36-05` | AVL mutation certificates and path costs | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T05-04`, {ref}`PR-T06-03`, {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T36-04` | algorithms |
| 221 | {ref}`PR-T36-06` | Tombstone probing and byte-span adapters | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T05-04`, {ref}`PR-T06-03`, {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T36-05` | algorithms |
| 222 | {ref}`PR-T36-07` | Graph frontier refinement and checked distances | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T05-04`, {ref}`PR-T06-03`, {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T36-06` | algorithms |
| 223 | {ref}`PR-T36-08` | Typed and context interfaces over existing kernels | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T05-04`, {ref}`PR-T06-03`, {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T36-07` | See project-specific evidence contract |
| 224 | {ref}`PR-T37-01` | Exhaustive four-bit ALU from Boolean slices | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T08-04` | algorithms |
| 225 | {ref}`PR-T37-02` | Real instruction bytes and register/address boundaries | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T37-01` | See project-specific evidence contract |
| 226 | {ref}`PR-T37-03` | Clocked toy datapath and commit certificates | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T37-02` | See project-specific evidence contract |
| 227 | {ref}`PR-T37-04` | Forwarding, stalls and branch squash schedule | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T37-03` | See project-specific evidence contract |
| 228 | {ref}`PR-T37-05` | Cache tags, miss mechanisms and dimensional costs | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T37-04` | algorithms |
| 229 | {ref}`PR-T37-06` | MCU master paths and DMA placement ledger | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T37-05` | See project-specific evidence contract |
| 230 | {ref}`PR-T37-07` | Call ABI versus reset and exception frame | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T37-06` | algorithms |
| 231 | {ref}`PR-T37-08` | Work, cycles and repeated native measurements | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T08-04`, {ref}`PR-T37-07` | algorithms |
| 232 | {ref}`PR-T38-01` | UART architecture, frame and baud laboratory | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05` | See project-specific evidence contract |
| 233 | {ref}`PR-T38-02` | Synchronous USART clock and sampling laboratory | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-01` | See project-specific evidence contract |
| 234 | {ref}`PR-T38-03` | I2C pull-up and wired-AND investigation | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-02` | See project-specific evidence contract |
| 235 | {ref}`PR-T38-04` | I2C transaction and conditional bus recovery | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-03` | See project-specific evidence contract |
| 236 | {ref}`PR-T38-05` | SPI four-mode transaction and ownership laboratory | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-04` | See project-specific evidence contract |
| 237 | {ref}`PR-T38-06` | RS-232 voltage and DTE/DCE boundary | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-05` | See project-specific evidence contract |
| 238 | {ref}`PR-T38-07` | RS-422 one-driver two-receiver experiment | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-06` | See project-specific evidence contract |
| 239 | {ref}`PR-T38-08` | RS-485 direction-safe reusable byte channel | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-07` | communications |
| 240 | {ref}`PR-T38-09` | Classic CAN codec and bounded bxCAN control | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-08` | See project-specific evidence contract |
| 241 | {ref}`PR-T38-10` | ISO CAN FD metadata and external-controller boundary | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-09` | communications |
| 242 | {ref}`PR-T38-11` | CAN FD controller bring-up and ownership boundary | {ref}`PR-T38-10` | communications |
| 243 | {ref}`PR-T38-12` | Physical CAN FD dual-rate exchange and recovery | {ref}`PR-T38-11` | communications |
| 244 | {ref}`PR-T39-01` | OSI service and encapsulation ledger | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-12` | See project-specific evidence contract |
| 245 | {ref}`PR-T39-02` | Ethernet media and checked frame boundary | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-12`, {ref}`PR-T39-01` | See project-specific evidence contract |
| 246 | {ref}`PR-T39-03` | MAC addressing and bounded learning bridge | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-12`, {ref}`PR-T39-02` | See project-specific evidence contract |
| 247 | {ref}`PR-T39-04` | RMII clock and bounded PHY management | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-12`, {ref}`PR-T39-03` | See project-specific evidence contract |
| 248 | {ref}`PR-T39-05` | MAC/DMA reachability and initialization dependencies | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-12`, {ref}`PR-T39-04` | communications |
| 249 | {ref}`PR-T39-06` | Checked Ethernet receive frame channel | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-12`, {ref}`PR-T39-05` | communications |
| 250 | {ref}`PR-T39-07` | Ethernet transmit publication and bounded recovery | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-12`, {ref}`PR-T39-06` | communications |
| 251 | {ref}`PR-T39-08` | Cross-layer frame handoff and failure evidence | {ref}`PR-T04-04`, {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T31-05`, {ref}`PR-T38-12`, {ref}`PR-T39-07` | communications |
| 252 | {ref}`PR-T40-01` | Seven boundary contracts and encapsulation ledger | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06` | See project-specific evidence contract |
| 253 | {ref}`PR-T40-02` | Two-segment RS-485 grant scheduler and measured adapter | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T40-01` | communications |
| 254 | {ref}`PR-T40-03` | Incremental escaped link framer with independent CRC | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T40-02` | communications |
| 255 | {ref}`PR-T40-04` | Static two-link router with hop and lifetime oracles | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T40-03` | communications |
| 256 | {ref}`PR-T40-05` | Stop-and-wait message transport with atomic reassembly | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T40-04` | communications |
| 257 | {ref}`PR-T40-06` | Negotiated session, dialogue token and retained-state resume | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T40-05` | communications |
| 258 | {ref}`PR-T40-07` | Canonical TLV presentation with strict UTF-8 | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T40-06` | communications |
| 259 | {ref}`PR-T40-08` | Routed SET/GET exchange and physical family closure | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T40-07` | communications |
| 260 | {ref}`PR-T41-01` | Controlled Ethernet endpoint and ownership ledger | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05` | See project-specific evidence contract |
| 261 | {ref}`PR-T41-02` | Actual PHY readiness, MDIO deadlines and link incarnation | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T41-01` | See project-specific evidence contract |
| 262 | {ref}`PR-T41-03` | Original normal-ring MAC/DMA driver with reuse recovery | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T41-02` | communications |
| 263 | {ref}`PR-T41-04` | Untagged Ethernet and one-peer ARP state | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T41-03` | communications |
| 264 | {ref}`PR-T41-05` | Fixed IPv4, odd-span checksums and exact Echo replies | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T41-04` | communications |
| 265 | {ref}`PR-T41-06` | Passive TCP with wrap, overlap, reset and close traces | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T41-05` | communications |
| 266 | {ref}`PR-T41-07` | RTO estimation, one-flight pressure and zero-window probes | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T41-06` | communications |
| 267 | {ref}`PR-T41-08` | Shared HTTP core over custom TCP and measured cable closure | {ref}`PR-T18-04`, {ref}`PR-T27-05`, {ref}`PR-T28-06`, {ref}`PR-T29-05`, {ref}`PR-T41-07` | communications |
| 268 | {ref}`PR-T42-01` | Owned socket lifecycle | {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T24-06`, {ref}`PR-T32-07` | communications |
| 269 | {ref}`PR-T42-02` | Incremental header and authority parser | {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T24-06`, {ref}`PR-T32-07`, {ref}`PR-T42-01` | communications |
| 270 | {ref}`PR-T42-03` | Unambiguous body framing | {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T24-06`, {ref}`PR-T32-07`, {ref}`PR-T42-02` | communications |
| 271 | {ref}`PR-T42-04` | Chunk decoder with separate trailers | {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T24-06`, {ref}`PR-T32-07`, {ref}`PR-T42-03` | communications |
| 272 | {ref}`PR-T42-05` | Router and HTTP response serializer | {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T24-06`, {ref}`PR-T32-07`, {ref}`PR-T42-04` | communications |
| 273 | {ref}`PR-T42-06` | Persistent output and close state | {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T24-06`, {ref}`PR-T32-07`, {ref}`PR-T42-05` | communications |
| 274 | {ref}`PR-T42-07` | Deadline and failure campaigns | {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T24-06`, {ref}`PR-T32-07`, {ref}`PR-T42-06` | communications |
| 275 | {ref}`PR-T42-08` | Independent client and parsing-security defense | {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T24-06`, {ref}`PR-T32-07`, {ref}`PR-T42-07` | communications |
| 276 | {ref}`PR-T43-03` | Instructions, graph cycles and a constrained decision | {ref}`PR-T13-04`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T34-06`, {ref}`PR-T43-02` | foundations |
| 277 | {ref}`PR-T43-04` | Simultaneous storage and a conditional stack certificate | {ref}`PR-T13-04`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T34-06`, {ref}`PR-T43-03` | foundations |
| 278 | {ref}`PR-T43-05` | Block traffic rather than a cache-time guess | {ref}`PR-T13-04`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T34-06`, {ref}`PR-T43-04` | algorithms |
| 279 | {ref}`PR-T43-06` | Return is not completion | {ref}`PR-T13-04`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T34-06`, {ref}`PR-T43-05` | foundations |
| 280 | {ref}`PR-T43-07` | Accepted bytes, buffered time and requested sync | {ref}`PR-T13-04`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T34-06`, {ref}`PR-T43-06` | foundations |
| 281 | {ref}`PR-T43-08` | Paired observations with an honest tail | {ref}`PR-T13-04`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T34-06`, {ref}`PR-T43-07` | foundations |
| 282 | {ref}`PR-T43-09` | PMU estimates with unavailable outcomes | {ref}`PR-T13-04`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T34-06`, {ref}`PR-T43-08` | See project-specific evidence contract |
| 283 | {ref}`PR-T43-10` | Read-only cgroup scope and conditional control | {ref}`PR-T13-04`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T34-06`, {ref}`PR-T43-09` | foundations |
| 284 | {ref}`PR-T43-11` | Compiler observations are a changed program | {ref}`PR-T13-04`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T34-06`, {ref}`PR-T43-10` | foundations |
| 285 | {ref}`PR-T43-12` | A conditional Cortex-M cycle certificate | {ref}`PR-T13-04`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T23-06`, {ref}`PR-T34-06`, {ref}`PR-T43-11` | foundations |
| 286 | {ref}`PR-T44-04` | Optimized image and saved-state investigation | {ref}`PR-T03-05`, {ref}`PR-T07-06`, {ref}`PR-T44-03` | See project-specific evidence contract |
| 287 | {ref}`PR-T44-05` | Memory evidence reconciliation | {ref}`PR-T03-05`, {ref}`PR-T07-06`, {ref}`PR-T44-04` | foundations |
| 288 | {ref}`PR-T44-06` | Partial I/O and descriptor autopsy | {ref}`PR-T03-05`, {ref}`PR-T07-06`, {ref}`PR-T44-05` | foundations |
| 289 | {ref}`PR-T44-07` | Race, deadlock and lost-wakeup dossier | {ref}`PR-T03-05`, {ref}`PR-T07-06`, {ref}`PR-T44-06` | See project-specific evidence contract |
| 290 | {ref}`PR-T44-08` | Reset, HardFault and DMA investigation | {ref}`PR-T03-05`, {ref}`PR-T07-06`, {ref}`PR-T44-07` | See project-specific evidence contract |
| 291 | {ref}`PR-T44-09` | Tool disagreement and differential diagnosis | {ref}`PR-T03-05`, {ref}`PR-T07-06`, {ref}`PR-T44-08` | foundations |
| 292 | {ref}`PR-T44-10` | Delayed blind causal repair | {ref}`PR-T03-05`, {ref}`PR-T07-06`, {ref}`PR-T44-09` | foundations |
| 293 | {ref}`PR-T45-04` | Stack capacity, high water and exhaustion experiment | {ref}`PR-T03-05`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T45-03` | foundations |
| 294 | {ref}`PR-T45-05` | Fixed RAM, startup and collision diagnosis | {ref}`PR-T03-05`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T45-04` | foundations |
| 295 | {ref}`PR-T45-06` | Leak and acquisition-prefix repair | {ref}`PR-T03-05`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T45-05` | foundations |
| 296 | {ref}`PR-T45-07` | Resize, stale alias and release repair | {ref}`PR-T03-05`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T45-06` | foundations |
| 297 | {ref}`PR-T45-08` | Initialization and publication repair | {ref}`PR-T03-05`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T45-07` | See project-specific evidence contract |
| 298 | {ref}`PR-T45-09` | Asynchronous lifetime and reentrant scratch repair | {ref}`PR-T03-05`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T45-08` | foundations |
| 299 | {ref}`PR-T45-10` | Unseen memory causal repair dossier | {ref}`PR-T03-05`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T45-09` | foundations |
| 300 | {ref}`PR-T46-01` | Values and a portable bytecode image | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T32-07` | runtime |
| 301 | {ref}`PR-T46-02` | Structural and stack-height verification | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T46-01` | runtime |
| 302 | {ref}`PR-T46-03` | Checked dispatch and operand stack | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T46-02` | runtime |
| 303 | {ref}`PR-T46-04` | Explicit call frames and recursion | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T46-03` | runtime |
| 304 | {ref}`PR-T46-05` | A bounded hosted expression compiler | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T46-04` | runtime |
| 305 | {ref}`PR-T46-06` | Conditionals, mutation and bounded loops | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T46-05` | runtime |
| 306 | {ref}`PR-T46-07` | Arena phase and object handles | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T46-06` | runtime |
| 307 | {ref}`PR-T46-08` | Nonmoving mark-sweep and temporary roots | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T46-07` | runtime |
| 308 | {ref}`PR-T46-09` | Guest, collection and host budgets | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T46-08` | runtime |
| 309 | {ref}`PR-T46-10` | Embedding and bounded store services | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T46-09` | runtime |
| 310 | {ref}`PR-T46-11` | One VM core across Linux, Thumb and a kernel | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T46-10` | runtime |
| 311 | {ref}`PR-T46-12` | Runtime defense, mutation and delayed transfer | {ref}`PR-T03-05`, {ref}`PR-T04-04`, {ref}`PR-T07-06`, {ref}`PR-T14-08`, {ref}`PR-T15-09`, {ref}`PR-T18-04`, {ref}`PR-T32-07`, {ref}`PR-T46-11` | runtime |
| 312 | {ref}`PR-T47-01` | A byte-exact page codec | {ref}`PR-T03-05`, {ref}`PR-T15-09`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T36-08`, {ref}`PR-T43-12`, {ref}`PR-T45-10` | storage |
| 313 | {ref}`PR-T47-02` | A bounded clean page cache | {ref}`PR-T03-05`, {ref}`PR-T15-09`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T36-08`, {ref}`PR-T43-12`, {ref}`PR-T45-10`, {ref}`PR-T47-01` | storage |
| 314 | {ref}`PR-T47-03` | B+tree lookup and ordered scan | {ref}`PR-T03-05`, {ref}`PR-T15-09`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T36-08`, {ref}`PR-T43-12`, {ref}`PR-T45-10`, {ref}`PR-T47-02` | storage |
| 315 | {ref}`PR-T47-04` | Insertion, splits and root growth | {ref}`PR-T03-05`, {ref}`PR-T15-09`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T36-08`, {ref}`PR-T43-12`, {ref}`PR-T45-10`, {ref}`PR-T47-03` | storage |
| 316 | {ref}`PR-T47-05` | Deletion, redistribution and page reuse | {ref}`PR-T03-05`, {ref}`PR-T15-09`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T36-08`, {ref}`PR-T43-12`, {ref}`PR-T45-10`, {ref}`PR-T47-04` | storage |
| 317 | {ref}`PR-T47-06` | Bounded no-steal transactions | {ref}`PR-T03-05`, {ref}`PR-T15-09`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T36-08`, {ref}`PR-T43-12`, {ref}`PR-T45-10`, {ref}`PR-T47-05` | storage |
| 318 | {ref}`PR-T47-07` | Full-page redo and commit outcomes | {ref}`PR-T03-05`, {ref}`PR-T15-09`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T36-08`, {ref}`PR-T43-12`, {ref}`PR-T45-10`, {ref}`PR-T47-06` | storage |
| 319 | {ref}`PR-T47-08` | Checkpoint generations and reclamation | {ref}`PR-T03-05`, {ref}`PR-T15-09`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T36-08`, {ref}`PR-T43-12`, {ref}`PR-T45-10`, {ref}`PR-T47-07` | storage |
| 320 | {ref}`PR-T47-09` | Recovery and repeated interruption | {ref}`PR-T03-05`, {ref}`PR-T15-09`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T36-08`, {ref}`PR-T43-12`, {ref}`PR-T45-10`, {ref}`PR-T47-08` | storage |
| 321 | {ref}`PR-T47-10` | Crash schedules and adversarial oracles | {ref}`PR-T03-05`, {ref}`PR-T15-09`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T36-08`, {ref}`PR-T43-12`, {ref}`PR-T45-10`, {ref}`PR-T47-09` | storage |
| 322 | {ref}`PR-T47-11` | Linux file I/O and amplification | {ref}`PR-T03-05`, {ref}`PR-T15-09`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T36-08`, {ref}`PR-T43-12`, {ref}`PR-T45-10`, {ref}`PR-T47-10` | storage |
| 323 | {ref}`PR-T47-12` | VM and kernel storage boundaries | {ref}`PR-T03-05`, {ref}`PR-T15-09`, {ref}`PR-T21-04`, {ref}`PR-T32-07`, {ref}`PR-T36-08`, {ref}`PR-T43-12`, {ref}`PR-T45-10`, {ref}`PR-T47-11` | storage |
| 324 | {ref}`PR-T48-01` | Boot contracts and the long-mode transition | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T16-09`, {ref}`PR-T23-06`, {ref}`PR-T27-05`, {ref}`PR-T32-07`, {ref}`PR-T37-08`, {ref}`PR-T46-12`, {ref}`PR-T47-12` | kernel |
| 325 | {ref}`PR-T48-02` | Bounded boot information and memory reservations | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T16-09`, {ref}`PR-T23-06`, {ref}`PR-T27-05`, {ref}`PR-T32-07`, {ref}`PR-T37-08`, {ref}`PR-T46-12`, {ref}`PR-T47-12`, {ref}`PR-T48-01` | kernel |
| 326 | {ref}`PR-T48-03` | Serial diagnosis and normalized exception frames | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T16-09`, {ref}`PR-T23-06`, {ref}`PR-T27-05`, {ref}`PR-T32-07`, {ref}`PR-T37-08`, {ref}`PR-T46-12`, {ref}`PR-T47-12`, {ref}`PR-T48-02` | kernel |
| 327 | {ref}`PR-T48-04` | Physical frames and explicit page-table ownership | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T16-09`, {ref}`PR-T23-06`, {ref}`PR-T27-05`, {ref}`PR-T32-07`, {ref}`PR-T37-08`, {ref}`PR-T46-12`, {ref}`PR-T47-12`, {ref}`PR-T48-03` | kernel |
| 328 | {ref}`PR-T48-05` | Ring-3 isolation and guarded user storage | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T16-09`, {ref}`PR-T23-06`, {ref}`PR-T27-05`, {ref}`PR-T32-07`, {ref}`PR-T37-08`, {ref}`PR-T46-12`, {ref}`PR-T47-12`, {ref}`PR-T48-04` | kernel |
| 329 | {ref}`PR-T48-06` | Timer preemption with a nonpreemptible kernel | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T16-09`, {ref}`PR-T23-06`, {ref}`PR-T27-05`, {ref}`PR-T32-07`, {ref}`PR-T37-08`, {ref}`PR-T46-12`, {ref}`PR-T47-12`, {ref}`PR-T48-05` | kernel |
| 330 | {ref}`PR-T48-07` | Static ELF64 and a read-only boot archive | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T16-09`, {ref}`PR-T23-06`, {ref}`PR-T27-05`, {ref}`PR-T32-07`, {ref}`PR-T37-08`, {ref}`PR-T46-12`, {ref}`PR-T47-12`, {ref}`PR-T48-06` | kernel |
| 331 | {ref}`PR-T48-08` | Bounded syscalls and revocable process handles | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T16-09`, {ref}`PR-T23-06`, {ref}`PR-T27-05`, {ref}`PR-T32-07`, {ref}`PR-T37-08`, {ref}`PR-T46-12`, {ref}`PR-T47-12`, {ref}`PR-T48-07` | kernel |
| 332 | {ref}`PR-T48-09` | Modern PCI virtio discovery and split queues | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T16-09`, {ref}`PR-T23-06`, {ref}`PR-T27-05`, {ref}`PR-T32-07`, {ref}`PR-T37-08`, {ref}`PR-T46-12`, {ref}`PR-T47-12`, {ref}`PR-T48-08` | kernel |
| 333 | {ref}`PR-T48-10` | Block completion, flush and persistence boundaries | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T16-09`, {ref}`PR-T23-06`, {ref}`PR-T27-05`, {ref}`PR-T32-07`, {ref}`PR-T37-08`, {ref}`PR-T46-12`, {ref}`PR-T47-12`, {ref}`PR-T48-09` | kernel |
| 334 | {ref}`PR-T48-11` | Init, bounded console shell and teardown | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T16-09`, {ref}`PR-T23-06`, {ref}`PR-T27-05`, {ref}`PR-T32-07`, {ref}`PR-T37-08`, {ref}`PR-T46-12`, {ref}`PR-T47-12`, {ref}`PR-T48-10` | kernel |
| 335 | {ref}`PR-T48-12` | One portable VM core inside a user process | {ref}`PR-T46-12`, {ref}`PR-T48-11` | kernel |
| 336 | {ref}`PR-T48-13` | VM, durable store and kernel reboot integration | {ref}`PR-T47-12`, {ref}`PR-T48-12` | kernel |
| 337 | {ref}`PR-T48-14` | Kernel diagnosis, negative controls and delayed transfer | {ref}`PR-T08-04`, {ref}`PR-T09-05`, {ref}`PR-T16-09`, {ref}`PR-T23-06`, {ref}`PR-T27-05`, {ref}`PR-T32-07`, {ref}`PR-T37-08`, {ref}`PR-T46-12`, {ref}`PR-T47-12`, {ref}`PR-T48-13` | kernel |
| 338 | {ref}`PR-T49-01` | Privilege, traps and a syscall observatory | {ref}`PR-T23-01`, {ref}`PR-T25-01` | os_observatory |
| 339 | {ref}`PR-T49-02` | Processes, threads and lifetime ownership | {ref}`PR-T49-01` | os_observatory |
| 340 | {ref}`PR-T49-03` | FCFS, SJF, SRTF and round-robin scheduling | {ref}`PR-T49-02` | os_observatory |
| 341 | {ref}`PR-T49-04` | Feedback and proportional-share scheduling | {ref}`PR-T49-03` | os_observatory |
| 342 | {ref}`PR-T49-05` | Working sets and page-fault observation | {ref}`PR-T49-04` | os_observatory |
| 343 | {ref}`PR-T49-06` | Page replacement from a reference string | {ref}`PR-T49-05` | os_observatory |
| 344 | {ref}`PR-T49-07` | Descriptors and open-file-description lifetimes | {ref}`PR-T49-06` | os_observatory |
| 345 | {ref}`PR-T49-08` | Namespaces, readiness and device completion | {ref}`PR-T49-07` | os_observatory |
| 346 | {ref}`PR-T49-09` | CPU and memory budgets with cgroups | {ref}`PR-T49-08` | os_observatory |
| 347 | {ref}`PR-T49-10` | I/O accounting and interference | {ref}`PR-T49-09` | os_observatory |
| 348 | {ref}`PR-T50-01` | Freestanding image and runtime dependency audit | {ref}`PR-T29-01`, {ref}`PR-T37-07` | execution |
| 349 | {ref}`PR-T50-02` | Static task pools, stacks and handles | {ref}`PR-T50-01` | execution |
| 350 | {ref}`PR-T50-03` | Cooperative assembly context switching | {ref}`PR-T50-02` | execution |
| 351 | {ref}`PR-T50-04` | Ready, blocked, sleeping and exited states | {ref}`PR-T50-03` | execution |
| 352 | {ref}`PR-T50-05` | Timers and an absolute-deadline queue | {ref}`PR-T50-04` | execution |
| 353 | {ref}`PR-T50-06` | PendSV, SVC and timer-driven preemption | {ref}`PR-T29-02`, {ref}`PR-T50-05` | execution |
| 354 | {ref}`PR-T50-07` | Nested interrupt masks and ISR handoff | {ref}`PR-T50-06` | execution |
| 355 | {ref}`PR-T50-08` | Floating-point task context and lazy stacking | {ref}`PR-T50-07` | execution |
| 356 | {ref}`PR-T50-09` | Exit, join and deferred reclamation | {ref}`PR-T50-08` | execution |
| 357 | {ref}`PR-T50-10` | Cancellation and task-fault containment | {ref}`PR-T50-09` | execution |
| 358 | {ref}`PR-T50-11` | Tickless idle and wakeup accounting | {ref}`PR-T50-10` | execution |
| 359 | {ref}`PR-T50-12` | Integrated original MCU runtime | {ref}`PR-T50-11` | execution |
| 360 | {ref}`PR-T51-01` | Atomic wait enrollment and lost-wakeup prevention | {ref}`PR-T25-03`, {ref}`PR-T50-04` | execution |
| 361 | {ref}`PR-T51-02` | Counting and binary semaphores | {ref}`PR-T51-01` | execution |
| 362 | {ref}`PR-T51-03` | Owned mutexes and lock ordering | {ref}`PR-T51-02` | execution |
| 363 | {ref}`PR-T51-04` | Transitive priority inheritance | {ref}`PR-T50-06`, {ref}`PR-T51-03` | execution |
| 364 | {ref}`PR-T51-05` | Condition variables and Mesa-style predicates | {ref}`PR-T51-04` | execution |
| 365 | {ref}`PR-T51-06` | Bounded message queues with copied payloads | {ref}`PR-T51-05` | execution |
| 366 | {ref}`PR-T51-07` | Mailboxes and synchronous rendezvous | {ref}`PR-T51-06` | execution |
| 367 | {ref}`PR-T51-08` | Event flags with any/all semantics | {ref}`PR-T51-07` | execution |
| 368 | {ref}`PR-T51-09` | Counted task notifications | {ref}`PR-T51-08` | execution |
| 369 | {ref}`PR-T51-10` | Reusable barriers and generations | {ref}`PR-T51-09` | execution |
| 370 | {ref}`PR-T51-11` | Once initialization and failure policy | {ref}`PR-T51-10` | execution |
| 371 | {ref}`PR-T51-12` | Reader-writer locks and admission fairness | {ref}`PR-T51-11` | execution |
| 372 | {ref}`PR-T51-13` | Timeout, cancellation and wakeup arbitration | {ref}`PR-T50-05`, {ref}`PR-T51-12` | execution |
| 373 | {ref}`PR-T51-14` | Object teardown and owner-death recovery | {ref}`PR-T51-13` | execution |
| 374 | {ref}`PR-T52-01` | Rate-monotonic and deadline-monotonic scheduling | {ref}`PR-T30-04` | execution |
| 375 | {ref}`PR-T52-02` | Earliest-deadline-first dispatch | {ref}`PR-T52-01` | execution |
| 376 | {ref}`PR-T52-03` | Response-time analysis with blocking and jitter | {ref}`PR-T51-04`, {ref}`PR-T52-02` | execution |
| 377 | {ref}`PR-T52-04` | Immediate priority ceilings | {ref}`PR-T51-03`, {ref}`PR-T52-03` | execution |
| 378 | {ref}`PR-T52-05` | Aperiodic service within periodic workloads | {ref}`PR-T52-04` | execution |
| 379 | {ref}`PR-T52-06` | Constant-bandwidth-server reservation | {ref}`PR-T52-05` | execution |
| 380 | {ref}`PR-T52-07` | Admission, overload and deadline-miss policy | {ref}`PR-T52-06` | execution |
| 381 | {ref}`PR-T52-08` | Tick rollover, phase and tickless timing | {ref}`PR-T52-07` | execution |
| 382 | {ref}`PR-T52-09` | Partitioned multicore real-time scheduling | {ref}`PR-T52-08` | execution |
| 383 | {ref}`PR-T52-10` | Global EDF and multiprocessor counterexamples | {ref}`PR-T52-09` | execution |
| 384 | {ref}`PR-T52-11` | Compare the original runtime with FreeRTOS | {ref}`PR-T50-12`, {ref}`PR-T51-06`, {ref}`PR-T52-10` | os_observatory |
| 385 | {ref}`PR-T52-12` | Linux fair and real-time scheduling comparison | {ref}`PR-T49-09`, {ref}`PR-T52-11` | os_observatory |
| 386 | {ref}`PR-T53-01` | ACPI MADT and processor discovery | {ref}`PR-T25-03`, {ref}`PR-T48-03` | kernel_smp |
| 387 | {ref}`PR-T53-02` | Secondary-CPU trampoline and startup | {ref}`PR-T53-01` | kernel_smp |
| 388 | {ref}`PR-T53-03` | Per-CPU execution state and stacks | {ref}`PR-T53-02` | kernel_smp |
| 389 | {ref}`PR-T53-04` | Local APIC timers and interprocessor interrupts | {ref}`PR-T53-03` | kernel_smp |
| 390 | {ref}`PR-T53-05` | SMP spinlocks and interrupt context | {ref}`PR-T53-04` | kernel_smp |
| 391 | {ref}`PR-T53-06` | Per-CPU ready queues | {ref}`PR-T50-04`, {ref}`PR-T51-03`, {ref}`PR-T53-05` | kernel_smp |
| 392 | {ref}`PR-T53-07` | Remote wakeup and reschedule delivery | {ref}`PR-T51-01`, {ref}`PR-T53-06` | kernel_smp |
| 393 | {ref}`PR-T54-01` | Dual images and static peripheral ownership | {ref}`PR-T37-06`, {ref}`PR-T50-01` | execution |
| 394 | {ref}`PR-T54-02` | Dual boot and epoch handshake | {ref}`PR-T54-01` | execution |
| 395 | {ref}`PR-T54-03` | Shared-SRAM SPSC channel | {ref}`PR-T25-03`, {ref}`PR-T54-02` | execution |
| 396 | {ref}`PR-T54-04` | HSEM and coalesced doorbells | {ref}`PR-T54-03` | execution |
| 397 | {ref}`PR-T54-05` | Cache visibility and cache-line ownership | {ref}`PR-T54-04` | execution |
| 398 | {ref}`PR-T54-06` | DMA buffers and CPU ownership transitions | {ref}`PR-T29-04`, {ref}`PR-T54-05` | execution |
| 399 | {ref}`PR-T54-07` | Cross-core clocks and latency measurement | {ref}`PR-T54-06` | execution |
| 400 | {ref}`PR-T54-08` | Peer failure and protocol recovery | {ref}`PR-T51-13`, {ref}`PR-T54-07` | execution |
| 401 | {ref}`PR-T54-09` | Interrupt-to-task pipelines on both cores | {ref}`PR-T50-12`, {ref}`PR-T54-08` | execution |
| 402 | {ref}`PR-T54-10` | AMP end-to-end observatory | {ref}`PR-T54-09` | execution |
| 403 | {ref}`PR-T55-01` | SPSC publication and acquire-release ordering | {ref}`PR-T25-03`, {ref}`PR-T53-05` | execution |
| 404 | {ref}`PR-T55-02` | Compare-and-swap and progress classifications | {ref}`PR-T55-01` | execution |
| 405 | {ref}`PR-T55-03` | ABA and identity reuse | {ref}`PR-T55-02` | execution |
| 406 | {ref}`PR-T55-04` | A bounded pool for concurrent nodes | {ref}`PR-T55-03` | execution |
| 407 | {ref}`PR-T55-05` | Hazard-pointer publication and reclamation | {ref}`PR-T55-04` | execution |
| 408 | {ref}`PR-T55-06` | Michael-Scott queue with bounded reclamation | {ref}`PR-T51-06`, {ref}`PR-T55-05` | execution |
| 409 | {ref}`PR-T55-07` | Epoch reclamation and quiescent states | {ref}`PR-T55-06` | execution |
| 410 | {ref}`PR-T55-08` | Read-copy-update as an ownership protocol | {ref}`PR-T55-07` | execution |
| 411 | {ref}`PR-T55-09` | Concurrent histories and linearizability checking | {ref}`PR-T55-08` | execution |
| 412 | {ref}`PR-T55-10` | End-to-end progress and contention audit | {ref}`PR-T55-09` | execution |
| 413 | {ref}`PR-T56-01` | Process and thread objects as distinct lifetimes | {ref}`PR-T48-08`, {ref}`PR-T53-06` | kernel_smp |
| 414 | {ref}`PR-T53-08` | Affinity, migration and load balance | {ref}`PR-T53-07`, {ref}`PR-T56-01` | kernel_smp |
| 415 | {ref}`PR-T53-09` | TLB shootdown and page reclamation | {ref}`PR-T48-04`, {ref}`PR-T53-08` | kernel_smp |
| 416 | {ref}`PR-T53-10` | Parallel reduction with deterministic partitioning | {ref}`PR-T53-09` | kernel_smp |
| 417 | {ref}`PR-T53-11` | Bounded parallel work pool | {ref}`PR-T51-06`, {ref}`PR-T53-10` | kernel_smp |
| 418 | {ref}`PR-T53-12` | Dependency DAG execution and scaling limits | {ref}`PR-T53-11` | kernel_smp |
| 419 | {ref}`PR-T56-02` | Spawn, exit, wait and process reaping | {ref}`PR-T56-01` | kernel_smp |
| 420 | {ref}`PR-T56-03` | Demand-zero faults and frame ownership | {ref}`PR-T48-04`, {ref}`PR-T56-02` | kernel_smp |
| 421 | {ref}`PR-T56-04` | Restricted copy-on-write cloning | {ref}`PR-T53-09`, {ref}`PR-T56-03` | kernel_smp |
| 422 | {ref}`PR-T56-05` | Bounded process channels | {ref}`PR-T51-06`, {ref}`PR-T56-04` | kernel_smp |
| 423 | {ref}`PR-T56-06` | Shared-memory objects and explicit synchronization | {ref}`PR-T56-05` | kernel_smp |
| 424 | {ref}`PR-T56-07` | Asynchronous block requests and completion ownership | {ref}`PR-T48-10`, {ref}`PR-T56-06` | kernel_smp |
| 425 | {ref}`PR-T56-08` | Read-only boot archive as a file service | {ref}`PR-T48-07`, {ref}`PR-T56-07` | kernel_smp |
| 426 | {ref}`PR-T56-09` | VM execution as a bounded service | {ref}`PR-T48-12`, {ref}`PR-T56-08` | kernel_smp |
| 427 | {ref}`PR-T56-10` | Single-owner persistent store service | {ref}`PR-T48-13`, {ref}`PR-T56-09` | kernel_smp |
| 428 | {ref}`PR-T56-11` | Service restart and reboot recovery | {ref}`PR-T56-10` | kernel_smp |
| 429 | {ref}`PR-T56-12` | Physical dual-core application and OS comparison | {ref}`PR-T52-11`, {ref}`PR-T52-12`, {ref}`PR-T54-10`, {ref}`PR-T56-11` | os_observatory |
