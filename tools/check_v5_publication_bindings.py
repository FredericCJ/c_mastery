"""Read-only guards for the narrowly qualified B06 teaching attachments."""
from copy import deepcopy
import hashlib
from pathlib import Path
import unittest
from unittest.mock import patch
from common import ROOT, load, read_json
import check_v5
import rebind_b06_documents as proof
import v3_common


class TeachingPublicationBindings(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.base = check_v5.baseline()
        cls.taxonomy, _, cls.projects, cls.resources = load()
        cls.data = read_json(ROOT / 'curriculum/v5/units.json')
        cls.artifacts = read_json(ROOT / 'curriculum/v5/artifacts.json')
        cls.recall = read_json(ROOT / 'curriculum/v5/recall.json')

    def errors(self, base=None):
        return check_v5.check_preserved(base or self.base, self.data, self.artifacts,
                                       self.recall, self.projects, self.resources, self.taxonomy)

    def test_actual_accepted_contracts_and_exact_successors_pass(self):
        self.assertEqual(self.errors(), [])
        originals = proof.read(proof.PROJECTS)['projects']
        for project in ('PR-T28-06', 'PR-T29-04'):
            path = 'curriculum/projects/' + project + '.md'
            self.assertEqual(hashlib.sha256(originals[project].encode('utf-8')).hexdigest(),
                             self.base['teaching'][path])
            self.assertNotEqual(hashlib.sha256((ROOT / path).read_bytes()).hexdigest(),
                                self.base['teaching'][path])

    def test_wrong_predecessor_digest_is_rejected_by_preservation(self):
        base = deepcopy(self.base)
        path = 'curriculum/projects/PR-T28-06.md'
        base['teaching'][path] = '0' * 64
        self.assertTrue(any(path in error for error in self.errors(base)))

    def test_unrelated_project_cannot_use_b06_proof(self):
        with patch('check_legacy_assessments.baseline_inspection_matches', return_value=True) as helper:
            self.assertFalse(check_v5.publication_successor_covers(
                'curriculum/projects/PR-T28-05.md', '0' * 64))
            helper.assert_not_called()

    def test_private_attachment_version_and_semantic_changes_are_rejected(self):
        path = 'curriculum/projects/PR-T28-06.md'
        target = ROOT / path
        original = target.read_bytes()
        read_bytes = Path.read_bytes
        for suffix in (b'\nassessor/firmware_lifecycle/secret.c\n',
                       b'\nVersion: invented-successor-v99\n',
                       b'\nThe driver may release storage before device quiescence.\n'):
            with self.subTest(suffix=suffix), patch.object(Path, 'read_bytes',
                    lambda file: original + suffix if file == target else read_bytes(file)):
                self.assertFalse(check_v5.publication_successor_covers(path, self.base['teaching'][path]))

    def test_failed_authoritative_proof_is_rejected(self):
        with patch.object(proof, 'validate', return_value=['missing exact current export']):
            for project in ('PR-T28-06', 'PR-T29-04'):
                path = 'curriculum/projects/' + project + '.md'
                self.assertFalse(check_v5.publication_successor_covers(path, self.base['teaching'][path]))

    def test_duplicate_export_record_is_rejected(self):
        read = proof.read
        report = deepcopy(read(proof.REPORT))
        exports = report['exports']
        self.assertEqual(len(exports), 18)
        exports[1] = deepcopy(exports[0])
        with patch.object(proof, 'read', lambda name: report if name == proof.REPORT else read(name)):
            path = 'curriculum/projects/PR-T29-04.md'
            self.assertFalse(check_v5.publication_successor_covers(path, self.base['teaching'][path]))

    def test_v3_retained_predecessors_and_current_attachments_pass(self):
        self.assertEqual(v3_common.check_v3_migration(self.taxonomy, self.projects), [])

    def test_v3_wrong_predecessor_and_renamed_retained_project_fail(self):
        for field, value in [('baseline_sha256', '0' * 64),
                             ('new_ids', ['PR-T28-05'])]:
            migration = deepcopy(read_json(ROOT / 'curriculum/migration_v3.json'))
            row = next(r for r in migration['projects'] if r['old_id'] == 'PR-T28-06')
            row[field] = value
            self.assertTrue(v3_common.check_v3_migration(self.taxonomy, self.projects, migration))

    def test_v3_failed_attachment_proof_and_redirected_project_fail(self):
        with patch.object(proof, 'validate', return_value=['missing current public export']):
            errors = v3_common.check_v3_migration(self.taxonomy, self.projects)
            self.assertTrue(any('PR-T28-06' in error for error in errors))
            self.assertTrue(any('PR-T29-04' in error for error in errors))
        projects = deepcopy(self.projects)
        projects['PR-T28-06']['path'] = ROOT / 'curriculum/projects/PR-T28-05.md'
        self.assertTrue(v3_common.check_v3_migration(self.taxonomy, projects))


if __name__ == '__main__':
    unittest.main()
