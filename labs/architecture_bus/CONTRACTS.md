# PR-T37-06: Exact paths, typed placement and DMA lease return

This required successor implements the three T37.08 criteria and original
seven-day variants. It reuses the selected T27/B06 linker recipe, the actual
T28 DMA state machine and the original typed objects from PR-T37-02. T37.03's
register/extent/object prerequisites retain their separately executed byte
package; this project adds their address/placement composition. The original
project, criteria and historical evidence are unchanged. Shared legacy and
`labs/peripheral_depth/dma/CONTRACTS.md` contracts apply explicitly.

## Sources, prerequisites and environment

Read the registered actual RM0090 Rev21 pp59–64,68 and1010 passages and bus
figures. The new source register is
`research/v5/expansion/legacy_architecture_bus/resources.json`. Know C lifetime,
character access to object representations, numeric bounds versus pointers,
linker sections and borrowed payloads. Bring the actual `PdTransfer` producer,
the B06 selected F439 `f439_service.ld` and the completed PR-T37-02 typed-object
map. Missing prior learner implementations remain required; assessor reference
source is selected separately and never exported as completed learner work.

The core is ISO C17 with fixed arrays and no allocator/library implementing
placement, arbitration or ownership. GNU section/linker attributes and the
Arm toolchain are explicit platform mechanisms. Host GCC/Clang/Python and
sanitizers are verification tools. Whole-image MPS2 checks run this original
model with callbacks; they do not execute F439 DMA, prove bus reachability or
observe physical completion. Actual linked F439 addresses are independently
checked in a separate audit-only image. No board is flashed or probed.

The map selector accepts documented F439 or changed F407 and manual revision21.
Unknown part/manual revision is rejected. This does **not** validate a real
silicon revision, package, board, clocks or boot pins. Those remain unobserved
and therefore exclude physical execution claims. Reuse the actual B06 target
qualification before any later separately authorized board run; no guess at a
REV_ID is introduced here. Main-flash/no-SRAM-remap data paths are selected.

## Concrete graph and placement operations

Implement `mb_map` and `mb_place`. `MBMap` describes a selected six-port
subgraph: CPU D-bus to CCM; CPU S-bus to ordinary SRAM/APB; DMA1 memory port to
ordinary SRAM and its dedicated peripheral port to APB1; DMA2 memory port to
ordinary SRAM and peripheral port to ordinary SRAM/APB1/APB2. The two APB
windows contain reserved holes and are graph nodes, not blanket register-access
permission. `mb_place` accepts only the four RAM-region nodes.

This subgraph deliberately does not turn unlisted ports into negative claims.
The full F405/407 figure has eight bus-matrix masters: CPU I/D/S, DMA1 memory,
DMA2 memory/peripheral, Ethernet and USB HS. F439 adds LCD-TFT and DMA2D.
DMA1's peripheral path is a dedicated APB1 path outside those matrix masters.
The CPU D-bus has the direct CCM path; code/remap paths through I/D buses and
external memory, Ethernet/USB/LCD/DMA2D traffic are outside this data-ledger
selection. Annotate those omissions/conditions when drawing the full source
figure, rather than inventing a universal six-master processor.

Both parts have SRAM1 `[0x20000000,0x2001c000)`112KiB, SRAM2
`[0x2001c000,0x20020000)`16KiB and CCM `[0x10000000,0x10010000)`64KiB.
Only F439 in this selection has SRAM3 `[0x20020000,0x20030000)`64KiB.
Ordinary DMA-reachable SRAM is192KiB versus128KiB; totals including CCM are
256KiB versus192KiB. Do not invent SRAM3 for the F407 changed task.

For an admitted region/master, require a nonzero whole declared allocation,
alignment a power of two1..32, aligned start, start inside the region and
`bytes <= region_bytes - (address - base)`. Check bounds before computing an
end or forming a pointer. One allocation may not cross the named bank, even
though the underlying `pd_validate` accepts the wider contiguous F439 aperture.
Unknown selectors return `MB_INVALID`, missing master connection returns
`MB_UNREACHABLE`, and out-of-region extent returns `MB_EXTENT`. The offset
output is preserved on rejection. Numeric admission cannot establish allocated
storage, lifetime, object type, actual master reachability or CPU ownership.

Replay the original CCM/SRAM1-crossing/SRAM3-valid/end-overrun candidates.
The valid candidate at0x20020010,64 bytes produces offset16. Check both parts,
every selected port, region endpoints andUINT32_MAX sizes using independent
unbounded integer interval arithmetic. Reject late-invalid admission before
publishing any caller state. Empty allocations are rejected, not modeled as DMA.

## Actual typed objects and map change

Reuse `ab_cpu_only[2]` and `ab_dma_candidate[2]` from the unchanged
`labs/architecture_bytes/placement.c`; they are actual `AWRecord` arrays, not
declared character storage cast to a struct. `ledger_objects.c` adds one16-byte
source array. Compile actual objects with the target toolchain. Both audit
linker scripts **include the unchanged B06 `f439_service.ld`** for its owned
code/RAM region; the extension only places these project objects.

The baseline places the CPU-local records at0x20000000, source at0x20000010 and
destination at0x20020000. The changed script moves only CPU-local records to
0x10000000; both DMA source and destination addresses/16-byte extents stay
identical. Inspect actual symbols/maps and derive `linked_map.h` from those
ELFs, then feed the observed numerical placements to the ledger. A deliberately
wrong script moves the destination to CCM and must be rejected by the map audit.
The audit entry loops and provides no real reset/init protocol; no claim of
bootable F439 firmware follows. Host execution separately establishes live
typed storage and character accesses, while target initialization/observation
remains a distinct obligation. The learner export's example header is visibly
an example; replace it with a header derived from your own real link.

## Compose the actual DMA producer

`MBLease` starts zero-initialized, remains at a stable address and is single
owner/non-reentrant. `mb_bind` borrows distinct live source/destination storage
of1..64 bytes for its enclosing lifetime; corresponding numeric identities and
bank names must be accurate under the target adapter. Host/emulator mocks use
logical map tags without casting those numbers to pointers. The source and
destination ranges must be actually disjoint as well as numerically disjoint;
C cannot validate a dishonest cross-object association. Never mutate public
control fields or access payload through another alias while lent.

Binding checks both complete single-bank extents, then calls actual
`pd_validate(PD_F439,0,...)`. The selected transfer uses equal width1 and no
cache. `mb_give` calls actual `pd_submit`, copying the extent and giving both
payload leases to the producer. Callbacks obey the frozen bounded/non-reentrant
contract. `mb_poll` delegates to `pd_poll`; `mb_finish` delegates to
`pd_finalize`. `mb_cpu_write`/`mb_cpu_read` return BUSY in **every nonzero phase**,
including READY3. A ready result does not itself return the payload lease.

Concrete sequence: fill source, give generation1, reject foreground write and
read unchanged, observe EN=0/TC with FIFO nonempty and remain busy, reject a
wrong snapshot token, then observe matching token/EN0/FIFO empty/TC and reach
READY. Foreground access is still rejected. Only matching finalize returns
the lease; read the synthetic copied byte42, then write77. Repeated finalize
and reused token are stale. The original producer owns stop/cancellation,
uncertain progress and generation semantics; this package does not replace
them with an EN-bit boolean. A timeout cannot authorize reuse. The selected
callback copy is an original deterministic model, not physical DMA execution.

## Arbitration and APB changed conditions

`mb_arbitrate` is a declared model with three continuously pending masters
CPU,DMA2,DMA1 and a fixed nonempty active mask. One burst is nonpreemptible;
rotate in that order, skipping inactive masters. Admit1..8 rounds, positive
maximum burst1..8 beats and maximum wait0..8 cycles per beat. Every active
master's positive beat count and per-beat wait must fit those bounds. A burst
costs `beats*(1+wait)` model ticks. Emit each master/start/finish/wait.

For unit bursts/no waits, the original six grants repeat CPU,DMA2,DMA1 and
take six model ticks. For beats2/4/1 and waits0/1/2, costs are2/8/3; first
starts are0,2,10 and second-round CPU starts13. Its recurrent wait is11 ticks.
Since at most two other masters intervene, continuously pending wait is at most
`2*max_beats*(1+max_wait)`; a tighter bound is the sum of the other active
burst costs. This requires finite response per beat, no locked/unbounded burst,
fixed admitted masters, this arbitration point and no hidden bridge/clock delay.
RM0090's round-robin statement alone supplies none of those elapsed-time bounds.
If a beat can wait indefinitely, bounded grant counts do not bound waiting time.
This model is the exact changed task, not a claim about F439 WCET or throughput.

`mb_bridge` expands admitted8/16-bit data into the32-bit bridge vector and
passes32-bit data unchanged:0x12→0x12121212 and0x1234→0x12341234. Reject an
out-of-width input without changing output. Separately `mb_usart_width` accepts
16/32 and rejects8, following actual RM0090p1010. A transformable8-bit bus
value is not permission to access that USART register. Reserved fields, register
side effects, clocks and protocol state still require their own contracts;
neither helper writes MMIO or simulates those register effects.

## Deliverables, verification and assessment

D1 is the source-labeled graph, family difference and arithmetic/lifetime proof.
D2 is the core plus actual one-way producer/linker/object imports. D3 is full
symbol/map evidence, independent interval/lease/arbitration/bridge certificates,
admission failures and intended defect diagnoses. D4 completes each original
changed task on a new case after at least seven days. D5 states resource costs
and evidence limits. Map/placement are bounded constant work; arbitration is
O(rounds*masters), with at most24 records. The lease wrapper adds no payload
copy on give and retains caller storage until actual finalize; producer costs
remain explicit. Audit emitted imports and final target dependency closure.

Run `topology`, `placement`, `lease`, `arbitration`, `bridge` under GCC/Clang
debug/optimized/sanitized profiles and a dedicated M4 model image. Independently
check12 map rows,960 extent cases plus actual link mapping, six adverse lease
states,14 active-mask/burst schedules and15 bridge cases. Confirm intended
mutants for false CCM reachability, invented F407 SRAM3, missing end bound,
READY-phase early reuse, original producer's EN-only completion, lost rotation,
ignored beat waits, missing bridge replication and unauthorized byte access.
Compiler errors and unrelated crashes do not count as mutation evidence.

For .08a, level2 requires correct source-labeled paths and extents; level3
completes `AX-PR-T37-06-T37.08.a-V1` by deriving F407's missing SRAM3. For .08b,
level2 requires the actual linked typed buffer and producer-owned lease return;
level3 completes `AX-PR-T37-06-T37.08.b-V1`, moving only the CPU temporary into
CCM without changing DMA permission. For .08c, level2 separates grant order,
bridge vectors and USART width; level3 completes
`AX-PR-T37-06-T37.08.c-V1` with explicit burst/wait premises and a bounded-wait
derivation. Level0 violates the corresponding contract; level1 repeats supplied
answers without their premises. Author runs do not satisfy learner delay or
prove absolute mastery.

Graduated hints: first follow a named master port to a named slave; then check
the last byte rather than only the first; next distinguish transfer READY from
finalize; finally count other bursts and bound each beat independently. For
the bridge, read the peripheral's allowed widths before interpreting expansion.
Separate assessor reasoning is under `assessor/architecture_bus/ANALYSIS.md`.

## Security, validation and limits

All inputs and adverse traces are original synthetic data; link controls are
never executed on a device. No arbitrary MMIO or external DMA interaction is
authorized. Harmless failure is an incorrect synthetic placement, ownership or
grant result. Physical bus/electrical/latency evidence, external review and
independent delayed learner assessment remain unclaimed. Current source-bound
author campaign and export receipts are in `legacy_architecture_bus`; exact
silicon qualification, clock/errata applicability and physical initialization
remain separate from the documentary map and model results.
