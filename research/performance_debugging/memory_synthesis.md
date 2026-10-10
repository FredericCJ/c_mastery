# T45 research synthesis and authorship proposal

The proposed laboratory adds controlled failure observation, contract repair and delayed transfer to the existing memory foundations. It does not require rebuilding an allocator, repeating startup bring-up or implementing another complete debugger. Ten substantive topics, thirty assessable objectives and ten milestones cover the requested failure families. Teaching remains pending the integrator's scoped memory research gate.

## Evidence and retrieval lineage

The current Knowledge generation was pinned once as `20261009T020803Z-three-humble-intake-candidate02`. The ruleset is 1.3.0. Registry SHA-256 is `f5cb3ab194aceba992f1ed1d5c721d0022ad1b18b2dea0c515b77ae16b3033e9`; roster is `d7ef2cd30ee4599fbd270cbc6adfcd1c360067ca07edb0013b9eb45a69111d0e`. Census pointer SHA is `49159423d1772fc68379240dbf6e1cc29bee7823ccaae9ea827670d5685f2ae5`, referenced manifest `1758eecbae671da98ecb58f8561ff4fc347168ebf421b1d877ea49685bc6bad7`. Database pointer SHA is `5d03167547ed7e3fa772e3f510dbaab63d43f5261b85f46d450a7ad49ac3a348`, manifest `fe90beba8a4da4362bc1a1fef4418f83d4a0d0e59bdcadf55aefbb004f059b5d`.

Historical path prefixes were resolved read-only against the user's active library. No pointers, library files, graph data or catalog artifacts were changed; no excluded archive or opaque package interior was traversed. The private pin record also preserves the exact SWE and data-structure graph hashes. Public rebuilding needs none of these private captures.

Discovery and passage evidence are distinct. Exact-title and multiword library misses led to narrower queries; they did not establish absence. SWE led to the ASan paper and current tool authorities. Data-structure nodes suggested bounded ownership histories without supplying language-safety authority. The existing Effective C second-edition payload was actually inspected and retains its existing ID/hash: `f1c6f94f-d6b0-59bd-b176-68eec8cd5fec`, SHA `f1ca1014452d1cb769384879a306bd9993b9cd75c475ef72e575290c3dd81537`. Its 2024 publisher release and 2025 copyright are separate facts. An alternate bundle-local manifestation was verified privately, then the selected passages were recaptured from the existing canonical citation's payload.

## Existing coverage and bounded additions

The audit read selected actual mechanisms and project contracts in all ten requested domains. It establishes the following handoffs, without asserting that a learner has already completed them.

| Foundation | Reused mechanism | Added T45 work |
|---|---|---|
| T03 | Object identity, extents, indeterminate state, nonzero resize and invalidation | Predict the first invalid state, compare observations, repair and transfer |
| T11 | Compiler helpers, instrumented dependencies and runtime entry | Audit native versus diagnostic image differences before interpreting a result |
| T13 | Allocation interception, conditional stack bounds and execution-context effects | Separate capacity exhaustion, corruption, retention and incomplete observation |
| T16 | Checked layouts, allocator policies, generation handles and lifetime regions | Defect/repair histories using the prior implementation rather than a second allocator |
| T21 | Partial acquisition, rollback, cancellation and first-cause context | Every failure prefix, lost owners, reporter limits and late completion |
| T27 | Reserved stack, fixed load/run sections and startup ranges | Link-region overflow, startup error, declared collision and stray write as different cases |
| T29 | Nested exceptions, reentrancy and independent DMA ownership | Explicit reclaim permission and separate host, core and device observations |
| T31 | Matching image, trusted fault frame, static stack and changed-pattern evidence | Competing causes and delayed symptoms, with watch timing/frame limits |
| T32 | Independent oracles and separate dynamic-tool builds | Paired negative/repaired controls and detection-coverage counterexamples |
| T35 | Checked resource limits and hardening audit | Explain which repair prevents a defect and which mitigation only changes consequences |

The main hosted entry reuses selected T03, T21 and T32 leaves. Embedded stack/startup/DMA exercises import selected T27/T29/T31 leaves and can be deferred until that route is ready. T45 has no mandatory T43 or T44 dependency; T44 independently retains an early debugging route. Full new-domain completion is not charged for a reused earlier implementation.

## Source comparisons, decisions and limits

The sixteen records in `memory_resources.json` define assignable boundaries. Exact edition and component capture hashes accompany every new manual passage; reused sources retain their identity. Source selection favors the language contract, then the relevant versioned tool, then an explicit target contract. A graph verification flag, contents page or uninspected title is insufficient.

Effective C supplies bounded memory and NUL/extent explanations. Portable drills deliberately exclude zero-size resize and C23-only allocation/deallocation APIs. A cleared owner is only one variable; it does not repair another alias. Source illustrations are not supplied solutions.

The four LLVM 21.1.0 manuals have different purposes. ASan addresses exercised address/lifetime violations; MSan needs suitable instrumentation coverage for initialization-use claims; UBSan's default group omits unsigned overflow and some other checks; LSan observes a separately declared leak phase. These are document contracts, not installed-tool or execution claims. [LLVM ASan](https://releases.llvm.org/21.1.0/tools/clang/docs/AddressSanitizer.html), [MSan](https://releases.llvm.org/21.1.0/tools/clang/docs/MemorySanitizer.html), [UBSan](https://releases.llvm.org/21.1.0/tools/clang/docs/UndefinedBehaviorSanitizer.html), [LSan](https://releases.llvm.org/21.1.0/tools/clang/docs/LeakSanitizer.html).

Memcheck 3.27.1 provides a separate supported-host comparison for initialization propagation, invalid release and leak reachability. A machine-level root is not necessarily an application owner. Reachable retained storage can violate a resource budget without being a lost-owner leak. [Memcheck manual](https://valgrind.org/docs/manual/mc-manual.html).

The ASan 2012 paper supports shadow/redzone/quarantine reasoning and historical false-negative examples. Its benchmark overhead, old platform choices and redzone details are not present-tool predictions. [USENIX author paper](https://www.usenix.org/system/files/conference/atc12/atc12-final39.pdf).

GCC 15.2's three-field `.su` format and qualifiers provide conditional frame evidence; the current development format is not substituted. A complete bound still needs the relevant call path, dynamic bounds and permitted nesting. Binutils 2.45 supplies fixed region, LMA, assertion and map semantics; it does not provide the allocator or startup policy. [GCC stack reports](https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/Developer-Options.html), [GNU ld regions](https://sourceware.org/binutils/docs-2.45/ld/MEMORY.html).

FreeRTOS V11.2.0's actual source makes fill-byte scanning and word units explicit. Its sampled check is an observation with limits. The general laboratory will not require a new RTOS port. Linux man-pages 6.19 supplies verified child-only resource-limit setup for native exhaustion. An exhausted-stack recovery handler is outside this drill. [FreeRTOS source](https://github.com/FreeRTOS/FreeRTOS-Kernel/tree/V11.2.0), [getrlimit](https://man7.org/linux/man-pages/man2/getrlimit.2.html).

The shared GDB record is a development snapshot, with target-specific watch resources and scope. Actual Arm DWT pages qualify any exact-PC assumption: watchpoint events are asynchronous and capabilities are implementation-defined. Existing accepted Cortex-M4 fault/frame and F439 SRAM/DMA passages support only their bounded target scope. No new MPU recipe or independent-DMA watch guarantee is assigned. [GDB watchpoints](https://sourceware.org/gdb/current/onlinedocs/gdb.html/Set-Watchpoints.html), [Arm architecture manual](https://documentation-service.arm.com/static/606dc36485368c4c2b1bf62f).

Small Memory Software, uninspected allocator surveys and the C++ memory book are not assigned. The exact pointer-book query missed; no passage was invented. T44 independently inspected its held embedded-debugging book and owns that citation. Its broader debugger examples are unnecessary prerequisites for T45. Advanced allocator mathematics, whole-kernel stack proof, device-specific guard setup and complete cache protocols remain outside this bounded laboratory.

## Proposed experiments and finite evidence

Every milestone follows a prediction, one-factor negative control, observation, first-invalid-state diagnosis, contract repair and unseen transfer. Deliberate failures remain in learner-owned hosted processes. Native undefined behavior has no prescribed value, crash signal, PC or diagnostic string. Hardware extensions first validate a defined reserved storage contract and use existing qualified adapters; a host event history never proves physical execution.

The following are original finite contracts proposed for fixtures. They are arithmetic/state evidence, not tool-output recordings or a solved C target.

| Milestone | Hours | Concrete fixture and required separate observation |
|---|---:|---|
| PR-T45-01 | 4 | Detection-coverage matrix and paired trial manifest; run native, ASan/UBSan and separate unsanitized Memcheck controls with versions/build identities |
| PR-T45-02 | 8 | Capacity 8: index −1 and index 8 rejected; seven data octets plus NUL fit, eight plus NUL do not; unchanged sentinels; add a legal-address wrong-cell control |
| PR-T45-03 | 8 | Original 16-bit size model: header 12 + count × stride 7; count 9360 gives 65532, 9361 fails addition, 9363 fails multiplication; compare native `size_t` boundaries separately |
| PR-T45-04 | 12 | Frames 32 + 48 + 64 plus basic nested contexts 80 + 72 require 296 bytes; reserve 320 leaves 24. One declared extended frame adds 72 and exceeds the reserve; compare changed-pattern samples and child-only native exhaustion |
| PR-T45-05 | 8 | Fixed 1024-byte RAM model: data 96, bss 400, retained 64, heap 160, stack 256 leaves 48 between dynamic regions; guard 32 fits. Heap 192 leaves 16 and fails the guard. A separate fixed placement exceeds the region; actual map/startup observations remain separate |
| PR-T45-06 | 8 | Three acquired resources with failure at each stage; reverse cleanup and no live resources; distinct lost-root, lost-child, interior-root and reachable-retention histories |
| PR-T45-07 | 12 | Nonzero resize failure preserves old generation/bytes; success invalidates aliases even when a numeric address token is reused; double/interior/foreign release and free-null histories |
| PR-T45-08 | 8 | Distinct accessible and initialized state; partial record fields cannot publish; model propagation and consuming branch, then native supported initialization-use controls with multiple fill patterns |
| PR-T45-09 | 10 | Submit, timeout, cancel request, late completion, confirmed quiescence and reuse histories; exactly one completion owner; reentrant scratch-buffer counterexample |
| PR-T45-10 | 8 | Unseen dossier: plausible crash symptom with competing extent, lifetime and initialization histories; earliest-invalid-event proof, repair invariant and delayed transfer |

The two synthetic stack examples explicitly stipulate a descending, single stack with permitted two-level preemption and exact frame contributions. They do not silently combine MSP/PSP or assign floating-point frame cost to every target. Real stack estimates must identify ABI, compiler, optimization, task/handler stack selection, lazy/extended frame configuration, indirect callees and unknown depth.

The RAM experiment stipulates a custom shared interval. `.bss` stays fixed after link; a runtime write into it is corruption, not growth. Link placement can succeed while an allocator violates its reserved interval, and correct placement cannot compensate for a wrong startup range. Actual pointer comparisons between unrelated objects are not used to implement the synthetic checker.

Ownership histories use integer identities/generations in the oracle. They do not dereference, compare or pass stale C pointers to a library. Successful resize tests save necessary data before the call, then use only the returned owner. Clearing one alias, or accepting `free(NULL)`, is not a complete lifetime repair. Uninitialized-value propagation is a detector model, not permission to evaluate indeterminate C values.

## Workload, assessment and gate request

The workload is reading 20 + implementation 54 + verification 32 + assessment 8 + consolidation 8 = 122 integer hours. The ten projects sum 86 = 54 + 32; route totals must not add the projects again. Implementation/verification splits are respectively 2/2, 5/3, 5/3, 8/4, 5/3, 5/3, 8/4, 5/3, 6/4, 5/3. Supplemental MSan and physical routes substitute within declared verification time where feasible; substantial unavailable-environment setup or advanced extensions require separately disclosed optional time.

Assessment requires an initial detection prediction, minimized first invalid state, repaired invariant, semantic regression evidence, explicit remaining observation limits and delayed transfer. A mitigation-only repair, altered expected data, unsupported tool invocation or missing physical evidence cannot close the corresponding claim. Math remediation targets interval arithmetic, checked multiplication/addition, graph reachability and maximum-path/nesting sums rather than a whole unrelated mathematics course.

The planned oracle will enforce exact recursive fixture set, unique fixture/project/module bindings, strict JSON booleans versus integers, field/range checks, recomputed finite results and independent corruption controls. Actual learner C, Linux tool and board execution remain learner gates. Proposed verifier name is `tools/verify_memory_lab_fixtures.py`; proposed fixture directory is `fixtures/memory_lab/`.

Research is ready for a scoped gate on the five staged files. It supports the proposed concrete laboratory, while installed-tool availability, measured native stack usage and physical DMA/fault observations remain deliberately open until learner execution.
