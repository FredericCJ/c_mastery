# Reflection between broad and focused searches

The broad search establishes a useful literature spine, but the reference graph and document catalog answer different questions. The graph connects ideas and historical works; it does not establish local possession or full-text suitability. The catalog establishes retained instances and provenance, but its search is metadata-only and its enrichment is limited. An empty query is not evidence that a publication is absent.

## Questions raised by the first pass

1. Which edition and manifestation will each reading assignment use? The held CS:APP third edition is the global edition. Modern C exists in several retained editions. Reading locators must be checked against the actual selected file rather than copied from another syllabus.
2. Which text explains a mechanism, and which document establishes its contract? Foundational C books, allocator examples and concurrency papers remain educationally valuable, but old implementation assumptions are not current ISO guarantees. Selected K&R and Hanson examples need critique rather than unqualified imitation.
3. Where do source boundaries change? C atomics, POSIX synchronization, kernel memory barriers and MCU device ordering describe different environments. Their explanations must meet at explicit boundaries, not a single undifferentiated concurrency lesson.
4. What does successful verification establish? A compiler experiment demonstrates one configuration. Frama-C proof obligations, bounded model checking and source audits need their assumptions and exclusions. A success count does not establish all runtime obligations or physical behavior.
5. Can the hardware exercises use actual checked manuals? The STM32F439ZI has device-specific documents and errata. The STM32 reference manual, board user manual and core programming manual must be treated separately. Direct ST downloads sometimes fail; an ST-hosted archived English revision is a valid pinned source but cannot silently stand in for a newer catalog revision.
6. How much reading is necessary before small projects? Assign mechanism-sized passages and give original worked reasoning. A bibliography of large books is not a study sequence. Deeper historical reading and uninspected acquisitions remain separate extensions.

## Consequences for the focused search

Follow citation trails for language history, information hiding, correctness, allocators, UNIX and memory-model foundations. Search official release archives for startup source and manuals when live documentation is unavailable. Check exact compiler/library support separately. Inspect numerical error, scheduling and formal-verification passages before promising their teaching depth. Resolve board and peripheral documentation through attributable vendor copies and record the inspected revision.

The focused pass must close essential support gaps before module authorship. Sources found only through bibliographic or contents records cannot supply required passage assignments. Nonblocking acquisition leads remain disclosed, and no whole-book inspection claim is inferred from a bounded passage review.
