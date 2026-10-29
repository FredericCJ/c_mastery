# Revalidate components at ABI and hardware boundaries

A portable algorithm can be reused after its contract is established, but the
consumer target changes integer widths, alignment, startup and helper dependencies.
Keep ISO C guarantees, compiler extensions, calling conventions, OS behavior and
hardware observations separate in the evidence record.

Move the bounded record and timer mechanisms through existing reset/startup,
interrupt and DMA laboratories. Inspect the linked image and undefined symbols.
The active freestanding policy forbids libc, libgcc/compiler-rt, libatomic, HAL and
OpenAMP dependencies in the target mechanism. Host oracles remain separate tools.
An apparently ordinary arithmetic or atomic expression can introduce a helper;
verify effective linkage rather than assuming a source spelling is freestanding.

Revisit ring ownership when a device writes memory. CPU release/acquire reasoning
alone does not establish cache maintenance, device access permissions or DMA
quiescence. A stale interrupt after restart needs generation and ownership checks.
Physical UART, CAN and Ethernet traces remain separate from an emulator or a
synthetic descriptor history. No route migration closes the existing board gates.

Graph worklists reappear in SMP scheduling; deadline structures reappear in an
original runtime. A measured maximum is not a WCET guarantee. Account for blocking,
interrupt interference and scheduler assumptions when making a timing claim.

Transfer changes the target ABI or places the buffer in a region with different
cache/device properties. Require an audit of every inherited assumption, an
explicit helper/dependency ledger and the appropriate host, image, emulator or
physical observation. Preserve an unavailable-hardware result honestly while
continuing unrelated hosted work.
