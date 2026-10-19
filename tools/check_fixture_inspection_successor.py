"""Fail-closed predecessor inspection qualification; isolated mutation controls."""
from pathlib import Path
import copy
import json
import tempfile
import unittest

import fixture_inspection_successor as successor


class FixtureInspectionSuccessor(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.packet = json.loads((successor.ROOT / successor.PACKET).read_text(encoding='utf8'))
        cls.report = json.loads((successor.ROOT / cls.packet['current_validation']['path']).read_text(encoding='utf8'))
        cls.temporary = tempfile.TemporaryDirectory(prefix='fixture-inspection-successor-')
        cls.addClassCleanup(cls.temporary.cleanup)
        cls.root = Path(cls.temporary.name).resolve()
        names = set(cls.report['source_sha256']) | set(cls.report['input_sha256'])
        names |= {row['predecessor'] for row in cls.packet['tools'].values()}
        names |= {cls.packet['current_validation']['path'], successor.PACKET}
        names |= {row[stream]['path'] for row in cls.report['commands'] for stream in ('stdout', 'stderr')}
        for name in names:
            target = cls.root / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes((successor.ROOT / name).read_bytes())

    def setUp(self):
        self.proof = copy.deepcopy(type(self).packet)
        self.receipt = copy.deepcopy(type(self).report)
        self.changed = {}
        self.addCleanup(self.restore)

    def restore(self):
        for name, before in self.changed.items():
            path = self.root / name
            if before is None:
                if path.exists(): path.unlink()
            else:
                path.write_bytes(before)

    def write(self, name, data):
        path = self.root / name
        if name not in self.changed:
            self.changed[name] = path.read_bytes() if path.exists() else None
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)

    def remove(self, name):
        self.write(name, b'')
        (self.root / name).unlink()

    def commit_receipt(self):
        name = self.proof['current_validation']['path']
        raw = (json.dumps(self.receipt, indent=2) + '\n').encode()
        self.write(name, raw)
        self.proof['current_validation']['sha256'] = successor.digest(raw)

    def rejected(self, contains=None):
        errors = successor.validate(self.root, self.proof)
        self.assertTrue(errors, 'Invalid successor qualified a historical inspection')
        if contains: self.assertIn(contains, ' '.join(errors))

    def mutation(self, name, before, after, repin=True):
        raw = (self.root / name).read_bytes()
        self.assertIn(before, raw)
        changed = raw.replace(before, after, 1)
        self.write(name, changed)
        if repin:
            sha = successor.digest(changed)
            self.proof['tools'][name]['current_sha256'] = sha
            self.receipt['source_sha256'][name] = sha
            self.commit_receipt()

    def test_current_proof_and_exact_passages(self):
        self.assertEqual(successor.validate(), [])
        self.assertEqual(successor.validate(self.root, self.proof), [])
        for name, original in successor.ORIGINALS.items():
            for passages in original['passages']:
                binding = dict(path=name, sha256=original['sha256'], passages=passages)
                self.assertTrue(successor.qualifies(binding, self.root / name, self.root))

    def test_wrong_old_hash_rejected(self):
        self.proof['tools']['tools/verify_embedded_fixtures.py']['predecessor_sha256'] = '0' * 64
        self.rejected('predecessor changed')

    def test_wrong_current_hash_rejected(self):
        self.proof['tools']['tools/verify_networking_fixtures.py']['current_sha256'] = '0' * 64
        self.rejected('Current verifier changed')

    def test_modified_predecessor_cannot_be_repinned(self):
        row = self.proof['tools']['tools/verify_embedded_fixtures.py']
        raw = (self.root / row['predecessor']).read_bytes() + b'\n# changed\n'
        self.write(row['predecessor'], raw)
        row['predecessor_sha256'] = successor.digest(raw)
        self.rejected('predecessor changed')

    def test_changed_math_with_attempted_current_repin_rejected(self):
        self.mutation('tools/verify_embedded_fixtures.py',
                      b'return int(value, 0) if isinstance(value, str) else value',
                      b'return int(value, 0) if isinstance(value, str) else 0')
        self.rejected('inspected model/import/constant/guard changed')

    def test_changed_import_with_attempted_repin_rejected(self):
        self.mutation('tools/verify_embedded_fixtures.py', b'import statistics', b'import math as statistics')
        self.rejected('inspected model/import/constant/guard changed')

    def test_changed_constant_with_attempted_repin_rejected(self):
        self.mutation('tools/verify_embedded_fixtures.py', b'ROOT = Path(__file__).resolve().parents[1]',
                      b'ROOT = Path(__file__).resolve().parents[0]')
        self.rejected('inspected model/import/constant/guard changed')

    def test_shadowing_original_definition_rejected(self):
        before = (self.root / 'tools/verify_embedded_fixtures.py').read_bytes()
        for suffix in (b'\nrecompute = lambda value: value\n', b'\nif True:\n    recompute = lambda value: value\n',
                       b'\nexec("recompute = None")\n', b'\n@print\ndef added(): pass\n',
                       b'\ndef added(x=globals().clear()): pass\n',
                       b'\ndef added(x: globals().__setitem__("recompute", None)): pass\n',
                       b'\ndef added() -> globals().__setitem__("recompute", None): pass\n',
                       b'\ndef sum(values): return 0\n', b'\n__builtins__ = {}\n'):
            with self.subTest(suffix=suffix), self.assertRaises(ValueError):
                original = (self.root / self.proof['tools']['tools/verify_embedded_fixtures.py']['predecessor']).read_bytes()
                successor.retained_nodes(original, before + suffix)

    def test_missing_report_rejected(self):
        self.remove(self.proof['current_validation']['path'])
        self.rejected('Missing or redirected')

    def test_missing_required_dependency_rejected(self):
        self.remove('tools/network_fixture_templates.py')
        self.rejected('Missing or redirected')

    def test_omitted_required_dependency_rejected(self):
        del self.receipt['source_sha256']['tools/check_embedded_fixture_contracts.py']
        self.commit_receipt(); self.rejected('Exact six')

    def test_extra_dependency_cannot_replace_exact_six(self):
        self.receipt['source_sha256']['tools/unrelated.py'] = '0' * 64
        self.commit_receipt(); self.rejected('Exact six')

    def test_stale_fixture_input_rejected(self):
        name = 'fixtures/embedded/interrupt_time_capture.json'
        self.write(name, (self.root / name).read_bytes() + b'\n')
        self.rejected('fixture/catalog validation inputs changed')

    def test_missing_catalog_input_map_rejected(self):
        self.receipt['input_sha256'] = {}
        self.commit_receipt(); self.rejected('fixture/catalog validation inputs changed')

    def test_failed_and_malformed_receipt_flags_rejected(self):
        for field, value in (('schema_version', True), ('passed', 1), ('passed', False),
                             ('historical_model_nodes_unchanged', 1), ('physical_observations', False),
                             ('physical_observations', 1), ('scope', 'Physical mastery established')):
            with self.subTest(field=field, value=value):
                self.receipt = copy.deepcopy(type(self).report); self.receipt[field] = value
                self.commit_receipt(); self.rejected()

    def test_failed_or_wrong_command_identity_rejected(self):
        for field, value in (('exit_code', 1), ('exit_code', False), ('passed', 1), ('id', 'unrelated'),
                             ('argv', ['python', '-c', 'print("pass")'])):
            with self.subTest(field=field, value=value):
                self.receipt = copy.deepcopy(type(self).report); self.receipt['commands'][0][field] = value
                self.commit_receipt(); self.rejected()

    def test_omitted_duplicate_raw_commands_rejected(self):
        self.receipt['commands'].pop(); self.commit_receipt(); self.rejected('Exactly four')
        self.receipt = copy.deepcopy(type(self).report)
        self.receipt['commands'][1] = copy.deepcopy(self.receipt['commands'][0])
        self.commit_receipt(); self.rejected('duplicate current validation command')

    def test_changed_raw_output_rejected_even_with_unchanged_success_flag(self):
        row = self.receipt['commands'][0]
        self.write(row['stdout']['path'], b'{}')
        self.rejected('Changed successor evidence')

    def test_repinned_empty_raw_verifier_output_rejected(self):
        row = self.receipt['commands'][0]; raw = b'{}'
        self.write(row['stdout']['path'], raw); row['stdout']['sha256'] = successor.digest(raw)
        self.commit_receipt(); self.rejected('Missing or failed verifier result')

    def test_repinned_failed_raw_regression_output_rejected(self):
        row = self.receipt['commands'][2]; raw = b'Ran 16 tests in 0.1s\n\nFAILED (failures=1)\n'
        self.write(row['stderr']['path'], raw); row['stderr']['sha256'] = successor.digest(raw)
        self.commit_receipt(); self.rejected('Missing successful regression output')

    def test_boolean_verifier_count_cannot_stand_for_zero_observations(self):
        row = self.receipt['commands'][0]
        data = json.loads((self.root / row['stdout']['path']).read_text()); data['physical_observations'] = False
        raw = json.dumps(data).encode(); self.write(row['stdout']['path'], raw)
        row['stdout']['sha256'] = successor.digest(raw)
        self.commit_receipt(); self.rejected('Wrong current fixture coverage or physical claim')

    def test_broadened_or_unrelated_inspection_passages_rejected(self):
        name, original = next(iter(successor.ORIGINALS.items()))
        for passages in (['Whole current file and target execution'], original['passages'][0] + ['New main orchestration']):
            binding = dict(path=name, sha256=original['sha256'], passages=passages)
            self.assertFalse(successor.qualifies(binding, self.root / name, self.root))
        binding = dict(path=name, sha256=original['sha256'], passages=original['passages'][0])
        self.assertFalse(successor.qualifies(binding, self.root / 'tools/verify_networking_fixtures.py', self.root))

    def test_invalid_pinned_paths_rejected(self):
        for path in ('../outside.json', 'tools\\bad.json', str(self.root / 'absolute.json'), './tools/fixture_inspection_successor.py'):
            with self.subTest(path=path), self.assertRaises(ValueError):
                successor.local(self.root, path)

    def test_unknown_packet_shape_and_boolean_schema_rejected(self):
        self.proof['schema_version'] = True; self.rejected('Exact two-tool')
        self.proof = copy.deepcopy(type(self).packet); self.proof['independent_mastery'] = True
        self.rejected('Exact two-tool')

    def test_duplicate_receipt_keys_rejected(self):
        raw = b'{"passed":false,"passed":true}'
        self.write(self.proof['current_validation']['path'], raw)
        self.proof['current_validation']['sha256'] = successor.digest(raw)
        self.rejected('Duplicate successor JSON key')


if __name__ == '__main__':
    unittest.main()
