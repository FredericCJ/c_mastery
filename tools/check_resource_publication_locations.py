"""Isolated guards for the single metadata-only public bibliography location."""
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import resource_publication_locations as locations


class PublicationLocations(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.policy = json.loads((locations.ROOT / locations.POLICY).read_text(encoding='utf8'))
        self.inputs = [locations.POLICY, locations.REGISTER, locations.INSPECTION,
                       locations.CORROBORATION, 'curriculum/v5/baseline.json', 'references/resources.json']
        for name in self.inputs:
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes((locations.ROOT / name).read_bytes())
        self.resource = locations.row(self.read('references/resources.json'), locations.RESOURCE)

    def read(self, name):
        return json.loads((self.root / name).read_text(encoding='utf8'))

    def put(self, name, value):
        (self.root / name).write_text(json.dumps(value), encoding='utf8')

    def check(self, policy=None):
        return locations.locations(root=self.root, policy=self.policy if policy is None else policy,
                                   resource=self.resource)

    def reject(self, edit):
        policy = deepcopy(self.policy)
        edit(policy)
        with self.assertRaises(ValueError):
            self.check(policy)

    def test_official_location_preserves_originals_without_private_payload_access(self):
        before = {name: (self.root / name).read_bytes() for name in self.inputs}
        original_resource = deepcopy(self.resource)
        self.assertFalse((self.root / locations.CAPTURE).exists())
        original_open = Path.open

        def metadata_only(path, *args, **kwargs):
            self.assertNotIn('private', path.parts, 'private payload must never be opened')
            return original_open(path, *args, **kwargs)

        with patch.object(Path, 'open', metadata_only):
            self.assertEqual(locations.publication_url(self.resource, root=self.root),
                             'https://www.st.com/resource/en/reference_manual/dm00031020.pdf')
        self.assertEqual(self.resource, original_resource)
        self.assertEqual({name: (self.root / name).read_bytes() for name in self.inputs}, before)

    def test_unaffected_resource_has_no_override(self):
        source = {'id': 'R-unaffected', 'url': 'local:R-other'}
        self.assertIsNone(locations.publication_url(source, root=self.root))
        self.assertEqual(source, {'id': 'R-unaffected', 'url': 'local:R-other'})

    def test_actual_publisher_uses_projection_without_changing_source_map(self):
        import publish
        source_map = {locations.RESOURCE: deepcopy(self.resource)}
        original_map = deepcopy(source_map)
        before = {name: (self.root / name).read_bytes() for name in self.inputs}
        with patch.object(publish, 'ROOT', self.root):
            link = publish.resource_page_link(source_map[locations.RESOURCE], source_map)
        self.assertEqual(link, '[Authoritative resource page]('
                         'https://www.st.com/resource/en/reference_manual/dm00031020.pdf)')
        self.assertEqual(source_map, original_map)
        self.assertEqual({name: (self.root / name).read_bytes() for name in self.inputs}, before)

    def test_current_resource_scope_change_rejected(self):
        values = self.read('references/resources.json')
        current = locations.row(values, locations.RESOURCE)
        current['inspection']['scope'] += '; invented additional inspection'
        self.put('references/resources.json', values)
        with self.assertRaisesRegex(ValueError, 'current resource'):
            self.check()

    def test_rendered_resource_change_rejected(self):
        self.resource['locators'] = ['another passage']
        with self.assertRaisesRegex(ValueError, 'rendered resource'):
            self.check()

    def test_generated_registry_ids_do_not_change_resource_identity(self):
        self.resource['work_id'] = 'generated-work'
        self.resource['edition_id'] = 'generated-edition'
        self.resource['inspection_ids'] = ['generated-inspection']
        self.assertIn(locations.RESOURCE, self.check())

    def test_stale_row_digest_rejected(self):
        self.reject(lambda p: p['mappings'][0].update(current_row_sha256='0' * 64))

    def test_stale_registration_bytes_rejected(self):
        path = self.root / locations.REGISTER
        path.write_bytes(path.read_bytes() + b'\n')
        with self.assertRaisesRegex(ValueError, 'changed metadata'):
            self.check()

    def test_changed_registration_scope_cannot_be_repinned(self):
        values = self.read(locations.REGISTER)
        row = locations.row(values, locations.RESOURCE)
        row['inspection']['scope'] = 'Whole manual inspected'
        self.put(locations.REGISTER, values)
        policy = deepcopy(self.policy)
        policy['mappings'][0]['registration']['sha256'] = hashlib.sha256(
            (self.root / locations.REGISTER).read_bytes()).hexdigest()
        policy['mappings'][0]['registration']['row_sha256'] = locations.digest(row)
        with self.assertRaisesRegex(ValueError, 'registration binding'):
            self.check(policy)

    def test_unequal_capture_identity_rejected(self):
        self.reject(lambda p: p['mappings'][0].update(capture_sha256='0' * 64))

    def test_changed_inspection_capture_cannot_be_repinned(self):
        values = self.read(locations.INSPECTION)
        values['private_source_identities'][0]['sha256'] = '0' * 64
        self.put(locations.INSPECTION, values)
        policy = deepcopy(self.policy)
        policy['mappings'][0]['inspection']['sha256'] = hashlib.sha256(
            (self.root / locations.INSPECTION).read_bytes()).hexdigest()
        with self.assertRaisesRegex(ValueError, 'inspection binding'):
            self.check(policy)

    def test_changed_corroborating_capture_cannot_be_repinned(self):
        values = self.read(locations.CORROBORATION)
        record = locations.row(values, locations.CORROBORATING_ID)
        record['inspection']['capture']['sha256'] = '0' * 64
        self.put(locations.CORROBORATION, values)
        policy = deepcopy(self.policy)
        policy['mappings'][0]['corroboration']['sha256'] = hashlib.sha256(
            (self.root / locations.CORROBORATION).read_bytes()).hexdigest()
        policy['mappings'][0]['corroboration']['row_sha256'] = locations.digest(record)
        with self.assertRaisesRegex(ValueError, 'corroborating binding'):
            self.check(policy)

    def test_unsafe_or_uncorroborated_public_urls_rejected(self):
        for url in [locations.CAPTURE, 'http://www.st.com/resource/en/reference_manual/dm00031020.pdf',
                    'https://evil.example/resource/en/reference_manual/dm00031020.pdf',
                    'https://www.st.com/resource/en/reference_manual/another.pdf',
                    'https://www.st.com/resource/en/reference_manual/dm00031020.pdf#invented', None]:
            with self.subTest(url=url):
                self.reject(lambda p: p['mappings'][0].update(public_url=url))

    def test_missing_duplicate_or_unexpected_mappings_rejected(self):
        self.reject(lambda p: p.update(mappings=[]))
        self.reject(lambda p: p['mappings'].append(deepcopy(p['mappings'][0])))
        self.reject(lambda p: p['mappings'][0].update(id='R-another'))

    def test_extra_fields_rejected_at_every_policy_level(self):
        edits = [lambda p: p.update(resources=[]),
                 lambda p: p['mappings'][0].update(inspection_replaced=True)]
        for field in ('registration', 'inspection', 'corroboration'):
            edits.append(lambda p, field=field: p['mappings'][0][field].update(extra='unadmitted'))
        for edit in edits:
            self.reject(edit)

    def test_accepted_resource_is_not_admitted(self):
        baseline = self.read('curriculum/v5/baseline.json')
        baseline['resources'][locations.RESOURCE] = deepcopy(self.resource)
        self.put('curriculum/v5/baseline.json', baseline)
        with self.assertRaisesRegex(ValueError, 'accepted resource'):
            self.check()

    def test_missing_metadata_and_escape_attempt_rejected(self):
        (self.root / locations.INSPECTION).unlink()
        with self.assertRaisesRegex(ValueError, 'missing or redirected'):
            self.check()
        self.reject(lambda p: p['mappings'][0]['inspection'].update(path='../source_access.json'))

    def test_duplicate_current_resource_rejected(self):
        values = self.read('references/resources.json')
        values['resources'].append(deepcopy(self.resource))
        self.put('references/resources.json', values)
        with self.assertRaisesRegex(ValueError, 'duplicate resource'):
            self.check()

    def test_actual_current_projection_is_valid(self):
        self.assertEqual(locations.locations(), {
            locations.RESOURCE: 'https://www.st.com/resource/en/reference_manual/dm00031020.pdf'})


if __name__ == '__main__':
    unittest.main(verbosity=2)
