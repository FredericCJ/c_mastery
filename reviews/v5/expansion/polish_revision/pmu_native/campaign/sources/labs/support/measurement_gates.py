"""Separate validity gates for paired timing and PMU comparisons.

Supplied course support, shared by the learner runner
(labs/performance_followthrough/native.py) and the author campaigns
(tools/run_native_pmu.py, tools/run_deadline_pilot.py, tools/analyze_native_pmu.py
through tools/measurement_gates.py), so both apply exactly the same rules.
Standard library only.

A comparison is reported in five independent dimensions so that one failure
cannot hide behind another:

acquisition     every PMU row was measured, scheduled (0 < running <= enabled),
                read with distinct event IDs and cleaned up (`not-applicable`
                for timer-only workloads)
scheduling      `full` (running == enabled everywhere) or `multiplexed`
timing          calibration reached the timing target (`adequate`), or did not
                (`inadequate`, or `inadequate-exhausted` when the repetition
                limit ran out first)
comparability   complete pairs with equal checksums (equal work)
interpretation  `supported` only when every gate passes; `refused` when the
                data are invalid; otherwise `inconclusive`

Raw per-pair observations are always kept. Rankings (median paired
difference, which variant was faster) are produced only for supported
regimes, so an inadequately timed or multiplexed run cannot feed a ranking or
a competence claim.
"""
from __future__ import annotations

import statistics

SOURCE = __file__

# Timing target: the measured interval must dwarf what reading the clock costs
# and what the clock can resolve. 100x the worst observed clock-read bracket
# keeps the bracket under 1% of the interval; 1000x the resolution keeps
# quantization under 0.1%; 50 us is a floor below which scheduler and
# frequency transitions dominate on a general-purpose host.
TIMING_FLOOR_NS = 50_000
OVERHEAD_FACTOR = 100
RESOLUTION_FACTOR = 1000


def timing_target_ns(*, resolution_ns, overhead_max_ns, floor_ns=TIMING_FLOOR_NS):
    """Smallest measured interval that the timing gate accepts on this clock."""
    if resolution_ns <= 0 or overhead_max_ns < 0:
        raise ValueError("clock resolution must be positive and overhead non-negative")
    return max(floor_ns, OVERHEAD_FACTOR * overhead_max_ns, RESOLUTION_FACTOR * resolution_ns)


def calibration_state(attempts, *, max_repetitions, target_ns=None):
    """Summarize a calibration.

    `attempts`: [{repetitions, min_elapsed_ns[, target_ns]}] in the order they
    were run. Each attempt may carry its own target (clock overhead is measured
    afresh on every run); otherwise `target_ns` applies to all of them. The last
    attempt decides. `max_repetitions` is the calibration limit, or None for a
    fixed workload that cannot be lengthened (inadequate, never exhausted).
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


def evaluate_regime(rows, calibration, *, required_pairs, pmu=True):
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
        gates["acquisition"] = gates["scheduling"] = "not-applicable"

    if calibration["sufficient"]:
        gates["timing"] = "adequate"
    else:
        gates["timing"] = "inadequate-exhausted" if calibration.get("exhausted") else "inadequate"
        reasons.append(f"timing below the {calibration['target_ns']} ns target at "
                       f"{calibration['repetitions']} repetitions")

    pairs = {}
    for r in rows:
        pairs.setdefault(r["pair"], {})[r["variant"]] = r
    complete = {p: v for p, v in pairs.items() if set(v) == {0, 1}}
    unequal = [p for p, v in complete.items() if v[0]["checksum"] != v[1]["checksum"]]
    if unequal:
        gates["comparability"] = "not-comparable"
        reasons.append(f"unequal checksums in pairs {sorted(unequal)[:5]}")
    elif len(complete) < required_pairs or len(complete) != len(pairs):
        gates["comparability"] = "incomplete-pairs"
        reasons.append(f"{len(complete)} complete pairs of {required_pairs} required")
    else:
        gates["comparability"] = "comparable"

    if gates["acquisition"] == "invalid" or gates["comparability"] == "not-comparable":
        gates["interpretation"] = "refused"
    elif reasons:
        gates["interpretation"] = "inconclusive"
    else:
        gates["interpretation"] = "supported"

    def per_rep(r):
        return r["elapsed_ns"] / r["repetitions"]

    result = dict(gates=gates, reasons=reasons, pairs=len(complete),
                  raw_elapsed_ns_per_repetition={str(v): [per_rep(pairs[p][v]) for p in sorted(complete)]
                                                 for v in (0, 1)})
    if gates["interpretation"] == "supported":
        differences = [per_rep(v[1]) - per_rep(v[0]) for _, v in sorted(complete.items())]
        result["ranking"] = dict(median_paired_delta_ns=statistics.median(differences),
                                 v1_faster_pairs=sum(d < 0 for d in differences),
                                 pairs=len(differences),
                                 limit="Descriptive for this host and build; no population, "
                                       "universal-ordering or WCET claim.")
    return result
