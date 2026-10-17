# Firmware lifecycle and recoverable updates laboratory

C17 freestanding-compatible core tested by a hosted synthetic-flash harness. F439 and H745 board profiles require their actual reference manual, silicon errata, linker map and board revision before a physical run. No flashing is performed by this batch.

Portable core controls are independent of required guest/board observations. Ordinary init copies compiling learner stubs, tests and contracts. Explicit `-DCMASTERY_ASSESSOR_ROOT=/absolute/repository/assessor` selects reference sources; never use that flag for learner evidence.

Real erase/program granularity, torn writes, cache maintenance, boot ROM interaction, watchdog reset and physical power-interruption evidence remain required on dedicated boards. Model success is not secure-boot certification.

See the canonical project briefs and `CONTRACTS.md`.
