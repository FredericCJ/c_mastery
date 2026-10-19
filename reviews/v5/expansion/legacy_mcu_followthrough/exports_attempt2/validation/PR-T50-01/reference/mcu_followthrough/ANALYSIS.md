# T50 private assessor derivations

Completed reference work is never exported as a learner solution. Each record below addresses one exact criterion and its original changed condition. Models,source inspection,native execution and actual emulation have separate scopes.

## AX-PR-T50-01-T50.01.a

The small fixture provides reset/main/memcpy but not __aeabi_uidiv; its library list is empty. Thus one unresolved symbol is a dependency gap, not evidence that a forbidden archive was linked. Actual mcu.elf closes through explicit original objects. C17 freestanding alone establishes neither reset vectors nor Cortex-M alignment nor an executable entry.

Delayed: Compile dependencies.c for M0/M4,32/64-bit operations,O0/O2 and both compilers. Inspect undefined symbols before predicting the final link: M4 integer UDIV can be an instruction while64-bit division and64-bit atomics still request support. Never silently resolve the atomic fallback with libatomic.

Evidence selectors: direct-gcc-O2, direct-clang-O2, bad-lma-gcc, bad-bss-gcc, bad-helper-gcc, reject-forbidden-gcc, dependencies-gcc-cortex-m0-O2-symbols, dependencies-clang-cortex-m4-O2-symbols.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-01-T50.01.b

The raw flash binary avoids QEMU loading a NOLOAD RAM program header over the intentional poison. Reset copies .data and .data2 by LOADADDR, clears [__bss_start,__bss_end), preserves retained0x52544e44, then enters C. Target32-bit compare-exchange executes; the64-bit atomic inspection object is intentionally not linked. Original bounded division and explicitly imported byte functions close permitted calls.

Delayed: Move .data2 from0x20010000 to0x20020000 without changing its flash load bytes. The retained word remains unchanged. A copied old address would fail the moved marker even if ordinary .data and BSS passed.

Evidence selectors: direct-gcc-O2, direct-clang-O2, bad-lma-gcc, bad-bss-gcc, bad-helper-gcc, reject-forbidden-gcc, dependencies-gcc-cortex-m0-O2-symbols, dependencies-clang-cortex-m4-O2-symbols.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-01-T50.01.c

forbidden.c requests puts and its freestanding link is rejected. MF_BAD_LMA shifts only the first copy source and fails tag1. MF_BAD_DIVISION returns0 for UINT32_MAX/1 and fails tag5. MF_BAD_BSS fails tag3. The repaired profile has the same fixtures; no expected result is weakened.

Delayed: Repeat with the other compiler and O0/O2; retain each object symbol table. An optimizer eliminating a call changes the dependency observation, not the language contract. The final map must still contain no libc/libgcc/compiler-rt/libatomic/startup archive.

Evidence selectors: direct-gcc-O2, direct-clang-O2, bad-lma-gcc, bad-bss-gcc, bad-helper-gcc, reject-forbidden-gcc, dependencies-gcc-cortex-m0-O2-symbols, dependencies-clang-cortex-m4-O2-symbols.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-02-T50.02.a

MfHandle=((uint64_t)generation<<5)|(slot+1). Lookup rejects zero, slot fields17..31, high bits above37, UNUSED state and generation mismatch. Two-step admission budget0/1 leaves the previous kernel and output unchanged; budget1 rolls back the raw cmx reservation before any stack write. The capacity8 arithmetic fixture remains3,7,7,4,6, separate from the16-slot implementation.

Delayed: Startup-only creation needs no running-task race; dynamic creation must hold the same saved-PRIMASK domain as dispatch/wait publication. Integrated recovery executes dynamic admission only after the old task has exited, been joined, and physically switched away.

Evidence selectors: core-gcc-O2, core-clang-O2, spill-gcc, direct-fp-clang-O0, integrated-gcc-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-02-T50.02.b

At savedSP the8 integer software words precede EXC_RETURN at8, then r0..r3/r12 at9..13, LR14,PC15,xPSR16. xPSR.T=1 and EXC_RETURN=0xfffffffd. Caller words>=128; base mod8=0; usable bytes round down to a multiple of8. Tests exhaust15 applications, preserve refusals and retire UINT32_MAX generations.

Delayed: 129 words provide128 usable words; word128 is excluded. savedSP is(top-17 words), hence mod8=4, while the hardware boundary/top is8-aligned. Do not round the top upward outside caller-owned storage.

Evidence selectors: core-gcc-O2, core-clang-O2, spill-gcc, direct-fp-clang-O0, integrated-gcc-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-02-T50.02.c

Direct tasks reserve3*768*4=9216 bytes. Preemptive tasks reserve4*1024*4=16384 plus idle1024; integrated tasks reserve5*768*4=15360 plus idle1024. Linker reserves8192 bytes above static data for MSP. .su files describe compiled C frames, not assembly or complete call chains. Read exact image symbols and serial watermarks rather than copying pilot numbers.

Delayed: MF_SPILL grows each recursive local array from19 to67 volatile words, with depths1/2/3. New .su/disassembly and actual high-water records supersede the old measured path. Analytical extra live storage is at least48*4 bytes per active nested invocation, plus compiler-dependent spill changes.

Evidence selectors: core-gcc-O2, core-clang-O2, spill-gcc, direct-fp-clang-O0, integrated-gcc-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-03-T50.03.a

direct.S saves r4..r11/LR=36 bytes and a4-byte pad; it changes SP and returns through LR. Its40-byte frame has no hardware words. The frozen stage1 calls cmx_port_yield, whose PendSV path receives a hardware frame, saves nine software words and exception-returns. The fixture36 rounded40 is arithmetic for its chosen model, not a universal PendSV offset.

Delayed: FP direct context adds64 bytes for s16..s31 and stores the selected per-task FPSCR in the existing pad, totaling104. AAPCS permits volatile s0..s15 but asynchronous preemption does not. The selected r9 role is ordinary preserved v6; another platform reservation needs a new ledger.

Evidence selectors: direct-gcc-O0, direct-clang-O2, direct-fp-gcc-O2, direct-r8-gcc, direct-return-clang, retained-stage1.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-03-T50.03.b

Three tasks execute7/8/9 probe visits at different recursive depths. Initial r4 carries task ID,r5 entry,LR mf_direct_enter. The thunk calls entry and then mf_direct_return, which marks completion and yields permanently. The main context owns a fourth saved SP and resumes only unfinished tasks. Every C-call boundary stays8-aligned.

Delayed: GCC/Clang O0/O2 and explicit frame-pointer profiles retain the public ABI while changing generated frames. The third deeper task and MF_SPILL transfer require new high-water records, not an assumption that the first two stacks bound every path.

Evidence selectors: direct-gcc-O0, direct-clang-O2, direct-fp-gcc-O2, direct-r8-gcc, direct-return-clang, retained-stage1.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-03-T50.03.c

MF_BAD_REGISTER zeroes restored r8 and the direct assembly probe fails tag44(0x2c). The repaired path restores all8 integer words. The retained stage1 campaign is explicitly labeled exception-mediated and never serves as the direct-switch proof. A stale frame after stack reuse is outside the lifetime contract; handle rejection is tested without executing C through expired storage.

Delayed: MF_BAD_RETURN executes a defined UDF when a task entry returns, giving a target fault signature instead of the expected controlled exit. This is a separate image from the r8 mutant and diagnoses the return-policy boundary.

Evidence selectors: direct-gcc-O0, direct-clang-O2, direct-fp-gcc-O2, direct-r8-gcc, direct-return-clang, retained-stage1.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-04-T50.04.a

The fixture trace is running,blocked,ready,running,exited,free. Mf scans actual cmx states, so there is no separate mutable ready-list node to duplicate. It first converts a prior RUNNING task to READY, skips idle during the ordinary scan, then chooses idle only if no candidate exists. Saved-stack execution ownership is separate from logical current.

Delayed: Replacing a ready queue with a scan removes list membership but not state ownership or serialization. variants.c checks both ascending and reverse equal-priority scans, giving1,2 versus2,1; reverse policy changes service order while preserving membership and idle exclusion.

Evidence selectors: core-gcc-O2, core-clang-O2, variant-gcc-O2, variant-clang-O0, preemptive-gcc-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-04-T50.04.b

variants.c enumerates6^5=7776 histories and38880 post-operation checks using the actual Mf/cmx provider. cmx_exit refuses WAITING state; cmx_cancel removes an existing wait once. Repeated cancellation refuses without changes. Reap requires EXITED and a different current/executing context. After all ordinary tasks block or exit, dispatch selects permanent idle.

Delayed: The fixed-priority test adds a more urgent ordinary task and checks selection while retaining the same lifecycle invariant. Equal-priority scan reversal is tested separately; priority selection cannot make an EXITED or WAITING task eligible.

Evidence selectors: core-gcc-O2, core-clang-O2, variant-gcc-O2, variant-clang-O0, preemptive-gcc-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-04-T50.04.c

All six permutations yield CMX_OK,CMX_CLOSED or CMX_TIMEOUT according to the first event. Later events cannot change task.result or add a wait node. Service accounting follows RUNNING intervals; ready time is waiting-for-CPU and WAITING time is blocked, not execution. Re-entering take after a successful wake can consume a second token or block again.

Delayed: mf_take captures the task ID before suspension, enrolls under PRIMASK, pends, restores the incoming mask, then returns that task result after actual exception-mediated resumption. The target first task blocks and second gives; handler,PRIMASK and BASEPRI attempts refuse before enrollment.

Evidence selectors: core-gcc-O2, core-clang-O2, variant-gcc-O2, variant-clang-O0, preemptive-gcc-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-05-T50.05.a

The implementation accepts at-now in[0,2^31), poll delta in[0,2^31), and defines due by(now-at)<2^31. Wrap tests cross UINT32_MAX with exact/just-before cases. At the half-range both directions are ambiguous and outside valid ordering. All pending deadlines must remain within the maintained observation horizon; skipping an unseen full wrap invalidates the premise.

Delayed: mf_due64 uses the same proof with2^63. Width postpones ambiguity; it does not eliminate it. Explicit64-bit wrap/half-range checks remain separate from the host32-bit timer queue.

Evidence selectors: core-gcc-O2, core-clang-O2, variant-gcc-O2, variant-clang-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-05-T50.05.b

Polling chooses oldest due time then insertion sequence, emits the old ticket and increments its slot generation; UINT32_MAX retires. A canceled ticket cannot cancel its replacement. Full16 and UINT64_MAX sequence cases preserve earlier entries. The comparison transfer implements both sorted array and binary heap, with independent selection oracle over128 histories/2048 expiries and comparison/move counters.

Delayed: At the same logical instant, resource give and timeout are serialized. Give-first delivers OK and detaches the deadline; timeout-first delivers TIMEOUT and a later give leaves a token. The six actual cmx permutations include both. Reverse equal-deadline ordering changes only identity order, not due membership.

Evidence selectors: core-gcc-O2, core-clang-O2, variant-gcc-O2, variant-clang-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-05-T50.05.c

For work3,6,2, absolute releases are10,20,30 while relative releases are10,23,39. From phase30,period10,now57 the skip policy returns60 and skipped2. mf_periodic uses bounded64-step restoring division, so a large number of missed releases does not produce an unbounded catch-up loop.

Delayed: For burst work3,27,1,absolute requested phases remain10,20,30; completion of the27-unit job occurs47, so skip selects50 and counts two missed releases30,40. Start delay/jitter comes from actual dispatch, while execution is completion-start; neither equals the period automatically.

Evidence selectors: core-gcc-O2, core-clang-O2, variant-gcc-O2, variant-clang-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-06-T50.06.a

Basic total is8 hardware words plus9 software=68 bytes. From an8-aligned preexception PSP, savedPSP mod8=4; the C chooser runs on the independent8-aligned MSP. Fixture alignment72 does not authorize inserting an extra PSP word that the restore routine never skips. EXC_RETURN0xfffffffd selects Thread/PSP/basic.

Delayed: For extended FP, software adds16 high words and hardware18 low/FPSCR/reserved words:204-byte payload, up to208 with architecture alignment. The selected actual Armv7-M frame has r0..xPSR at hardware words0..7 and low FP at8..25. The raw-frame pilot and live forced low-FP probes verify this interpretation; incorrect offset18 for the basic PC failed the diagnostic premise.

Evidence selectors: preemptive-gcc-O2, preemptive-clang-O0, preempt-fp-gcc-O2, long-isr-gcc, preempt-exc-gcc, preempt-r8-clang.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-06-T50.06.b

SysTick priority0x80 requests PendSV0xff. mf_async_probe loops until four ticks advance without any yield instruction, then checks all r4..r11 and, in FP profiles, every low/high FP word. MF_BAD_REGISTER fails r8 tag134; MF_BAD_EXC_RETURN violates the selected return stack and reaches a fault diagnostic. Neither is accepted merely for a build failure.

Delayed: MF_NESTED pends NMI from SysTick. NMI has a dedicated single outstanding payload/sequence record and invokes no scheduler API; SysTick consumes it only after NMI return. Its separately stacked MSP frame and FP use coexist with the suspended PSP frame.

Evidence selectors: preemptive-gcc-O2, preemptive-clang-O0, preempt-fp-gcc-O2, long-isr-gcc, preempt-exc-gcc, preempt-r8-clang.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-06-T50.06.c

The first task pends three times under PRIMASK; requests increase by3 while completed switches remain unchanged until unlock. Final serial counts distinguish ticks,requests,switches and nested events. The chosen experiment does not assume one context switch per request. Measurements use the emulated counter and instruction configuration, not a physical latency guarantee.

Delayed: MF_LONG_ISR adds a bounded300-iteration handler loop at the same SysTick priority. Scheduler policy remains unchanged; extra delay belongs to ISR interference. Compare its source/disassembly and counter records with the same compiler/optimization baseline.

Evidence selectors: preemptive-gcc-O2, preemptive-clang-O0, preempt-fp-gcc-O2, long-isr-gcc, preempt-exc-gcc, preempt-r8-clang.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-07-T50.07.a

The trace0→80→48→80→0 is executed on target. Request112 under80 leaves80; it must not weaken the existing threshold. With four implemented bits, values are multiples of16. mf_basepri validates representation before modifying output;3-bit48 is rejected while64 is admitted, and8-bit1 is legal.

Delayed: Changing priority-bit count changes valid encodings and readback assumptions. The native3/4/8-bit cases do not claim another physical NVIC implementation was measured; target settings are bound to the selected emulator configuration.

Evidence selectors: core-gcc-O2, core-clang-O2, variant-clang-O2, preemptive-gcc-O2, bad-mask-gcc, log-mask-clang, uart-borrowed-clang.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-07-T50.07.b

Integrated SysTick publishes bounded batches into a4-sample queue under the same PRIMASK domain as consumers; UART IRQ drains at most4 bytes per entry. The seven finite arrival-boundary cases preserve one accepted event. Handler/PRIMASK/BASEPRI mf_take attempts return INVALID before waiter enrollment. NMI uses a distinct mailbox and cannot call the scheduler.

Delayed: The nested-reservation counterexample lets A and B both reserve head0; B publishes22, then A overwrites11 and publishes head1, losing B. Volatile or release/acquire cannot repair duplicate reservation. Serialize producers, allocate separate mailboxes, or use a genuinely multiproducer algorithm.

Evidence selectors: core-gcc-O2, core-clang-O2, variant-clang-O2, preemptive-gcc-O2, bad-mask-gcc, log-mask-clang, uart-borrowed-clang.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-07-T50.07.c

The target records SysTick down-counter read overhead and the23-iteration masked body. MF_BAD_MASK forces inner exit to zero and fails tag160 while the outer80 mask should remain. ISR blocking is independently refused. This port uses finite16-slot scans and no allocator or blocking call inside its mask domain; memory/bus/interrupt timing still needs a target-specific bound.

Delayed: MF_LOG_MASK adds text/hex formatting and a semihost call inside the measured body. Its new call graph and measured count supersede the original interval; semihost transport cost and host delays are not board UART timing. Keep the logging variant separate from the normal policy.

Evidence selectors: core-gcc-O2, core-clang-O2, variant-clang-O2, preemptive-gcc-O2, bad-mask-gcc, log-mask-clang, uart-borrowed-clang.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-08-T50.08.a

FPCCR.ASPEN=1; LSPEN chooses lazy/eager. The initial integer frame uses basic EXC_RETURN. On an FP task PendSV tests bit4 and saves/restores s16..s31. Hardware owns s0..s15/FPSCR.204-byte payload is51 words, with architecture padding considered separately; the same number does not describe direct104-byte frames.

Delayed: Eager profiles show LSPACT0 and the first low word already written. Lazy profiles show LSPACT1, first word still0xa5a5a5a5, then actual first FP instruction writes it and clears LSPACT. These are explicit observations of the selected QEMU model, not timing equivalence across boards.

Evidence selectors: preempt-fp-gcc-O0, preempt-fp-gcc-O2, preempt-fp-clang-O0, preempt-fp-clang-O2, preempt-s31-gcc, direct-fp-gcc-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-08-T50.08.b

The asynchronous probe fills s0..s15 and s16..s31; the integer fault/supervisor paths coexist. MF_BAD_HIGH corrupts s31 and must fail its named tag155. The per-task rounding test adds1.0 and2^-24: nearest-even produces0x3f800000; round-up produces0x3f800001. FPSCR is captured before the calculation changes cumulative flags.

Delayed: Direct context explicitly saves/restores full FPSCR despite AAPCS allowing some status bits to vary. Asynchronous context restores it from hardware. A shared rounding policy would require deliberate normalization at every boundary and different expectations; copying only high registers is insufficient.

Evidence selectors: preempt-fp-gcc-O0, preempt-fp-gcc-O2, preempt-fp-clang-O0, preempt-fp-clang-O2, preempt-s31-gcc, direct-fp-gcc-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-08-T50.08.c

The actual first low-FP word changes from paint to a task sentinel in lazy mode; eager mode starts with that sentinel. The ledger therefore reserves the full104-byte hardware extended frame even before it is written. Each selected nested handler can add its own hardware frame plus compiled C/assembly usage on MSP while the task frame remains live on PSP.

Delayed: MF_NESTED adds NMI while SysTick is active and uses FP there. Recompute simultaneous PSP extended frame plus SysTick/NMI MSP frames and .su call chains. The8KiB linker reserve is a selected finite budget; observed minimum MSP alone does not prove all unexecuted fault paths fit.

Evidence selectors: preempt-fp-gcc-O0, preempt-fp-gcc-O2, preempt-fp-clang-O0, preempt-fp-clang-O2, preempt-s31-gcc, direct-fp-gcc-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-09-T50.09.a

Mf tracks executing independently from kernel.current. mf_reap refuses the executing slot and also respects cmx current checks. target return exits then pends and never resumes ordinary work. The fixture reference counts2,1,0 are a lifetime analogy only: a zero count cannot change the hardware SP.

Delayed: Detached tasks reject joins; a supervisor owns final reap after EXITED and switch-away. Host lifecycle tests exercise detached reuse. The actual integrated supervisor joins the failed task, reaps from another PSP, and reuses its stack with a new generation.

Evidence selectors: core-gcc-O2, core-clang-O2, integrated-gcc-O2, preemptive-clang-O0, owner-fault-gcc.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-09-T50.09.b

A canceled joiner relinquishes target.joiner; another waiter can then own it. mf_join_result requires EXITED, matching joiner and successful stored completion, then sets consumed. A second delivery refuses. Executing-owner reap refuses even if the result was consumed. Native tests preserve output values on refusal.

Delayed: The supervisor fault policy is poison, not normal mutex release. The worked native two-mutex case closes both, repairs shared state under stopped-world ownership, removes ownership, then exits; peers receive CLOSED. The target unsafe-owner variant halts with both owners retained and does not invent a repair.

Evidence selectors: core-gcc-O2, core-clang-O2, integrated-gcc-O2, preemptive-clang-O0, owner-fault-gcc.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-09-T50.09.c

The lifecycle test observes result73 then replacement91 through different generations; the old handle becomes invalid. Generation UINT32_MAX cannot wrap: the slot remains retired EXITED and admission uses another free slot. Borrowed caller stacks remain allocated until successful reap, even when detached or faulted.

Delayed: Moving result payloads into caller-owned storage extends that storage through publication and exactly one consumer read. A pointer to an automatic entry-local object expires on return and is invalid; demonstrate the ownership trace without dereferencing expired C storage. The integer result API deliberately avoids that pointer premise.

Evidence selectors: core-gcc-O2, core-clang-O2, integrated-gcc-O2, preemptive-clang-O0, owner-fault-gcc.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-10-T50.10.a

The6 give/cancel/timeout orders preserve the first completion. Repeated mf_fault retains first code/PC and refuses a second transition. The five-phase pair mutation starts4+6, temporarily writes7, then3; cancellation before publication rolls back both, later cancellation preserves committed7+3. Cleanup occurs once and observers see only sum10.

Delayed: An arbitrary asynchronous kill between the two writes would expose7+6 if it unlocked. The changed safe-point policy delays cleanup until rollback or commit; cancellation request itself is not permission to release inconsistent resources.

Evidence selectors: core-gcc-O2, core-clang-O2, variant-gcc-O2, owner-fault-clang, mpu-gcc, canary-gcc, stack-bound-clang, preemptive-gcc-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-10-T50.10.b

UsageFault UDF records CFSR.UNDEFINSTR and stacked PC, calls mf_fault, redirects the otherwise unusable return PC to an abandoned loop, and pends dispatch. The supervisor observes one failed task, consumes -1, reaps elsewhere and peers continue. Handler-mode or stacking faults refuse local recovery; frozen cmx_fault remains whole-image termination.

Delayed: MF_OWNER_FAULT acquires two protected resources then faults. mf_fault records the cause, marks fatal and retains ownership; scheduling is refused and the image reports the unsafe-owner path. Only an explicit application repair/poison policy could permit peers to resume.

Evidence selectors: core-gcc-O2, core-clang-O2, variant-gcc-O2, owner-fault-clang, mpu-gcc, canary-gcc, stack-bound-clang, preemptive-gcc-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-10-T50.10.c

MF_BAD_CANARY changes the low stack word and fails tag106 on the next switch. MF_BAD_STACK_BOUND supplies a below-low saved address and fails tag105 before dereference. These are defensive checks, not memory isolation. The UDF target exercises actual exception entry and local termination. A blocked native fault cancels its wait before exit.

Delayed: MF_MPU_FAULT configures a32-byte aligned AP000/XN region with privileged default background, enables MemManage, then stores to it. Actual CFSR0x82 identifies DACCVIOL+valid address and the guard remains unchanged. This demonstrates a selected forbidden access, not all overflow patterns or physical-board isolation.

Evidence selectors: core-gcc-O2, core-clang-O2, variant-gcc-O2, owner-fault-clang, mpu-gcc, canary-gcc, stack-bound-clang, preemptive-gcc-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-11-T50.11.a

101*32768=3309568; ceiling over1000000 is4. Exact duration is4000000/32768 microseconds. A maximum3 yields3counts then conservative residual10us, which requires1count. Zero means no interval. Independent inequalities require(count-1)*1e6<us*hz<=count*1e6 for positive unsplit requests.

Delayed: SysTick periodic encoding uses LOAD=counts-1 with positive counts; the CMSDK one-shot adapter instead uses RELOAD0,VALUE=count. The source manual single-shot wording has an inconsistent example, so the selected encoding is stated and executed rather than silently mixing timer models.

Evidence selectors: core-gcc-O2, core-clang-O2, sleep-gcc-O0, sleep-clang-O2, sleep-stopped-gcc.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-11-T50.11.b

The target uses CMSDK timer0 at25MHz as an independent elapsed source and timer1 as a one-shot wake. Boundaries0..5 inject pending SysTick around the critical sequence; boundary6 arms an early interrupt during WFI; boundary7 is full expiry. WFI with PRIMASK1 wakes on enabled pending work, then the saved mask is restored. The second finish refuses without adding elapsed time.

Delayed: MF_STOP_CLOCK disables the elapsed source before WFI and fails the early-wake elapsed check. A timer that stops in sleep cannot measure that interval. An always-on RTC or externally clocked timer needs a new address/frequency/clock-gating contract; this emulator experiment makes no deep-sleep energy claim.

Evidence selectors: core-gcc-O2, core-clang-O2, sleep-gcc-O0, sleep-clang-O2, sleep-stopped-gcc.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-11-T50.11.c

Early wake leaves positive remaining counts; full expiry has elapsed>=requested and zero remaining. A65000-count wait splits into at most25000-count pieces and finishes in three arms, accounting observed overhead once per arm. The pure MfSleep model rejects overshoot by its narrower model contract; the actual MfNap adapter records it rather than silently discarding time.

Delayed: Conversion examples use32768Hz, while actual CMSDK hardware uses25MHz and SysTick early wake selects the CPU clock. Changing source frequency requires new calibration/rounding/error bounds. Physical evidence would need board clock configuration,oscillator uncertainty,wake trace and current/energy measurements; none is inferred here.

Evidence selectors: core-gcc-O2, core-clang-O2, sleep-gcc-O0, sleep-clang-O2, sleep-stopped-gcc.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-12-T50.12.a

Periodic input emits one sample every2 ticks through30, with a12-sample burst at6. Full input/pool rejects; full output drops the processed result; full diagnostics increments a loss counter. Every attempted ID ends rejected,dropped or delivered. A CPU hog remains preemptible while a distinct worker faults. Deferred sink starts at18, making output pressure overlap recovery.

Delayed: MF_BORROWED replaces copied values with stable generation-tagged pool leases and prevents reuse until processing completes. This is an executed software borrowed-buffer variant, not physical DMA. The separate abandon/completion traces require device completion or acknowledged reset before release; a timeout/cancel request alone is insufficient.

Evidence selectors: integrated-gcc-O2, integrated-clang-O0, uart-borrowed-gcc, uart-borrowed-clang, integrated-m7-gcc, integrated-m7-clang, variant-gcc-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-12-T50.12.b

Supervisor joins/reaps the failed slot from another PSP, dynamically admits a replacement in the same caller stack and checks a new generation. Conservation:attempted=accepted+rejected;accepted=processed;processed=delivered+output_drop;computed checksum=consumed+dropped checksum. Fixture RAM14848/free1536 and cycles240 are independent arithmetic. Actual static sizes and compiler .su/map records are different evidence.

Delayed: Diagnostic capacity2 remains full during the recovery window and increments loss without blocking; the later sink drains bounded entries. Actual UART transfer drains at most4 peripheral bytes per IRQ, receives64 host-injected bytes through QEMU CMSDK, and retains the same conservation/failure rules. No board UART timing or loss guarantee follows.

Evidence selectors: integrated-gcc-O2, integrated-clang-O0, uart-borrowed-gcc, uart-borrowed-clang, integrated-m7-gcc, integrated-m7-clang, variant-gcc-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

## AX-PR-T50-12-T50.12.c

The campaign binds native,model and actual emulator records to exact sources and binaries. Public stubs remain incomplete and isolated exports must fail until explicit completed providers are supplied. A seven-day transfer is authored, not administered. The negative images establish detection sensitivity only for their named defects, not absolute mastery.

Delayed: The M7/AN500 transfer changes selected CPU and FPU flags to Cortex-M7/fpv5-d16 while retaining softfp AAPCS and the common modeled timer map. The executed instruction subset and exception frame still match; board SRAM/cache/TCM/interrupt routing and physical timing require their own revalidation. Emulator caches are not a board coherence claim.

Evidence selectors: integrated-gcc-O2, integrated-clang-O0, uart-borrowed-gcc, uart-borrowed-clang, integrated-m7-gcc, integrated-m7-clang, variant-gcc-O2.

Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.

