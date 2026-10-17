# B04 inspection and depth decisions

Actual passages were read on 2026-10-17: RFC1982 sections3.1,3.2,5;
RFC9112 sections5.2,6.1,6.2,6.3; Linux DMA API HOWTO coherent-mapping ordering
warning and cache-line-sharing constraints. Registers bind fetched document bytes
without copying complete works into publication. Only these passages support
the external claims; bibliography and module reading lists were inspected as
metadata, not silently upgraded to source reading.

RFC1982 deliberately leaves half-range pairs unordered and limits serial addition.
RFC9112 distinguishes framing precedence and connection handling; the new grammar
is explicitly narrower and uses strict CL+TE rejection. Linux coherent mappings
still require ordering; noncoherent cache-line sharing can affect neighboring
objects. Neither Linux API text nor a host state machine establishes STM32 bus,
register or electrical behavior.

Substantively inspected existing mechanisms:

- `labs/algorithms/tests/test_algo.c` alu_test/cache_test: exhaustive four-bit
  arithmetic flags, compulsory/conflict/capacity trace discrimination and empty
  states. Retain these actual mechanisms; no claim that arithmetic or cache
  simulation was wholly absent. Pipeline/control timing still needs a separate
  deeper review and experimental extension.
- `labs/communications/tests/http.c`: all input split points, CL+TE and duplicate
  CL rejection, chunks/extensions/trailers, output pressure and deadline cases.
  Retain the accepted incremental parser; X-T42-001 isolates disagreement and
  transactional normalization, not a replacement parser or proof of complete HTTP.
- `labs/communications/tests/mcu.c` spi/dma: bounded transactions, stuck-mode
  timeout, transfer failure and unchanged read output, explicit DMA barriers,
  receive fragmentation/error/CRC-length controls. Preserve this executable
  evidence. Physical transceiver/cache/board outcomes are separate.
- `labs/runtime/include/cvm.h`, `labs/storage/include/store.h` were also read
  for cross-batch integration contracts, not reassigned as B04 coverage.

The full306-leaf structural inventory is in `b04_depth_audit.json`. Its unreviewed
leaves are explicitly unreviewed, not accepted merely because a unit exists.
The first five new exercises assess10 bounded claims. Broader parent objectives,
pipeline scheduling, serial electrical constraints, original protocol integration,
performance uncertainty and causal debugging remain active expansion work.

No newly inspected published C17 text, complete book corpus, reviewer independence,
physical target run or learner assessment is claimed. The C17 baseline remains;
original contracts separate C object validity, checked arithmetic, API policy,
Linux guidance, native ISA behavior and board obligations.
