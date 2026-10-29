# DMA leases across cancellation, visibility and software reset

This required successor supplies the executable work for the unchanged six
criteria and six seven-day transfer tasks in `PR-T29-04`: `T29.07.a` through
`T29.08.c`. Their stable AX IDs, baseline fixture, historical hours and delayed
assessment timing remain authoritative. Families: construction, semantic
reasoning, boundary diagnosis, failure/recovery, verification, resource analysis
and changed-assumption composition. The exact criterion mapping and inspected
passages are in `research/v5/expansion/legacy_advanced_lease/`.

Prerequisites are C17 integer/object/lifetime rules, original freestanding
startup and runtime, T27 memory regions, the T28.10 `struct pd_transfer`/`struct fw_peer` contracts,
and T29.01–.06 exception and deferred-service ownership. Supply those learner
artifacts before assessing integration. Author campaigns explicitly import the
assessor implementations; they do not count as learner integration. No heap,
libc, compiler runtime, atomics library, CMSIS or HAL is a target dependency.
Host Python, sanitizers and independent oracles are verification tools only.
C23 adds no automatic DMA ownership or reset-domain guarantee to these C17 APIs.

The exercise uses one serialized CPU service, one admitted DMA stream and
exclusive buffers. Interrupt/callback entry must marshal metadata into that
service or hold the already-proved local exclusion primitive. Calling this API
concurrently is outside its contract: ordinary C fields do not become atomic
because they describe a device. The local lock excludes only its declared CPU
participants; a second core, NMI or device requires a separate protocol.

## Representation and ownership

Implement `lease.c`, `bridge.c`, `order.c` and the target driver, leaving the
supplied headers stable. `struct xd_ticket` contains a nonzero epoch and generation.
`struct xd_lease` has nine 32-bit fields and no hidden allocation. Its phase is one of
QUARANTINE, CPU, DEVICE, STOPPING or SERVICE. Zero initialization means
QUARANTINE, never permission to touch a possibly active device's storage.
Within an epoch, generation increments without wrap; UINT32_MAX retires it.
The supervisor epoch also refuses wrap. Neither number is a cryptographic
capability. Forging proof fields or corrupting the state violates the trusted
port boundary and cannot be repaired by a generation comparison.

`xd_recover` accepts only QUARANTINE and a supervisor fence whose old epoch is
at least the retained epoch, whose new epoch is exactly old+1, and whose stop
and write-drain observations are true. The supervisor lives outside the
software reset domain and must possess the current hardware identity. The
numerical inequality does not establish that identity. Recovery resets the
generation and terminal counters. `xd_submit` accepts 1..16384 bytes only from
quiescent CPU ownership, advances the generation, clears result fields and
transfers exclusive ownership to the device. Output tickets stay unchanged
on failure. Arguments and output objects must be valid, suitably aligned,
nonoverlapping objects for each call; no pointer may alias the state.

`xd_cancel` changes DEVICE to STOPPING, retaining the entire payload lease.
It is a request, not proof of stop. A repeated request returns DUPLICATE;
neither a timeout nor reset grants CPU ownership. `xd_observe` first checks
both ticket fields, before accessing the payload or mutating current state.
A proof must match the ticket, bound complete<=possible<=length, report EN=0,
no pending device writes and completed visibility work. A COMPLETE requires
the entire length; STOPPED requires a cancel request. ERROR retains a
conservative possibly-written extent. Rejected calls leave state unchanged.

The first valid terminal result creates SERVICE ownership and count=1. An
identical notification is DUPLICATE. A stop confirmation after COMPLETE is
DUPLICATE; a completion after STOPPED is STALE. Neither changes the chosen
result. `xd_reclaim` moves SERVICE to CPU and returns the result once. Later
metadata-only notifications may still exist, but must fail ticket validation
before any buffer access. A device write is not a metadata notification:
all old writes must already be impossible before reuse.

All core operations are O(1), use fixed storage, and contain no polling loop.
Explain `sizeof`/alignment and stack use in the selected ABI. The actual DMA
provider has separate bounded extents/cache-line costs; cancellation latency
is not bounded merely because requesting cancellation takes constant time.

## 07.a — terminal selection and actual driver composition

Replay the original fixture `submit,cancel_request,complete,stop_confirm,reclaim`.
Record phase, owner, ticket, status, complete/possible byte counts and terminal
count after every operation. Require DEVICE, STOPPING, SERVICE, SERVICE, CPU;
the chosen terminal is COMPLETE exactly once. Also run zero/oversize submit,
wrong epoch/generation, repeated reclaim, pending writes, invalid progress
intervals and generation/epoch exhaustion without partial state mutation.

Implement `xd_pd_cancel` and `xd_pd_finish` against the actual prerequisite
`pd_cancel/pd_poll/pd_finalize`. Validate compatible ticket, total length,
active phases and callbacks before consuming the provider result. The provider
retains `PD_CANCELLED` after a cancel request even if all bytes arrived. This
successor's contract reports COMPLETE for a non-error full byte count after
confirmed quiescence. Preserve the underlying status for diagnosis; do not
rewrite its API or treat two different result policies as equivalent.
Errors never become success merely because NDTR reaches zero. No finalization
or payload read precedes the provider's drain and visibility guarantees.

Submit the code, transition table, trace replay, policy mapping and a proof
that at most one reclaim succeeds. The transfer assessor changes callback
order so stop wins first; defend why a later completion cannot overwrite it.

## 07.b — visibility is a chain of distinct obligations

Draw CPU descriptor/payload writes -> device fetch -> device result writes ->
CPU result observation -> deferred publication. Label every edge with its
actual compiler, ISA, SoC, cache, ownership and software contract. Register
programming in the selected DMA2 port is not a hardware descriptor ring;
apply descriptor visibility reasoning to the explicitly synthetic descriptor
example without inventing a descriptor-fetch capability for this controller.

Use source=42, destination=9, untouched destination neighbour=55, transfer
length32. Compare cacheless SRAM with a two-line write-back noncoherent model.
Model speculative destination refill while application reads are forbidden.
Inject EN=0 while a FIFO write is pending: poll and reclaim must remain busy.
After the delayed write and TC/FIFO-empty observation, post-invalidate must
make42 visible and preserve55. Removing destination clean, post-invalidate
or a required completion barrier must be distinguishable, not described as
one generic 'memory barrier bug'. For partial8-byte cancellation, return8/8;
for an error, retain a conservative0/32 interval under this chosen workload.

Supply visibility traces, a line-ownership diagram, invariant assertions and
an actual imported-provider run. The changed condition uses a cache-equipped
M7 and demands whole exclusive32-byte lines; source/destination ranges and
capacities must pass the retained provider's region/alias/overlap validation.

## 07.c — stale tickets and reset domains

Run generation7 cancellation, refuse reuse during pending writes, then permit
generation8 and inject callback7. Compare the entire current state before and
after rejection. Make epoch differ while generation is equal. Explain the
finite-counter exhaustion policy instead of assuming wrap can never occur.

In the reset variant, clear only `struct xd_lease` while a separate engine and
supervisor remain alive. The engine still owns a queued write. Zero state
must refuse submit; a stop request alone must refuse recovery. Drain old
writes, complete visibility, mint a fresh supervisor epoch, then enable reuse.
Retain an old metadata callback and show its rejection. Also demonstrate,
inside this synthetic model only, that a fabricated quiescence proof permits
a late engine write to overwrite a harmless sentinel. Ticket checks cannot
stop bus masters. Do not reset shared `struct pd_transfer` state while an active
hardware stream is still associated with it.

The hardware protocol is in `curriculum/guides/interrupt-lease-board-protocol.md`.
Actual power/reset lines, retention, other bus masters and peer state must be
qualified for the named board. A software-only experiment is not evidence of
all chip reset modes. A failed peer cannot be declared quiescent by erasing
its outstanding-work counter or forcibly unlocking a hardware semaphore.

## 08.a — distinguish the four proposed repairs

Construct explicit failing traces for compiler memory clobber alone, DMB/DSB
alone, cache maintenance alone and local interrupt masking alone. State which
agent can still violate each property. DMB orders applicable explicit memory
accesses; DSB also waits for prior access completion as defined by the ISA.
Neither commands the independent DMA engine to stop. A compiler clobber does
not issue a CPU fence. Cleaning one line does not establish an ownership
protocol or invalidate a later speculative copy. Interrupt masks do not
exclude the DMA or the other core. ISO C alone specifies none of the selected
MMIO register semantics. `volatile` is not the missing synchronization proof.

Implement the four named target ordering primitives and audit their actual
objects. For the changed condition, compose the selected F439 DMA port's
compiler clobber, DSB, stop/drain checks and lease transfer. Investigate the
deliberate pair of adjacent identical DSB SY instructions with no intervening
memory operation. Compare one versus two using31 alternating-order paired
blocks,128 stores/barriers per block; record all samples and object identity.
Explain from complete instruction definitions why one suffices for the same
prior-access completion obligation. Do not remove the DMA stop protocol,
cache operations, or unrelated barriers on this evidence. Emulator timer
ticks measure the declared virtual execution only; physical costs remain
unobserved. Report physical results only through the board capture protocol.

## 08.b — F439 versus a verified M7 route

Audit the unchanged actual F439 `pd_target_io` implementation and its linked
closure: DMA2 stream0 M2M, normal SRAM, same source/destination widths, exclusive
buffers, selected clock enable/readback and status handling. Reject CCM for
DMA2 and reject cache-enabled F439 configuration. Flash ART instruction cache
and literal-pool data cache accelerate Flash accesses; they are not an SRAM
core data cache. Do not call invented SCB cache flush operations on this M4.

Repeat the audit for the declared H745 M7 selected port. Reinspect RM0399 Rev4
SRAM3 `[0x30040000,0x30048000)` and the provider's reserved metadata/exclusive
payload range `[0x30040040,0x30048000)`, DMA2 route and PM0253 Rev5 maintenance
registers. Use the normal SRAM3 address, not its M4 code alias or DTCM. Explain
the32-byte line, PoC clean/invalidate, privileged accesses, DSB/ISB sequences,
and asynchronous fault limitation. Inherit the provider's inspected errata
and boot/clock/MPU preconditions; a link is not qualification of a board.

Deliver two object/link dependency audits and a source-to-register table.
`board_build.py` builds a port closure at the declared code address, not a boot
image; it never flashes or executes hardware MMIO. Review every object import
and the final empty undefined-symbol set, not only include files and flags.

## 08.c — shared-cache ownership and its boundary

Compare three evidence classes: F439 cacheless SRAM contract, the explicit
noncoherent cache model, and the retained H745 shared-memory protocol. Import
the real `fw_cache_range` and `fw_peer_step` providers. Record each line's dirty,
valid, backing-memory and owner states. Begin work on both cores, finish only
one, and verify confirmation remains busy. Declare a peer failure and verify
its outstanding count is retained and confirmation remains failed. Explain
why HSEM plus local interrupt exclusion still needs cache visibility and why
an unresponsive peer cannot be force-released as a recovery shortcut.

For the changed line-sharing condition, place unrelated dirty77 at the last
byte of the receive line; invalidation must expose the deliberately lost
neighbour in the bounded model. Repair the interface with whole-line exclusive
ownership and reject misaligned address/capacity/length, or implement a fully
specified copying bounce-buffer alternative with its costs. Padding alone is
insufficient unless the linker/allocation and ownership rules preserve it.

## Workflow, evidence and assessment

Run `python3 labs/interrupt_depth/lease/run.py --build build/my-lease` against
your implementation. The public tests intentionally reject the untouched stub.
Use `--source` only to select an explicitly identified learner/reference file;
there is no automatic reference fallback. `target_build.py --help` lists all
required explicit producer inputs. Supply the unchanged predecessor boot,
linker, provider headers/implementations and your original runtime. Keep hosted
tools outside target dependencies. Assessors separately run the source-bound
campaign, independent9^4-word transition oracle, compiled semantic mutants,
sanitizers, four M4 profiles and eight selected F439/H745 port closures.

Submit source, source/target identity ledger, six criterion-indexed reasoning
sections, ordinary and changed-condition traces, object imports/maps,
measurement raw data and limitations. A successful build alone fails. Each
criterion is scored0..3:0 missing/incorrect;1 ordinary output only or unexplained
assertions;2 correct representation, invariant, boundaries and discriminating
verification;3 independent derivation and changed-condition defense, including
a counterexample to an overstated guarantee. Require>=2 for every criterion
and>=3 on the independently selected delayed transfer. Finite tests establish
only their recorded bounds; independent performance is a separate event.

Hints, in order: (1) name who can still write each byte; (2) separate requests
from observations and callbacks from bus writes; (3) place epoch/generation
checks before mutable access and visibility before publication; (4) use the
provider's complete/possible byte interval, not TC alone, to choose a result.
Assessor solutions and transfer schedules remain under `assessor/`.

Security scope: only these original teaching programs, synthetic byte arrays,
the named emulator or a dedicated qualified development board. The attacker
may delay/replay metadata, request cancellation/reset and choose admitted
lengths; arbitrary proof fabrication is a separate compromised-trust-boundary
counterexample. Harmless success is a synthetic sentinel/property violation.
A crash or undefined C behavior is not proof of code execution or privilege
escalation. Require repaired rejection tests and retain limits for corrupted
supervisor state, unqualified hardware and missing physical observations.
