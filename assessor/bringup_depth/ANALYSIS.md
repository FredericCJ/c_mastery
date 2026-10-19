# T27 assessor analysis and transfer protocol

Keep this file and all reference implementations out of learner exports.
Original30 ordinary criteria and30 seven-day variants retain their exact IDs.
Assess each causal claim separately; one successful reset cannot discharge all
startup obligations. Grade0 for a false contract or unsafe out-of-bounds access;
1 for reproducing the happy path;2 for the correct mechanism plus the specified
boundary and a discriminating negative control;3 only for an independently
performed changed-condition task. Author execution never awards learner level3.

For T27.01, accept an explicitly unknown physical board marking, not an invented
revision. The family-wide256KiB includes64KiB CCM and192KiB ordinary SRAM; the
4KiB backup SRAM is a distinct domain. ST-LINK MCO8MHz is board configuration,
not a Cortex-M property. PM0214's generic interrupt wording and the F439 device
vector extent must not be averaged: the actual selected F439 template has91
external entries. A changed header claiming192KiB total SRAM is a scope error
if it silently excludes CCM; neither its typedef nor a successful link proves
electrical reachability. H745 adds two cores, TCM/cache/bank ownership and reset
coordination; existing selected-startup contracts must be re-read before reuse.

A real documentation conflict is PM0214 Rev10 p205: its SIZE-field prose has an
inconsistent minimum/bit-string, while its formula2^(SIZE+1) and Table43 both
identify32bytes with encoding4. The supplied flawed header case uses3 and is
rejected for this selected32-byte guard. Keep the conflicting passage in the
dossier, derive4 from the formula, inspect actual generated register writes and
test the selected model. Do not pretend the publication had no inconsistency or
generalize the emulator observation to every MPU architecture.

For T27.02/.03, BOOT0=0 selects flash regardless of BOOT1; BOOT0=1/BOOT1=0
selects system memory; both1 selects SRAM in the inspected RM21 table. Aliasing
changes what reset fetches at zero, not where the flash physical address is.
An offset application cannot make a bootloader disappear by changing ENTRY in
an ELF. Require the actual bootstrap and vector relocation plus pending-source
normalization. Reject ARM/even entries, truncated words and unsupported encoding
before decoding. Treat one-past stack top as a valid empty descending stack;
an ordinary data pointer at that address is not dereferenceable. A vector table
is live storage: the RAM variant owns it throughout all exceptions, while a
temporary automatic copy becomes invalid on return.

For T27.04/.05, require the half-open interval derivation and widened or checked
arithmetic. A loop that copies size+1 violates the no-touch invariant even when
it merely changes padding. The independent oracle uses sets rather than the C
subtraction test. An empty copy record is valid and creates no pointer access.
The linker and image checker use distinct load/run addresses and separately
check the second bank. CCM placement can be valid for CPU data and invalid for
DMA simultaneously. A cache-equipped H745 bank needs both correct reachability
and an ownership/cache-maintenance protocol; a DSB alone does not clean cache
lines. The MPU guard detects the demonstrated aligned store, not arbitrary
overflows that jump over it. Require real nested-frame evidence plus `.su` scope.

For T27.06/.07, ready on the last poll succeeds because readiness is tested
before the exhaustion branch. Each phase resets its own counter. A timeout
must not activate a UART whose clock was never established. Separate frequency
validity from source readiness. Reconfiguring a live PLL is rejected; post-start
failure invalidates timer/baud settings through the epoch before service resumes.
The topological preflight prevents a late discovered cycle from partially
running hooks; a callback failure is different and leaves a completed prefix.
An acceptable alternative uses DFS with a three-color proof and equally bounded
storage. C does not provide RAII, automatic constructors or a returning reset
handler contract. `__libc_init_array` is an imported mechanism to inspect, not
proof that libc is present or permitted. Negative unresolved-helper links are
translation/closure tests, distinct from compiled semantic mutants.

For T27.08, the inspected unmodified vendor pair is the F429 GCC reset template
v2.6.10 and STM32CubeF4 v1.28.1 F429ZI linker script. The sibling F439 reset
template is separately pinned. The F429 linker declares `.ccmram` but explicitly
requires the startup to copy it; the reset copies only `.data`. That mismatch is
the required negative example, not a reason to claim all vendor code is broken.
Its SystemInit runs before initialized C data/BSS and its runtime hook precedes
main. A relocated MPS2 exercise adapter must label its substituted SystemInit
and original restricted hook implementation; that execution is not STM32 RCC
validation. Preserve the exact vendor source, record every derived script change,
compare semantic sentinels/ownership and never demand byte-for-byte equality.

For T27.09, invalidate an old commit first. The cut at zero stores may preserve
old state; demanding rejection there is an incorrect oracle. Every partially
replaced inactive slot leaves the other valid slot selected. Generation0 can be
newer than255; half-range ambiguity prevents guessing after unbounded downtime.
CRC/FNV does not authenticate an attacker-controlled record. The teaching target
uses synthetic bytes only; the harmless security success condition is accepting
an altered record when the checksum check is deliberately removed. Do not call
that code execution. Repair length/version/checksum/commit validation and retain
the mutation regression. A DMA request to stop is not completion: memory cannot
be handed to the application until the engine and all callbacks are quiescent.

For T27.10, a valid earliest marker must not use uninitialized globals or UART.
The retained stage and image check distinguish no-vector, copy and zero defects.
Repair the combined image's wrong LMA first; the BSS failure should then become
observable. More stack cannot fix either source. Cold QEMU startup with no GDB
and an AIRCR software reset are executed categories; watchdog/reset-pin/power
trials on an identified board remain separate pending observations.

Hints are graduated: first ask which owner and authority establishes the state;
then ask for one interval/state trace before writing code; finally point to the
specific public invariant, not the solution. For delayed assessment, provide
only the original criterion's changed task and fresh generated image/input,
withhold this analysis and expected logs, record the previous attempt timestamp,
and require >=7days before awarding delayed evidence. Assess unfamiliar compiler,
bank layout, RAM-vector owner, hook DAG, interrupted generation replacement and
bootloader entry independently. A finite passing campaign supports these bounded
mechanisms; it cannot establish absolute mastery, all reset behavior or WCET.

The lifecycle variant makes two previously prose-only obligations executable.
For .07.b-DELAYED, three generations share one machine startup; exactly task-owned
bytes clear each time. Returning with two cleanup owners cannot permit generation
reuse. A stale completion must not decrement the new generation's count. The
UINT_MAX boundary retires permanently. Mutants permit early restart and ignore
generation checks; both must compile and fail on these causal assertions.

For .09.b-DELAYED, stop acknowledgment closes device activity but does not release
callback borrows. The correct trace is request/ack/callback/callback/transfer,
or request/callback/callback/ack/transfer. An acknowledgment-only mutant fails
before the first callback completes. The Python oracle uses a set of outstanding
callback identities, while C uses a count: 2916 event words yield 14580 checked
observations, including impossible extra callbacks, sticky acknowledgments and
terminal timeout. Neither implementation is a physical DMA adapter. A physical
adapter must prove its device-specific acknowledgment and callback guarantees;
the cross-target coherence task remains a reasoned contract plus capture protocol.
