# Assessor-only SPI analysis and transfer package

Do not publish these solutions with learner artifacts. References are explicit assessor dependencies, never learner work. Physical and delayed learner observations remain unexecuted.

<a id="07.a"></a>

## 07.a

At16MHz APB2 with BR3, SCK=16MHz/16=1MHz. CPOL=mode/2, sample leading for phase0 and trailing for phase1; sample eight MSB-first bits. Figure248 was actually rendered/inspected. For request9f,00,00 and response00,ef,40 the transport records all3 bytes. An application response code00 is not the synthetic protocol's a5. Three byte calls with independent CS intervals fail the partner's composite transaction contract even if each individual byte succeeds.

The state proof is: only begin owns CS; neither a write nor TX buffer empty releases it; final receive plus a fresh TXE/BSY0 observation starts hold; only elapsed hold or acknowledged reset-followed-by-hold reaches finish. A zero hold permits immediate release only after the fresh final observation. PS_MUTANT1 changes CPHA;3 releases at BSY1;7 reuses the pre-start status snapshot after instantaneous DMA memory completion. All must compile and fail semantically. The last is a real cross-phase race, not a timing constant substitution.

Delayed packet: retain the mode and1MHz rate, supply partner requiring one CS interval across command and response, setup2us/hold3us; choose a clock starting UINT32_MAX-3. Require a timing derivation through wrap, a trace with48clock transitions and one CS interval, and a counterexample made by joining three individually successful byte calls. No copying of the ordinary timeline.

<a id="07.b"></a>

## 07.b

PIO timeout prefixes are (sent,received)=(0,0) before TXE,(1,0) before RXNE,(3,3) during final BSY. TX ownership may end before final wire completion. Cancellation cannot retract bytes already sampled by the partner. Dropping CS on timeout can terminate an incomplete command; retaining CS can occupy a bus indefinitely. The specified policy retains CS until normal quiescence or explicitly authorized SPI-only reset, with all DMA owners acknowledged first.

Reference fw_spi actually releases CS after a busy timeout; composition.c executes that counterexample and preserves the frozen implementation. It remains a bounded teaching primitive, not a valid substitute for this successor's lifetime contract. sd_spi_poll supplies matching completed bytes, but its stopped argument is an assumed acknowledgement. These comparisons expose changed contracts instead of declaring old and new components equivalent.

Halfword representation is necessary for this documented direct-mode adapter: one byte value per uint16_t, upper bits0 on packing, low8 on extraction. uint16_t buffer lengths are2*count, NDTR=count. Raw uint8_t arrays and merely casting to uint16_t do not satisfy this. Target controls reject wrong channel, AF and byte-width DMA configuration. Distinct memory acknowledgements retain only the still-owned buffer; neither implies wire idle. Mutant4 releases at the stop request, mutant2 omits RX consumption. Record original helpers and actual retained object/import closure, not only compiler flags.

Delayed packet: independently deliver TX stop acknowledgement at t1, wire idle at t2, RX stop acknowledgement at t4, hold2; earliest CS release is t6. Reverse the DMA acknowledgements and inject DMA error after a partial prefix; count and ownership must remain distinct. Also cancel during setup before DMA starts and reset only after both acknowledgements. Supply all64 tied/ordered acknowledgement triples with four holds and ordinary/wrapping clocks. oracle.py computes the independent event-poset maximum and low-octet conversion, rather than executing a Python copy of the C state machine. A stuck acknowledgement must leave ownership held; a liveness guarantee requires an additional hardware/supervisor premise.

<a id="07.c"></a>

## 07.c

Version0 [a5,ef,40] and version1 [a6,40,ef] both decodeef40. The ordinary fixture [00,ef,40] fails device status without corrupting prior output. Mutant5 ignores status;6 keeps the old endian rule after the version changes. Separate (transport result, device result) and forbid decoding an incomplete buffer. Valid alternatives include a result structure or explicit two-stage API, provided failure leaves outputs well-defined and partial data is not promoted to success.

Delayed packet: change only the synthetic command contract to version1, leaving CPOL/CPHA, frame size and controller binary unchanged. Require evidence that the same controller tests still pass and new status/endianness counterexamples fail. A reset/retry of a non-idempotent command may apply twice; an acknowledgement lost after application does not establish non-application. The supplied register-read command is idempotent only under the declared immutable synthetic register premise.

## Criterion-specific rubric

Score each objective separately.0: invalid ownership, wrong wire rule, fabricated target claim or missing artifact.1: ordinary output matches but the indicated misconception survives.2: derives the stated contract, implements its positive/negative paths, explains every relevant counterexample and bounds its evidence.3: independently solves the corresponding changed packet after the recorded seven-day delay, revises affected proofs/costs and diagnoses an unfamiliar failing trace. Author campaigns support functional assessor readiness only; they do not award learner scores. A student may choose another valid representation but must rebuild costs and the target address/width proof.

The generated clock/provider/state tests are host-side oracles. Native register models check actual address/bit selections but do not emulate analog timing or all STM32 effects. PPC64 exercises endian/data model portability of the pure driver, not a big-endian STM32. M4 links are audited images, not booted board firmware. Physical capture and target/ISR integration use BOARD_CAPTURE.md and T29. Parallel authorship is not independent review.
