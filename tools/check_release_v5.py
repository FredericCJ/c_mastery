"""Fifth-edition release guards: the v4 guard suite bound to reviews/v5 and the v5 entry points.

The sandboxed cases exercise accept_v5.pinned_receipt, package_v5 promotion and
the v5 campaign wrappers; no real receipt, archive or historical file changes.
"""
import unittest
from pathlib import Path

import accept_v4
import accept_v5
import package_v4
import package_v5
import run_legacy_matrix_v4
import run_legacy_matrix_v5
import run_targets_v4
import run_targets_v5
from check_release_v4 import ReleaseGuards
from check_receipt_preservation import ReceiptPreservationGuards
from check_candidate_author_environment import CandidateEnvironmentGuards
from v5_common import RECEIPTS


class FifthEditionGuards(ReleaseGuards):
    RECEIPTS = RECEIPTS
    MODULES = (accept_v4, package_v4, run_targets_v4, run_legacy_matrix_v4,
               accept_v5, package_v5, run_targets_v5, run_legacy_matrix_v5)
    ACCEPT = accept_v5
    PACKAGE = package_v5
    TARGETS = run_targets_v5
    LEGACY = run_legacy_matrix_v5

    def test_fifth_edition_archives_are_named_by_the_manifest(self):
        self.assertEqual(len(package_v5.ARCHIVES), 4)
        self.assertTrue(all(name.startswith('c_mastery_v5_') for name in package_v5.ARCHIVES))

    def test_source_archive_keeps_scoped_package_evidence(self):
        import zipfile
        keep = ['reviews/v5/expansion/legacy_network_driver/package_validation.json',
                'reviews/v5/expansion/source_reconciliation/package_guard.json',
                'reviews/v4/package_report.json', 'planning/package_requirements.md']
        omit = [self.RECEIPTS + '/package_report.json', self.RECEIPTS + '/package_build.log',
                'reviews/private/package_capture.txt', 'reviews/__pycache__/package_cache.pyc']
        for name in keep + omit:
            self.write(name, 'synthetic packaging fixture\n')
        files = package_v4.source_files({'files': {'curriculum/example.md': 'unused'}}, self.RECEIPTS)
        archive = self.candidate / 'sources.zip'
        package_v4.write_zip(archive, files, self.root)
        with zipfile.ZipFile(archive) as bundle:
            actual = set(bundle.namelist())
            self.assertTrue(set(keep) <= actual)
            self.assertFalse(set(omit) & actual)
            self.assertEqual(bundle.read(keep[0]), b'synthetic packaging fixture\n')

    def test_review_candidate_extraction_rejects_escape(self):
        from check_candidate_exports_v5 import verify_members
        verify_members(['curriculum/example.md', 'reviews/provider/package_validation.json'], self.candidate)
        for member in ('../outside.md', str((self.sandbox/'outside.md').resolve())):
            with self.assertRaises(ValueError):
                verify_members([member], self.candidate)

    def test_review_candidate_checks_all_regenerated_sources(self):
        import hashlib
        import json
        from check_candidate_exports_v5 import verify_identity
        def identity(files):
            return {'files': files, 'sha256': hashlib.sha256(json.dumps(files, sort_keys=True).encode()).hexdigest()}
        # These are canonical source paths outside the release FIXED_POINT list.
        expected = identity({'labs/sample/src/target.c': 'original', 'research/sample/contract.json': 'original'})
        verify_identity(expected, identity(dict(expected['files'])))
        for changed in ({**expected['files'], 'labs/sample/src/target.c': 'regenerated'},
                        {**expected['files'], 'labs/sample/src/unexpected.c': 'new'},
                        {'labs/sample/src/target.c': 'original'}):
            with self.assertRaisesRegex(ValueError, 'canonical source identity differs'):
                verify_identity(expected, identity(changed))
        with self.assertRaises(ValueError):
            verify_identity(expected, {**expected, 'sha256': 'wrong aggregate'})

    def test_review_candidate_hashes_without_reading_whole_file(self):
        import hashlib
        from unittest.mock import patch
        from check_candidate_exports_v5 import sha
        path = self.root/'large-archive.zip'
        payload = b'bounded archive hash fixture\0' * 4096
        path.write_bytes(payload)
        with patch.object(Path, 'read_bytes', side_effect=AssertionError('Unbounded read')):
            self.assertEqual(sha(path), hashlib.sha256(payload).hexdigest())

    def test_review_candidate_rejects_redirected_export_source(self):
        from check_candidate_exports_v5 import verify_export_files
        ordinary = self.root/'ordinary.c'
        ordinary.write_text('synthetic source\n')
        verify_export_files([ordinary], self.root)
        with self.assertRaises(ValueError):
            verify_export_files([self.root/'missing.c'], self.root)
        with self.assertRaises(ValueError):
            verify_export_files([self.root], self.root)
        outside = self.sandbox/'outside.c'
        outside.write_text('synthetic assessor fixture\n')
        with self.assertRaises(ValueError):
            verify_export_files([outside], self.root)
        redirected = self.root/'redirected.c'
        try:
            redirected.symlink_to(outside)
        except (OSError, NotImplementedError):
            self.skipTest('Creating symlinks is unavailable')
        with self.assertRaises(ValueError):
            verify_export_files([redirected], self.root)

    def test_author_recipes_and_answer_cards_are_source_bundle_only(self):
        for name in ('research/v5/expansion/legacy_depth/author.py',
                     'research/v5/expansion/legacy_diagnostics/T46.cards',
                     'research/v5/modern/author_adt.py',
                     'research/v5/modern/adt_cards.json',
                     'research/v5/expansion/legacy_depth/validation.json'):
            self.assertFalse(package_v5.learner_file(Path(name)), name)
        for name in ('research/v5/expansion/legacy_depth/resources.json',
                     'research/v5/modern/adt_resources.json',
                     'research/enhancement/networking_contracts.json',
                     'curriculum/v5/expansion/legacy_assessments/PR-T01-01.json',
                     'labs/api_depth/src/api_depth.c'):
            self.assertTrue(package_v5.learner_file(Path(name)), name)

    def test_actual_exercise_source_registers_remain_in_learner_bundle(self):
        from common import ROOT, read_json
        records = read_json(ROOT/'curriculum/v5/exercises.json')['exercises']
        records += [read_json(p) for p in
                    (ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-*.json')]
        for record in records:
            for source in record['source_records']:
                self.assertTrue(package_v5.learner_file(Path(source['path'])), source['path'])


def main():
    suite = unittest.TestSuite(unittest.defaultTestLoader.loadTestsFromTestCase(group)
                               for group in (FifthEditionGuards, ReceiptPreservationGuards, CandidateEnvironmentGuards))
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    if not result.wasSuccessful():
        raise SystemExit(1)
    print(f"{result.testsRun} isolated fifth-edition release guards passed; {len(result.skipped)} platform skips.")


if __name__ == '__main__':
    main()
