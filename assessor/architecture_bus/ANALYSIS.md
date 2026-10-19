# Private memory/bus analysis

The exact original three criteria and V1 tasks remain unchanged. Distinguish
documented numerical topology, actual linked storage, ISO object validity,
borrowed CPU access and physical observation; none subsumes the next.

For .08a, the F407 changed graph loses SRAM3 and the LCD/DMA2D matrix masters.
It retains112+16KiB ordinary SRAM and64KiB direct CPU CCM. F439's ordinary
192KiB must not be confused with its256KiB total including CCM. DMA1's dedicated
peripheral/APB1 path is not its memory master; the latter can reach ordinary
SRAM. The public six-port selection omits code/remap/external paths rather than
claiming them absent. Require a conditional edge annotation for an unseen
boot/remap question and a source gap when the changed part is not in RM0090.

For .08b, reject the SRAM1 candidate ending16 bytes into SRAM2 even though the
frozen Pd aperture intentionally permits the combined ordinary address range.
This is a stronger project allocation policy. Address+size in32-bit arithmetic
can wrap; the independent oracle uses mathematical interval ends. Negative
addresses are not admitted by this unsigned typed API; a signed wrapper must
reject before conversion. A valid numeric interval does not identify a live
object. The actual unchanged AWRecord arrays supply typed objects and target
symbols; the source is a separate16-byte character array. Reusing characters
to inspect/write representations is permitted, while inventing AWRecord
objects inside a declared character array is a different invalid premise.

Both target audit scripts include the unchanged selected B06 link recipe. The
CPU-local records move from0x20000000 to0x10000000, while source0x20000010 and
destination0x20020000 stay fixed. The audit-only entry is not a reset proof.
Runtime tests use real host/emulator storage plus logical numeric tags; there
is no cast from a numerical F439 address to a host pointer and no physical DMA.

Actual PdPoll requires matching generation, disabled stream, empty FIFO and TC.
The callback model deliberately presents EN0/TC while the FIFO still drains;
the preserved lease is the discriminating counterexample. READY3 permits
finalize, not another CPU writer. The new early-release mutant lets a byte
change before finalize; the frozen producer's PD_MUTANT5 exposes the separate
EN-only error. Model copying is performed only by the original test's hardware
actor while the CPU lease is absent, not by the production CPU-access wrapper.
Changing the numeric tags without changing the real backing allocation is a
dishonest adapter outside the C component's ability to verify.

For .08c, recurrent CPU wait is8+3=11 model ticks under the concrete three
burst costs2/8/3. The uniform upper bound is2*4*3=24, not an assertion that each
wait actually takes24. Continuously ready requests, finite per-beat response,
fixed contenders and no extra bridge delay are indispensable. An arbiter can
be fair in grant order yet stall forever on one unbounded response. Bounded
bursts alone are insufficient. The byte bridge value0x12121212 is valid as a
transformation, but USART's16/32-bit access rule rejects the initiating8-bit
access. General bridge behavior does not authorize a register operation.

Independent evidence uses direct source graph rows, mathematical intervals,
complete six-state lease certificates and sum-of-other-service bounds. Exact
emitted imports and final model-image helpers must remain closed. Additional
physical clocks, SRAM arbitration latency, electrical limits, silicon identity
and errata work is not fabricated; external and delayed learner results remain
separate from this author package.
