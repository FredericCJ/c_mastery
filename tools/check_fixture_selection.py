"""Regression checks for mandatory explicit and retained fixture oracles."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import check_fixtures as fixtures


class FixtureSelection(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        (self.root/'tools').mkdir()
        (self.root/'curriculum').mkdir()
        for name in fixtures.required_oracles(self.root):
            (self.root/'tools'/name).write_text('pass\n', encoding='utf8')

    def declare(self, names):
        (self.root/'curriculum/edition.json').write_text(json.dumps(
            {'exercise_expansion': {'fixture_oracles': names}}), encoding='utf8')

    def test_explicit_nonlegacy_oracle_is_selected(self):
        name = 'verify_protocol_completion.py'
        self.declare([name])
        (self.root/'tools'/name).write_text('pass\n', encoding='utf8')
        self.assertIn(name, [p.name for p in fixtures.selected_oracles(self.root)])

    def test_missing_explicit_oracle_is_rejected(self):
        self.declare(['verify_protocol_completion.py'])
        with self.assertRaisesRegex(ValueError, 'Missing required fixture oracles'):
            fixtures.selected_oracles(self.root)

    def test_duplicate_declarations_run_once_and_order_is_stable(self):
        name = 'verify_protocol_completion.py'
        self.declare([name, name, 'verify_language_fixtures.py'])
        (self.root/'tools'/name).write_text('pass\n', encoding='utf8')
        names = [p.name for p in fixtures.selected_oracles(self.root)]
        self.assertEqual(names, sorted(set(names)))
        self.assertEqual(names.count(name), 1)

    def test_legacy_discovery_is_retained(self):
        name = 'verify_extra_fixtures.py'
        (self.root/'tools'/name).write_text('pass\n', encoding='utf8')
        self.assertIn(name, [p.name for p in fixtures.selected_oracles(self.root)])

    def test_invalid_declared_paths_and_nonverifiers_are_rejected(self):
        for name in ('../verify_escape.py', 'tools/verify_inside.py',
                     'tools\\verify_inside.py', '/verify_absolute.py',
                     'C:/verify_absolute.py', 'unrelated.py', 'verify_bad.py.txt'):
            with self.subTest(name=name):
                self.declare([name])
                with self.assertRaisesRegex(ValueError, 'Invalid fixture oracle basename'):
                    fixtures.selected_oracles(self.root)

    def test_selection_failure_invalidates_previous_receipt(self):
        self.declare(['verify_missing.py'])
        report = self.root/'reviews/fixture_checks.json'
        report.parent.mkdir()
        report.write_text('{"passed": true}', encoding='utf8')
        with patch.object(fixtures, 'ROOT', self.root):
            with self.assertRaisesRegex(ValueError, 'Missing required fixture oracles'):
                fixtures.main()
        self.assertFalse(json.loads(report.read_text())['passed'])

    def exercise_inputs(self, names):
        path = self.root/'curriculum/v5/exercises.json'
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps({'exercises': [{'fixtures': names}]}), encoding='utf8')

    def test_learner_support_is_checked_separately_from_fixture_inventory(self):
        (self.root/'fixtures').mkdir()
        (self.root/'fixtures/cases.json').write_text('{}', encoding='utf8')
        (self.root/'labs/demo').mkdir(parents=True)
        header = self.root/'labs/demo/api.h'
        header.write_text('/* learner contract */\n', encoding='utf8')
        self.exercise_inputs(['fixtures/cases.json', 'labs/demo/api.h'])
        self.assertEqual(fixtures.check_fixture_catalog({}, self.root), {'fixtures/cases.json'})
        header.unlink()
        with self.assertRaisesRegex(ValueError, 'Missing supplied exercise fixture path'):
            fixtures.check_fixture_catalog({}, self.root)

    def test_missing_and_unreferenced_fixture_checks_remain_strict(self):
        folder = self.root/'fixtures'
        folder.mkdir()
        (folder/'orphan.json').write_text('{}', encoding='utf8')
        self.exercise_inputs(['fixtures/missing.json'])
        with self.assertRaisesRegex(ValueError, 'missing=.*missing.json.*unreferenced=.*orphan.json'):
            fixtures.check_fixture_catalog({}, self.root)

    def test_assessor_and_escaping_fixture_paths_are_rejected(self):
        for name in ('assessor/answer.c', '../outside', 'labs/../assessor/answer.c',
                     'labs\\demo.h', '/fixtures/cases.json'):
            with self.subTest(name=name):
                self.exercise_inputs([name])
                with self.assertRaisesRegex(ValueError, 'Invalid fixture reference'):
                    fixtures.check_fixture_catalog({}, self.root)


if __name__ == '__main__':
    unittest.main()
