# Follow-through source inspection

2026-10-17: actually read N1570 6.5p6–7 (effective type/access), 6.5.6p8
(array-relative pointer arithmetic) and 6.2.5p9 (unsigned arithmetic) from the
[WG14 primary draft](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf).
These support the bounded type/range arguments, not a claim to inspect published
C17. The typed transfer adapter reserves a declared `FDValue`; it explicitly
changes the earlier no-declared-type allocation premise. Its slot control logic
does not create a general-purpose object-lifetime system.

The [DR452 issue](https://www.open-std.org/jtc1/sc22/wg14/issues/c11c17/issue0452.html)
was reopened. Its historical proposed discussion is not used as a new rule for
the transfer reference. The existing registered final C17 correction remains
separate. Opening the public N2176 PDF through the web tool returned an internal
error; this failed attempt is not source inspection or evidence of absence.

The original algorithms, fixtures and state machines in the transfer package
are curriculum constructions. Existing parent source IDs remain unchanged.
No inaccessible standard, library index, search hit or generated coverage row
has been relabelled an inspected source. No new taxonomy leaf is needed for
these existing changed-condition competencies.

## Glossary authority reconciliation, 2026-10-17

The five `*vocabulary_corrections.json` records preserve each original pending
authority and its replacement. These are bounded author inspections, not a
retroactive enlargement of an accepted inspection or independent review. The
corresponding resource registers give the exact locators and retained hashes.
Definitions were checked against body passages, not inferred from book indexes.
Private extraction files are excluded from learner/source distribution.

### Language and implementation boundaries

The retained [N3096](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3096.pdf)
PDF is the April 2023 prepublication draft. Actual readings cover the individual
definitions and paragraphs enumerated in `vocabulary_resources.json`: translation,
execution, concurrency, scope/linkage, representations, conversions, expressions,
declarators, initialization, atomics, checked arithmetic and Annex C. This is not
inspection of published C17 or C23. Corrections distinguish C release sequences
from the C++ rule, representation comparison from value equality in compare/exchange,
and edition-specific compound-literal, prototype and initialization rules.

[GCC15.2 overflow builtins](https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/Integer-Overflow-Builtins.html)
were read for their infinite-precision intermediate and conversion/result contract.
The transfer experiment stages the result before publishing it; a failed builtin
call alone does not promise unchanged caller storage. The six-profile and C23
comparisons are author execution, separate from source inspection.

[N2676](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2676.pdf), dated
2021-03-24, was read at the Introduction opening and §§1.2–1.3, printed pp.5–7.
It is a draft technical specification proposing a provenance-aware memory model.
The storage-instance and abstract-address explanation does not imply that the
runtime stores provenance bits, nor that all compilers implement this model.
The retained PDF hash is in `memory_vocabulary_resources.json`. The web reader
timed out on later opens; a successful direct download and actual private text
reading supplied the stated inspection. No uninspected later clause is claimed.

### Runtime, scheduling and hardware

FreeRTOS11.2.0 `stack_macros.h`, the registered portions of `tasks.c`, and the
yield/delay/high-water/hook declarations in `task.h` were actually read. Exact
line bounds are in `runtime_vocabulary_resources.json`. The hook is
`vApplicationStackOverflowHook`; a historical invented setter was corrected.
Fill-pattern scanning reports a bounded observation in stack words, not a proof
of the greatest possible stack demand. Cooperative scheduling does not eliminate
interrupt or multicore interference.

[Liu and Layland1973](https://bears.ece.ucsb.edu/class/ece253/papers/laylandliu77.pdf)
§§2–4, printed pp.47–49, were read, including assumptions A1–A5 and the
critical-instant argument. Release, response and deadline definitions are tied
to that task model; arbitrary blocking, release jitter and deadlines different
from periods require a different analysis. No measured maximum is relabelled a
worst-case guarantee.

The retained Armv7-M manual is **DDI0403E.e ID021621 (2021)**. Readings include
B1.5.1–4, the named late-arrival/tail-chaining prose in B1.5.11–12,
B1.5.17–19 power/event behavior, B3.3.1–5 SysTick, B3.4.1 NVIC, and the
specified fault-status/valid-address fields in B3.2.14–18. Exact page bounds
are in `hardware_vocabulary_resources.json`; not every UsageFault field or
exception pseudocode path was read. PendSV priority is configurable, HardFault
has a fixed priority, and SysTick requests on the count-to-zero event. WFI and
WFE have distinct wake conditions; software must allow for spurious wakeup.

The retained **RM0090Rev21 (June2024)** readings cover §§2.1.11–2.3,
8.3.3–5,10.1–2,10.3.13–14,21.1–21.4.1 and33.6.7, at the precise pages
registered. In particular, stopping DMA requires the documented disabled-state
observation, not merely writing a stop request. The Ethernet ownership bit
governs a separate descriptor lifecycle. GPIO BSRR avoids an ODR read/modify/write
sequence but does not lock ODR against other agents. IWDG and WWDG have different
service conditions. This is manual inspection, not flashing, board execution or
timing evidence. The retained revision/hash takes precedence over any later
document returned by a live vendor URL.

### Hosted processes, synchronization and evidence vocabulary

The authorized local TLPI first-edition PDF was inspected at printed pages
95–97,418–419,447–449,515–519,541–545,553–554,639–640,889–892,
1093–1097,1108–1111,1117–1120,1124–1127 and1136–1137. Its physical PDF
page is printed page plus44. Only the used locators are registered; extraction
of a neighboring page does not establish inspection. Historical details were
checked against current named interface documentation where consequential.

Actual Linux man-pages6.19 readings were
[fork DESCRIPTION/RETURN](https://man7.org/linux/man-pages/man2/fork.2.html),
[subreaper DESCRIPTION/HISTORY](https://man7.org/linux/man-pages/man2/PR_SET_CHILD_SUBREAPER.2const.html),
[dup DESCRIPTION/NOTES](https://man7.org/linux/man-pages/man2/dup.2.html),
[semaphore overview DESCRIPTION](https://man7.org/linux/man-pages/man7/sem_overview.7.html)
and [sem_post ERRORS/NOTES](https://man7.org/linux/man-pages/man3/sem_post.3.html).
Corrections cover shared open-file descriptions, multithreaded fork, subreaper
adoption, atomic descriptor replacement, and the distinction between POSIX
signal safety and hardware-interrupt safety. Advisory locking families retain
their different release/inheritance contracts; a shared mapping does not
provide a synchronization protocol.

The official POSIX2024 `pthread_rwlock_rdlock` URL returned HTTP403. The actual
readings instead use the authorized **POSIX.1-2017 reproduction** for
[read locking](https://man7.org/linux/man-pages/man3/pthread_rwlock_rdlock.3p.html)
and [mutex priority protocols](https://man7.org/linux/man-pages/man3/pthread_mutexattr_getprotocol.3p.html),
DESCRIPTION and ERRORS. They are explicitly not relabelled POSIX2024. Reader
limits, admission policy, priority inheritance and ceilings do not imply an
unqualified fairness, performance or response-time guarantee.

[Michael–Scott1996](https://www.cs.rochester.edu/u/scott/papers/1996_PODC_queues.pdf)
Introduction, PDF pp.1–2, supplied the progress and retained-storage
counterexamples. Its historical use of “lock-free” for absence of locks must
be distinguished from the modern global progress definition. A bounded number
of queued elements is not automatically bounded memory or completion latency.

The claim-scope, working-unit, diagnostic-hypothesis and fixed-pool definitions
are **course operational contracts**. The named sections of
`curriculum/v4/workflow.md`, `curriculum/guides/assessment.md` and
`curriculum/projects/PR-P5-02.md` were read. They are not attributed to unrelated
TLPI/CSAPP passages. Historical unit hours do not limit required study depth.

### Memory hierarchy and mitigation

The authorized CSAPP Third Global Edition PDF was read at printed pp.315–316
(stack corruption),322–324 (guard detection),650–652 (cache organization),
849–851 (translation/fault path) and871–872 (private mapping/COW). These are
bounded passages, not whole-chapter inspections. Explicitly shared mappings
remain shared across fork; the private-copy explanation does not override that
contract. A page fault need not perform disk I/O or terminate a process.

The Linux kernel documentation's
[False Sharing](https://docs.kernel.org/kernel-hacking/false-sharing.html)
mechanism, detection and mitigation sections were actually read. A writer and
reader of independent data can interfere; two writers are not necessary.
Padding has footprint/cache/TLB costs. No current kernel release number is
inferred from an unversioned live page.

[GCC15.2 Instrumentation Options](https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/Instrumentation-Options.html)
was read for the four stack-protector selection options and adjacent
stack-check/stack-clash distinction. A guard check does not prevent the invalid
access or establish complete control-flow protection. Compiler-emitted guard
and failure dependencies require an actual image audit in freestanding work.

## Remaining source limitations

The published C17 and C23 texts were not obtained in this pass. N1570, N3096
and N2676 retain their exact identities and cannot discharge a claim about
having inspected those publications. The failed N2176 web open and unavailable
POSIX2024 page remain recorded access attempts. An authorized catalog search
found no directly usable published standard in the inspected results; this
does not establish that no legitimate copy exists. All new source gates are
same-author traceability, not external technical review.

## Actual service integration and comparison boundaries

The complete original `labs/service_depth/adapters/CONTRACTS.md`, public ABI,
guest probes and separate kernel/receipt references were authored and inspected.
Their source register is `service_adapter_resources.json`. The retained ELF,
frame, mapping, async block and transaction implementations are actual imported
components, not fresh nominal ports with equivalent names. Underlying ISA and
virtio normative claims retain their earlier bounded source records; no new
whole-specification inspection is claimed here.

The Linux man-pages6.19 `fsync(2)` DESCRIPTION/ERRORS and man-pages6.18 `_exit(2)`
DESCRIPTION/NOTES were actually read for the host comparison. File flush does
not by itself establish durability of containing directory entries, and device
completion/error contracts remain relevant. `_exit` closes descriptors without
flushing C streams; the Linux wrapper/raw-system-call process/thread distinction
was retained. These are Linux hosted contracts, not ISO C or CMK guest guarantees.
The guest uses its own explicit virtio flush path and synthetic image.

The comprehensive assessment clarification now lives in the additive
`curriculum/guides/comprehensive_v5.md`. The historical assessment guide was
restored to its original bytes so capstone source pins remain honest; the new
guide preserves its rubric and separately states comprehensive required scope,
unbounded study time and the six evidence dimensions.

## Request reset and actual device-error successor

Virtio1.2 official HTML identifies Committee Specification01,1 July2022.
Actual inspected passages are2.4.1/2.4.2 and4.1.4.3. Reset completion is observed
through device status; the device then ceases queue interaction until reinitialization.
This supports a queue-memory ownership premise, not a conclusion about an earlier
write's outcome. The new original request contract and corresponding model,
two-request adapter and guest probes were also inspected. Document hashes and
bounded claims are in request_adapter_resources.json; no whole-specification read
or physical validation is asserted.

The QEMU v10.2.1 tagged blkdebug documentation Rules/Events sections were read
for event, errno, sector and once matching. The installed QEMU identifies10.2.1;
the installed Linux asm-generic/errno-base.h defines EIO as5. A one-shot write
error at sector96 is confined to the original synthetic guest image. The earlier
master documentation identifies11.1.50 and is not relabeled as the installed
version; the exact tagged passage is the implementation authority.
