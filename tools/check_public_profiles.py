"""Focused adversarial checks of public archive and tool-resolution boundaries."""
import os
import json
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import textwrap
import unittest
import zipfile
from unittest.mock import patch

from native_tools import resolve
from public_exports import extract, learner_file, safe_name, write_archive
from common import ROOT


class PublicProfileTests(unittest.TestCase):
    def test_declared_public_assessment_assets_are_packaged(self):
        for path in (ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-*.json'):
            data = json.loads(path.read_text())
            for name in data.get('supplied_artifacts', []):
                self.assertTrue(learner_file(name), path.stem+': required public asset omitted: '+name)

    def test_public_schema_never_reads_restricted_evidence(self):
        script = textwrap.dedent('''
            import sys
            from pathlib import Path
            sys.path.insert(0, sys.argv[1])
            attempts = []
            def forbid(event, args):
                if event == 'open' and isinstance(args[0], (str, bytes)):
                    name = args[0].decode() if isinstance(args[0], bytes) else args[0]
                    if {'reviews', 'private'} & set(Path(name).parts):
                        attempts.append(name)
                        raise AssertionError('Public check read restricted evidence: ' + name)
            sys.addaudithook(forbid)
            from public_validation import schema_checks
            result = schema_checks()
            assert not attempts, attempts
            assert result['baseline_preservation'].startswith('passed:'), result
        ''')
        result = subprocess.run([sys.executable, '-c', script, str(Path(__file__).resolve().parent)],
                                capture_output=True, text=True, timeout=120)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_public_document_proof_rejects_tampered_teaching(self):
        from common import ROOT
        from public_teaching_preservation import qualifies, BASE, GUIDE, F439
        baseline = json.loads((ROOT/'curriculum/v5/baseline.json').read_text())
        mapping = json.loads((ROOT/BASE/'encoding_map.json').read_text())
        public_rows = [row for row in mapping['documents'] if row['original'].startswith('labs/')]
        projects = ['curriculum/projects/PR-T28-06.md', 'curriculum/projects/PR-T29-04.md']
        names = [BASE+'/encoding_map.json', BASE+'/bindings.json', BASE+'/predecessor_projects.json', GUIDE, F439, *projects]
        names += [row[key] for row in public_rows for key in ('original', 'current')]
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in names:
                target = root/name
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(ROOT/name, target)
            for project in projects:
                self.assertTrue(qualifies(project, baseline['teaching'][project], root=root), project)
            for name in (projects[0], public_rows[0]['current'], BASE+'/encoding_map.json', BASE+'/bindings.json'):
                target = root/name
                original = target.read_bytes()
                target.write_bytes(original + b'\nunauthorized teaching change\n')
                self.assertFalse(qualifies(projects[0], baseline['teaching'][projects[0]], root=root), name)
                target.write_bytes(original)
            self.assertFalse(qualifies(projects[0], '0'*64, root=root), 'Altered predecessor identity accepted')

    def test_repeated_validation_keeps_immutable_campaigns(self):
        from public_validation import run
        def command(argv, output, label, **kwargs):
            argv = list(map(str, argv))
            if '--report' in argv:
                receipt = Path(argv[argv.index('--report') + 1])
                if receipt.exists():
                    return dict(label=label, passed=False, classification='existing-evidence')
                receipt.write_text('immutable simulated campaign')
            return dict(label=label, passed=True)
        with tempfile.TemporaryDirectory() as directory, \
                patch('public_validation.source_identity', return_value={'sha256': 'fixture', 'files': {}}), \
                patch('public_validation.identify', return_value={}), \
                patch('public_validation.schema_checks', return_value={}), \
                patch('public_validation.run_command', side_effect=command), \
                patch('public_validation.foundation_checks', return_value=[]), \
                patch('public_validation.algorithm_checks', return_value=[]), \
                patch('check_fixtures.selected_oracles', return_value=[]):
            first = run(directory)
            second = run(directory)
            self.assertTrue(first['passed'])
            self.assertTrue(second['passed'], 'A second public run reused immutable campaign paths')
            self.assertNotEqual(first['run_directory'], second['run_directory'])

    def test_withheld_paths(self):
        for name in ['assessor/deadlines/transfer.md', 'research/v5/pilot/answers.md',
                     'tools/author_pilot.py', 'reviews/raw.json', 'research/private/capture.txt']:
            self.assertFalse(learner_file(name), name)
        for name in ['labs/deadline/include/api.h', 'curriculum/v5/routes/contextual-v1.json',
                     'research/v5/pilot/resources.json', 'cmastery/route.py',
                     'research/enhancement/networking_contracts.json']:
            self.assertTrue(learner_file(name), name)

    def test_bad_names(self):
        for name in ['../secret', '/tmp/secret', 'a/../secret', 'a//b', 'C:/secret', 'a\\b']:
            with self.assertRaises(ValueError):
                safe_name(name)

    def test_archive_roundtrip_and_symlink(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root/'input').write_text('public bytes')
            write_archive(root/'good.zip', ['input'], root)
            extract(root/'good.zip', root/'out')
            self.assertEqual((root/'out/input').read_text(), 'public bytes')
            (root/'redirect').symlink_to(root/'input')
            with self.assertRaises(ValueError):
                write_archive(root/'bad.zip', ['redirect'], root)
            with zipfile.ZipFile(root/'escape.zip', 'w') as archive:
                archive.writestr('../outside', 'denied')
            with self.assertRaises(ValueError):
                extract(root/'escape.zip', root/'reject')
            self.assertFalse((root/'outside').exists())

    def test_override_does_not_silently_fall_back(self):
        self.assertTrue(Path(resolve('python', sys.executable)).is_file())
        with self.assertRaises(FileNotFoundError):
            resolve('python', '/definitely-not-installed/python')


if __name__ == '__main__':
    unittest.main()
