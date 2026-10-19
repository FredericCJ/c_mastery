# T48.02–14 private assessor derivations

Do not export this file or the completed providers. Each exact original and delayed task retains its own evidence selectors. Author execution does not establish independent learner, external review or physical observations.

## AX-PR-T48-02-T48.02.a

Original task: Implement the tag walker with total-size and offset bounds checked before reading size or rounding. Test size7/8/21, truncated size fields, unknown tags and larger mmap stride; preserve a cursor ledger.

Parse inside the supplied span, then inside the declared total; raw tag size includes its eight-byte header. Advance by checked round-up, not raw size. Cursor8 plus raw21 gives29, rounded32. Sizes7, truncation at every byte, unknown tags and mmap stride40 are separate fixtures. kf_boot stages the result; no borrowed pointer is published until the complete stream validates.

Changed condition: Use a buffer whose valid total size ends exactly on a tag boundary.

An exact total boundary is accepted even with a larger supplied buffer; no byte beyond declared total becomes a tag. Overflow is checked before addition or alignment. The end tag must be size8 and finish the declared stream.

Evidence selectors: contracts:tags.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-02-T48.02.b

Original task: Build a page reservation oracle from overlapping usable/reserved half-open physical intervals, plus kernel, module and boot-information spans. Permute input order and include partial-page overlaps.

A frame is usable only when one usable interval contains the entire page and no reserved interval overlaps any byte. Thus reserved wins regardless of tag order. The endpoint-set Python oracle checks128 independent randomized layouts;24 permutations cover explicit kernel, boot-info and module reservations. Partially covered pages remain unavailable.

Changed condition: Move the module into a formerly free interval and recompute capacity.

Move the module reservation and recompute the same set difference; do not reuse a high-water mark that accidentally reserves unrelated holes. Empty reservations do not consume pages. Invalid and overflowing intervals refuse before any allocator state changes.

Evidence selectors: contracts:reservations, contracts:independent-set-oracle.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-02-T48.02.c

Original task: Snapshot parse outputs, insert a late duplicate required tag, missing terminator or overflowing base+length, and verify refusal preserves the documented output state and no allocation occurs.

Late duplicate required tags, missing terminal tag and overflowing map base+length refuse transactionally: the caller result and allocator state retain their sentinels. Validation precedes frame publication. A correct early tag cannot authorize allocation before the late stream is known valid.

Changed condition: Add a validated optional tag without making required-tag absence acceptable.

The optional command-line tag requires a terminating NUL within its raw extent and at most one instance. Ignoring it does not relax mandatory map/module/end checks. The optional-tag mutant is killed by its own malformed optional fixture.

Evidence selectors: contracts:tags, contracts:optional-killed.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-03-T48.03.a

Original task: Derive byte offsets of the 15 saved GPRs, vector/error pair and architectural frame from interrupts.S; compare a hardware-error and synthesized-zero entry, C offsetof values and 16-byte call alignment.

The trap contract is22 qwords:15 saved GPRs, vector, error, RIP, CS, RFLAGS, RSP, SS. Every field has an offsetof assertion and the canonical size is176. Exception entry supplies or synthesizes one error slot, not two. The assembly wrapper records C-entry RSP before a prologue; its modulo16 must be8.

Changed condition: Change diagnostic metadata by one word and repair call alignment without changing iret state.

Case16 adds an assembly metadata word and removes it at the matching boundary so the canonical C frame and return layout remain correct. Changing one push without the corresponding offset/cleanup is not a valid changed frame. O0/O2 and both compilers execute this route.

Evidence selectors: guest:case0, guest:case16.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-03-T48.03.b

Original task: Trigger one original user divide fault and one page fault in separate guests. Capture vector, error, RIP, CS and process identity through bounded serial output; verify return for a nonfatal interrupt and termination for the fault policy.

Separate actual guests execute user divide error0/vector0 and user page fault vector14/error7, preserving RIP,CS,pid and CR2. The first record is fixed-size and emitted before optional text. Normal int80 return and user fault termination are distinct paths; surviving workers retain registers and private memory.

Changed condition: Exhaust the diagnostic output budget and preserve a minimal first-fault record.

Case5 sets optional diagnostic budget to zero. The minimal first record still precedes the truncation marker. A truncated log is insufficient evidence of complete diagnostics; here the bounded primary record is the required artifact.

Evidence selectors: guest:case1, guest:case0, guest:case5.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-03-T48.03.c

Original task: Compare the same exception class from user and kernel mode in disposable images. Show user termination and peer progress, but a fatal kernel diagnostic stop; retain first-fault state if a second fault occurs during reporting.

A user exception marks only that process dead, revokes its handle and schedules peers. The same divide in ring0 is fatal. Case3 raises a kernel invalid opcode while processing the first user fault: nested increments but the original vector/RIP/CS/pid remain unchanged.

Changed condition: Inject a fault while the ordinary kernel stack is unavailable and state which observation remains possible.

Case4 deliberately switches the owned guest to an unmapped ordinary kernel stack, then faults. The actual double-fault frame lies within the separately allocated IST stack. This proves the configured TCG route, not recovery from arbitrary corruption or triple fault.

Evidence selectors: guest:case0, guest:case2, guest:case3, guest:case4.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-04-T48.04.a

Original task: Implement reserved/free/allocated frame states; exhaust [3MiB,4MiB), release in a permuted order, and reject unaligned, reserved, foreign and duplicate releases with unchanged state.

The hosted pool contains exactly256 pages in[3MiB,4MiB). Allocation exhausts that set; release order i*73 mod256 is a permutation. Unaligned, reserved, foreign and double release all preserve state. A frame address is not sufficient authority unless its state says allocated.

Changed condition: Introduce a reserved hole and derive the exact returned set, not just a smaller total.

A nine-frame reserved hole800..808 reduces capacity to247. The oracle checks the missing addresses, not only the count. The real guest reserves its actual kernel/archive/boot extents separately; host addresses are never substituted for guest frame authority.

Evidence selectors: contracts:frames, contracts:reservations.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-04-T48.04.b

Original task: Inject frame exhaustion at each mapping allocation for a multi-page user request. Snapshot PTEs/free frames, enforce zero-fill before exposure, and require rollback of newly acquired pages. Distinguish boot-loader panic policy from a recoverable syscall allocation API.

For requests1..16, every failure prefix is injected;152 hosted cases check restored PTEs/free count and unchanged earlier mappings. The guest separately injects each of16 prefixes into a real16-page syscall and checks rollback before reporting failure. Success zeroes all newly exposed bytes. Boot construction failures are fatal; recoverable syscall allocation failures preserve the old process.

Changed condition: Allocate a range crossing an existing mapping and refuse without damaging it.

A requested range crossing an existing PTE refuses during preflight before allocating its first new frame. The older mapping and its0x73 bytes remain unchanged; rollback must release only frames acquired by this request.

Evidence selectors: contracts:allocation, guest:case22.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-04-T48.04.c

Original task: Trace process exit through marking dead, selecting a peer, changing CR3/TSS stack and reclaiming frames. Identify static page-table/kernel-stack storage versus allocated user frames and show which objects the current code actually reclaims.

Dead is not reclaimable. The hosted lifecycle requires no current/CR3/TSS owner before freeing user frames, then a separate executing-stack handoff before retiring a dynamic slot. Real guest records active CR3 and executing RSP at reclamation: old static kernel stacks/page-table arrays remain allocated while user frames are released. Terminal exit switches to permanent CR3/TSS first.

Changed condition: Design dynamic process admission and derive the extra teardown and slot-generation obligations.

A dynamic admission slot advances generation only after mappings and stack ownership are gone. UINT32_MAX retires the slot rather than wrapping. Tests distinguish current, CR3, TSS and old static stack authority; the missing-CR3 mutant is detected by a distinct ownership arrangement.

Evidence selectors: contracts:lifecycle, guest:case0, contracts:handoff-killed.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-05-T48.05.a

Original task: Derive effective permissions along all page-table levels for the process window, kernel identity region, text, data and guard. In an intentionally flawed guest map a dedicated synthetic kernel sentinel page as user-writable; compare corrected permissions.

Access combines all four levels: present AND, user AND, writable AND under CR0.WP=1, execute allowed only when no NX bit under EFER.NXE=1. The196608-case finite checker exercises these combinations. Case7 deliberately grants user write to one dedicated non-control sentinel page; case0 removes it and observes fault7 with unchanged sentinel.

Changed condition: Move the sentinel behind a different upper-level table and repeat the full effective-permission argument.

Case8 moves the sentinel to another upper-level table. All levels must be audited again; fixing only the leaf is insufficient when a different ancestor controls access. Neither model nor guest claims SMAP,SMEP,PKU,reserved-bit or physical-CPU coverage.

Evidence selectors: contracts:permissions, guest:case7, guest:case0, guest:case8.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-05-T48.05.b

Original task: Run two original workers storing different values at the same virtual address and a third attempting the synthetic supervisor write. Record CR3 identities, saved fault address/code, sentinel contents and peer counters in flawed/corrected guest copies.

Two actual worker ELFs store different PID-derived values at the same virtual address, then run12million iterations without yielding while checking all15 GPRs. A third process attempts the dedicated kernel sentinel write. Fault vector14/error7/CSring3, unchanged sentinel, distinct CR3 roots and both worker completions establish the bounded isolation relation.

Changed condition: Attempt data-page instruction fetch and derive its different fault evidence.

Case9 allocates writable NX data, stores a return instruction and attempts an instruction fetch. The actual fault is14/error21 at that heap address. Write permission and execute permission are independent; successful data writes cannot establish executable authority.

Evidence selectors: guest:case0, guest:case9.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-05-T48.05.c

Original task: Analyze a stack jump that skips a one-page guard and lands in another mapped region. Compare with a sequential overflow reaching the guard; state exact writable regions and independent compiler/runtime checks.

Case10 faults on a normal write to the guard. Case11 moves RSP across it directly into a deliberately mapped lower page and writes successfully, showing that an unmapped guard is not an arithmetic bounds check on stack growth. Both are owned guest demonstrations.

Changed condition: Add explicit stack probing and identify the size assumptions needed for its argument.

Case12 touches each4096-byte step and faults at the guard before reaching the lower page. The argument requires maximum probe stride no larger than the guaranteed guard width, no alternate unprobed stack adjustment and the stated mappings; it is not a compiler-wide stack-clash claim.

Evidence selectors: guest:case10, guest:case11, guest:case12.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-06-T48.06.a

Original task: Separate a pure next-runnable policy from saved-frame/address-space transfer. Replay alive=[true,false,true],current0 for four selections and then exercise zero/one runnable states.

The pure selector treats only live slots as candidates and returns explicit no-task when all are dead. With alive[T,F,T], current0 produces2,0,2,0. Zero/one-live and24 alive/current combinations are checked independently of context assembly.

Changed condition: Replace round-robin with weighted choice while preserving the same context mechanism.

The smooth weighted variant keeps selection credits separate from the saved frame. Weights1:2 produce100:200 selections over300; invalid weights preserve credits. Case17 uses that policy in the same actual context-switch path. This finite schedule is not a production fairness guarantee.

Evidence selectors: contracts:scheduling, guest:case17.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-06-T48.06.b

Original task: Run nonyielding users with GPR and private-memory sentinels; record timer-origin switches and PIC acknowledgements. Mutate one saved register or omit EOI in separate disposable builds.

The real PIT timer preempts two non-yielding workers, and the handler acknowledges the PIC before switching frames. Case13 corrupts a saved worker r15 and fails the GPR comparison; case14 omits EOI and fails the required timer/preemption evidence. Each mutation has a separate causal detector.

Changed condition: Add an exited slot between runnable peers and retain correct stack ownership.

Case18 places the exiting fault slot between the two surviving workers. Subsequent choices skip it while preserving peer progress and register state. Scheduling policy tests alone would not establish this assembly/interrupt behavior.

Evidence selectors: guest:case0, guest:case13, guest:case14, guest:case18.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-06-T48.06.c

Original task: Measure a peer's delay during a bounded synchronous kernel poll and compare with an equal-length user loop. Report clock source, emulator scheduling, poll count and nonpreemptible interval separately.

The user loop admits timer progress; the synchronous kernel poll runs with interrupts disabled and records equal tick values at both ends. Device polling and a separate bounded pause loop have separate poll counts and emulator TSC intervals. TSC observations are neither physical WCET nor proof of constant callback time.

Changed condition: Change quantum and device completion delay independently and explain which latency component changes.

The matrix varies PIT divisor11932/5966 and completion delay0/2000000 independently. Device delay is inserted after notification and before completion inspection. Changing the quantum cannot preempt that disabled-interrupt interval; measured wall time/TSC/poll count remain distinct observables.

Evidence selectors: guest:case15, guest:quantum-delay-matrix.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-07-T48.07.a

Original task: Construct ELF64 byte fixtures that fail late: ninth PT_LOAD, filesz>memsz, wrapped offsets, entry outside executable bytes, overlapping pages and a writable executable segment. Validate all headers before mapping and snapshot frame state on refusal.

The complete ELF descriptor array is validated before mapping: late ninth LOAD, filesz>memsz, wrapped file extent, entry outside executable storage, overlapping pages and W+X all refuse. The host loader snapshots PTE/frame state and zeroes the memory tail only after successful admission.

Changed condition: Create nonoverlapping byte extents that share a page and explain the permission conflict.

Two byte-disjoint LOAD ranges can still share a4KiB page and demand conflicting permissions. CMK refuses this profile before allocation instead of merging flags and accidentally creating W+X. The test uses that exact same-page counterexample.

Evidence selectors: contracts:images, guest:case23.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-07-T48.07.b

Original task: Implement CMKA1 lookup with unique bounded names and disjoint table/payload spans. Load filesz1,memsz4096 and verify the remaining4095 bytes are zero; test a zero-fill tail spanning two pages.

CMKA1 validates bounded unique NUL-terminated names, table/payload extents and disjoint payloads. The one-entry fixture begins payload at56 and ends at64. ELF filesz1/memsz4096 leaves4095 zero bytes; the two-page case checks8191 zero bytes, not only the final byte.

Changed condition: Use an accepted ELF without section headers and show program-header loading still works.

Case19 clears section-header metadata in archived ELFs and still boots their PT_LOAD segments. Program headers drive execution; section tables are unnecessary for this loader. Invalid archive output pointers and lengths remain unchanged.

Evidence selectors: contracts:archives, contracts:images, guest:case19.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-07-T48.07.c

Original task: Compare the exact CMK acceptance table with the inspected generic ELF program-header source: ET_EXEC, TLS, interpreter, alignment and permissions. Provide a valid general ELF case intentionally refused by CMK.

CMK admits only its fixed static ET_EXEC, x86-64, bounded user window and permission/alignment profile. TLS, INTERP, DYNAMIC, ET_DYN and generic ELF align1 examples are refused even where general ELF permits them. This is a documented subset, not proof that such files are malformed ELF.

Changed condition: Design a read-only additional archive entry type without implying dynamic linking support.

The separate CMKA2 codec uses the legacy reserved type word for1=ELF and2=read-only data, with a version2 header. It checks expected type, unknown types, remaining reserved word and overlaps. CMKA1 is unchanged; read-only data does not introduce dynamic linking or executable entry points.

Evidence selectors: contracts:images, contracts:archives.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-08-T48.08.a

Original task: Specify int0x80 register/error ABI and checked copy-in/out. Use spans ending exactly at a page edge, crossing into a guard, wrapping length and targeting read-only text; stage validation before any observable output.

int80 uses RAX number/result and RDI/RSI/RDX arguments; negative means refusal. Copy validation walks every page before output or device submission, checking wrap, upper bound, permissions and entire extent. Tests cover last mapped byte plus an unmapped next byte, read-only destination and kernel pointers.

Changed condition: Change zero-length span policy and propagate it consistently through all services.

The chosen zero-length rule still requires a pointer within the user window: valid heap pointer/length0 succeeds, USER_LIMIT/length0 refuses. This explicit convention is exercised in the guest and must be preserved consistently across copyin/out; ISO C alone does not define syscall pointer authority.

Evidence selectors: guest:case0, contracts:images.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-08-T48.08.b

Original task: Implement process-local slot/generation handles and close/reuse tests. Derive token65537 for slot0 generation1, reject high-bit truncation, foreign process tokens and old tokens after reuse; specify generation exhaustion.

A token carries a16-bit generation and1-based slot;65537 means generation1/slot0. High bits, zero slot, foreign process table and stale generation refuse. Closing advances generation; maximum generation retires the slot. Resource identity and process-local handle authority are separate.

Changed condition: Allow a delegated read-only handle and derive explicit rights propagation.

Read-only delegation installs a distinct destination-table token with no amplified rights. Closing the source does not destroy the fixture immutable resource42; its lifetime is explicit and independent. The new token cannot write, and stale tokens cannot acquire the newly reused source slot.

Evidence selectors: contracts:handles, guest:case0.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-08-T48.08.c

Original task: In original user programs issue unknown calls, kernel pointers, oversized allocation and closed handles while a peer updates a synthetic counter. Snapshot kernel resource counts and sentinel data before/after each rejection.

Actual guests issue an unknown syscall, kernel pointer, allocation17, closed disk handle and invalid user destination. Status/free frames/sentinel and both peers remain intact. A matching error alone would not prove that resource or device state was preserved.

Changed condition: Combine a valid handle with an invalid destination and verify neither authority nor state is consumed.

The valid disk token with an invalid destination is refused before device_submissions increments, free count changes or token consumption. The same handle remains usable and can be closed afterward. Case24 omits revocation as a separate negative control.

Evidence selectors: guest:case0, guest:case24.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-09-T48.09.a

Original task: Build synthetic PCI capability chains with loops, truncated BAR windows, duplicate required capabilities, absent VERSION_1/FLUSH and overflowing notification multiplication. Derive an initialization state machine and reject before DRIVER_OK.

The checked numeric PCI snapshot validates aligned acyclic capability traversal, extents, duplicate kinds, BAR aperture bounds and required common/notify/device regions. Eleven malformed fixtures preserve output. The actual guest sizes and restores its own BARs before enabling DMA, then uses this parser. VERSION_1 and FLUSH are required before DRIVER_OK.

Changed condition: Change a BAR base while preserving legal extents and recompute notification address.

Relocating BAR0 to0x90000000 changes all derived region addresses. Notification offset uses64-bit product of uint16 queue offset and uint32 multiplier, then bounds the two-byte store inside the BAR capability. The synthetic overflow/extent refusal precedes any MMIO authorization.

Evidence selectors: device:pci, guest:case0.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-09-T48.09.b

Original task: Trace ownership of header, bounce page, status, descriptors and ring slots across one request; exercise16-bit index65535→0 with eight slots7→0. Inject a duplicate/wrong used ID and a late completion after timeout.

A split queue transfers ownership of header, bounce bytes, status and descriptors together. The host model uses distinct storage for heads0 and3,16-bit index65535→0 and ring slot7→0, out-of-order completions, wrong IDs and duplicates. Timeout poisons the device and retains the storage even after a late completion.

Changed condition: Permit two outstanding requests and derive per-request storage/generation needs before implementing it.

Two outstanding requests have separate buffers and local ticket generations. The used-ring wire ID has no generation: a protocol-violating old duplicate after legal reuse is indistinguishable from a new completion. The executable counterexample makes this limit explicit; reset acknowledgement is an external quiescence premise, not a local epoch field.

Evidence selectors: device:queue.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-09-T48.09.c

Original task: Annotate each descriptor store, compiler barrier, CPU ordering instruction, MMIO notification and completion read in the actual driver. Explain the x86 coherent identity-DMA assumptions and construct a reordered abstract countertrace if publication ordering is removed.

The actual driver stores header/data/status/descriptors, publishes available ring/index with compiler memory clobbers and mfence, then notifies MMIO; completion inspection follows another barrier. The linked disassembly contains the actual fences. The six visibility orders admit only descriptor→publication→notification; premature notification can observe unpublished or stale state.

Changed condition: Port the argument to a noncoherent architecture and list cache/IOMMU changes requiring new evidence.

Noncoherent DMA adds cache clean/invalidate and ownership boundaries; an IOMMU adds a mapping lifetime and device-visible address authority. The explicit model shows CPU cache value42 while device memory remains0, and differing bus/physical addresses. The x86 coherent identity-DMA TCG run does not discharge either added premise.

Evidence selectors: device:ordering, guest:case0.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-10-T48.10.a

Original task: Draw request states submitted, completed-success, completed-error, timed-out-unknown and flush-confirmed. Inject timeout after publication; prohibit buffer reuse and show why an error is not proof that no write occurred.

Submission, successful completion, device error, timeout/unknown and successful flush are separate states. A write may have persistent effects before an error; neither error nor timeout means rollback. The queue preserves timed-out buffers and a4097-prefix-cut medium model supplies concrete partial-persistence counterexamples.

Changed condition: Add a reset acknowledgement contract and identify exactly when reclamation becomes justified.

Only observed reset acknowledgement authorizes reclaiming quarantined storage. ack0 leaves the complete queue unchanged; ack1 invalidates old tickets before new submission. A boolean test fixture models this external authority but does not establish physical reset completion.

Evidence selectors: device:queue, device:durability.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-10-T48.10.b

Original task: Implement checked page-to-sector conversion and the67-page slice starting at16. Test page1023/1024 of a4MiB image, slice endpoints15/16/82/83 and large caller integers before narrowing.

Virtio sector addresses use512-byte units; a4096-byte page uses8 sectors. A4MiB disk has1024 pages, so1023 maps to8184 while1024 refuses. The store adapter admits logical0..66 and maps only physical[16,83), even on a larger raw disk.

Changed condition: Change sector size in a model and refuse unsupported real-device geometry.

The changed4096-byte-sector profile is explicitly unsupported by the fixed adapter. Hosted geometry refuses it; case30 compiles the changed profile and the actual guest refuses before use. This is a hypothetical changed ABI profile, not a claim that virtio logical block-size negotiation changes its512-byte sector field.

Evidence selectors: device:geometry, guest:case30, guest:case31.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-10-T48.10.c

Original task: Run patterned write→flush→read and a fresh-guest reboot with the identical ordinary image. Retain backend cache options and full byte comparison; remove flush in a control and explain why a surviving value is still insufficient.

Case31 writes all4096 bytes of pages0,82,1023, observes write completions and FLUSH success, then starts a fresh QEMU process on the same ordinary4MiB file. The second guest and host compare all12288 bytes. Source, image, command and cache=writeback identities are retained.

Changed condition: Use the storage simulator to cut a declared write/flush event and compare its distinct failure model.

Case32 omits FLUSH and records survival after clean emulator shutdown without calling it durability. The separate byte-prefix model enumerates4097 cuts where unflushed data may be old, partial or complete; acknowledged flush requires all bytes. Neither experiment simulates a physical power loss.

Evidence selectors: guest:case31, guest:case32, device:durability.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-11-T48.11.a

Original task: Implement the64-byte shell buffer and explicit overflow/reset policy using only the CMK ABI. Specify whether capacity includes terminator; test empty line, maximum accepted line, overlong line followed by help and partial input across reads.

The line capacity64 includes its terminator:63 payload bytes fit,64 enter discard mode until newline. An overlong line ending in exit cannot become an executable exit command; a following help still works. Empty, maximum, overflow and partial input are distinct tests.

Changed condition: Accept CRLF while retaining a precise split-chunk policy.

CR completes the line and suppresses one following LF even across input chunks. The actual driver sends help-CR and a later LF, then splits echo text across writes. The command language is bounded printable text plus CR/LF and explicit Ctrl-D termination.

Evidence selectors: contracts:lines, guest:case20.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-11-T48.11.b

Original task: Drive help, echo and exit through the actual serial input channel. Assert archive names, parser responses, process handle revocation and post-switch frame accounting; distinguish unavailable input from EOF/exit.

The actual serial session drives help, echo, stats and exit through the checked boot-archive shell ELF. READ returning0 means currently unavailable; the process yields and waits, rather than interpreting it as EOF. Exit revokes capabilities and reclaims user frames after an address-space handoff.

Changed condition: Interrupt a partial line with process termination and verify cleanup.

Case21 delivers an incomplete echo line, confirms the emulator remains alive, then sends explicit Ctrl-D. The unfinished command is not executed; the shell reports its partial byte count and follows the ordinary teardown path. Input events and offsets are retained separately from output.

Evidence selectors: guest:case20, guest:case21.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-11-T48.11.c

Original task: Design one bounded read-only statistics command returning copied numeric data. State its ABI, buffer limit, failure semantics and privileges; compare boot archive lookup with dynamic process creation and a general filesystem.

KF_STATS copies version,size,pid,free frames,ticks,preemptions and device submissions into a validated writable user span. It exposes numbers, not kernel addresses or writable kernel structures. The object size is fixed and the copy happens only after complete range validation.

Changed condition: Add a versioned result structure and reject unsupported versions without partial output.

Version2/9 requests refuse without changing the sentinel-filled output. The shell and init both exercise this rule. A future version needs a declared size/field compatibility policy; guessing a larger native struct is not ABI compatibility.

Evidence selectors: guest:case0, guest:case20.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-12-T48.12.a

Original task: Link the same hashed CVM core into a ring3 ELF through explicit adapters; run identical verified bytecode hosted and in guest and compare semantic result/error traces.

The baseline host and ring3 builds consume the same SHA-bound assessor/runtime/cvm.c and the same19-byte enum-encoded fixture. Six semantic rows compare exactly, including failure result sentinel, callback count, remaining fuel and allowance. Only adapters and target flags differ.

Changed condition: Load bytecode from a checked archive span and reject a truncated image before running.

Case28 loads an18-byte span from the checked boot archive. Container validation rejects it before any VM run or callback, with run_calls0. The archive byte identity is recorded independently of the valid hosted fixture.

Evidence selectors: runtime:case26, runtime:case28.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-12-T48.12.b

Original task: Audit undefined symbols, map sections, static workspace and worst permitted nested call paths for CVM plus user adapter. Include verifier scratch, exception/kernel stacks separately and inspect generated helper calls.

Every VM ELF closes undefined symbols with owned byte helpers and has a retained map/disassembly/.su report. Cvm is3216 bytes and CvmProgram1076; baseline verifier scratch5120 is automatic, not static. Native call-chain accounting includes simultaneous callers and the audited callback, while VM recursion uses16 explicit frames. User128KiB, kernel8KiB and IST8KiB are separate budgets.

Changed condition: Tighten the user stack and move suitable scratch to caller-owned static storage with a nonreentrancy contract.

The exact-source registered scratch profile borrows a caller-owned5120-byte static workspace, rejects nested acquisition before mutation, and releases it on error/success. Its actual4KiB user stack runs successfully at O0/O2 with both compilers; unchanged baseline faults in the unmapped stack. GCC O2 conservative paths are6400 versus2352 bytes. This bound is for these sources, flags and callback only.

Evidence selectors: runtime:case26, runtime:case27, runtime:case29, device:reentry-atomic.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-12-T48.12.c

Original task: Execute callback increment41→42 with allowance1, then zero allowance and callback failure. Count actual callbacks and ensure failed calls do not masquerade as success; separate instruction fuel from host allowance.

INT41,HOST1,RET with fuel32/allowance1 returns42 after one callback. Allowance0 yields HOST_BUDGET with zero callbacks; callback error yields HOST_ERROR and preserves result−99. Fuel1 stops before HOST, while fuel2 permits HOST then exhausts before RET: instruction budget and external side effects are separate.

Changed condition: Use a callback that consumes a bounded internal loop and derive a combined service bound.

The bounded callback performs exactly256 counted iterations. Under independently justified per-op bound Cvm and per-loop-step bound Cstep, service work is at most3*Cvm+Centry+256*Cstep+Cexit for this program. No measured instruction count or allowance alone supplies physical time bounds, and arbitrary callbacks need new premises.

Evidence selectors: runtime:case26, runtime:case27.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-13-T48.13.a

Original task: Draw the full bytecode→named service→store→page adapter→syscall→virtio path, with ownership and error translation at every edge. Verify the adapter confines pages to[16,83) and uses caller-owned workspace.

The real chain is CVM HOST→vh_host/store_host→store transaction→67-page adapter→int80 validated copy→owned virtio bounce/queue. Static caller-owned VM/program/store objects fit user data; the opened store has one serialized owner. The adapter maps only[16,83), and outer disk regions remain zero in the captured image.

Changed condition: Add a read-only service client while keeping exclusive transaction ownership.

A read-only Vh client can query through the same owner but its assignment is denied before store reads/writes/flushes change. Native C can forge the structure; this is embedding authority, not native pointer isolation. The guest still owns the sole disk handle and serializes all store calls.

Evidence selectors: guest:case33, guest:case34.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-13-T48.13.b

Original task: Run the two-boot commit/lost-reply experiment with shared VM/store sources and same disk. Preserve first CVM_HOST_ERROR, actual successful store commit/flush evidence, second VM GET42 and store-check output.

Case33 commits42 through the actual legacy service and returns an indeterminate callback reply, producing CVM_HOST_ERROR. A fresh second guest opens the same store/disk and reads42, then checks the store. Identical core/store/provider and disk identities bind the two boots.

Changed condition: Cut before commit acknowledgement in the storage simulator and compare allowed old/new outcomes.

The separate prefix-cut model and the frozen service crash campaign distinguish modeled cuts before commit acknowledgement from clean QEMU process exit. Recovery admits only the transaction contract outcome; a successful flush is a durable boundary, whereas a failed response alone cannot select old versus new. The actual guest does not claim a physical cut.

Evidence selectors: guest:case33, device:durability, services:74-cuts.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-13-T48.13.c

Original task: Give caller histories with identical HOST_ERROR but one failed-before-commit and one committed-before-reply-loss. Design a query or durable request-ID protocol before retrying a non-idempotent increment.

Cases34/35 produce the same HOST_ERROR after a committed increment with lost reply and before a refused commit. Reboot query distinguishes42 from41. Retrying a non-idempotent increment blindly would produce43 in the first case; the versioned service commits value and operation receipt atomically.

Changed condition: Persist a deduplication record atomically with the value and analyze retention/exhaustion.

The receipt is17 bytes: version1,idLE64,deltaLE32,resultLE32. Latest exact duplicate ID1/delta1 returns42 without a generation change; changed payload and ID gap refuse. Only one strictly sequential client is covered. Older receipts are not retained; ID UINT64_MAX admits only its exact duplicate and cannot advance or wrap. Exhaustion requires a new durable epoch/retention contract, never deleting the receipt and retrying an old operation.

Evidence selectors: guest:case34, guest:case35, services:receipt-exhaustion.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-14-T48.14.a

Original task: Diagnose a withheld original kernel mutation using host contract tests, guest saved state and a minimal causal trace. Explain an alternative hypothesis and the observation that rules it out.

Host mutation controls remove optional-tag validation, reservation precedence, rollback release, CR3 handoff, duplicate-cap checks and timeout quarantine. Separate actual guests corrupt a saved register, omit EOI, retain a stale handle and inject a late malformed ELF. Each diagnosis names the first divergent state and an alternate discriminator, not only a missing final marker.

Changed condition: After seven days change a resource limit and repeat without opening the assessor implementation.

Case25 changes admissible allocation from16 pages to1. The2-page request refuses with unchanged free count; smaller requests remain meaningful. Reserve the exact case for a learner only after at least7 days and freeze their prediction before showing the reference trace. Author execution is preparation, not the delayed assessment.

Evidence selectors: contracts:mutation-controls, device:mutation-controls, guest:case13, guest:case14, guest:case23, guest:case24, guest:case25.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-14-T48.14.b

Original task: Build from a fresh directory, bind source/image/tool hashes and deliberately test a stub, malformed ELF and stale handle. Reject timeout, stale logs and mismatched image receipts.

Fresh-directory campaigns retain exact compiler/source/image hashes, commands, exit codes and raw logs. Incomplete learner stubs fail host contracts; malformed ELF refuses before allocation; stale-handle mutation fails the user check. The guard rejects altered sources, missing runs, mismatched images and invented timeout success.

Changed condition: Change compiler optimization and explain any changed observation point.

O0/O2 comparisons keep semantic predicates fixed while recording stack sizes and compiler-dependent observations. A changed observation does not license rewriting expected semantics after seeing a result. Build and log identity are checked together; old serial output is never sufficient for a new image.

Evidence selectors: contracts:stub-incomplete, device:stub-incomplete, guest:case23, guest:case24, guard:negative-controls.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

## AX-PR-T48-14-T48.14.c

Original task: Prepare a delayed assessor-held variant combining one mapping-limit change and one known interface contract. Require prediction, implementation, failure injection and an explicit list of physical/SMP/general-OS claims still unassessed.

The dossier binds the selected memory map, task/frame/queue/stack limits, original ABI and known failure policy. It states absent physical timing, full hardware, general OS and SMP claims. Held-back allocation/interface conditions have their own predictions and failure records; no global success marker substitutes for them.

Changed condition: Integrate a later T56 process/thread mechanism and revisit every single-core assumption used here.

The separate frozen CMK2 T56 provider runs1/2/4 CPUs with explicit baseline VM/store linkage. Process address-space ownership is separated from TCB saved frames; per-CPU current/CR3/TSS stacks, coarse acquire/release kernel lock, reference counts and acknowledged TLB invalidation replace one-CPU assumptions. Actual thread/channel/COW/device/service paths pass; missing remote acknowledgement and masked MSI-X controls fail. The ack control and its non-mutant counterpart contain identical wait-entry/completion diagnostics in copied sources: the timeout counts only after an actual remote wait starts. Earlier runs whose detector required a later affinity marker remain inconclusive, not successful causal controls. This prepares a delayed revisit and does not make the T48 successor SMP-safe.

Evidence selectors: guest:case25, smp:1-2-4, smp:tlb-ack-msix-controls.

Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.

