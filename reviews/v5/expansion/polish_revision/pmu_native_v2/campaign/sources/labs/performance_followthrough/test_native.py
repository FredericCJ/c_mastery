#!/usr/bin/env python3
"""Tests for the learner runner native.py: capability diagnostics with
injected errno values, provenance refusal, digit-only numbers and report
gate rechecking. No PMU access, compiler or author checkout is needed.

    python3 test_native.py
"""
from __future__ import annotations

import argparse
import errno
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import native  # noqa: E402

FACTS = {"perf_event_paranoid": "2", "seccomp": "0", "no_new_privs": "0",
         "lsm": "unconfined", "container": "none", "virtualization": "none"}


def probe(status="opened", error=None, sampling="opened"):
    group = {"status": status} if error is None else {"status": status, "errno": error}
    return {"counting_group": group, "sampling": {"status": sampling}}


class DoctorTests(unittest.TestCase):
    def reasons(self, report_probe, **facts):
        return native.explain(report_probe, dict(FACTS, **facts))

    def test_open_group_needs_nothing(self):
        self.assertEqual(self.reasons(probe()), [])
        self.assertEqual(native.access_state(probe(), []), "available")

    def test_paranoid_above_two_names_paranoid_and_cap_perfmon(self):
        text = " ".join(self.reasons(probe("denied", errno.EACCES), perf_event_paranoid="3"))
        self.assertIn("perf_event_paranoid=3", text)
        self.assertIn("CAP_PERFMON", text)

    def test_denial_with_permissive_paranoid_never_blames_paranoid(self):
        for paranoid in ("-1", "0", "1", "2"):
            text = " ".join(self.reasons(probe("denied", errno.EPERM), perf_event_paranoid=paranoid,
                                         seccomp="2", no_new_privs="1"))
            self.assertIn("seccomp", text)
            self.assertIn("NoNewPrivs", text)
            self.assertNotIn("lower", text)

    def test_denial_without_seccomp_names_lsm_or_container(self):
        text = " ".join(self.reasons(probe("denied", errno.EACCES), perf_event_paranoid="-1",
                                     lsm="docker-default (enforce)", container="docker"))
        self.assertIn("security module", text)
        self.assertIn("docker-default", text)
        self.assertIn("container docker", text)
        self.assertNotIn("lower", text)

    def test_enosys_is_named(self):
        text = " ".join(self.reasons(probe("error", errno.ENOSYS)))
        self.assertIn("ENOSYS", text)
        self.assertIn("CONFIG_PERF_EVENTS", text)

    def test_any_other_errno_is_named(self):
        reasons = self.reasons(probe("error", errno.EINVAL))
        self.assertTrue(reasons)
        self.assertIn("errno 22", reasons[0])
        self.assertEqual(native.access_state(probe("error", errno.EINVAL), reasons), "blocked")

    def test_unavailable_pmu_is_named(self):
        text = " ".join(self.reasons(probe("unavailable", errno.ENOENT), virtualization="kvm"))
        self.assertIn("kvm", text)

    def test_probe_that_did_not_run_is_named_and_access_unknown(self):
        for broken in ({"status": "not-run", "reason": "pmu_probe.c or a C compiler is missing"},
                       {"status": "build-error", "output": "cc: not found"},
                       {"status": "probe-error", "exit_code": 139, "output": ""}):
            reasons = native.explain(broken, FACTS)
            self.assertTrue(reasons, broken)
            self.assertIn("probe", reasons[0])
            self.assertEqual(native.access_state(broken, reasons), "unknown")

    def test_sampling_refusal_is_reported_when_counting_works(self):
        text = " ".join(self.reasons(probe(sampling="unavailable")))
        self.assertIn("sampling", text)

    def test_status_fields_are_recorded(self):
        facts = native.process_security("Name:\tx\nSeccomp:\t2\nSeccomp_filters:\t1\nNoNewPrivs:\t1\n")
        self.assertEqual(facts["seccomp"], "2")
        self.assertEqual(facts["no_new_privs"], "1")


class ProvenanceTests(unittest.TestCase):
    def test_assessor_component_is_detected_after_resolution(self):
        self.assertTrue(native.assessor_sources([Path("/x/assessor/performance_followthrough/pmu.c")]))
        self.assertTrue(native.assessor_sources([Path("labs/../assessor/algorithms/algo.c")]))
        self.assertFalse(native.assessor_sources([Path("/x/labs/performance_followthrough/pmu.c"),
                                                  Path("/x/assessors-notes/pmu.c")]))

    def test_assessor_source_needs_the_test_label(self):
        with self.assertRaises(SystemExit):
            native.require_label([Path("/x/assessor/pmu.c")], test_implementation=False)
        self.assertEqual(native.require_label([Path("/x/assessor/pmu.c")], test_implementation=True),
                         native.PROVENANCE_TEST)
        self.assertEqual(native.require_label([Path("/x/labs/pmu.c")], test_implementation=False),
                         native.PROVENANCE_LEARNER)


class DecimalTests(unittest.TestCase):
    def test_digits_only(self):
        self.assertEqual(native.decimal("4096"), 4096)
        for text in ("", " 4096", "4_096", "+2", "-1", "0x10", "1e3", "٣", "4096 "):
            with self.assertRaises(argparse.ArgumentTypeError, msg=repr(text)):
                native.decimal(text)


class ReportRecheckTests(unittest.TestCase):
    @staticmethod
    def run_document(elapsed):
        brackets = [1_000] * 31
        rows = [dict(pair=p, variant=v, elapsed_ns=elapsed, repetitions=8, status="measured", running=10,
                     enabled=10, cleanup_errno=0, instruction_id=1, cycle_id=2, checksum=5,
                     clock_resolution_ns=1, clock_brackets_ns=brackets)
                for p in range(31) for v in (0, 1)]
        trials = [dict(rows[0], elapsed_ns=300_000), dict(rows[1], elapsed_ns=300_000)]
        stored = dict(gates=dict(interpretation="supported"), reasons=[], pairs=31,
                      ranking=dict(median_paired_delta_ns=-5.0, v1_faster_pairs=31, pairs=31, limit="x"))
        return {"kind": "native-run", "provenance": "LEARNER", "request": dict(family="numeric", n=8, pairs=31),
                "calibration": dict(repetitions=8, target_ns=100_000,
                                    attempts=[dict(repetitions=8, trials=trials)]),
                "rows": rows, "evaluation": stored}

    def test_ranking_is_printed_only_after_rechecking_the_gates(self):
        text = native.describe(self.run_document(elapsed=50_000))  # rows below their 100 us target
        self.assertIn("ranking: none", text)
        self.assertIn("differs from the stored", text)

    def test_supported_document_keeps_its_ranking(self):
        text = native.describe(self.run_document(elapsed=300_000))
        self.assertIn("interpretation=supported", text)
        self.assertNotIn("ranking: none", text)


if __name__ == "__main__":
    unittest.main()
