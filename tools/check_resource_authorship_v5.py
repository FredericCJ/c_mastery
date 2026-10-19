"""Isolated negative controls for exact, additive author-credit metadata."""
from copy import deepcopy
import json
from pathlib import Path
import tempfile
import unittest
from common import ROOT
from resource_identity_v5 import add_missing_authors, digest
from resource_inputs_v5 import discover, sha


class AuthorshipAdditions(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.name = 'research/v5/expansion/example/resources.json'
        self.row = {'id': 'R-EXAMPLE', 'title': 'Original source', 'edition': '1',
                    'inspection': {'scope': 'Original technical passage'}, 'assignable': True}
        self.write('curriculum/v5/baseline.json', {'resources': {'R-ACCEPTED': 'unchanged'}})
        self.write(self.name, {'resources': [self.row]})
        self.credit_name = 'research/v5/expansion/credit/inspection.json'
        self.credit = {'records': {'credit': {'authors': ['Named Author'],
            'resource_ids': ['R-EXAMPLE'], 'status': 'credit-inspected',
            'source': {'url': 'https://example.invalid/primary', 'capture_sha256': '1' * 64},
            'locator': 'Title-page author byline', 'observation': 'Explicit byline',
            'method': 'Direct primary inspection', 'date': '2026-10-19',
            'limits': 'Credit metadata only'}}}
        self.write(self.credit_name, self.credit)
        self.entry = {'id': 'R-EXAMPLE', 'register': self.name,
            'register_sha256': sha(self.root / self.name), 'original_sha256': digest(self.row),
            'authors': ['Named Author'], 'incoming_sha256': digest(dict(self.row, authors=['Named Author'])),
            'evidence': {'path': self.credit_name, 'sha256': sha(self.root / self.credit_name), 'record': 'credit'}}
        self.policy = {'schema_version': 1, 'operation': 'add-absent-authors-only',
                       'required_ids': ['R-EXAMPLE'], 'additions': [self.entry]}
        self.provenance = {'R-EXAMPLE': [{'path': self.name, 'sha256': sha(self.root / self.name), 'role': 'registration'}]}

    def write(self, name, value):
        p = self.root / name
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(json.dumps(value), encoding='utf8')

    def apply(self, rows=None):
        self.write('research/v5/resource_authorship_additions.json', self.policy)
        return add_missing_authors(self.root, rows or [self.row], self.provenance)

    def test_exact_addition_preserves_original_object_and_file(self):
        before = (self.root / self.name).read_bytes()
        rows, applied = self.apply()
        self.assertEqual(rows, [dict(self.row, authors=['Named Author'])])
        self.assertNotIn('authors', self.row)
        self.assertEqual((self.root / self.name).read_bytes(), before)
        self.assertEqual(applied, [self.entry])

    def test_omitted_and_duplicate_addition_rejected(self):
        for additions in ([], [self.entry, self.entry]):
            self.policy['additions'] = additions
            with self.assertRaises(ValueError): self.apply()

    def test_accepted_resource_rejected(self):
        self.write('curriculum/v5/baseline.json', {'resources': {'R-EXAMPLE': 'original'}})
        with self.assertRaises(ValueError): self.apply()

    def test_existing_authors_cannot_be_replaced_even_when_empty(self):
        for authors in (['Different author'], []):
            with self.assertRaises(ValueError): self.apply([dict(self.row, authors=authors)])

    def test_original_register_row_and_corrected_hash_each_bind(self):
        for key in ('register_sha256', 'original_sha256', 'incoming_sha256'):
            original = self.entry[key]
            self.entry[key] = '0' * 64
            with self.subTest(key=key), self.assertRaises(ValueError): self.apply()
            self.entry[key] = original

    def test_unrelated_source_producer_rejected(self):
        self.provenance['R-EXAMPLE'][0]['path'] = 'research/v5/other_resources.json'
        with self.assertRaises(ValueError): self.apply()

    def test_evidence_mutation_and_wrong_resource_credit_rejected(self):
        self.credit['records']['credit']['resource_ids'] = ['R-OTHER']
        self.write(self.credit_name, self.credit)
        with self.assertRaises(ValueError): self.apply()
        self.entry['evidence']['sha256'] = sha(self.root / self.credit_name)
        with self.assertRaises(ValueError): self.apply()

    def test_new_uninspected_author_or_empty_credit_rejected(self):
        for authors in (['Invented author'], [], [''], ['Named Author', 'Named Author']):
            self.entry['authors'] = authors
            with self.subTest(authors=authors), self.assertRaises(ValueError): self.apply()

    def test_non_author_fields_cannot_be_changed(self):
        self.entry['title'] = 'Rewritten title'
        with self.assertRaises(ValueError): self.apply()
        del self.entry['title']
        with self.assertRaises(ValueError): self.apply([dict(self.row, assignable=False)])

    def test_evidence_path_cannot_escape(self):
        self.entry['evidence']['path'] = '../credit.json'
        with self.assertRaises(ValueError): self.apply()

    def test_actual_36_credits_and_12_original_registers(self):
        policy = json.loads((ROOT / 'research/v5/resource_authorship_additions.json').read_text(encoding='utf8'))
        inputs = {e['register']: e['register_sha256'] for e in policy['additions']}
        self.assertEqual(len(inputs), 12)
        self.assertEqual(len(policy['required_ids']), 36)
        rows, report = discover(ROOT)
        self.assertEqual({e['id'] for e in report['authorship_additions']}, set(policy['required_ids']))
        actual = {r['id']: r for r in rows}
        accepted = json.loads((ROOT / 'curriculum/v5/baseline.json').read_text(encoding='utf8'))['resources']
        self.assertFalse(set(policy['required_ids']) & set(accepted))
        for entry in policy['additions']:
            self.assertEqual(digest(actual[entry['id']]), entry['incoming_sha256'])
        for name, expected in inputs.items(): self.assertEqual(sha(ROOT / name), expected)


if __name__ == '__main__':
    unittest.main()
