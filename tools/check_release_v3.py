"""Exercise release guards in isolated fixtures; no real acceptance is changed."""
from contextlib import ExitStack, redirect_stdout
import io
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch
import zipfile
import accept_v3
import package_v3
import v2_common
from common import ROOT, dump, read_json, sha


class ReleaseGuards(unittest.TestCase):
    def setUp(self):
        parent = ROOT / 'build/release_guard_checks'
        if parent.resolve() != ROOT.resolve() / 'build/release_guard_checks':
            raise ValueError('Unsafe release-test directory')
        parent.mkdir(parents=True, exist_ok=True)
        temporary = tempfile.TemporaryDirectory(prefix='case_', dir=parent)
        self.root = Path(temporary.name).resolve()
        if self.root.parent != parent.resolve():
            raise ValueError('Unsafe release-test fixture')
        # The resolved fixture is checked before TemporaryDirectory can remove it.
        self.addCleanup(temporary.cleanup)
        self.stack = ExitStack()
        self.addCleanup(self.stack.close)
        self.stack.enter_context(patch.object(package_v3, 'ROOT', self.root))
        self.stack.enter_context(patch.object(accept_v3, 'ROOT', self.root))
        self.stack.enter_context(patch.object(v2_common, 'ROOT', self.root))
        self.stack.enter_context(redirect_stdout(io.StringIO()))
        self.write('curriculum/example.md', 'accepted teaching\n')
        self.write('labs/example/family.json', '{"projects": []}\n')
        self.write('assessor/example/answer.c', 'int answer(void) { return 42; }\n')
        self.write('tools/build_v3.py', '# The rebuild subprocess is controlled by this test.\n')
        self.write('build/html/index.html', '<html>accepted handbook</html>\n')
        self.write('c_programming_mastery_curriculum.md', '# Accepted handbook\n')
        identity = v2_common.source_identity()
        for name in ['publication', 'lab_matrix', 'cli_checks', 'target_matrix', 'os_comparison']:
            record = {'status': 'passed', 'input_sha256': identity['sha256']}
            if name == 'lab_matrix':
                record['families'] = [{'family': 'example', 'passed': True}]
            dump(self.root / 'reviews/v3' / (name + '.json'), record)
        for name in ['review_curriculum', 'review_execution', 'review_kernel', 'review_observatory']:
            dump(self.root / 'reviews/v3' / (name + '.json'),
                 {'accepted': True, 'scope': 'Synthetic release fixture only.',
                  'inspected_paths': {'curriculum/example.md': sha(self.root / 'curriculum/example.md')},
                  'findings': []})
        dump(self.root / 'reviews/publication_inventory.json', {'modules': 1, 'projects': 0})
        # Establish a genuinely accepted prior state, then mutate only a receipt
        # or review in negative cases. The acceptance validator itself is real.
        accept_v3.main()
        self.old_archives = {}
        for name in package_v3.ARCHIVES:
            path = self.root / 'dist' / name
            path.parent.mkdir(exist_ok=True)
            with zipfile.ZipFile(path, 'w') as archive:
                archive.writestr('previous-release.txt', name)
            self.old_archives[name] = sha(path)
        dump(self.root / 'reviews/v3/package_report.json', {'status': 'passed', 'passed': True})

    def write(self, relative, text):
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding='utf-8', newline='\n')

    def change_record(self, name, **changes):
        path = self.root / 'reviews/v3' / (name + '.json')
        record = read_json(path)
        record.update(changes)
        dump(path, record)

    def unchanged_failed_release(self):
        for name, digest in self.old_archives.items():
            self.assertEqual(sha(self.root / 'dist' / name), digest, name)
        report = read_json(self.root / 'reviews/v3/package_report.json')
        self.assertEqual(report['status'], 'failed')
        self.assertIs(report['passed'], False)
        self.assertNotIn('archives', report)

    def rebuild(self, command, **kwargs):
        """Supply reproducible output; this is not a substitute for Sphinx."""
        check = Path(kwargs['cwd'])
        output = check / 'build/html/index.html'
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_bytes((self.root / 'build/html/index.html').read_bytes())
        return subprocess.CompletedProcess(command, 0, 'synthetic rebuild\n', '')

    def test_same_source_failed_receipt_invalidates_prior_acceptance(self):
        before = v2_common.source_identity()['sha256']
        self.change_record('target_matrix', status='failed')
        self.assertEqual(before, v2_common.source_identity()['sha256'])
        with patch.object(package_v3.subprocess, 'run') as run:
            with self.assertRaisesRegex(ValueError, 'Missing, failed or stale check: target_matrix'):
                package_v3.main()
            run.assert_not_called()
        self.assertFalse((self.root / 'build/release_candidates').exists())
        self.unchanged_failed_release()

    def test_revoked_review_invalidates_prior_acceptance(self):
        self.change_record('review_kernel', accepted=False)
        with patch.object(package_v3.subprocess, 'run') as run:
            with self.assertRaisesRegex(ValueError, 'Review not accepted: review_kernel'):
                package_v3.main()
            run.assert_not_called()
        self.unchanged_failed_release()

    def test_failed_rebuild_preserves_all_prior_archives(self):
        failure = subprocess.CompletedProcess([], 7, 'build output', 'simulated failure')
        with patch.object(package_v3.subprocess, 'run', return_value=failure) as run:
            with self.assertRaisesRegex(ValueError, 'Extracted-source build failed'):
                package_v3.main()
            self.assertEqual(run.call_count, 1)
        self.assertIn('simulated failure', (self.root / 'reviews/v3/package_build.log').read_text())
        self.assertEqual(len(list((self.root / 'build/release_candidates').glob('*/*.zip'))), 4)
        self.unchanged_failed_release()

    def test_html_mismatch_preserves_all_prior_archives(self):
        def mismatch(command, **kwargs):
            result = self.rebuild(command, **kwargs)
            (Path(kwargs['cwd']) / 'build/html/index.html').write_text('different output', encoding='utf-8')
            return result
        with patch.object(package_v3.subprocess, 'run', side_effect=mismatch):
            with self.assertRaisesRegex(ValueError, 'Extracted build differs from published HTML'):
                package_v3.main()
        self.unchanged_failed_release()

    def test_source_mutation_during_rebuild_blocks_promotion(self):
        def mutate(command, **kwargs):
            result = self.rebuild(command, **kwargs)
            self.write('curriculum/example.md', 'changed while packaging\n')
            return result
        with patch.object(package_v3.subprocess, 'run', side_effect=mutate):
            with self.assertRaisesRegex(ValueError, 'Canonical source changed during packaging'):
                package_v3.main()
        self.unchanged_failed_release()

    def test_review_revoked_during_rebuild_blocks_promotion(self):
        def revoke(command, **kwargs):
            result = self.rebuild(command, **kwargs)
            self.change_record('review_kernel', accepted=False)
            return result
        with patch.object(package_v3.subprocess, 'run', side_effect=revoke):
            with self.assertRaisesRegex(ValueError, 'Export input changed during packaging: reviews/v3/review_kernel.json'):
                package_v3.main()
        self.unchanged_failed_release()

    def test_promotion_failure_rolls_back_replaced_archives(self):
        original = Path.replace
        def replace(path, destination):
            if (path.name == package_v3.ARCHIVES[2] and path.parent.name != 'previous'
                    and Path(destination).parent == self.root / 'dist'):
                raise OSError('simulated promotion failure')
            return original(path, destination)
        with patch.object(package_v3.subprocess, 'run', side_effect=self.rebuild):
            with patch.object(Path, 'replace', replace):
                with self.assertRaisesRegex(OSError, 'simulated promotion failure'):
                    package_v3.main()
        self.unchanged_failed_release()

    def test_success_promotes_four_archives_and_reports_actual_hashes(self):
        with patch.object(package_v3.subprocess, 'run', side_effect=self.rebuild):
            package_v3.main()
        report = read_json(self.root / 'reviews/v3/package_report.json')
        self.assertEqual(report['status'], 'passed')
        self.assertEqual(len(report['archives']), 4)
        for record in report['archives']:
            path = self.root / record['path']
            self.assertTrue(zipfile.is_zipfile(path))
            self.assertEqual(sha(path), record['sha256'])
            self.assertNotEqual(sha(path), self.old_archives[path.name])
        with zipfile.ZipFile(self.root / 'dist' / package_v3.ARCHIVES[0]) as archive:
            self.assertIn('reviews/v3/acceptance.json', archive.namelist())
            self.assertNotIn('reviews/v3/package_report.json', archive.namelist())
        with zipfile.ZipFile(self.root / 'dist' / package_v3.ARCHIVES[1]) as archive:
            self.assertFalse(any('assessor' in Path(name).parts for name in archive.namelist()))


def main():
    result = unittest.TextTestRunner(verbosity=2).run(
        unittest.defaultTestLoader.loadTestsFromTestCase(ReleaseGuards))
    if not result.wasSuccessful():
        raise SystemExit(1)
    print(f'{result.testsRun} isolated release guards passed; actual release receipts unchanged.')


if __name__ == '__main__':
    main()
