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
import tempfile
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
    """Inside the checkout the refusal is anchored to the checkout's own
    assessor/ tree (by path, and by content for copies); in a learner
    workspace without assessor/ the label is self-asserted."""

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.assessor = self.root / "repo/assessor"
        (self.assessor / "performance_followthrough").mkdir(parents=True)
        self.reference = self.assessor / "performance_followthrough/pmu.c"
        self.reference.write_text("/* reference */\n")
        self.user = self.root / "home/assessor"  # a user's own directory named assessor
        self.user.mkdir(parents=True)
        (self.user / "pmu.c").write_text("/* my work */\n")
        self.copy = self.root / "home/copied.c"
        self.copy.write_text("/* reference */\n")

    def tearDown(self):
        self.temporary.cleanup()

    def test_checkout_assessor_path_is_detected(self):
        self.assertTrue(native.assessor_sources([self.reference], self.assessor))

    def test_user_directory_named_assessor_is_not_refused(self):
        self.assertFalse(native.assessor_sources([self.user / "pmu.c"], self.assessor))

    def test_copy_of_an_assessor_file_is_detected_by_content(self):
        self.assertTrue(native.assessor_sources([self.copy], self.assessor))

    def test_without_an_assessor_tree_the_label_is_self_asserted(self):
        self.assertFalse(native.assessor_sources([self.reference, self.copy], None))
        self.assertEqual(native.require_label([self.copy], test_implementation=False, assessor_root=None),
                         native.PROVENANCE_LEARNER)

    def test_assessor_source_needs_the_test_label(self):
        with self.assertRaises(SystemExit):
            native.require_label([self.copy], test_implementation=False, assessor_root=self.assessor)
        self.assertEqual(native.require_label([self.copy], test_implementation=True,
                                              assessor_root=self.assessor), native.PROVENANCE_TEST)

    def test_checkout_anchor_is_this_repository(self):
        repository = Path(native.__file__).resolve().parents[2]
        expected = repository / "assessor" if (repository / "assessor").is_dir() else None
        self.assertEqual(native.ASSESSOR_ROOT, expected)


class SampleRecheckTests(unittest.TestCase):
    """A stored sampling verdict is never trusted: lossless and the loss
    accounting are re-derived from the raw fields."""

    RAW = dict(kind="sampling", status="measured", stage="complete", parser_status=0, family="numeric", n=8,
               samples=100, drain_mode="final", data_pages=8, sample_period=200000, precise_ip=0, drains=1,
               inband_lost=0, hardware_lost=0, kernel_lost=0, lost_support="supported", throttled=False,
               throttle_records=0, unthrottle_records=0, unknown=0, records=100,
               loss_accounting="complete", lost_lower_bound=0, total_lost=0, unreported_lost=0, lossless=True)

    def document(self, **override):
        return {"kind": "native-sample", "provenance": "LEARNER", "summary": dict(self.RAW, **override)}

    def test_clean_observation_stays_lossless(self):
        text = native.describe(self.document())
        self.assertIn("loss: lossless", text)

    def test_throttled_observation_is_never_lossless(self):
        text = native.describe(self.document(throttle_records=5, unthrottle_records=5, unknown=10))
        self.assertNotIn("loss: lossless", text)
        self.assertIn("complete-except-throttled", text)
        self.assertIn("differs from the stored", text)

    def test_unknown_records_block_lossless(self):
        self.assertFalse(native.rederive_loss(dict(self.RAW, unknown=3))["lossless"])
        # throttle and unthrottle records are interpreted, not unknown
        self.assertFalse(native.rederive_loss(dict(self.RAW, throttle_records=1, unthrottle_records=1,
                                                   unknown=2))["lossless"])

    def test_nothing_measured_claims_nothing(self):
        for override in (dict(status="no-samples", samples=0), dict(status="invalid", stage="read")):
            loss = native.rederive_loss(dict(self.RAW, **override))
            self.assertFalse(loss["lossless"], override)

    def test_unparsed_ring_is_unknown_with_kernel_lower_bound(self):
        loss = native.rederive_loss(dict(self.RAW, status="invalid", stage="parser", parser_status=2,
                                         kernel_lost=598))
        self.assertEqual(loss["accounting"], "unknown")
        self.assertEqual(loss["lower_bound"], 598)
        self.assertFalse(loss["lossless"])

    def test_missing_kernel_count_is_unknown(self):
        loss = native.rederive_loss(dict(self.RAW, kernel_lost=None, inband_lost=4))
        self.assertEqual(loss["accounting"], "unknown")
        self.assertEqual(loss["lower_bound"], 4)

    def test_documents_without_raw_loss_fields_are_refused(self):
        for missing in ("throttled", "stage"):
            summary = dict(self.RAW)
            del summary[missing]
            with self.assertRaises(ValueError):
                native.describe({"kind": "native-sample", "provenance": "LEARNER", "summary": summary})


class SharedConstantTests(unittest.TestCase):
    def test_pairs_default_is_the_shared_constant(self):
        args = native.parser().parse_args(["run", "--build", "b", "--family", "numeric", "--n", "8",
                                           "--report", "r"])
        self.assertEqual(args.pairs, native.load_gates().REQUIRED_PAIRS)


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
