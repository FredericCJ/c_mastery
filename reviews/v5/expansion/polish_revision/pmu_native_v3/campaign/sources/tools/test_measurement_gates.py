"""Tests for the measurement gates (labs/support/measurement_gates.py, re-exported
by tools/measurement_gates.py) and for how tools/analyze_native_pmu.py applies
them to receipts. Run: python3 tools/test_measurement_gates.py"""
from pathlib import Path
import re
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import measurement_gates as gates  # noqa: E402

FAST_CLOCK = [1_000] * 31  # 1 us brackets: target max(50 us, 100 us, 1 us) = 100 us


def row(pair, variant, elapsed=1_000_000, running=100, enabled=100, status="measured",
        checksum=42, cleanup=0, repetitions=8, ids=(1, 2), brackets=FAST_CLOCK):
    return dict(pair=pair, variant=variant, elapsed_ns=elapsed, running=running, enabled=enabled,
                status=status, checksum=checksum, cleanup_errno=cleanup, repetitions=repetitions,
                instruction_id=ids[0], cycle_id=ids[1], clock_resolution_ns=1,
                clock_brackets_ns=list(brackets), clock_overhead_max_ns=max(brackets))


def complete(pairs=31, **kwargs):
    rows = []
    for pair in range(pairs):
        rows.append(row(pair, 0, elapsed=2_000_000, **kwargs))
        rows.append(row(pair, 1, elapsed=1_000_000, **kwargs))
    return rows


ADEQUATE = dict(sufficient=True, exhausted=False, target_ns=100_000, repetitions=8)


class ClockCostTests(unittest.TestCase):
    """The clock-read cost is the upper quartile (nearest rank) of the
    brackets: robust to up to 7 preempted brackets out of 31."""

    def test_seven_outliers_do_not_move_the_cost(self):
        self.assertEqual(gates.clock_cost_ns([1_000] * 24 + [4_000_000] * 7), 1_000)

    def test_eight_outliers_do(self):
        self.assertEqual(gates.clock_cost_ns([1_000] * 23 + [4_000_000] * 8), 4_000_000)

    def test_order_does_not_matter(self):
        brackets = [4_000_000] * 7 + [1_500] * 12 + [1_400] * 12
        self.assertEqual(gates.clock_cost_ns(brackets), 1_500)

    def test_no_brackets_is_an_error(self):
        with self.assertRaises(ValueError):
            gates.clock_cost_ns([])


class TimingTargetTests(unittest.TestCase):
    def test_floor_applies_to_fast_clocks(self):
        self.assertEqual(gates.timing_target_ns(resolution_ns=1, brackets_ns=[20] * 31), 50_000)

    def test_cost_and_resolution_raise_the_target(self):
        self.assertEqual(gates.timing_target_ns(resolution_ns=1, brackets_ns=FAST_CLOCK), 100_000)
        self.assertEqual(gates.timing_target_ns(resolution_ns=4_000, brackets_ns=[20] * 31), 4_000_000)

    def test_one_preempted_bracket_does_not_inflate_the_target(self):
        # Recorded hpet host: brackets ~1.5-1.9 us with one 39.9 us outlier.
        brackets = [1_466] * 20 + [1_886] * 10 + [39_949]
        self.assertEqual(gates.timing_target_ns(resolution_ns=1, brackets_ns=brackets), 188_600)

    def test_rejects_missing_clock_facts(self):
        with self.assertRaises(ValueError):
            gates.timing_target_ns(resolution_ns=0, brackets_ns=[20])
        with self.assertRaises(ValueError):
            gates.timing_target_ns(resolution_ns=1, brackets_ns=[-1])

    def test_row_target_prefers_an_explicit_target(self):
        self.assertEqual(gates.row_target_ns(dict(target_ns=7)), 7)
        self.assertEqual(gates.row_target_ns(row(0, 0)), 100_000)
        with self.assertRaises(ValueError):
            gates.row_target_ns(dict(elapsed_ns=5))


class CalibrationStateTests(unittest.TestCase):
    def test_reaching_target_is_adequate(self):
        state = gates.calibration_state([dict(repetitions=1, min_elapsed_ns=10_000),
                                         dict(repetitions=2, min_elapsed_ns=60_000)],
                                        target_ns=50_000, max_repetitions=1024)
        self.assertTrue(state["sufficient"])
        self.assertFalse(state["exhausted"])
        self.assertEqual(state["repetitions"], 2)

    def test_exhausted_calibration_is_marked(self):
        state = gates.calibration_state([dict(repetitions=512, min_elapsed_ns=10_000),
                                         dict(repetitions=1024, min_elapsed_ns=20_000)],
                                        target_ns=50_000, max_repetitions=1024)
        self.assertFalse(state["sufficient"])
        self.assertTrue(state["exhausted"])

    def test_each_attempt_is_judged_against_its_own_target(self):
        state = gates.calibration_state([dict(repetitions=1, min_elapsed_ns=60_000, target_ns=50_000),
                                         dict(repetitions=2, min_elapsed_ns=90_000, target_ns=120_000)],
                                        max_repetitions=1024)
        self.assertFalse(state["sufficient"])
        self.assertEqual(state["target_ns"], 120_000)

    def test_fixed_workload_is_inadequate_not_exhausted(self):
        state = gates.calibration_state([dict(repetitions=1, min_elapsed_ns=10_000, target_ns=50_000)],
                                        max_repetitions=None)
        self.assertFalse(state["sufficient"])
        self.assertFalse(state["exhausted"])

    def test_calibration_without_attempts_is_rejected(self):
        with self.assertRaises(ValueError):
            gates.calibration_state([], target_ns=50_000, max_repetitions=1024)


class CalibrateLoopTests(unittest.TestCase):
    """One calibration loop for every runner: double the repetitions until the
    shorter variant reaches HEADROOM times its target."""

    @staticmethod
    def measure_linear(ns_per_repetition, status="measured", log=None):
        def measure(repetitions):
            if log is not None:
                log.append(repetitions)
            return [row(0, v, elapsed=ns_per_repetition * repetitions, repetitions=repetitions, status=status)
                    for v in (0, 1)]
        return measure

    def test_doubles_until_headroom_is_reached(self):
        log = []
        state = gates.calibrate(self.measure_linear(10_000, log=log))
        # target 100 us, headroom 2: 200 us needs 20 repetitions -> 32
        self.assertEqual(log, [1, 2, 4, 8, 16, 32])
        self.assertTrue(state["sufficient"])
        self.assertEqual(state["repetitions"], 32)
        self.assertEqual(state["attempts"][-1]["target_ns"], 100_000)

    def test_stops_at_the_repetition_limit(self):
        state = gates.calibrate(self.measure_linear(10))
        self.assertEqual(state["repetitions"], gates.MAX_REPETITIONS)
        self.assertTrue(state["exhausted"])

    def test_stops_at_once_on_failed_acquisition(self):
        log = []
        state = gates.calibrate(self.measure_linear(10, status="denied", log=log))
        self.assertEqual(log, [1])
        self.assertEqual(state["aborted"], "acquisition")
        self.assertFalse(state["sufficient"])

    def test_rebuild_rederives_targets_from_trials(self):
        trials = [row(0, v, elapsed=150_000, repetitions=4, brackets=[1_000] * 24 + [10**7] * 7) for v in (0, 1)]
        state = gates.rebuild_calibration([dict(repetitions=4, target_ns=10**9, trials=trials)])
        self.assertEqual(state["target_ns"], 100_000)
        self.assertTrue(state["sufficient"])


class SharedDefinitionTests(unittest.TestCase):
    def test_tools_and_learner_runner_share_one_definition(self):
        support = Path(__file__).resolve().parents[1] / "labs/support/measurement_gates.py"
        self.assertTrue(support.is_file())
        self.assertEqual(Path(gates.SOURCE).resolve(), support.resolve())

    def test_protocol_constants_are_defined_once(self):
        self.assertEqual(gates.REQUIRED_PAIRS, 31)
        self.assertEqual(gates.MAX_REPETITIONS, 1024)
        self.assertEqual(tuple(gates.MEASUREMENT_CFLAGS), ("-O2", "-g"))

    def test_collector_repetition_bound_matches(self):
        header = (Path(__file__).resolve().parents[1] / "labs/performance_followthrough/pmu_workloads.h").read_text()
        match = re.search(r"#define PF_WORKLOAD_MAX_REPETITIONS (\d+)u", header)
        self.assertIsNotNone(match)
        self.assertEqual(int(match.group(1)), gates.MAX_REPETITIONS)


class RegimeGateTests(unittest.TestCase):
    def test_supported_regime_has_ranking(self):
        result = gates.evaluate_regime(complete(), ADEQUATE, required_pairs=31)
        self.assertEqual(result["gates"]["interpretation"], "supported")
        self.assertIn("ranking", result)
        self.assertEqual(result["ranking"]["v1_faster_pairs"], 31)

    def test_exhausted_calibration_keeps_raw_but_drops_ranking(self):
        calibration = dict(sufficient=False, exhausted=True, target_ns=50_000, repetitions=8)
        result = gates.evaluate_regime(complete(), calibration, required_pairs=31)
        self.assertEqual(result["gates"]["timing"], "inadequate-exhausted")
        self.assertEqual(result["gates"]["interpretation"], "inconclusive")
        self.assertNotIn("ranking", result)
        self.assertIn("timing", " ".join(result["reasons"]))
        self.assertEqual(len(result["raw_elapsed_ns_per_repetition"]["0"]), 31)

    def test_rows_below_their_own_target_make_timing_inadequate(self):
        rows = complete()
        for r in rows[:20]:  # recorded case: dispatch 8192 had 20 of 62 rows below target
            r["elapsed_ns"] = 90_000
        result = gates.evaluate_regime(rows, ADEQUATE, required_pairs=31)
        self.assertEqual(result["gates"]["timing"], "inadequate")
        self.assertEqual(result["gates"]["interpretation"], "inconclusive")
        self.assertNotIn("ranking", result)
        self.assertIn("20 of 62 rows below", " ".join(result["reasons"]))

    def test_rows_at_other_repetitions_make_timing_inadequate(self):
        # 10 ns rows at 1 repetition against a calibration adequate at 1024.
        calibration = dict(sufficient=True, exhausted=False, target_ns=100_000, repetitions=1024)
        rows = complete(repetitions=1)
        for r in rows:
            r["elapsed_ns"] = 10
        result = gates.evaluate_regime(rows, calibration, required_pairs=31)
        self.assertEqual(result["gates"]["timing"], "inadequate")
        self.assertNotIn("ranking", result)

    def test_rows_at_calibrated_repetitions_but_short_are_inadequate(self):
        calibration = dict(sufficient=True, exhausted=False, target_ns=100_000, repetitions=8)
        result = gates.evaluate_regime(complete(), calibration, required_pairs=31)
        self.assertEqual(result["gates"]["timing"], "adequate")
        short = complete()
        short[7]["elapsed_ns"] = 99_999
        self.assertEqual(gates.evaluate_regime(short, calibration, required_pairs=31)["gates"]["timing"],
                         "inadequate")

    def test_never_scheduled_row_refuses_interpretation(self):
        rows = complete()
        rows[5]["running"] = 0
        result = gates.evaluate_regime(rows, ADEQUATE, required_pairs=31)
        self.assertEqual(result["gates"]["acquisition"], "invalid")
        self.assertEqual(result["gates"]["interpretation"], "refused")
        self.assertNotIn("ranking", result)

    def test_multiplexing_is_reported_not_hidden(self):
        rows = complete(running=50, enabled=100)
        result = gates.evaluate_regime(rows, ADEQUATE, required_pairs=31)
        self.assertEqual(result["gates"]["scheduling"], "multiplexed")
        self.assertEqual(result["gates"]["interpretation"], "inconclusive")

    def test_unequal_work_within_a_pair_is_not_comparable(self):
        rows = complete()
        rows[1]["checksum"] = 43
        result = gates.evaluate_regime(rows, ADEQUATE, required_pairs=31)
        self.assertEqual(result["gates"]["comparability"], "not-comparable")
        self.assertEqual(result["gates"]["interpretation"], "refused")

    def test_unequal_work_across_pairs_is_not_comparable(self):
        rows = complete()
        rows[10]["checksum"] = rows[11]["checksum"] = 43  # pair 5 agrees with itself only
        result = gates.evaluate_regime(rows, ADEQUATE, required_pairs=31)
        self.assertEqual(result["gates"]["comparability"], "not-comparable")
        self.assertEqual(result["gates"]["interpretation"], "refused")

    def test_duplicate_rows_are_refused(self):
        rows = complete() + [row(3, 1)]  # 63 rows
        result = gates.evaluate_regime(rows, ADEQUATE, required_pairs=31)
        self.assertEqual(result["gates"]["comparability"], "duplicate-rows")
        self.assertEqual(result["gates"]["interpretation"], "refused")
        self.assertNotIn("ranking", result)

    def test_incomplete_pairs_are_inconclusive(self):
        result = gates.evaluate_regime(complete(pairs=5), ADEQUATE, required_pairs=31)
        self.assertEqual(result["gates"]["comparability"], "incomplete-pairs")
        self.assertEqual(result["gates"]["interpretation"], "inconclusive")
        self.assertNotIn("ranking", result)

    def test_no_rows_never_ranks_or_raises(self):
        result = gates.evaluate_regime([], ADEQUATE, required_pairs=0)
        self.assertEqual(result["gates"]["comparability"], "incomplete-pairs")
        self.assertNotIn("ranking", result)

    def test_cleanup_error_invalidates_acquisition(self):
        rows = complete()
        rows[0]["cleanup_errno"] = 9
        result = gates.evaluate_regime(rows, ADEQUATE, required_pairs=31)
        self.assertEqual(result["gates"]["acquisition"], "invalid")

    @staticmethod
    def timer_rows(**override):
        return [dict(dict(pair=p, variant=v, elapsed_ns=200_000 * (2 - v), checksum=7, repetitions=1,
                          target_ns=100_000), **override)
                for p in range(31) for v in (0, 1)]

    def test_timer_only_rows_skip_pmu_gates(self):
        calibration = dict(ADEQUATE, repetitions=1)
        result = gates.evaluate_regime(self.timer_rows(), calibration, required_pairs=31, pmu=False)
        self.assertEqual(result["gates"]["acquisition"], "not-applicable")
        self.assertEqual(result["gates"]["interpretation"], "supported")

    def test_failed_timer_only_rows_are_refused(self):
        calibration = dict(ADEQUATE, repetitions=1)
        rows = self.timer_rows()
        rows[4]["status"] = "failed"
        result = gates.evaluate_regime(rows, calibration, required_pairs=31, pmu=False)
        self.assertEqual(result["gates"]["acquisition"], "invalid")
        self.assertEqual(result["gates"]["interpretation"], "refused")
        rows = self.timer_rows()
        rows[9]["valid"] = False
        result = gates.evaluate_regime(rows, calibration, required_pairs=31, pmu=False)
        self.assertEqual(result["gates"]["interpretation"], "refused")


class AnalyzerCalibrationTests(unittest.TestCase):
    """tools/analyze_native_pmu.py re-derives every calibration from its raw
    trials with the current rules; it never trusts a stored verdict."""

    @staticmethod
    def analyzer():
        import analyze_native_pmu
        return analyze_native_pmu

    @staticmethod
    def observations(family, n, repetitions, elapsed=(2_000_000, 1_000_000)):
        rows = complete(repetitions=repetitions)
        for r in rows:
            r["elapsed_ns"] = elapsed[r["variant"]]
        return [dict(r, kind="count", family=family, n=n, cpu_before=1, cpu_after=1,
                     raw_cycles=10, raw_instructions=20) for r in rows]

    def test_schema1_exhausted_calibration_loses_its_ranking(self):
        trials = [row(0, v, elapsed=40_000, repetitions=1024) for v in (0, 1)]
        data = {"calibration": [dict(family="layout", n=256, repetitions=512, target_ns=195_600,
                                     sufficient=False, trials=trials),
                                dict(family="layout", n=256, repetitions=1024, target_ns=195_600,
                                     sufficient=False, trials=trials)],
                "observations": self.observations("layout", 256, 1024)}
        state = self.analyzer().calibration_for(data, "layout", 256)
        self.assertTrue(state["exhausted"])
        summary = self.analyzer().summarize(data)[0]
        self.assertEqual(summary["gates"]["timing"], "inadequate-exhausted")
        self.assertEqual(summary["gates"]["interpretation"], "inconclusive")
        self.assertNotIn("ranking", summary)
        self.assertNotIn("descriptive", summary)

    def test_schema2_stored_verdict_is_rederived_from_attempts(self):
        # The stored state claims sufficiency; the raw trials do not support it.
        trials = [row(0, v, elapsed=60_000, repetitions=8) for v in (0, 1)]
        stored = dict(sufficient=True, exhausted=False, target_ns=50_000, repetitions=8, max_repetitions=1024,
                      attempts=[dict(repetitions=8, min_elapsed_ns=60_000, target_ns=50_000, trials=trials)])
        data = {"calibration": [dict(family="numeric", n=8192, **stored)],
                "observations": self.observations("numeric", 8192, 8)}
        state = self.analyzer().calibration_for(data, "numeric", 8192)
        self.assertFalse(state["sufficient"])
        self.assertEqual(state["target_ns"], 100_000)
        summary = self.analyzer().summarize(data)[0]
        self.assertEqual(summary["gates"]["timing"], "inadequate")
        self.assertNotIn("ranking", summary)

    def test_blocked_hardware_calibration_without_trials_is_readable(self):
        # What the sweep records when the capability probe is not measured.
        stored = gates.calibration_state([dict(repetitions=8, min_elapsed_ns=0, target_ns=1)],
                                         max_repetitions=None)
        rows = [dict(r, status="denied", running=0, enabled=0)
                for r in self.observations("numeric", 256, 8)[:2]]
        data = {"calibration": [dict(family="numeric", n=256, **stored)], "observations": rows}
        state = self.analyzer().calibration_for(data, "numeric", 256)
        self.assertFalse(state["sufficient"])
        self.assertFalse(state["rederived"])
        summary = self.analyzer().summarize(data)[0]
        self.assertEqual(summary["gates"]["acquisition"], "invalid")
        self.assertEqual(summary["gates"]["interpretation"], "refused")
        self.assertNotIn("ranking", summary)

    def test_schema2_supported_regime_is_ranked(self):
        trials = [row(0, v, elapsed=300_000, repetitions=8) for v in (0, 1)]
        stored = dict(attempts=[dict(repetitions=8, min_elapsed_ns=300_000, target_ns=100_000, trials=trials)])
        data = {"calibration": [dict(family="numeric", n=8192, **stored)],
                "observations": self.observations("numeric", 8192, 8)}
        summary = self.analyzer().summarize(data)[0]
        self.assertEqual(summary["gates"]["interpretation"], "supported")
        self.assertIn("ranking", summary)
        self.assertIn("descriptive", summary)


if __name__ == "__main__":
    unittest.main()
