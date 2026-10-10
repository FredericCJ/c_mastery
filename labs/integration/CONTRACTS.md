# Component handoff contracts

| Scenario | Actual functions reused | New boundary |
|---|---|---|
| P1 | `f_take`, `f_parse_i32`, `algo_ring_put` | One-byte length and signed-decimal payload; publish only complete valid records. |
| P2 | `algo_ring_take`, `f_write_all` | The active byte is owned by `InTransfer` across writer stalls. Accepted bytes are never replayed. |
| P3 | Earlier `in_records` | Bytewise input, a 31-byte payload ceiling, and an explicit terminal malformed-input state. |
| P4 | `fw_dma`, `algo_ring_put` | Transfer completion becomes a queue event only after matching-generation quiescence. |
| P5 | `f_cancel`, `f_complete`, `algo_ring_take` | `F_PENDING` means cancellation was requested; queue destruction waits for completion. |
| P6 | `f_checked_size`, `v4_filter`, `v4_measure` | Filter history survives chunk boundaries and output extent is checked before any mutation. |
| Linux | `cvm_compile`, `cvm_run`, `store_page_encode` | A bounded VM result crosses into a checksummed store-page format; corrupt bytes fail independent decoding. |
| MCU | `fw_adc`, `algo_ring_put/take`, `v4_filter`, `fw_watchdog` | Failed acquisition cannot publish a sample or falsely report progress. |

Inputs and component objects must remain live and exclusively owned during an
operation. Callers provide valid initialized components and truthful capacities.
For P2, queue values are bytes (0–255); its active transfer survives a failed
call. For P5, `completed` is a trustworthy adapter observation, not a guess
based on elapsed time. These synchronous tests do not establish concurrent
memory ordering, cancellation races or physical completion.

The Linux integration uses the original CVM expression syntax, for example
`(fn main () (* 6 7))`, with finite fuel and resource budgets. A page-codec check
does not establish durable storage; the parent capstone still requires its
transaction, crash-recovery, VM, process and dependency-closure campaigns.

For every handoff record artifact ID, source digest, contract revision,
ownership rule and accepted execution profile. A changed allocator, queue
capacity, thread context, completion rule, image format or board adapter
requires revalidating the affected consumer. Preserve unrelated evidence.
