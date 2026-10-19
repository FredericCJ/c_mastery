# Assessor analysis: changed serial contracts

The nine-bit counterexample is M1/PCE1: payload8 plus parity1 is the same nine
hardware positions as M1/PCE0 payload9. LBCL removes the ninth clock pair in
both. A mutant that appends a parity clock must fail despite producing an
otherwise plausible frame. The required capture premise is stable data around
the selected CK edge; identifying CPHA alone cannot establish setup/hold.

For80MHz/115200, q2778 gives integer43/fraction26. Exact error magnitude is
25600/320025600, about79.994 ppm. The79/80 admission pair checks the rational
boundary without rounded integer baud. Fraction-mask15 is a real encoding bug.
Neither this calculation nor an emulator establishes board clock accuracy.

At−120 mV the characterized−130/−100 Schmitt example keeps either prior low or
high. The public datasheet's full guarantee envelope alone returns unknown;
typical thresholds do not select a device-specific history. Zero differential
is above the guaranteed positive threshold maximum and therefore failsafe high
under the declared powered/connected premise, unlike the earlier symmetric
unknown zone. Physical hysteresis and propagation remain unobserved.

Generation rejection must precede cn_feed. Reset to a newer identity abandons
old input deliberately; returning MORE on a stale chunk is wrong even if the
next complete frame happens to decode. Inspect state, consumed and output.
Counterexamples at UINT32_MAX reject restart1 without erasing the high-water
identity. The actual imported decoder is not copied into this adapter.

The selected ISO2024 rules widen receiver admission relative to the original
strict local codec; they do not assert that every difference is historically
new. In particular final EOF has distinct receiver/transmitter validation.
Protocol exception leaves error counters alone; a separate classified receive
error moves REC127 to128 and enters passive under the actual fault model.
The decision assumes prior CRC/stuffing/fixed-tail validation, and does not
manufacture a conforming full MAC from six flags.

32-bit mask IDE/RTR bits prevent an extended or remote frame from aliasing a
standard-data predicate. List mode's second word is another identifier rather
than a mask, so its capacity argument differs. The two old-versus-new queue
histories produce identical changed observations but incompatible requested
old outputs: missing information proves impossibility without an extra copy or
service bound. NART is independently executed by the frozen controller package.

Assess the exact original variant and its prerequisites, independent prediction,
implementation, intended mutant rejection and limits. A rehearsed reference or
author campaign is not an independent seven-day learner result. Public stubs
must compile and fail behaviorally; no reference source is exported.
