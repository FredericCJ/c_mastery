# Private criterion analysis: PR-T28-03

Keep this file, references, differential probes and delayed solutions out of the
learner package. The original three criterion IDs and seven-day requirements
remain unchanged. This is author support, not independent assessor review.

## 06.a

The supplied example has16,000 timer input clocks per output period and16 input
clocks per counter increment;1000 counter states imply ARR999. CCR250 therefore
gives250 high states,1000 low+high states and duty1/4. PSC15 is not a divisor of15.
The exact tick is1µs. Zero duty has CCR0; edge full duty needs CCR=ARR+1.
The16-bit profile rejects ARR65535 because full duty would need65536. An
alternative with forced-active OC mode is valid if its switching transient and
mode register contract are implemented and assessed separately.

At1001Hz, the ideal span is1000000/1001; nearest is999, giving1000000/999Hz.
Relative frequency error is1/999999, slightly above1ppm: zero or1ppm rejects,
2ppm accepts. A rational comparison avoids float cancellation and premature
integer truncation. The implementation's base admission prevents product
overflow; all caller-visible output is assigned only after validation. Require
the candidate to show that guarantee for each early return and narrowing.

Delayed .a: at16MHz/16 with1kHz center alignment choose ARR500, CCR125 for25%
and CCR500 for100%. The phase sequence has1000 states, two update events, and
250 high states for CCR125. At ARR8 the sequences for CCR4/7 are8/14 high states;
CCR8 is constantly high, CCR0 constantly low. `pt_level` uses direction at
equality and explicit extremes, matching the visually inspected Figure165.
Copying ARR+1 into the center period gives the wrong501-tick answer. Six core
mutants include both wrong-period formulas; private phase enumeration is a
different oracle from repeated use of `pt_ratios`.

Alternative valid solutions include an exact-only planner, a minimum-frequency
error search or a forced-output extrema implementation **only when the public
selected policy is also implemented or exposed as a separate, fully specified
mode**. Silent clipping or changing the criterion's rounding policy fails.
Costs are bounded constant work and fixed caller-owned storage, no allocation.
The target calls actual `pu_rates`; reference composition also calls actual
`fw_timer`. At10 counter ticks and250permille, nearest gives CCR3 while that
provider truncates to2. This is a contract difference, not a broken oracle.

## 06.b

After250 old ticks, writing ARR499/CCR125/375 leaves active ARR999/CCR250/750.
The next old overflow latches all three plus PSC, resets CNT and raises UIF.
The deliberately wrong immediate-publication and split-second-channel variants
must compile and then fail their specific assertions. Rejected translation is
not a semantic counterexample. The finite5-event language covers tick, UG,
UDIS toggle, URS toggle and staging across all3125 words in each count mode;
public long traces cover actual boundary crossing beyond that short language.

UG with UDIS set resets the counter/divider but preserves active shadows; URS
suppresses the software-triggered update request/UIF, not shadow publication.
The diagnostic update counter saturates, not wraps. The target owns a coherent
register group; it validates before writing, excludes other writers and holds
UDIS while writing all values. A hardware overflow during that interval can be
deferred: there is no promised first-update latency. The native target control
injects a possible update at every preload write and rejects missing UDIS.
No amount of `volatile` makes a multi-register sequence atomic to hardware.

Delayed .b: channels1/2 share TIM3's counter/update; CCR125/375 latch together.
Different timers require a verified internal/external trigger route, enabled
slave clock and understood prescaler/reset phases. RM0090 Table99 maps TIM3
TRGO to TIM4 ITR2, with the relevant master/slave rules on p634. A candidate may
derive a TIM3→TIM4 reset/trigger arrangement, but cannot claim physical zero
skew without observing routing/output delays. In this package TIM4 is used for
capture, so repurposing it requires releasing that ownership first. A correct
changed design must say whether it synchronizes starts, repeated resets or
phase-locked periods; these are different contracts. Consecutive CEN writes
are the discriminating incorrect explanation, not a valid substitute.

## 06.c

Capture65530→4 under a known less-than-half-range acquisition interval means10
ticks. The same pair after one additional complete wrap is observationally
indistinguishable; the API cannot supply its own missing elapsed-time premise.
Overcapture is a loss indication, not a larger trustworthy timestamp. Reading
CCR before CC1OF closes the manual's named lost-overcapture window, but does not
turn a one-entry capture register into a queue. The target test injects an
overcapture immediately after the read and verifies output preservation and
write-zero clearing without erasing an unrelated pending UIF.

Synthetic ordinary uncertainty task:16MHz±100ppm,PSC15,ARR999,CCR250, observed
rising edges0/1000000ns and falling edge250000ns. Scope resolution10ns, per-edge
jitter5ns, timebase20ppm, skew3ns. Expected period is
[10000000000/10001,10000000000/9999]ns; observed is[999960,1000040]ns.
High interval is[249972,250028]ns. Conservative duty bounds divide high-low by
period-high and high-high by period-low. They contain1/4. The capture count1000
with one quantization tick and two declared extra ticks gives
[9970000000/10001,10030000000/9999]ns. These are deterministic assumptions,
not statistical confidence intervals and not values measured on a board.

Delayed .c:8MHz±10000ppm,PSC15,ARR499,CCR125, capture count500, scope100ns,
timebase100ppm. Recompute before looking at a capture. Expected period becomes
[100000000/101,100000000/99]ns and observed interval[999790,1000210]ns.
The same nominal1kHz/25% can pass a much wider uncertainty budget; it is not the
same evidence quality. Source changes without corresponding ARR/CCR changes
fail frequency consistency. A shared-clock loopback is insufficient for absolute
frequency: its ratio can remain perfect while the common clock is wrong.

The measurement checker rejects missing physical metadata/raw identity, binary
float inputs, nonpositive/ambiguous intervals, overcapture, unusable tolerance
and zero/full-duty edge inference. It separately detects wrong-duty and
wrong-period synthetic observations. Hardware acceptance additionally requires
the full raw stream, missing-record analysis and the protocol's scope/routing
and calibration evidence. The physical template remains blank/not-run.

## Rubric and evidence boundaries

Assess each original objective separately. Level1 arithmetic reproduction earns
no credit for the preload or measurement objective. Level2 requires its own
invariant, implementation/analysis, positive and discriminating rejection result,
and explanation of what changed assumptions invalidate. Level3 requires an
independent learner artifact after the original delay. Hints may progress from
counting states, to tracing active versus shadow, to an explicit uncertainty
ledger; record hints used. There is no learner-time ceiling. Alternative models
may be valid with explicit premises and an independently assessed mapping.
Native sanitizer success, finite enumeration, cross-link closure and structural
MMIO disassembly support only their stated bounds. They do not establish actual
electrical behavior, ISR latency, all compiler behavior or absolute mastery.
