"""Isolated resource-discovery regressions; optional read-only current-tree audit."""
from copy import deepcopy
import argparse
import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from resource_inputs_v5 import discover, sha


class ResourceDiscovery(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(); self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.policy = {'schema_version': 1, 'evidence_only': {}, 'pinned_wrappers': {}, 'history_components': ['history', 'history_*', 'history-*']}
        self.row = {'id': 'R', 'title': 'Actual source', 'authors': ['Author'], 'edition': 'Version 1', 'inspection': {'scope': ['p1'], 'limits': 'Only selected passage'}, 'locators': ['p1'], 'assignable': False, 'domains': ['T1']}
        self.name = 'research/v5/expansion/base/resources.json'
        self.put(self.name, {'resources': [self.row]})

    def put(self, name, value):
        p = self.root/name; p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(json.dumps(value), encoding='utf8'); return p

    def collect(self):
        return discover(self.root, self.policy)

    def test_one_source_preserves_inspection_and_false_eligibility(self):
        values, report = self.collect()
        self.assertEqual(values, [self.row]); self.assertEqual(len(report['registration_provenance']['R']), 1)

    def test_exact_duplicates_preserve_both_provenances(self):
        self.put('research/v5/copy_resources.json', [self.row])
        values, report = self.collect(); self.assertEqual(values, [self.row]); self.assertEqual(len(report['registration_provenance']['R']), 2)

    def test_generated_identity_does_not_hide_work_conflict(self):
        other = deepcopy(self.row); other.update(work_id='old', title='Other source')
        self.put('research/v5/copy_resources.json', [other])
        with self.assertRaisesRegex(ValueError, 'Conflicting canonical'): self.collect()

    def test_same_work_different_inspection_is_not_deduplicated(self):
        other = deepcopy(self.row); other['inspection']['scope'] = ['p2']
        self.put('research/v5/copy_resources.json', [other])
        with self.assertRaisesRegex(ValueError, 'Conflicting canonical'): self.collect()

    def test_history_excluded_but_similar_active_directory_retained(self):
        for part in ('history', 'history_01', 'history-old'):
            self.put('research/v5/expansion/x/'+part+'/resources.json', {'invalid': True})
        other = deepcopy(self.row); other['id'] = 'ACTIVE'
        self.put('research/v5/expansion/historybook/resources.json', [other])
        values, report = self.collect(); self.assertEqual(len(values), 2); self.assertEqual(len(report['excluded_history']), 3)

    def test_unknown_malformed_register_rejected(self):
        self.put('research/v5/expansion/new/resources.json', {'captures': []})
        with self.assertRaisesRegex(ValueError, 'Malformed canonical'): self.collect()

    def test_missing_edition_not_invented(self):
        del self.row['edition']; self.put(self.name, [self.row])
        with self.assertRaisesRegex(ValueError, 'edition'): self.collect()

    def test_narrative_inspection_not_promoted(self):
        self.row['inspection'] = 'read it'; self.put(self.name, [self.row])
        with self.assertRaisesRegex(ValueError, 'inspection'): self.collect()

    def test_duplicate_in_one_register_fails(self):
        self.put(self.name, [self.row, self.row])
        with self.assertRaisesRegex(ValueError, 'duplicate'): self.collect()

    def evidence(self):
        name = 'research/v5/expansion/evidence/resources.json'
        p = self.put(name, [{'id': 'E', 'locators': ['bounded'], 'scope': 'author evidence'}])
        self.policy['evidence_only'][name] = {'sha256': sha(p), 'role': 'author-evidence-only', 'reason': 'Reviewed evidence shape', 'ids': ['E']}
        return name

    def test_explicit_evidence_is_preserved_without_reading_promotion(self):
        name = self.evidence(); values, report = self.collect()
        self.assertEqual(values, [self.row]); self.assertIn(name, report['evidence_only'])

    def test_changed_evidence_bytes_fail(self):
        name = self.evidence(); (self.root/name).write_text('[]', encoding='utf8')
        with self.assertRaisesRegex(ValueError, 'classification'): self.collect()

    def test_evidence_disappearance_fails(self):
        name = self.evidence(); (self.root/name).unlink()
        with self.assertRaisesRegex(ValueError, 'disappeared'): self.collect()

    def wrapper(self, kind='registered_passages'):
        name = 'research/v5/expansion/wrapper/resources.json'
        passage = {k: deepcopy(self.row[k]) for k in ('id', 'title', 'locators')}
        passage['scope'] = 'Reuse selected passage only'
        ref = {'path': self.name, 'sha256': sha(self.root/self.name), ('passages' if kind == 'retained_registration' else 'registered_passages'): [passage]}
        self.put(name, {'retained_registration': ref} if kind == 'retained_registration' else {'resources': [ref]})
        self.policy['pinned_wrappers'][name] = kind
        return name, ref

    def test_both_explicit_wrapper_shapes(self):
        for kind in ('registered_passages', 'retained_registration'):
            name, _ = self.wrapper(kind); values, report = self.collect()
            self.assertEqual(values, [self.row]); self.assertIn(name, report['pinned_wrappers'])

    def test_stale_wrapper_hash_fails(self):
        name, ref = self.wrapper(); ref['sha256'] = '0'*64; self.put(name, {'resources': [ref]})
        with self.assertRaisesRegex(ValueError, 'Stale pinned'): self.collect()

    def test_expanded_wrapper_locator_fails(self):
        name, ref = self.wrapper(); ref['registered_passages'][0]['locators'].append('p2'); self.put(name, {'resources': [ref]})
        with self.assertRaisesRegex(ValueError, 'Unregistered pinned'): self.collect()

    def test_wrong_wrapper_identity_fails(self):
        name, ref = self.wrapper(); ref['registered_passages'][0]['title'] = 'Wrong'; self.put(name, {'resources': [ref]})
        with self.assertRaisesRegex(ValueError, 'Conflicting pinned'): self.collect()

    def test_wrapper_escape_and_historical_target_fail(self):
        name, ref = self.wrapper()
        for path in ('../outside.json', 'research/v5/expansion/history/resources.json'):
            ref['path'] = path; self.put(name, {'resources': [ref]})
            with self.assertRaisesRegex(ValueError, 'Noncanonical'): self.collect()

    def copy(self):
        item = deepcopy(self.row); item.update(reuse_from=self.name, domains=['T1', 'T2'])
        name = 'research/v5/expansion/copy/resources.json'; self.put(name, [item]); return name, item

    def test_explicit_copy_extends_domains_only(self):
        self.copy(); values, report = self.collect()
        expected = deepcopy(self.row); expected['domains'].append('T2')
        self.assertEqual(values, [expected]); self.assertEqual(len(report['registration_provenance']['R']), 2)

    def test_copy_cannot_promote_eligibility_or_scope(self):
        name, item = self.copy(); item['assignable'] = True; self.put(name, [item])
        with self.assertRaisesRegex(ValueError, 'reuse scope'): self.collect()

    def test_copy_cannot_remove_producer_domain(self):
        name, item = self.copy(); item['domains'] = ['T2']; self.put(name, [item])
        with self.assertRaisesRegex(ValueError, 'reuse scope'): self.collect()

    def test_copy_requires_real_direct_producer(self):
        self.copy(); (self.root/self.name).unlink()
        with self.assertRaisesRegex(ValueError, 'producer missing'): self.collect()

    def test_reuse_alias_never_adds_inspection(self):
        self.put('research/v5/alias_resources.json', [{'id': 'R', 'reuse': True}])
        values, _ = self.collect(); self.assertEqual(values, [self.row])
        self.put('research/v5/alias_resources.json', [{'id': 'R', 'reuse': True, 'inspection': {'scope': 'invented'}}])
        with self.assertRaisesRegex(ValueError, 'unverified'): self.collect()

    def integrate(self, accepted=None):
        from unittest.mock import patch
        import integrate_v5
        self.put('research/v5/resource_merge_policy.json', self.policy)
        with patch.object(integrate_v5, 'ROOT', self.root), patch('v5_common.baseline', return_value={'resources': accepted or {}}):
            return integrate_v5.merge_resources()

    def test_integrated_merge_is_byte_fixed_point_and_retains_other_registry_fields(self):
        path=self.put('references/resources.json', {'schema_version': 2, 'retained': 'metadata', 'resources': []})
        self.assertEqual(self.integrate(), 1); first=path.read_bytes()
        self.assertEqual(self.integrate(), 1); self.assertEqual(path.read_bytes(), first)
        self.assertEqual(json.loads(first)['retained'], 'metadata')
        self.assertIn('R', json.loads(first)['v5_registration_provenance'])

    def test_accepted_change_fails_before_registry_write(self):
        prior=deepcopy(self.row); prior['inspection']['limits']='Accepted original limits'
        path=self.put('references/resources.json', {'resources': [prior]}); first=path.read_bytes()
        with self.assertRaisesRegex(ValueError, 'Accepted resource record changed'):
            self.integrate({'R': 'retained accepted identity'})
        self.assertEqual(path.read_bytes(), first)

    def test_unknown_reuse_fails_before_registry_write(self):
        self.put('research/v5/alias_resources.json', [{'id': 'UNKNOWN', 'reuse': True}])
        path=self.put('references/resources.json', {'resources': []}); first=path.read_bytes()
        with self.assertRaisesRegex(ValueError, 'Unknown reused resource'):
            self.integrate()
        self.assertEqual(path.read_bytes(), first)

    def correction(self):
        from resource_identity_v5 import digest
        prior=deepcopy(self.row); prior['authors']=['Placeholder author']
        path=self.put('references/resources.json', {'resources': [prior]})
        archive=self.put('research/v5/expansion/history_01/previous_registry.json', {'resources': [prior]})
        evidence=self.put('research/v5/identity_evidence.json', {'records': {'R': {'scope': 'Observed actual byline'}}})
        entry={'id':'R','previous':prior,'incoming':self.row,'previous_sha256':digest(prior),'incoming_sha256':digest(self.row),'identity_fields':['authors'],'evidence':{'path':evidence.relative_to(self.root).as_posix(),'sha256':sha(evidence),'id':'R'}}
        policy={'schema_version':1,'previous_registry':{'path':archive.relative_to(self.root).as_posix(),'sha256':sha(archive)},'corrections':[entry]}
        self.put('research/v5/resource_identity_corrections.json',policy)
        return path,policy

    def test_exact_reviewed_unaccepted_identity_correction_is_fixed_point(self):
        path,_=self.correction();self.integrate();first=path.read_bytes();self.integrate()
        self.assertEqual(first,path.read_bytes());self.assertEqual(json.loads(first)['resources'],[self.row])

    def test_reviewed_correction_never_overrides_accepted_row(self):
        path,_=self.correction();old=path.read_bytes()
        with self.assertRaisesRegex(ValueError,'Accepted resource'):self.integrate({'R':'accepted'})
        self.assertEqual(old,path.read_bytes())

    def test_wrong_current_bytes_cannot_use_old_correction(self):
        path,_=self.correction();old=path.read_bytes();self.row['inspection']['scope']=['unreviewed'];self.put(self.name,[self.row])
        with self.assertRaisesRegex(ValueError,'Conflicting fifth-edition'):self.integrate()
        self.assertEqual(old,path.read_bytes())

    def test_wrong_prior_bytes_cannot_use_old_correction(self):
        path,_=self.correction();value=json.loads(path.read_text());value['resources'][0]['authors']=['Other placeholder'];self.put('references/resources.json',value);old=path.read_bytes()
        with self.assertRaisesRegex(ValueError,'Conflicting fifth-edition'):self.integrate()
        self.assertEqual(old,path.read_bytes())

    def test_changed_identity_evidence_fails(self):
        path,_=self.correction();old=path.read_bytes();self.put('research/v5/identity_evidence.json',{})
        with self.assertRaisesRegex(ValueError,'identity correction evidence'):self.integrate()
        self.assertEqual(old,path.read_bytes())

    def test_changed_identity_archive_fails(self):
        path,_=self.correction();old=path.read_bytes();self.put('research/v5/expansion/history_01/previous_registry.json',{'resources':[]})
        with self.assertRaisesRegex(ValueError,'historical resource registry'):self.integrate()
        self.assertEqual(old,path.read_bytes())

    def test_correction_cannot_change_title_even_with_matching_hashes(self):
        from integrate_v5 import merge_resource_record
        from resource_identity_v5 import digest
        old=deepcopy(self.row);new=deepcopy(self.row);new['title']='Different work'
        correction={'id':'R','previous_sha256':digest(old),'incoming_sha256':digest(new),'identity_fields':['title']}
        with self.assertRaisesRegex(ValueError,'Conflicting fifth-edition'):merge_resource_record(old,new,False,correction)


def main():
    ap = argparse.ArgumentParser(); ap.add_argument('--current', action='store_true'); ap.add_argument('--output'); args = ap.parse_args()
    result = unittest.TextTestRunner(verbosity=1).run(unittest.defaultTestLoader.loadTestsFromTestCase(ResourceDiscovery))
    root = Path(__file__).resolve().parents[1]
    report = {'status': 'passed' if result.wasSuccessful() else 'failed', 'tests': result.testsRun, 'failures': len(result.failures), 'errors': len(result.errors), 'input_sha256': {p: sha(root/p) for p in ('tools/resource_inputs_v5.py', 'tools/resource_identity_v5.py', 'tools/integrate_v5.py', 'tools/check_resource_inputs_v5.py', 'research/v5/resource_merge_policy.json', 'research/v5/resource_identity_corrections.json')}}
    if args.current:
        try:
            from resource_identity_v5 import corrections
            from integrate_v5 import merge_resource_record
            from v5_common import baseline
            values, evidence = discover(root); reviewed=corrections(root)
            previous=json.loads((root/'references/resources.json').read_text(encoding='utf8'))
            old={r['id']:r for r in previous['resources']};accepted=baseline()['resources']
            for r in values:
                if r.get('reuse'):
                    if r['id'] not in old:raise ValueError('Unknown reused resource: '+r['id'])
                else:merge_resource_record(old.get(r['id'],{}),r,r['id'] in accepted,reviewed.get(r['id']))
            report.update(resource_count=len(values), discovery=evidence,read_only_registry_merge='passed',reviewed_identity_corrections=len(reviewed))
        except ValueError as exc:
            report.update(status='failed', current_error=str(exc))
    if args.output:
        p = root/args.output; p.parent.mkdir(parents=True, exist_ok=True); p.write_text(json.dumps(report, indent=2)+'\n', encoding='utf8')
    print(json.dumps({k: v for k, v in report.items() if k not in {'discovery', 'input_sha256'}}))
    return 0 if report['status'] == 'passed' else 1


if __name__ == '__main__':
    raise SystemExit(main())
