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
    result = unittest.TextTestRunner(verbosity=2).run(
        unittest.defaultTestLoader.loadTestsFromTestCase(FifthEditionGuards))
    if not result.wasSuccessful():
        raise SystemExit(1)
    print(f"{result.testsRun} isolated fifth-edition release guards passed; {len(result.skipped)} platform skips.")


if __name__ == '__main__':
    main()
