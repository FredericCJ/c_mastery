# Cacheable shared metadata from cold reset

This is the required author implementation successor for
`G-B06-T59-07-TRANSFER`, `PR-T59-07` and `T59.07.a/b/c`. It retains the original
noncacheable baseline, original criterion text and physical transfer requirement.
Families B, C, D, F, G, I and J apply. Prerequisites are the completed
PR-T59-06 journal/selection artifact, PR-T54-03 shared-memory channel, and the
exact `fw_cache_range`, `fw_peer_step`, `fw_h745_step` and `st_cold_h745`
implementations with their supplied public contracts. The last provider has
no cacheable boot implementation: this successor supplies that missing part.

## Selected environment and representation

Use the C17 baseline, GCC and Clang. The target is the selected unswapped,
two-bank STM32H745xI, M7 flash vector at 0x08000000 and M4 at 0x08100000,
HSI64/reset domain dividers, privileged cold reset, BCM4=0 and BOOT pin0.
Authenticated provisioning and generation selection are prerequisite artifacts;
the demonstration generation7 is synthetic and not a persistent anti-replay
counter. No reset-independent recovery is inferred. Both local ordinary IRQs
remain masked for this finite teaching workload. NMI/fault handlers may not
enter the protocol. No DMA, debugger memory writer, second HSEM user, aliased
mapping, speculative user, or other bus master may own the reserved line.

Each ELF declares the same typed32-byte `struct fw_peer cb_shared` in `.peer` at
0x30040000. Both linker scripts reserve exactly32 bytes as NOLOAD, outside
private BSS/data/stack. M7 uses private DTCM at0x20000000; M4 uses private
SRAM1 at0x30000000. This is one logical inter-core object, not two copies in RAM.
ISO C does not specify these physical addresses, MMIO/cache visibility, cross-core
execution or HSEM; the selected compiler/ABI/ISA/SoC contract supplies them.
The access permission is privileged RW, execute-never. M7 maps normal
nonshareable write-back/write-allocate (`TEX=1,C=1,B=1,S=0`), M4 maps normal
noncacheable shareable (`TEX=1,C=0,B=0,S=1`). The word nonshareable is an MPU
attribute, not permission for concurrent software ownership. Coherence is
explicitly implemented by HSEM and whole-line maintenance.

## Operations, ordering and failure

`cb_boot(struct cb_io,g)` requires live callbacks/context, no reentry, no callback failure,
and exclusive cold startup. `g` must be nonzero and selected by trusted policy.
It checks disabled caches/MPU and held CPU2, admits8 or16 MPU regions, selects
the data cache and admits the five documented four-way,32-byte-line geometries
(32,64,128,256 or512 sets). Unsupported geometry refuses after the CSSELR
selector write; this is not an all-register rollback promise.

It invokes the actual cold-start provider for clock/option/mapping checks and
SRAM/HSEM clocks, then requires semaphore0 unlocked. It disables all stale MPU
region descriptors, programs region7, enables the privileged background map,
invalidates every data-cache set/way, completes barriers and enables only the
data cache. Instruction cache remains disabled. It initializes all eight peer
words through the now-cacheable mapping, invokes the actual whole-line clean
provider, and only then sets BOOT_C2. This one-way release cannot be undone by
returning an error. Unsupported assumptions, asynchronous BusFaults, or a late
external change require fail-stop and a separately controlled reset; return0
does not restore earlier clock/selector/MPU writes. No hot remapping is supported.

`cb_peer(struct cb_io,g)` executes on released M4 with its own reset system registers.
It verifies release and clock premises, programs its own noncached32-byte map,
and checks the existing generation without writing any peer word. A mismatch
refuses; it never repairs a stale generation by overwriting it. Firmware startup
clears only private BSS. Reading the immutable generation here does not grant
access to mutable peer state: all subsequent operations use the original HSEM
adapter, invalidate-before-read on M7 and clean-before-unlock.

The finite workload obtains a reservation, demonstrates that health cannot
replace returning it, returns it, publishes health, then M7 obtains current
joint readiness. A late failure clears readiness through the imported protocol.
Readiness is not durable image commit or proof of a powered device stopping.
All ownership-bearing device operations must be tracked by the caller and
actually drained before returning their reservations. Timeout retains them.

## Concrete work and assessment

D1: derive the32-byte MPU size encoding, `CB_M7_RASR`/`CB_M4_RASR`, four-way
set encoding, initialization/clean/release order and rejection boundary. Explain
why the original disabled-cache boot image did not implement this variant.
D2: implement `src/boot.c`, link both exact images with the completed imported
providers and show the two maps do not include `.peer` in data/BSS. D3: retain
host and CPU-model observations, independently encoded set/way words, compiler
versions, target maps/symbols/disassembly and compiler-helper closure. D4: kill
the six actual controls (early release, wrong mapping, missing cache invalidation,
missing clean, peer reinitialization, unsupported MPU-region count); isolate the violated property. D5: after
seven days change cache geometry and boot generation, rederive all set/way
addresses, and then perform the dedicated-board capture protocol when separately
authorized. D6: list exact source, model, execution and physical limits.

T59.07.a assesses line ownership, source authority and why a barrier alone cannot
make a stale generation current. T59.07.b assesses the actual cold-start image,
both linker reservations, full cache initialization and refusal paths.
T59.07.c assesses six causal controls, the changed cache geometry/generation
and separately recorded physical traces. Level0 violates authority/order;
level1 passes one startup without explaining the map or dirty-line boundary;
level2 supplies all source-bound D1-D6 author artifacts and exact limits;
level3 independently reconstructs the changed conditions after the recorded
delay. Author execution does not award learner or physical acceptance.

Hints: draw the line's cache and RAM versions; then identify the first point at
which CPU2 may execute; finally inspect both ELF section tables rather than
assuming `NOLOAD` means the startup loop cannot touch the address. Separate
assessor reasoning is `assessor/cacheable_boot_depth/ANALYSIS.md`.

## Verification, costs and boundaries

Run the source-bound `tools/run_cacheable_boot_depth.py` campaign. The independent
Cartesian oracle checks3968 set/way values across five geometries in every
host profile. Six compiled semantic controls and the compiling learner stub
must reject with `CB_CHECK`, never because compilation or sanitizers failed.
Actual selected M7/M4 ELFs compile/link under GCC/Clang O0/O3 with no runtime
libraries or undefined helper symbols. MPS2 executes the same original register
model on Cortex-M CPUs; it is not a model of STM32 physical cache/HSEM behavior.

Startup work is O(number of MPU regions + sets*4); register-model storage is
host verification only. Target storage is one shared32-byte line and private
control/stack; the map reserves4KiB for each stack. Code/section/stack records
must be interpreted under the emitted compiler configuration, not assumed
worst-case execution time. The board capture remains required. No physical
cache, DMA, flash, voltage, power-loss or reset trace is supplied by this package.

Source passages: PM0253 revision5 pp.219-220,224-230,240-244 and bounded
RM0399 revision4 boot/clock/HSEM passages in the registered provider. The final
published ISO C17/C23 access gaps are retained. Original synthetic teaching
images only; this task performs no flashing, option-byte changes or host
security changes. Harmless success is the two private `cb_status` words and
the current peer readiness record. A fail-stop status is not an exploit.
