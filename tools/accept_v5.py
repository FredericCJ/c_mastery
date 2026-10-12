"""Accept the fifth-edition delivery from current receipts; learner evidence stays separate."""
from datetime import datetime, timezone
from common import ROOT, dump, read_json, sha
from accept_v4 import pinned_receipt as pinned_v4
from v2_common import source_identity
from v5_common import RECEIPTS, V5, manifest

COUNT_KEYS = ['domains', 'projects', 'objectives', 'units', 'recall_sets', 'recall_variants', 'artifacts', 'families']
LEARNER_STATUS = {'learner_time_calibration': {'not-run'}, 'learning_effectiveness': {'not-measured'},
                  'learner_mastery': {'not-assessed'}}


def pinned_receipt(name, receipts=RECEIPTS):
    return pinned_v4(name, receipts)


def main():
    edition = manifest()
    identity = source_identity()
    publication = read_json(ROOT / RECEIPTS / 'publication.json')
    if publication.get('status') != 'passed' or publication.get('input_sha256') != identity['sha256']:
        raise ValueError('Current source-bound publication validation is required')
    receipts = {name: pinned_receipt(name) for name in edition['required_receipts']}
    matrix = read_json(ROOT / RECEIPTS / 'legacy_matrix.json')
    if {x['family'] for x in matrix['families'] if x['passed']} != set(edition['retained_families']):
        raise ValueError('Retained-family execution coverage differs')
    counts = read_json(V5 / 'summary.json')
    total = edition['expected']['total']
    for key in COUNT_KEYS:
        if counts.get(key) != total.get(key):
            raise ValueError('Release count contract differs: ' + key)
    if counts.get('new_executable_units') != sum(total['new_executable_units'].values()):
        raise ValueError('Release count contract differs: new_executable_units')
    status = dict(edition['status'])
    physical = status.get('physical_validation')
    if physical != 'not-run':
        campaign = ROOT / RECEIPTS / 'physical_matrix.json'
        if not campaign.is_file() or not read_json(campaign).get('passed'):
            raise ValueError('A physical validation status other than not-run requires a passed physical_matrix.json')
        receipts['physical_matrix.json'] = pinned_receipt('physical_matrix.json')
    for key, allowed in LEARNER_STATUS.items():
        if status.get(key) not in allowed:
            raise ValueError('Manifest claims learner evidence that no receipt can establish: ' + key)
    report = {'schema_version': 5, 'status': 'accepted', 'accepted': True, 'passed': True,
              'recorded_at': datetime.now(timezone.utc).isoformat(), 'input_sha256': identity['sha256'],
              'edition': edition['edition'], 'baseline_commit': edition['edition_baseline_commit'],
              'new_domains': edition['new_domains'], 'new_families': edition['new_families'],
              'receipts': receipts, 'publication_sha256': sha(ROOT / RECEIPTS / 'publication.json'), 'counts': counts,
              'scope': 'Authored and executable fifth-edition delivery, preserved fourth-edition identities, '
                       'validated host gates and declared target evidence.',
              'physical_observations': physical, 'learner_mastery': status['learner_mastery'],
              'learning_effectiveness': status['learning_effectiveness'],
              'learner_time_calibration': status['learner_time_calibration'],
              'open_evidence': ['Physical interface and board captures close only through recorded dossiers; '
                                'author bench runs are not learner calibration.',
                                'Independent learner implementations, explanations, formal arguments, delayed transfer '
                                'and time calibration are not established by author tests.']}
    dump(ROOT / RECEIPTS / 'acceptance.json', report)
    print('Fifth-edition delivery accepted. Physical and learner evidence remain explicitly separate.')


if __name__ == '__main__':
    main()
