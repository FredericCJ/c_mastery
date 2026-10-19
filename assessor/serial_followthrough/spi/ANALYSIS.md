# PR-T38-05 assessment analysis

This is assessor-only. The supplied public contracts plus unchanged criterion objects define the learner task. Author implementation and finite tests do not establish independent learning or physical behavior.

## T38.06.a and V1

For CPHA0 sample the first/leading edge of each bit; CPHA1 samples the trailing edge. CPOL only changes idle/high-low orientation. A6 is10100110;3C is00111100, each MSB first. The version1 payload bytes34,12 decode1234 hexadecimal while each wire byte remains MSB first. The header A6 is peer status, not data. A one-byte successful transport carries no payload; two bytes carry an incomplete payload. Do not call the three-byte decoder on invented padding. `ss_edges` checks262144 finite pairs, `ss_device` checks1/2/3 frame transcripts, and the independent oracle derives lengths and expected semantic packets without calling C. Copied provider edge mutation preserves frozen source and changes only sample selection.

## T38.06.b and V1

Final receive and TXE1 do not establish BSY0 or CS hold completion. RX/TX DMA ownership may retire independently. Cancellation must leave both lease flags held at the request point; a later reset check can mask an erroneous early release, so the test observes the flags immediately. Both acknowledgement orders are checked. Reset is refused until both EN0 observations and then invalidates readiness; hold begins at the actual reset tick. Error prefix counts are not peer commit.

The actual IRQ transfer first dispatches token1 via DMA, requests cancel, refuses reset with both streams and then one stream active, acknowledges the other, resets, observes hold, retires token1, reopens and dispatches token2. A software-pended MPS2 IRQ0 runs the imported exception wrapper and passes the old1 to `ss_event`. Correct ingress rejects before any register access. The SS_MUTANT1 reference intentionally omits ingress validation and replaces it with the active token; PsSpi's internal token test cannot repair identity already erased by its caller. This bounded demonstration is not a cryptographic capability or a test of physical SPI IRQ routing. Foreground calls are masked and no nested/parallel invocation is admitted.

## T38.06.c and V1

The original flawed send-only client preserves the first receive byte11 while losing the next22 with OVR. Corrected per-byte receive drain avoids it. Generic dummy padding is not a protocol: the original peer needs exactly command plus two dummy bytes and a checked status. The scheduler copies command data into fixed slots and queues indices with actual algo_ring. An active or DONE slot cannot be overwritten by another admission. A full queue rejects without advancing the high-water token. Consumers must take a result before dispatching the next; this is deliberate bounded backpressure.

Setup/hold deadlines are controller-admission-relative. They do not bound queue waiting or a failed hardware stop. `ss_take` checks both state and actual driver ownership and copies only received bytes after unpacking released halfwords. SS_MUTANT2 bypasses this barrier and is rejected before result retirement. F439 mapped checks reject CCM and crossing SRAM end, inspect AF/mode/channel/PSIZE/MSIZE/count, and do not claim that the RAM model performed DMA. Actual mapped source is compiled without library helpers; frozen full algorithm compilation is pruned only to the actual ring mechanisms claimed here.

## Evidence and costs

The campaign records six native compiler/profile combinations, four Cortex-M4 compiler/optimization runs, deliberately compiled semantic defects, source/command/output hashes, independent packet checks and explicit linked closure. Target stack bound uses the largest sequential test-root frame plus twice all other retained C frames and44 bytes for one basic exception/alignment/wrapper, overbounding interrupted foreground plus IRQ; assembly runtime/reset/IRQ paths are explicitly inspected. It is not a physical watermark, nested-interrupt bound or scheduling guarantee. Actual code size and disassembly are retained in the linked artifacts, not compared as universal constants. Newlib headers supply declarations only; no linked libc/libm/compiler-runtime code is accepted. The initially unused-parameter legacy scaffold failure and initially undetected premature-cancel mutant remain in attempt history.

Level3 requires a new unaided explanation and independently chosen counterexample after the specified delay. Reject claims that fixed tests prove mastery, that a timeout grants ownership, that a numerical token authenticates a malicious caller, or that MPS2 register models validate electrical hardware. Physical board/peer work remains required in the public capture contract.
