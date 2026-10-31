"""Deterministic tests of run_dossier.py's pure functions: event encoding,
gate rows, placement premises, window coverage and interval arithmetic.
No hardware, compiler or perf binary is used. Run: python3 test_run_dossier.py
"""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_dossier as rd  # noqa: E402

FORMATS = {"event": "config:0-7,32-35", "umask": "config:8-15", "edge": "config:18",
           "inv": "config:23", "cmask": "config:24-31"}


def window(state="measured", raw=(100, 200), ids=(7, 8), running=50, enabled=50, elapsed=1000,
           start=10, end=1010, cleanup=0, cpu=2, involuntary=0):
    return {"state": state, "configs": [192, 118][:len(raw)], "ids": list(ids), "raw": list(raw),
            "enabled": enabled, "running": running, "error": 0, "cleanup_errno": cleanup,
            "failed_event": -1, "clock_ok": True, "start_ns": start, "end_ns": end, "elapsed_ns": elapsed,
            "cpu_start": cpu, "cpu_end": cpu, "switches_ok": True, "involuntary_switches": involuntary,
            "voluntary_switches": 0}


SINGLE = {"windows": ["window"], "measured_window": "window", "checksum": "checksum",
          "units": "n", "units_times_repetitions": True}
PAIRED = {"windows": ["thread0.window", "thread1.window"], "measured_window": "thread0.window",
          "checksum": "thread0.measured", "units": "window_iterations", "units_times_repetitions": False}


class EncodingTests(unittest.TestCase):
    def test_parse_encoding_fields(self):
        self.assertEqual(rd.parse_encoding("cpu/event=0x43,umask=2/"), {"event": 0x43, "umask": 2})
        self.assertEqual(rd.parse_encoding("cpu/event=0xc0/"), {"event": 0xC0})

    def test_parse_encoding_rejects_other_pmus_and_garbage(self):
        for text in ("amd_iommu_0/csource=0x11/", "cpu/event=zz/", "cpu/event=0x40", "", "cpu//"):
            with self.subTest(text=text), self.assertRaises(ValueError):
                rd.parse_encoding(text)

    def test_config_places_split_event_bits(self):
        self.assertEqual(rd.config_from_fields({"event": 0x43, "umask": 2}, FORMATS), 0x0243)
        self.assertEqual(rd.config_from_fields({"event": 0x1C7}, FORMATS), 0x1000000C7)
        self.assertEqual(rd.config_from_fields({"event": 0x60, "umask": 0xE8}, FORMATS), 0xE860)

    def test_config_rejects_unknown_field_and_overflow(self):
        with self.assertRaises(ValueError):
            rd.config_from_fields({"period": 1}, FORMATS)
        with self.assertRaises(ValueError):
            rd.config_from_fields({"umask": 0x100}, FORMATS)
        with self.assertRaises(ValueError):
            rd.config_from_fields({"event": 0x1000}, FORMATS)

    def test_expected_encoding_must_match(self):
        self.assertTrue(rd.same_encoding("event=0x43,umask=0x02", {"event": 0x43, "umask": 2}))
        self.assertFalse(rd.same_encoding("event=0x43,umask=0x01", {"event": 0x43, "umask": 2}))
        self.assertFalse(rd.same_encoding("event=0x43", {"event": 0x43, "umask": 2}))


class StatisticsTests(unittest.TestCase):
    def test_nearest_rank_quantiles(self):
        values = list(range(1, 32))
        self.assertEqual(rd.nearest_rank(values, 0.5), 16)
        self.assertEqual(rd.nearest_rank(values, 0.25), 8)
        self.assertEqual(rd.nearest_rank(values, 0.75), 24)
        self.assertEqual(rd.nearest_rank([5], 0.75), 5)

    def test_sign_test_interval_for_31_pairs(self):
        interval = rd.sign_test_interval(list(range(31)))
        self.assertEqual((interval["low"], interval["high"]), (9, 21))  # ranks 10 and 22
        self.assertAlmostEqual(interval["coverage"], 0.97059, places=4)

    def test_sign_test_interval_absent_for_tiny_samples(self):
        self.assertIsNone(rd.sign_test_interval([1, 2, 3, 4, 5])["low"])
        self.assertAlmostEqual(rd.sign_test_interval(list(range(6)))["coverage"], 0.96875)

    def test_describe(self):
        summary = rd.describe([3, 1, 2])
        self.assertEqual((summary["n"], summary["median"], summary["min"], summary["max"]), (3, 2, 1, 3))


class GateRowTests(unittest.TestCase):
    def test_single_window_row(self):
        driver = {"checksum": 99, "repetitions": 4, "n": 10, "clock_resolution_ns": 1,
                  "clock_brackets_ns": [1] * 31, "window": window()}
        row = rd.gate_row(SINGLE, driver)
        self.assertEqual((row["status"], row["instruction_id"], row["cycle_id"]), ("measured", 7, 8))
        self.assertEqual((row["elapsed_ns"], row["checksum"], row["repetitions"], row["units"]), (1000, 99, 4, 40))

    def test_refused_group_is_not_measured(self):
        driver = {"checksum": 99, "repetitions": 1, "n": 10, "clock_resolution_ns": 1,
                  "clock_brackets_ns": [1] * 31, "window": window(state="denied", raw=(), ids=())}
        row = rd.gate_row(SINGLE, driver)
        self.assertEqual(row["status"], "denied")
        self.assertIsNone(row["instruction_id"])

    def test_two_windows_take_the_worst_schedule_and_any_failure(self):
        driver = {"repetitions": 1, "window_iterations": 64, "clock_resolution_ns": 1,
                  "clock_brackets_ns": [1] * 31,
                  "thread0": {"active": True, "measured": 64, "window": window(running=50, enabled=50)},
                  "thread1": {"active": True, "measured": 64, "window": window(running=30, enabled=60)}}
        row = rd.gate_row(PAIRED, driver)
        self.assertEqual((row["running"], row["enabled"], row["checksum"], row["units"]), (30, 60, 64, 64))
        driver["thread1"]["window"] = window(state="never-scheduled", raw=(), ids=())
        self.assertEqual(rd.gate_row(PAIRED, driver)["status"], "peer:never-scheduled")

    def test_inactive_peer_is_ignored(self):
        driver = {"repetitions": 1, "window_iterations": 64, "clock_resolution_ns": 1,
                  "clock_brackets_ns": [1] * 31,
                  "thread0": {"active": True, "measured": 64, "window": window()},
                  "thread1": {"active": False, "measured": 0, "window": window(state="empty", raw=(), ids=())}}
        self.assertEqual(rd.gate_row(PAIRED, driver)["status"], "measured")

    def test_cleanup_error_of_any_window_is_kept(self):
        driver = {"repetitions": 1, "window_iterations": 64, "clock_resolution_ns": 1,
                  "clock_brackets_ns": [1] * 31,
                  "thread0": {"active": True, "measured": 64, "window": window()},
                  "thread1": {"active": True, "measured": 64, "window": window(cleanup=9)}}
        self.assertEqual(rd.gate_row(PAIRED, driver)["cleanup_errno"], 9)


class PlacementTests(unittest.TestCase):
    TOPOLOGY = {0: {"siblings": {0, 1}}, 1: {"siblings": {0, 1}}, 2: {"siblings": {2, 3}},
                3: {"siblings": {2, 3}}, 4: {"siblings": {4, 5}}}

    def test_relations(self):
        allowed = {0, 1, 2, 3, 4}
        self.assertTrue(rd.placement_holds({"cpus": [2, 3], "relation": "smt-siblings"}, self.TOPOLOGY, allowed))
        self.assertFalse(rd.placement_holds({"cpus": [2, 4], "relation": "smt-siblings"}, self.TOPOLOGY, allowed))
        self.assertTrue(rd.placement_holds({"cpus": [2, 4], "relation": "different-core"}, self.TOPOLOGY, allowed))
        self.assertFalse(rd.placement_holds({"cpus": [2, 3], "relation": "different-core"}, self.TOPOLOGY, allowed))
        self.assertTrue(rd.placement_holds({"cpus": [2], "relation": "single"}, self.TOPOLOGY, allowed))

    def test_disallowed_or_repeated_cpu_fails(self):
        self.assertFalse(rd.placement_holds({"cpus": [2, 4], "relation": "different-core"}, self.TOPOLOGY, {2}))
        self.assertFalse(rd.placement_holds({"cpus": [2, 2], "relation": "different-core"}, self.TOPOLOGY, {2}))
        with self.assertRaises(ValueError):
            rd.placement_holds({"cpus": [2, 4], "relation": "nearby"}, self.TOPOLOGY, {2, 4})


class CoverageTests(unittest.TestCase):
    def test_peer_activity_covers_window(self):
        measured = window(start=100, end=200)
        self.assertEqual(rd.coverage(measured, window(start=90, end=150), tail_end=260), 1.0)
        self.assertEqual(rd.coverage(measured, window(start=150, end=180), tail_end=190), 0.4)
        self.assertEqual(rd.coverage(measured, window(start=300, end=400), tail_end=400), 0.0)

    def test_zero_length_window_has_no_coverage(self):
        self.assertIsNone(rd.coverage(window(start=5, end=5), window(start=0, end=9), tail_end=9))


class CarryoverTests(unittest.TestCase):
    def test_rows_are_split_by_the_variant_that_ran_before(self):
        spec = {"row": SINGLE}
        regime = {"name": "r", "variants": ["a", "b"]}

        def record(index, variant, elapsed):
            driver = {"checksum": 1, "repetitions": 1, "n": 10, "clock_resolution_ns": 1,
                      "clock_brackets_ns": [1] * 31, "window": window(elapsed=elapsed, raw=(40, 20 * elapsed))}
            return {"index": index, "regime": "r", "group": "g", "phase": "pair", "variant": variant,
                    "variant_name": "ab"[variant], "driver": driver}

        records = [record(0, 0, 10), record(1, 1, 50), record(2, 1, 50), record(3, 0, 30),
                   record(4, 0, 10), record(5, 1, 50)]
        previous = {r["index"]: before for before, r in zip(records, records[1:])}
        split = rd.carryover(spec, regime, records, previous)
        self.assertEqual(split["a"]["after_other_row"]["rows"], 1)
        self.assertEqual(split["a"]["after_other_variant"]["elapsed_ns_per_unit"]["median"], 3)
        self.assertEqual(split["a"]["after_same_variant"]["elapsed_ns_per_unit"]["median"], 1)
        self.assertEqual(split["b"]["after_same_variant"]["rows"], 1)
        self.assertEqual(split["b"]["after_other_variant"]["cycles_per_unit"]["median"], 100)


class QuietTests(unittest.TestCase):
    def test_load_parser(self):
        self.assertEqual(rd.one_minute_load("1.08 0.76 0.35 3/790 3644952\n"), 1.08)
        with self.assertRaises(ValueError):
            rd.one_minute_load("")

    def test_busy_fraction(self):
        before = {"cpu2": [10, 0, 10, 80, 0, 0, 0, 0]}
        after = {"cpu2": [20, 0, 20, 160, 0, 0, 0, 0]}
        self.assertAlmostEqual(rd.busy_fractions(before, after)["cpu2"], 0.2)


if __name__ == "__main__":
    unittest.main()
