# T28.06 timer implementation reconciliation

Origin: PR-T28-03, its three original criterion objects and three unchanged
seven-day variants. Historical `firmware_io.timer` calculates one edge-aligned
period with truncated duty; it does not cover active/shadow transfer, center
alignment, capture loss, two-channel publication or physical uncertainty.

This successor supplies a bounded C17 calculator/model, F439 TIM3 PWM/TIM4 capture
adapter, intentionally unfinished learner code, explicit component reuse,
independent rational/phase/event/capture oracle, compiled semantic controls,
separate assessor reasoning and a complete blank capture/analysis protocol.
The actual PR-T28-02 clock provider existed and passed its campaign before this
consumer was run. The full UART/serial module is linked as a reference dependency;
only `pu_rates` is claimed as a timer operation. `fw_timer` is compared under
equivalent integer-duty cases and an explicit differing-rounding counterexample.

Source evidence includes local RM0090 Rev21 text and rendered Figure165, current
DS9484 Rev14 alternate functions, and UM1974 Rev11 F439 connector/PA7 Ethernet
conflict. The RCC erratum passage is explicitly reused from the prior UART
inspection. No source inspection is inferred from the old bibliography.

The first probe run found an incorrect hand-entered expected row count (6936
waveform rows); the independently derived sum is5200. Row-by-row comparisons
already passed. The count was corrected to the explicit sum over ARR1..16,
not by reducing enumeration. A first executable-output path collided with an
existing build directory and was changed without deleting that directory.
Initial and final campaigns are retained separately when public/source edits
invalidate input identities. These are author defects, not external blockers.

Model and physical evidence differ: no physical clock, waveform, skew, ISR
latency, loading or calibration is claimed. T29 board startup/interrupt ownership
integration remains a required downstream implementation; current M4 images are
closure audits. Published ISO access, external technical review, independent
learner work and delayed performance remain separate. Remaining T28.07–.12 work
is not closed by this package.

The first public target audit rejected optimized valid timer addresses because
the shared decoder did not understand a full-width Thumb MOV immediate. A local
versioned decoder now handles that instruction and register moves. Unoptimized
address accesses are explicit volatile macros so actual device widths/addresses
remain visible in the object; no optimization cell or address requirement was
removed. The original frozen UART decoder and receipts were not edited.
