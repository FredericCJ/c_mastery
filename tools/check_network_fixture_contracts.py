"""Networking fixture ownership and unobserved-template regression controls."""
from copy import deepcopy
import json
from pathlib import Path
import tempfile
import unittest

from common import ROOT, parse
import verify_networking_fixtures as milestone
import network_fixture_templates as capture
import check_network_arp_capture
import check_network_ip_capture
import check_network_tcp_capture


class NetworkingFixtureContracts(unittest.TestCase):
    def setUp(self):
        self.files = sorted(p.relative_to(ROOT).as_posix()
                            for p in (ROOT/'fixtures/networking').rglob('*') if p.is_file())
        self.documents = {p: json.loads((ROOT/p).read_text(encoding='utf-8')) for p in self.files}
        self.projects = {f'curriculum/projects/{pid}.md': parse(ROOT/f'curriculum/projects/{pid}.md')[0]
                         for pid in capture.PROJECTS}
        self.modules = {domain: parse(ROOT/f'curriculum/modules/{domain}.md')[0] for domain in ('T40', 'T41')}
        self.bindings = capture.load_bindings(ROOT)

    def validate(self, documents=None, files=None, bindings=None, projects=None):
        capture.validate_catalog(self.modules, self.projects if projects is None else projects,
                                 self.documents if documents is None else documents,
                                 self.files if files is None else files,
                                 milestone.validate_catalog,
                                 self.bindings if bindings is None else bindings)

    def test_current_complete_catalog_and_all_original_oracles(self):
        self.validate()
        self.assertEqual(len(capture.MILESTONES), 16)
        self.assertEqual(len(capture.TEMPLATES), 3)
        docs = [self.documents[p] for p in sorted(capture.MILESTONES)]
        self.assertEqual(sum(milestone.count(d['expected']) for d in docs), 249)
        for document in docs:
            milestone.check_doc(document)
        self.assertEqual(milestone.type_controls(docs), 4)
        self.assertEqual(milestone.catalog_controls(self.modules, self.projects,
                         {p: self.documents[p] for p in capture.MILESTONES}, sorted(capture.MILESTONES)), 3)

    def test_unknown_missing_duplicate_or_renamed_files_rejected(self):
        for extra in ('fixtures/networking/nested/orphan.json', 'fixtures/networking/notes.txt'):
            with self.subTest(extra=extra), self.assertRaises(ValueError):
                self.validate(files=self.files + [extra])
        for name in capture.TEMPLATES:
            with self.subTest(missing=name), self.assertRaises(ValueError):
                self.validate(files=[p for p in self.files if p != name])
            with self.subTest(duplicate=name), self.assertRaises(ValueError):
                self.validate(files=self.files + [name])
            with self.subTest(renamed=name), self.assertRaises(ValueError):
                self.validate(files=[p if p != name else 'fixtures/networking/nested/'+Path(name).name for p in self.files])

    def test_document_ownership_cannot_be_substituted(self):
        for name in capture.TEMPLATES:
            docs = deepcopy(self.documents); docs.pop(name)
            with self.subTest(name=name), self.assertRaises(ValueError):
                self.validate(documents=docs)
        docs = deepcopy(self.documents); docs['fixtures/networking/other.json'] = {'observed': False}
        with self.assertRaises(ValueError):
            self.validate(documents=docs)

    def test_template_required_fields_and_unknown_fields(self):
        for name in capture.TEMPLATES:
            for key in self.documents[name]:
                docs = deepcopy(self.documents); del docs[name][key]
                with self.subTest(name=name, missing=key), self.assertRaises(ValueError):
                    self.validate(documents=docs)
            docs = deepcopy(self.documents); docs[name]['expected'] = {'physical_gate': True}
            with self.subTest(name=name, extra='expected'), self.assertRaises(ValueError):
                self.validate(documents=docs)

    def test_observation_version_and_container_types_are_strict(self):
        for name in capture.TEMPLATES:
            for key, values in [('observed', [True, 0, None, 'false']), ('schema_version', [True, 1.0, 2])]:
                for value in values:
                    docs = deepcopy(self.documents); docs[name][key] = value
                    with self.subTest(name=name, key=key, value=value), self.assertRaises(ValueError):
                        self.validate(documents=docs)
            for key, value in self.documents[name].items():
                if isinstance(value, (list, dict)):
                    docs = deepcopy(self.documents); docs[name][key] = None
                    with self.subTest(name=name, container=key), self.assertRaises(ValueError):
                        self.validate(documents=docs)

    def test_populated_or_changed_blank_templates_are_not_canonical(self):
        changes = [
            ('arp', 'operator', 'claimed operator'), ('arp', 'local_mac', 'ffffffffffff'),
            ('arp', 'calibration', {'units': 'milliseconds', 'maximum_uncertainty_ms': 1, 'derivation': 'claimed'}),
            ('ip', 'raw_capture', 'claimed.pcap'), ('ip', 'echoes', [{}]),
            ('tcp', 'events', [{}]), ('tcp', 'environment', {'board': 'claimed'}),
        ]
        for kind, key, value in changes:
            docs = deepcopy(self.documents); docs[f'fixtures/networking/{kind}_physical_capture.json'][key] = value
            with self.subTest(kind=kind, key=key), self.assertRaises(ValueError):
                self.validate(documents=docs)

    def test_exact_supplement_consumers_and_duplicate_references(self):
        for pid in capture.CONSUMERS:
            for value in ([], [capture.CONSUMERS[pid]]*2,
                          ['fixtures/networking/arp_physical_capture.json', 'fixtures/networking/ip_physical_capture.json']):
                bindings = deepcopy(self.bindings); bindings[pid] = value
                with self.subTest(pid=pid, value=value), self.assertRaises(ValueError):
                    self.validate(bindings=bindings)
        bindings = deepcopy(self.bindings); bindings['PR-T40-01'] = ['fixtures/networking/tcp_physical_capture.json']
        with self.assertRaises(ValueError):
            self.validate(bindings=bindings)
        bindings = deepcopy(self.bindings); del bindings['PR-T41-08']
        with self.assertRaises(ValueError):
            self.validate(bindings=bindings)

    def test_original_project_and_expected_value_rejections_remain(self):
        projects = deepcopy(self.projects)
        projects['curriculum/projects/PR-T41-04.md']['fixtures'].append('fixtures/networking/arp_physical_capture.json')
        with self.assertRaises(ValueError):
            self.validate(projects=projects)
        for name in sorted(capture.MILESTONES):
            document = deepcopy(self.documents[name]); document['expected'] = {}
            with self.subTest(name=name), self.assertRaises(ValueError):
                milestone.check_doc(document)

    def test_blank_templates_remain_rejected_by_physical_gates(self):
        for kind, checker in [('arp', check_network_arp_capture), ('ip', check_network_ip_capture),
                              ('tcp', check_network_tcp_capture)]:
            errors = checker.validate(self.documents[f'fixtures/networking/{kind}_physical_capture.json'], ROOT/'fixtures/networking')
            self.assertIn('physical capture not observed', errors)

    def test_duplicate_json_fields_never_last_win(self):
        for text in ('{"observed":true,"observed":false}',
                     '{"calibration":{"units":"seconds","units":"milliseconds"}}',
                     '{"project":"PR-T41-04","supplied_artifacts":[],"supplied_artifacts":["claimed"]}'):
            with self.subTest(text=text), self.assertRaises(ValueError):
                capture.decode(text)

    def test_redirected_fixture_is_not_the_named_canonical_input(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary); target = root/'target.json'; target.write_text('{}', encoding='utf-8')
            self.assertEqual(capture.read_document(target, root), {})
            link = root/'fixture.json'
            try:
                link.symlink_to(target)
            except OSError as error:
                self.skipTest('This platform does not permit the isolated symlink control: ' + str(error))
            with self.assertRaises(ValueError):
                capture.read_document(link, root)

    def test_redirected_supplement_is_not_an_ownership_binding(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary); folder = root/'curriculum/v5/expansion/legacy_assessments'; folder.mkdir(parents=True)
            for pid in capture.PROJECTS:
                (folder/(pid+'.json')).write_text(json.dumps({'project': pid, 'supplied_artifacts': self.bindings[pid]}), encoding='utf-8')
            self.assertEqual(capture.load_bindings(root), self.bindings)
            path = folder/'PR-T41-04.json'; target = root/'source.json'; target.write_bytes(path.read_bytes()); path.unlink()
            try:
                path.symlink_to(target)
            except OSError as error:
                self.skipTest('This platform does not permit the isolated symlink control: ' + str(error))
            with self.assertRaises(ValueError):
                capture.load_bindings(root)


if __name__ == '__main__':
    unittest.main()
