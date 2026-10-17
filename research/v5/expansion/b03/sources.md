# B03 passage inspection and reflection

On 2026-10-17 N1570's registered bounded thread/atomic/qualifier passages were read via primary public PDF and in-memory pdftotext. The source is a C11 draft; exact C17 publication remains an access gap. The release/acquire argument requires a reads-from edge and a reverse handoff before ordinary payload reuse. Merely observing no failures on x86 is insufficient. This authoring does not resolve consume/fence edition changes beyond the inspected passages.

The current Open Group write page returned403. Linux man-pages write(2) DESCRIPTION, RETURN VALUE, selected ERRORS and NOTES were actually read. Its partial-progress and interruption rules inform the synthetic callback protocol, whose -1/-2 values are deliberately not errno constants. Direct-I/O failures and durability need different contracts; they are not generalized from the local byte-stream model.

Existing PR-T23-02, PR-T24-04, PR-T25-03, PR-T28-01 and PR-T30-03 were read before authoring. They already state important distinctions. This wave adds precise APIs, state bounds, original failing candidates, source-bound tests and objective rubrics. It does not claim to execute actual OS cancellation or any peripheral. B3Device is a fully invented teaching device; its rules are defined by the lab contract, not borrowed from an unread board manual. The tick model distinguishes caller sampling assumptions from detectable modular ambiguity.

Missing evidence remains explicit: exact C17/POSIX text, weak-architecture concurrency runs, physical MMIO/interrupt timing, real cancellation API integration, and other B03 domain mechanisms. Models are executable analysis artifacts; passing them is not physical validation or mastery.
