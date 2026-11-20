## ADDED Requirements

### Requirement: Checked numeric inputs
Numeric command-line and environment inputs SHALL be accepted only when they
are non-empty ASCII decimal digits whose value lies within the requested range
in `uintmax_t`, before any narrowing.

#### Scenario: Value that would wrap when narrowed
- **WHEN** a 32-bit unsigned option receives "4294967297"
- **THEN** the request is rejected with a range error instead of becoming 1

#### Scenario: Empty precision request
- **WHEN** the sampling precision argument is an empty string
- **THEN** the request is rejected instead of becoming precision 0

### Requirement: Complete sample-loss accounting
A sampling observation SHALL report in-band loss records, hardware loss
records and the kernel loss count as separate fields, SHALL report the total
as unknown when the kernel count is unavailable, and SHALL be labelled
lossless only when the kernel count is known and zero and no hardware loss
was reported.

#### Scenario: Ring full until disable
- **WHEN** the ring stays full until the event is disabled and no loss record fits
- **THEN** the in-band total is reported as a lower bound and the kernel count supplies the total

#### Scenario: Kernel count unavailable
- **WHEN** the kernel rejects `PERF_FORMAT_LOST`
- **THEN** loss accounting is reported as unknown, never as zero

### Requirement: Gated timing interpretation
A paired comparison SHALL produce a ranking only when acquisition, scheduling,
timing adequacy and comparability all pass; otherwise it SHALL keep the raw
observations and report an inconclusive or refused interpretation with reasons.

#### Scenario: Exhausted calibration
- **WHEN** calibration reaches the repetition limit without reaching the timing target
- **THEN** the regime keeps its raw pairs, reports `inadequate-exhausted` timing and has no ranking
