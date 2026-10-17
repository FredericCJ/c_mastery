# Modern programming techniques from first principles (T65–T76)

Authorized 2026-10-16 as an extension of the fifth edition (see
`planning/decisions.md` and `planning/overhaul_v5.md`). All twelve domains now have
canonical modules, 114 projects, laboratory contracts, separate assessor sources,
objective rubrics and scoped author campaigns. The integrated manifest and
`research/v5/modern/coverage/` record their actual identities and census mapping.
The estimates below preserve the original plan; source-bound author validation,
independent review, physical validation and learner assessment remain distinct.

## Purpose

The curriculum implements higher-level mechanisms from first principles. These
domains do the same for modern programming techniques: each technique is built
in C17 from plain data, functions and, where needed, a small runtime or
generator, then exercised in a laboratory with contract tests, negative controls
and mutation rejection. Each project states what C17 can enforce and what a
language with the feature enforces statically, so a runtime check is never
presented as a proof.

Inputs:

- The modern-programming census: 100 techniques (M01–M26 memory, C01–C24
  control, P01–P26 polymorphism, D01–D24 data modeling) with a guarantee-class
  kernel, a cost-review sheet, misconceptions and a language crosswalk. It is a
  discovery aid; its 188 web sources become registered resources only after
  inspection.
- The SWE design-element catalog (about 1,083 elements), used to find approaches
  beyond language features. It holds no documents; hits are leads.
- The local library, the document source for passage inspection.

Census classification for C: 45 techniques map directly to plain C data and
functions, 22 need macro or `_Generic` instantiation, 12 need a small runtime, 10
are compile-time guarantees that C can only approximate by runtime enforcement or
compiler diagnostics, and 3 need a generator written in C. Eight have no
meaningful C analogue (M25, C08, C12, P09, P13, P24, P25, D11); they are taught as
non-equivalences in module notes and recall variants, not projects.

Imported from accepted domains instead of re-taught: M05 (T14.02), M13
(T16.03/04), M22 (T16.06, T19.09), M23 (T55.05/07), C24 (T51.11), D14 (T20.03).
T19 already covers operation tables, closures, generic containers, tagged unions
and iterators at contract depth; the new projects go further and state their
boundary against T19.

## Domain map

Domains are numbered in import order. T65–T70 form the phase "Modern
abstractions from first principles"; T71–T76 form "Modern execution, runtime and
data models". Both phases follow every phase T57–T64 introduce.

| ID | Domain | Family | Prefix | Strand | Projects |
|---|---|---|---|---|---|
| T65 | Error models and executable contracts (pilot) | `errors` | ER | toolbox | 9 |
| T66 | Ownership, lifetime and cleanup protocols | `ownership` | OW | toolbox | 10 |
| T67 | Algebraic data, views and boundary representations | `adt` | AD | toolbox | 10 |
| T68 | Generic programming, reflection and code generation | `metaprog` | MP | toolbox | 10 |
| T69 | Dispatch, closures and type erasure | `dispatch` | DS | toolbox | 11 |
| T70 | Data-oriented layout and persistent structures | `dod` | DO | toolbox | 8 |
| T71 | Typestate, protocols and state machines | `protocols` | TS | firmware | 8 |
| T72 | Iterators, coroutines and continuations | `coroutines` | CR | toolbox | 9 |
| T73 | Structured concurrency, channels and actors | `messaging` | SC | service | 10 |
| T74 | Managed-runtime object models | `object_runtime` | OR | appliance | 11 |
| T75 | Flow control and resilient services | `resilience` | RZ | service | 9 |
| T76 | Replicated and event-sourced state | `replicated` | RP | service | 9 |

Planned size: 114 projects of 6–10 hours, about 910 project hours plus about 28
hours per domain of reading, assessment and consolidation, roughly 1,250 hours in
total. `tools/write_program.py` computes the binding figure.

The later evidence-based exercise expansion treats those figures as historical
planning estimates, not a learner-time ceiling or an exercise quota. Mechanism,
boundary, failure and transfer coverage determines the added work. The required
exercise layer records that work separately from accepted project envelopes.

## Subtopics

One project per subtopic. Brackets give census IDs and SWE element names.
Imports name existing artifacts in `curriculum/v5/artifacts.json`; exports are
new artifact contracts.

### T65 Error models and executable contracts

Imports `bytes.checked`, `parser.integer`, `io.stream`, `codec.records`,
`records.tool`. Exports `errors.result`.

1. Optional and Result by instantiation, including niche encodings [C01, C02];
   boundary T18.07.
2. Propagation macros and error conversion [C04]; boundary T21.02. Goto and
   out-parameter forms only: statement expressions are not C17.
3. Partial progress with product returns [C03].
4. Map, and_then and railway composition [C06; railway-oriented-programming].
5. Applicative validation with bounded error accumulation [C07].
6. Typed exception boundaries over setjmp/longjmp, cleanup interplay and
   conversion to Result at the boundary [C05].
7. Arithmetic and conversion policies: checked, wrapping, saturating, fallible
   and serial-number arithmetic [C11; saturation-, checked- and
   serial-number-arithmetic]; boundary T04.
8. Executable contracts with off, observe and enforce modes [C23]; boundary
   T18.09.
9. Runtime refinement predicates [D07]; static checking belongs to T61. C08 and
   C12 are non-equivalence notes.

### T66 Ownership, lifetime and cleanup protocols

Imports `memory.pool`, `containers.array`, `testing.faults`, `bytes.checked`,
`io.async`, `errors.result`. Exports `ownership.shared`, `cleanup.scope`.

1. Owning handles and partial-initialization frontiers [M01, M19]; boundary
   T21.03, T18.05.
2. Scoped cleanup stacks (defer and errdefer), undo logs and publish-by-swap
   [C09, C10]; boundary T21.04/05.
3. Move-only values, consumption poisoning and must-use checking [M02, C16]: a
   GNU attribute plus a debug checked bit, because `[[nodiscard]]` is not C17.
4. Runtime borrow tracking and borrow splitting [M03, M04].
5. Interior-mutability cells [M20].
6. Strong and weak shared ownership, non-atomic and with `<stdatomic.h>` [M10];
   boundary T19.09. Cycle collection belongs to T74.
7. Copy-on-write values [M09]; boundary T56.04.
8. Thread affinity, guarded data and pinning assertions [M21, M18].
9. Adversarial allocators: fail-at-N, counting and poisoning [M12], extending
   the `containers.array` callbacks and `testing.faults` rather than introducing
   a second allocator interface; boundary T14.03, T16.01, T32.07.
10. Trusted core behind a checked façade [M24]; boundary T63 (offense).

### T67 Algebraic data, views and boundary representations

Imports `bytes.cursor`, `bytes.checked`, `codec.records`, `parser.integer`,
`io.frame`, `memory.pool`. Exports `adt.packet_view`, `adt.brand`.

1. Products, sums and exhaustive matching with `-Wswitch-enum` and
   tag-asserting accessors [D01–D03]; boundary T19.05.
2. Newtypes and dimensional quantities [D04, D05].
3. Typed flag sets and reserved bits [D23].
4. Smart constructors and validated text views [D06, M06].
5. Phantom-branded handles over pools [D10].
6. Owners versus views, and strided multidimensional views [M07, M08]; boundary
   T14.02, T64.
7. Recursive data in arenas with typed indices [D18]; D11 is a note on checking
   once and evaluating unchecked.
8. Lenses and prisms, with laws checked over exhaustive small domains [D16].
9. Evolution-aware sums and tolerant readers [D22; tolerant-reader]. This domain
   owns tolerant reading.
10. Zero-copy boundary parsing: a C port of the census packet view [D20 with
    M05, C02, D02, D06].

### T68 Generic programming, reflection and code generation

Imports `testing.faults`, `containers.ring`, `containers.array`,
`codec.records`. Exports `meta.reflect`, `testing.laws`.

1. Monomorphization by template headers, with associated types [P02, P12].
2. Constraint probes and const parameters [P01, P14].
3. Policy-based composition [P06].
4. Variadic machinery without `__VA_OPT__` or `##__VA_ARGS__` [P15]; boundary
   T06.04.
5. `_Generic` customization points and cv-preserving receivers [P21, P07];
   boundary T06.07.
6. Single-source X-macro reflection [P17, P22]; boundary T06.06.
7. Derived operations with laws and a property checker with shrinking [P19;
   property-based-testing], built on `testing.faults`; boundary T32.04.
8. Type-checked formatting into bounded sinks [C15].
9. A hygienic token-level macro expander written in C [P18].
10. A declarative build graph with content hashing and build-step table
    generators [D21, P16]; boundary T06.09.

### T69 Dispatch, closures and type erasure

Imports `io.stream`, `containers.array`, `memory.pool`, `measurement.capture`.
Exports `dispatch.erased_fn`, `plugin.host`.

1. Traits with nominal and structural conformance [P04, P05].
2. Fat pointers versus embedded table pointers [P08]; P13 variance through the
   `T **` to `const T **` rule; boundary T19.02.
3. Dictionary passing versus specialization [P03].
4. Closures with Fn, FnMut and FnOnce capture modes [P11]; boundary T19.03.
5. Owning erased callables with inline storage [P10], consuming the
   small-buffer storage owned by T70.
6. Multiple dispatch tables and caches [P23].
7. Intrusive containers, `container_of` and external polymorphism [container-of,
   external-polymorphism].
8. Opaque and parameterized modules, dependency injection [P20, P26;
   dependency-injection]; boundary T18.06.
9. Capabilities and authority attenuation [D13].
10. Plugin hosts with a versioned ABI and `dlopen` [plugin,
    component-configurator]; boundary T22.06.
11. Hot reload with state hand-off [hot-code-reload]. P09, P24, P25 and M25 are
    non-equivalence notes.

### T70 Data-oriented layout and persistent structures

Imports `memory.pool`, `containers.array`, `algorithms.radix`,
`algorithms.freelist`, `measurement.capture`, `meta.reflect`. Exports `dod.ecs`,
`persist.hamt`, `text.rope`.

1. Struct-of-arrays and hot/cold splitting from field lists [M16].
2. Inline-capacity containers, small-buffer storage and small strings [M14, M15].
3. SWAR lanes with masks, without intrinsics [M17]; boundary T64, T34.
4. Stable-address segmented storage and sparse sets [M26; sparse-set, slot-map];
   boundary T14.12.
5. Entity-component systems with archetype queries [D17].
6. Path-copying persistent collections and zippers [D15; zipper].
7. Hash array mapped tries [D15; hash-array-mapped-trie].
8. Ropes, piece tables and gap buffers [rope, piece-table, gap-buffer].

### T71 Typestate, protocols and state machines

Imports `firmware_io.gpio`, `firmware_io.uart`, `firmware_io.timer`,
`firmware_io.dma`, `firmware_io.watchdog`, `execution.tasks`,
`sync.copied_queue`, `containers.ring`, `adt.brand`. Exports `fsm.hsm`,
`driver.typestate`.

1. Typestate with consuming transitions, including a device lifecycle that ends
   in RecoveryRequired [D08].
2. Staged builders and presence masks [D09].
3. Session types as runtime protocol automata [D12].
4. Hierarchical state machines with run-to-completion dispatch [D24]; boundary
   T20.07.
5. Active objects: a hierarchical machine with an event queue on the MCU model.
   T73 actors are hosted and supervised.
6. Protothreads as stackless resumable functions [C17, first part].
7. Behavior trees [behavior-tree].
8. A typestate driver over the firmware register model.

### T72 Iterators, coroutines and continuations

Imports `io.reactor` (through an adapter), `io.deadline`, `io.stream`,
`bytes.cursor`, `parser.integer`. Exports `coro.frame`, `fiber.context`,
`parse.combinators`.

1. Lazy pull pipelines with fusion, internal versus external iteration and typed
   break [C13, C14]; boundary T19.07.
2. Volcano operators and dataflow pipelines [volcano-iterator-model,
   pipes-and-filters].
3. Explicit coroutine frames and generators, with measured frame allocation
   [C17, second part].
4. Stackful fibers with an x86-64 context switch in assembly [green-threads];
   cites the SysV psABI without teaching it.
5. Continuation-passing style, trampolines and defunctionalization by hand
   [continuation-passing-style, trampoline, defunctionalization]; compilers
   belong to T60.
6. Parser combinators over span cursors [D19].
7. Packrat memoization; precedence parsing belongs to T60 and T46.05.
8. Async state machines over a readiness reactor [reactor]; completion rings
   belong to T62.
9. Effect handlers with resumable continuations [C22].

### T73 Structured concurrency, channels and actors

Imports `work.pool`, `sync.condition_queue`, `sync.spsc`, `io.async`,
`io.deadline`, `testing.faults`, `coro.frame`. Exports `conc.nursery`,
`conc.channel`, `actor.runtime`.

1. Nurseries with scope-owned cancellation [C18]. This domain owns
   cancellation inside a scope.
2. CSP channels with select [csp-channel]; boundary T51.07.
3. Senders, receivers and operation states [C20].
4. Actors with mailboxes [C21].
5. Supervision trees and let-it-crash recovery [supervisor, let-it-crash,
   error-kernel].
6. Work-stealing job systems with Chase–Lev deques [work-stealing]; boundary
   `work.pool`.
7. Software transactional memory in the TL2 style
   [software-transactional-memory].
8. Multiversion snapshots [multiversion-concurrency-control].
9. Sequence locks [sequence-lock]; boundary T55.07/08.
10. Reactive streams with demand signaling [observable-streams,
    reactive-signals]; overflow policy belongs to T75.

### T74 Managed-runtime object models

Imports `runtime.vm`, `memory.pool`, `testing.faults`, `measurement.capture`.
Exports `rt.object_model`, `rt.incremental_gc`.

1. Pointer tagging and NaN-boxed values [pointer-tagging]; boundary T46.01.
2. String interning and symbol tables [sharing].
3. Hidden classes and shape transitions [hidden-class].
4. Monomorphic and polymorphic inline caches [inline-caching].
5. Tri-color incremental marking with write barriers [gc-write-barrier].
6. A generational copying nursery [M11].
7. Remembered sets and card marking.
8. Compaction through handles and forwarding [compaction].
9. Weak references, ephemerons and finalization queues [weak-reference].
10. Cycle collection for reference counting by trial deletion.
11. Safepoints and precise root maps [safepoint]; boundary T46.08.

### T75 Flow control and resilient services

Imports `io.deadline`, `io.async`, `ipc.local`, `io.reactor`,
`communications.tcp_fault`, `containers.ring`, `measurement.capture`,
`conc.channel`. Exports `flow.deadline`, `flow.limiter`, `flow.breaker`.

1. Cancellation tokens and deadline propagation across service boundaries [C19;
   deadline-propagation].
2. Backpressure policies: block, reject, drop, overwrite and degrade [C19;
   credit-based-flow-control]; boundary T24.11.
3. Timer wheels, as a third alternative in the PR-T50-05 decision [timer-wheel].
4. Token buckets [token-bucket].
5. Retry with jittered exponential backoff [retry-with-exponential-backoff-and-jitter].
6. Circuit breakers and bulkheads [circuit-breaker].
7. Hedged requests [hedged-request].
8. Idempotent receivers, deduplication windows and fencing tokens
   [idempotent-receiver, fencing-token]. This domain owns request idempotence.
9. Composition into the resilient telemetry service.

### T76 Replicated and event-sourced state

Imports `storage.pages`, `codec.records`, `io.frame`, `ipc.local`,
`testing.faults`, `persist.hamt`. Exports `log.event_store`, `crdt.state`.

1. Event logs, projections and snapshots [event-sourcing, memory-image].
2. Command–query separation with exactly-once projection by log position
   [cqrs].
3. Lamport, vector and hybrid logical clocks [logical-clock].
4. State-based CRDT counters and registers
   [conflict-free-replicated-data-type-crdt].
5. Observed-remove sets and delta-state CRDTs.
6. A sequence CRDT compared with operational transformation
   [operational-transformation].
7. Merkle trees and anti-entropy [merkle-tree, content-addressable-storage].
8. Schema evolution: field tags, varint and zigzag, canonical encoding
   [schema-evolution-via-field-tags, varint, zigzag-encoding,
   canonical-serialization].
9. Deterministic replay and rollback [deterministic-lockstep, rollback-netcode],
   driven by the T20.03 functional core.

Recorded exclusions: probabilistic sketches (algorithms territory), fuzzing and
model checking (T61), free monads and tagless-final interpreters (reading notes in
T69.09 and T72.09 only), and structured logging and metric types (rejected as
elements by the SWE catalog).

## Project and laboratory anatomy

1. Contract. The guarantee class from the census kernel (static rule, library
   invariant, runtime enforcement, optimization, layout or ABI, evolving), what
   C17 enforces and what it cannot, and the five questions: who destroys the
   value, who may access it and for how long, where it is stored and whether it
   can move, what identifies it, and what effects it has.
2. Implementation from scratch. `labs/<family>/src` holds stubs that fail with
   NOT_IMPLEMENTED and `assessor/<family>` the reference. C17 with
   `-Wall -Wextra -Wpedantic -Werror`; C23 comparisons only as CTest
   `-std=c2x -fsyntax-only` probes; POSIX through `_POSIX_C_SOURCE=200809L`
   (`_DEFAULT_SOURCE` only where `MAP_ANONYMOUS` is needed); pthreads by default.
3. Tests. C contract tests dispatched by case; negative controls drawn from the
   census misconceptions (coroutines allocate, a span is not memory safety,
   cancellation is not rollback, packed layout is not a wire format, among
   others); at least two mutants per CTest case, all rejected.
4. Evidence. `COST_REVIEW.md` from the census thirteen-dimension sheet, comparing
   implementations under an equivalent contract, with allocation counts from the
   T66 counting allocator and layout from `sizeof` and `offsetof`.
   The T65 pilot records direct operation and storage counts first; its allocation
   comparison is revisited after T66 supplies the counting allocator. Do not
   claim an import from a producer that has not yet been implemented.
5. Crosswalk reading. The Rust, C++, Go, Swift, Zig, Odin or C3 counterpart from
   registered documentation: what the language enforces statically and what the
   C version checks at run time.
6. Transfer. Each domain's last project lands in a recurring system through
   direct imports: `runtime.vm` and `storage.pages` for the appliance,
   `firmware_io.*` for the gateway, `io.*`, `ipc.local` and
   `communications.tcp_fault` for the service; never `system.gateway` or
   `system.vm_store`.
7. Catalog. One sealed decision per domain; one recall set per subtopic with
   three variants, the changed-condition variant built on a misconception; at
   least fifteen glossary terms per domain.

## Stages

- M0, done 2026-10-16: decisions, scope, phase and prefix reservations and
  ownership recorded in `planning/`.
- M1, research, can run beside T57–T64 work: copy the census into
  `research/v5/modern/census/`; write coverage-ledger fragments in
  `research/v5/modern/coverage/Txx.json` (disposition `project`, `imported`,
  `non-equivalence` or `excluded` with a reason); per domain
  `research/v5/modern/<author>_{resources.json,sources.md}`, a search log and a
  reflection, following broad search, reflection, focused search; at least three
  assignable resources with passage-inspected locators per domain; private
  extracts in `research/private/v5/<author>/`. Add ISO C17 locators, which the
  census lacks. Unheld books go to an acquisition list, never into assignments.
  Gate: `reviews/v5/research_gate_modern.json`.
- M2, tooling, after the domain-recipe tools exist, owned by the integrator:
  the modern gate branch in `tools/common.py` `require_research_gates()` and wider
  globs in `tools/integrate_v5.py`; per-domain manifest fragments merged by
  `tools/v5_common.py` `domains_manifest()`; the T57–T64 order pinned through
  `rules['order_prefix']` in `tools/author_v5.py` and asserted in
  `tools/check_v5.py`; authored unit minutes, executable flags and scaffolding
  honored by the unit generator; `tools/check_v3.py` reading
  `edition['new_domains']`; new receipts in `edition.json` and a new-family check
  in `tools/accept_v5.py`; per-family campaign profiles (clang/tsan) and
  per-mutant profiles in `tools/run_unit_campaigns_v5.py`; TSan, dl and threads
  probes in `cmastery/cli.py`; cross-family `dependencies` in each `family.json`;
  a `programming-model` vocabulary kind and resolution of shared terms
  (copy-on-write, generation-checked handle, context switch, deadline, idempotent
  redo); `tools/research_library.py` `swe()` searching element nodes;
  `tools/check_modern_coverage.py`; `tools/templates/cost_review.md`. Spikes
  before any prose: TSan on a Chase–Lev deque and a sequence lock (clang only,
  relaxed-atomic data words, no 16-byte compare-and-swap); x86-64 fibers under
  ASan and TSan (fiber-switch annotations, `.note.GNU-stack`); `dlopen` under
  ASan (MODULE libraries, `${CMAKE_DL_LIBS}`, the POSIX `*(void **)&fp = dlsym(...)`
  form).
- M3, pilot: T65 end to end through `tools/run_unit_campaigns_v5.py family
  errors`.
- M4, waves along the import graph, producers before consumers: T66, T67, T68;
  then T70, T69, T71; then T72; then T73, T74; then T75, T76. T70 freezes
  its small-buffer storage contract before the T69.05 consumer starts. TSan profiles for
  T73, T75, T76 and the atomic projects of T66; T72 and T74 are x86-64 only; all
  families run in WSL.
- M5, integration and acceptance: artifact consumers updated with old lists as
  prefixes, vocabulary lint at zero findings, reviews by non-authors, then the
  edition's stage 8 and 9 receipts, `accept_v5` and `package_v5`.

## Verification

- `tools/check_modern_coverage.py`: every census ID and selected SWE element has
  a disposition that resolves to a subtopic, an accepted topic, a recall variant
  or a recorded exclusion.
- `build.ps1` under PowerShell 7, including `check_v5`, `check_fixtures`,
  `validate`, `check_vocabulary --domain Txx` and the authored-content checks.
- `tools/run_unit_campaigns_v5.py family <family>` in WSL: the learner run fails
  at ctest, the assessor run passes on gcc and clang in debug, release and ASan
  (plus clang/tsan where declared), and every mutant is rejected.
- A learner round trip: `python -m cmastery init PR-T65-01 <dir> --unit <unit>`,
  then `python -m cmastery check <dir> --unit <unit>`.
- `tools/accept_v5.py`, run only after source changes are complete.
