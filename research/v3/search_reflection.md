# OS, real-time and multicore source investigation

Date: 2026-10-10. This record separates discovery, inspected content and implementation evidence. No full copyrighted book or paper is redistributed.

## First search: actual holdings and existing curriculum

The read-only inventory searched `F:/Knowledge/Library` for OS, real-time, concurrency, scheduling and synchronization titles, then narrowed to Computing books and embedded-system books to avoid robotics trajectory-planning noise. Metadata was checked against actual retained PDF/EPUB filenames. The source graphs were searched independently: `F:/Projects/Writing/corpora/SWE/_spine_work/id_index.tsv` and `F:/Projects/Writing/corpora/datastructures/explorer/data/corpus.json`.

Verified holdings include Anderson/Dahlin second-edition OS volumes I-IV (PDF identities3a5a4d40,7dd05d6f,59bfd2c4,8d89be68), Buttazzo third edition2011(39ea4cd6), Wang second edition2023(1d04b0bd), TLPI2010(f68a4c3b), Stevens IPC volume2(7db27ed9), Mastering FreeRTOS guide1.1.0(c715acbc), Li/Yao(4c5cb281) and Real-Time Systems Design and Analysis fourth edition(7685b10b). The Linux Kernel holdingcb9a7486 is historical. Unassigned holdings remain retrieval leads, not claims of complete reading.

The SWE graph identifies Buttazzo2023 while the held book is2011third edition. Those are separate editions. Some library sidecars preserve old `F:/Cours` paths; actual `F:/Knowledge` payload paths and newly calculated SHA256 values are authoritative for this investigation. The datastructures graph identifies Michael-Scott queue but does not establish paper possession or correct reclamation.

Existing resource identities for Liu-Layland, POSIX2024, TLPI, Michael-Scott1996, LKMM6.12, FreeRTOS11.2.0 and OSTEP were reused rather than silently creating replacement editions. Existing T25/T26/T30 and the one-core T48 kernel established the gap: API familiarity and bounded single-core kernels do not supply actual SMP execution, original blocking synchronization or full task lifetime handling.

## Reflection before the second search

The most useful organization is mechanism first, policy second, then measured system behavior. The local OS concurrency volume directly supports task context, locks/conditions and multicore scheduling. Buttazzo supports precise task-model assumptions and overhead. These two books supply a stronger spine than a large list of uninspected titles.

Three boundaries required new primary evidence: local interrupt masking versus global exclusion; cache-incoherent M7/M4 AMP versus x86 SMP; and lock-free pointer manipulation versus safe lifetime/reclamation. Historical pseudocode cannot define modern C atomics. A QEMU parallel trace cannot certify physical speedup or MCU cache/DMA behavior. A middleware application note can explain architecture without authorizing middleware inside the original runtime.

## Second search: primary documents and bounded inspection

Inspected actual PDF text from OSPP selected TCB/context-switch, lock/condition definition, uniprocessor/multiprocessor exclusion and scheduling passages. Inspected Buttazzo RM/EDF/DM/processor-demand introductions, resource-model assumptions and kernel-overhead argument. Exact selected locators, local payload hashes and limits are in `resources.json`.

Official ST AN5617revision2(January2026) supplies shared-memory and notification architecture; AN4839revision2(March2018) supplies cache-line and DMA-coherency reasoning. Arm DAI0298A printedpp.4 and15 supplies FP context responsibility and PendSV context. These are bounded supporting documents; exact RCC/HSEM/APIC/MADT register or table coding still requires the target authors' separate architecture receipts.

Primary synchronization material includes Dijkstra EWD123, corrected Michael-Scott author pseudocode, Mellor-Crummey/Scott lock pseudocode, Herlihy/Wing FIFO histories, Michael hazard ownership/retire-scan methodology and Sha/Rajkumar/Lehoczky inversion assumptions. Linux EEVDF and SCHED_DEADLINE documentation was inspected with an access date; execution records must still identify the actual kernel. FreeRTOS POSIX and CM4F sources are separate comparison authorities.

Some web PDF opens timed out. The Arm, Herlihy/Wing, Michael and Sha papers were subsequently retrieved into memory with Python and read through pdftotext. No timeout was recorded as content inspection. No claim of an entire source audit follows from reading a table of contents or an abstract.

## Authoring consequences

T49-T56 add92 distinct 4-12-hour milestones and276 assessable objectives. Each milestone has a specialized invariant, implementation task, failure campaign, observable evidence and delayed transfer. The default custom-image boundary excludes libc/libgcc/compiler-rt/libatomic/HAL/OpenAMP and existing kernels; original helpers and reviewed compiler headers/intrinsics are explicit. Linux/FreeRTOS comparisons remain separate builds.

Every project carries a finite independently recomputable example. The examples cover scheduling, quotas, page replacement, masks/frames, wait arbitration, permits, inheritance, barrier generations, capacity, cache spans, epochs, histories, refcounts and service lifetimes. Passing these examples does not execute a learner implementation. Machine gates are supplied by the family manifests, and physical evidence remains not-run until a real capture with build identity is imported.

The project graph uses explicit stage-level prerequisites. Early SMP boot does not wait for the entire old VM/store capstone; the VM/store and physical AMP prerequisites attach to the later projects that consume them. No previous objective or project identity is overwritten.
