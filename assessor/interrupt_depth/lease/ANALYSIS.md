# Assessor-only PR-T29-04 solution and transfer schedules

Use the unchanged criterion IDs in the public contract. Each row must earn its
own score; the narrow scalar transition test does not establish cache, target
or physical correctness. Source-bound current evidence is under
`reviews/v5/expansion/legacy_advanced_lease/`. No author run is an independent
learner attempt, seven-day assessment, external review or physical capture.

## 07.a

The phase partition separates cancellation request (STOPPING) from final result
(SERVICE). `terminal_count` changes only at DEVICE/STOPPING->SERVICE. Reclaim
changes SERVICE->CPU once; stale notifications cannot reopen a transfer. The
original ordinary fixture therefore chooses COMPLETE after cancel. The Pd
adapter validates matching epoch/generation/length before polling or consuming
anything; finalization supplies visibility and quiescence. Pd's cancellation
status is a request-history policy, so non-error full progress maps to COMPLETE.
An error retains ERROR. No ABI/API substitution changes the provider itself.

Withhold the sequence submit,cancel,stop8,complete32,reclaim. Expected result is
STOPPED8/8 once; completion is STALE and all terminal fields unchanged. Ask for
an alternative policy where cancel always wins and an explicit adapter contract;
it is valid in another API but does not satisfy this criterion unchanged.
Mutant1 frees during cancellation; mutant3 republishes terminal count. Both
compile and fail semantic assertions. Reject 'the ISR ran, therefore stopped'.

## 07.b

The provider first cleans source and destination whole lines, invalidates
destination, releases and enables. A clean preserves unchanged suffix bytes
before later invalidation. Device writes occur in backing memory. A clean
speculative refill can retain9 even after device42; post-invalidate is needed
before read and service publication. In the8-scenario composition, cached mode
performs two invalidations and15 modeled barrier events; cacheless performs
zero invalidations and3 barrier events. These counts are costs of the imported
implementation/model, not physical elapsed cycles. An independent visibility
schedule permits device fetch before the descriptor write becomes visible if
only compiler ordering is supplied. DMA2 register mode has no descriptor ring.

Withhold cached partial stop at8 and dirty neighbour55, then error after a
possible write. Expected8/8 or conservative0/32, never assume untouched suffix
on error. Missing destination clean loses55; missing post-invalidate reads9;
early finalization succeeds while pending writes remain. These selected real
provider mutants must compile and fail the corresponding assertions.

## 07.c

Generation7 is retained until no old write can occur. Only then can8 reuse the
storage. Equality checks on both epoch and generation precede state mutation;
old metadata may still arrive but carries no authority to read/write payload.
The independent symbolic oracle covers all6561 four-event words and26244
transitions from an active transfer, including quarantine/recovery. It does not
prove all histories. Exhaustion and reset are separately tested.

Withhold software zeroing while the engine has a queued write. The supervisor
and engine are outside that reset domain. Stop only clears EN; drain is a
distinct observation. A fabricated fence permits overwrite of99 by42 in a
local teaching model even though the new generation is different. Correctness
depends on a trustworthy quiescence proof, not on the numeric token. Accept a
retained monotonic epoch or equivalent restart handshake only with a precise
reset-domain assumption; reject 'memset state to idle', guessed stop delays,
discarded ownership counters and counter wrap. Physical full-chip reset effects
are not established by this software-reset model.

## 08.a

The ordering model enumerates descriptor/start/fetch schedules: three permitted
bus schedules after the start-before-fetch constraint, one violates descriptor
visibility; one remains after descriptor-before-start ordering. Cache dirtiness
and independent DMA writes are separate state dimensions. IRQ masking removes
neither. DMB instruction definition orders accesses; DSB completes prior accesses
and blocks following instructions, but neither writes DMA stop registers.

For transfer, show actual F439 port DSB/compiler clobber plus hardware stop/drain
and adapter ownership. The deliberately adjacent duplicate DSB SY has no
intervening access, context change or alternative observation between the pair.
The first already discharges the same prior-access completion obligation; one
retains the property. Actual four-profile disassembly checks zero/one/two
barrier instructions in the specified primitives. Paired31x128 measurements
record the virtual instruction-cost difference; they cannot justify a physical
WCET or deletion of a different barrier. The production Pd port remains
unchanged. Accept a conservative extra DSB if the learner identifies its cost
and does not claim necessity from a timing coincidence. A 'faster' variant
without equivalent correctness is not a valid comparison.

## 08.b

F439 SRAM is on the admitted AHB path; CCM is CPU-only for this route. Flash
ART retains64x128-bit instruction lines and8x128-bit literal-pool data rows
according to RM0090 p84; those facts do not create a core SRAM D-cache. The
F439 link must contain DSB and no `fw_h745_cache` or cache-maintenance MMIO.
The H745 link imports the actual original cache provider and uses PoC clean
0xE000EF68 / invalidate0xE000EF5C. PM0253 pp240–244 supplies privilege, ordering
and fault requirements; pp219–220 identifies8-word lines. RM0399 pp136–137
distinguishes normal SRAM3 from code aliases and DTCM. Provider exclusions,
boot setup and errata are retained. Eight linked port closures are not eight
physical board runs or complete boot images. Require the learner to identify
every imported helper and original implementation; a library-free source file
does not prove a library-free binary.

## 08.c

The imported FWPeer protocol retains peer1 outstanding=1 after peer0 completes
and reports peer failure. Confirmation remains FAILED even after a later valid
END. A stale epoch cannot decrement a current counter. A shared dirty neighbour
is overwritten by invalidate/refill despite a correct DMA byte count: ownership
must cover the line, not only nominal bytes. The API rejects a misaligned
one-byte destination and leaves output totals unchanged. Whole-line placement
and exclusive ownership are a valid repair. A bounce buffer is also valid if
copy timing, capacity/failure semantics and device quiescence are specified.

Withhold a receive buffer embedded beside dirty application state, then an
unresponsive second core. Require both the line restriction and the independent
peer-liveness limit. Moving to a cacheless M4 removes cache maintenance, not
device ownership, compiler/ISA ordering or the need for a recovery handshake.

Apply the public0..3 rubric separately. The delayed session must use unseen
ordering/layout/reset conditions after at least seven days, record the actual
learner artifacts, assessor identity and rationale, and leave it pending until
it occurs. Do not create a synthetic learner receipt from these solutions.
