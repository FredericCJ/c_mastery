"""Separate validity gates for paired timing and PMU comparisons.

Supplied course support, shared by the learner runner
(labs/performance_followthrough/native.py) and the author campaigns
(tools/run_native_pmu.py, tools/run_deadline_pilot.py, tools/analyze_native_pmu.py
through tools/measurement_gates.py), so both apply exactly the same rules and
constants. Standard library only.

A comparison is reported in five independent dimensions so that one failure
cannot hide behind another:

acquisition     every PMU row was measured, scheduled (0 < running <= enabled),
                read with distinct event IDs and cleaned up; for timer-only rows
                `not-applicable` unless a row reports a failure
scheduling      `full` (running == enabled everywhere) or `multiplexed`
timing          `adequate` only when the calibration reached its target AND
                every measured row ran at the calibrated repetitions and reached
                its own timing target; otherwise `inadequate` (or
                `inadequate-exhausted` when calibration ran out of repetitions)
comparability   complete pairs, one row per (pair, variant), and the same
                checksum (equal work) in every row
interpretation  `supported` only when every gate passes; `refused` when the
                data are invalid; otherwise `inconclusive`

Raw per-pair observations are always kept. Rankings (median paired
difference, which variant was faster) are produced only for supported
regimes, so an inadequately timed or multiplexed run cannot feed a ranking or
a competence claim.
"""
from __future__ import annotations

from collections import Counter
import math
import statistics

SOURCE = __file__

# Protocol constants, defined once for every runner.
REQUIRED_PAIRS = 31          # paired blocks per regime
MAX_REPETITIONS = 1024       # the collector's REPETITIONS bound
CALIBRATION_HEADROOM = 2     # calibrate to twice the target so run-to-run
                             # variation does not drop measured rows below it
MEASUREMENT_CFLAGS = ("-O2", "-g")  # optimization for every measured binary;
                                    # never combined with a sanitizer

# Timing target: the measured interval must dwarf what reading the clock costs
# and what the clock can resolve. The cost of a clock read is the upper
# quartile (nearest rank) of the back-to-back brackets the collector records:
# a preempted bracket measures the scheduler, not the clock, and with 31
# brackets up to 7 such outliers leave the estimate unchanged. 100x that cost
# keeps the clock under 1% of every accepted interval; 1000x the resolution
# keeps quantization under 0.1%; 50 us is a floor below which scheduler and
# frequency transitions dominate on a general-purpose host.
TIMING_FLOOR_NS = 50_000
OVERHEAD_FACTOR = 100
RESOLUTION_FACTOR = 1000
COST_QUANTILE = 0.75


def clock_cost_ns(brackets_ns):
    """Upper-quartile (nearest-rank) clock-read cost of `brackets_ns`."""
    ordered = sorted(brackets_ns)
    if not ordered or ordered[0] < 0:
        raise ValueError("clock brackets must be present and non-negative")
    return ordered[math.ceil(COST_QUANTILE * len(ordered)) - 1]


def timing_target_ns(*, resolution_ns, brackets_ns, floor_ns=TIMING_FLOOR_NS):
    """Smallest measured interval that the timing gate accepts on this clock."""
    if resolution_ns <= 0:
        raise ValueError("clock resolution must be positive")
    return max(floor_ns, OVERHEAD_FACTOR * clock_cost_ns(brackets_ns), RESOLUTION_FACTOR * resolution_ns)


def row_target_ns(row):
    """A row's own target: explicit `target_ns` (timer-only benches, whose
    clock is probed separately) or derived from the clock facts the
    collector recorded in that row."""
    if "target_ns" in row:
        return row["target_ns"]
    if "clock_brackets_ns" not in row or "clock_resolution_ns" not in row:
        raise ValueError("row has no timing target and no clock facts")
    return timing_target_ns(resolution_ns=row["clock_resolution_ns"], brackets_ns=row["clock_brackets_ns"])


def acquired(row):
    """A counting row that can enter calibration at all."""
    return row.get("status") == "measured" and row.get("running", 0) > 0


def calibration_state(attempts, *, max_repetitions, target_ns=None):
    """Summarize a calibration.

    `attempts`: [{repetitions, min_elapsed_ns[, target_ns]}] in the order they
    were run. Each attempt may carry its own target; otherwise `target_ns`
    applies to all of them. The last attempt decides. `max_repetitions` is the
    calibration limit, or None for a fixed workload that cannot be lengthened
    (inadequate, never exhausted).
    """
    if not attempts:
        raise ValueError("calibration needs at least one attempt")
    last = attempts[-1]
    target = last.get("target_ns", target_ns)
    if target is None:
        raise ValueError("calibration attempt has no timing target")
    sufficient = last["min_elapsed_ns"] >= target
    exhausted = (not sufficient and max_repetitions is not None
                 and last["repetitions"] >= max_repetitions)
    return dict(sufficient=sufficient, exhausted=exhausted, target_ns=target,
                repetitions=last["repetitions"], max_repetitions=max_repetitions,
                attempts=attempts)


def _attempt(repetitions, trials):
    return dict(repetitions=repetitions, min_elapsed_ns=min(t["elapsed_ns"] for t in trials),
                target_ns=max(row_target_ns(t) for t in trials), trials=trials)


def calibrate(measure, *, max_repetitions=MAX_REPETITIONS, headroom=CALIBRATION_HEADROOM):
    """The one calibration loop. `measure(repetitions)` returns one row per
    variant. Repetitions double from 1 until the shorter variant reaches
    `headroom` times its target or the limit is reached. A row that was not
    acquired stops calibration at once (`aborted: acquisition`)."""
    attempts = []
    repetitions = 1
    while True:
        trials = measure(repetitions)
        attempts.append(_attempt(repetitions, trials))
        if not all(acquired(t) for t in trials):
            state = calibration_state(attempts, max_repetitions=max_repetitions)
            return dict(state, sufficient=False, aborted="acquisition")
        if attempts[-1]["min_elapsed_ns"] >= headroom * attempts[-1]["target_ns"] or repetitions >= max_repetitions:
            return calibration_state(attempts, max_repetitions=max_repetitions)
        repetitions *= 2


def rebuild_calibration(attempts, *, max_repetitions=MAX_REPETITIONS):
    """Re-derive a recorded calibration from its raw trials with the current
    rules; stored targets and verdicts are never trusted."""
    return calibration_state([_attempt(a["repetitions"], a["trials"]) for a in attempts],
                             max_repetitions=max_repetitions)


def _acquisition(rows):
    reasons = []
    for r in rows:
        if r.get("status") != "measured":
            reasons.append(f"pair {r['pair']} variant {r['variant']}: status {r.get('status')}")
        elif not 0 < r["running"] <= r["enabled"]:
            reasons.append(f"pair {r['pair']} variant {r['variant']}: running/enabled {r['running']}/{r['enabled']}")
        elif r.get("cleanup_errno"):
            reasons.append(f"pair {r['pair']} variant {r['variant']}: cleanup errno {r['cleanup_errno']}")
        elif r.get("instruction_id") == r.get("cycle_id"):
            reasons.append(f"pair {r['pair']} variant {r['variant']}: duplicate event IDs")
    return reasons


def _timer_failures(rows):
    return [f"pair {r['pair']} variant {r['variant']}: status {r.get('status')}, valid {r.get('valid', True)}"
            for r in rows if r.get("status", "ok") not in ("ok", "measured") or not r.get("valid", True)]


def _timing(rows, calibration):
    if not calibration["sufficient"]:
        state = "inadequate-exhausted" if calibration.get("exhausted") else "inadequate"
        return state, [f"timing: calibration below the {calibration['target_ns']} ns target at "
                       f"{calibration['repetitions']} repetitions"]
    reasons = []
    other = [r for r in rows if r["repetitions"] != calibration["repetitions"]]
    if other:
        reasons.append(f"timing: {len(other)} of {len(rows)} rows ran at other than the calibrated "
                       f"{calibration['repetitions']} repetitions")
    below = [r for r in rows if r["elapsed_ns"] < row_target_ns(r)]
    if below:
        reasons.append(f"timing: {len(below)} of {len(rows)} rows below their own timing target")
    return ("inadequate" if reasons else "adequate"), reasons


def _comparability(rows, required_pairs):
    counts = Counter((r["pair"], r["variant"]) for r in rows)
    duplicates = sorted(key for key, count in counts.items() if count > 1)
    if duplicates:
        return "duplicate-rows", [f"duplicate (pair, variant) rows {duplicates[:5]}"]
    checksums = sorted({r["checksum"] for r in rows})
    if len(checksums) > 1:
        return "not-comparable", [f"{len(checksums)} different checksums across the regime's rows"]
    pairs = {}
    for r in rows:
        pairs.setdefault(r["pair"], set()).add(r["variant"])
    complete = [p for p, variants in pairs.items() if variants == {0, 1}]
    needed = max(1, required_pairs)
    if len(complete) < needed or len(complete) != len(pairs):
        return "incomplete-pairs", [f"{len(complete)} complete pairs of {needed} required"]
    return "comparable", []


def evaluate_regime(rows, calibration, *, required_pairs=REQUIRED_PAIRS, pmu=True):
    gates, reasons = {}, []
    if pmu:
        invalid = _acquisition(rows)
        gates["acquisition"] = "invalid" if invalid else "valid"
        reasons += invalid
        if invalid:
            gates["scheduling"] = "unknown"
        else:
            full = all(r["running"] == r["enabled"] for r in rows)
            gates["scheduling"] = "full" if full else "multiplexed"
            if not full:
                reasons.append("events were multiplexed; scaled counts and timing are not a clean comparison")
    else:
        failed = _timer_failures(rows)
        gates["acquisition"] = "invalid" if failed else "not-applicable"
        gates["scheduling"] = "not-applicable"
        reasons += failed

    gates["timing"], timing_reasons = _timing(rows, calibration)
    reasons += timing_reasons
    gates["comparability"], comparability_reasons = _comparability(rows, required_pairs)
    reasons += comparability_reasons

    if gates["acquisition"] == "invalid" or gates["comparability"] in ("not-comparable", "duplicate-rows"):
        gates["interpretation"] = "refused"
    elif reasons:
        gates["interpretation"] = "inconclusive"
    else:
        gates["interpretation"] = "supported"

    def per_rep(r):
        return r["elapsed_ns"] / r["repetitions"]

    paired = {}
    for r in rows:
        paired.setdefault(r["pair"], {})[r["variant"]] = r
    complete = sorted(p for p, v in paired.items() if set(v) == {0, 1})
    result = dict(gates=gates, reasons=reasons, pairs=len(complete),
                  raw_elapsed_ns_per_repetition={str(v): [per_rep(paired[p][v]) for p in complete]
                                                 for v in (0, 1)})
    if gates["interpretation"] == "supported" and complete:
        differences = [per_rep(paired[p][1]) - per_rep(paired[p][0]) for p in complete]
        result["ranking"] = dict(median_paired_delta_ns=statistics.median(differences),
                                 v1_faster_pairs=sum(d < 0 for d in differences),
                                 pairs=len(differences),
                                 limit="Descriptive for this host and build; no population, "
                                       "universal-ordering or WCET claim.")
    return result
