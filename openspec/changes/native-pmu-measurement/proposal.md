## Why

The native PMU collector narrowed command-line numbers before validating them,
counted only in-band sample-loss records after a final drain (so losses after
the last record that fitted were invisible and `lost: 0` meant "unknown"),
and let exhausted calibration feed paired rankings. Canonical obligations:
POLISH-PARSE, POLISH-LOSS, POLISH-CALIB and POLISH-REGRESS in
`research/v5/expansion/polish_revision/obligations.json`.

## What Changes

- Shared checked decimal parser `labs/support/cm_parse.{h,c}` with a specified
  grammar; all non-specimen command-line parsing uses it.
- `labs/performance_followthrough/pmu_loss.{h,c}`: read-format decoding with
  optional `PERF_FORMAT_LOST` and loss reconciliation that never sums in-band
  and kernel counts and reports an unavailable kernel count as unknown.
- The collector is split into parsing, workload, sampling, interpretation and
  report modules; sampling supports final and concurrent drains, bounded ring
  size and period, and honours the requested repetitions.
- `tools/measurement_gates.py`: separate acquisition, scheduling, timing,
  comparability and interpretation gates; rankings only for supported regimes.
- Sanitizer and thread-sanitizer profiles run where the documentation claims.

## Capabilities

### New Capabilities

- `native-pmu-measurement`: how native counting and sampling observations are
  acquired, validated, accounted and interpreted.

### Modified Capabilities

None.

## Impact

`labs/performance_followthrough/`, `labs/support/`, `tools/run_native_pmu.py`,
`tools/analyze_native_pmu.py`, `tools/run_deadline_pilot.py`,
`tools/public_validation.py`. Earlier PMU receipts are preserved unchanged; new
hardware evidence is recorded under `reviews/v5/expansion/polish_revision/`.
