"""Reconcile authored additions without modifying accepted counts or evidence.

This is accounting, not acceptance. Missing planned domains and unresolved
depth obligations remain explicit in the expansion ledger.
"""
from collections import Counter
from common import ROOT, dump, parse, read_json
from v5_common import baseline, domains_manifest
from author_v5 import vocabulary_sets


def reconcile():
    edition = read_json(ROOT/'curriculum/edition.json')
    base = baseline()
    recipes = domains_manifest()
    new_domains = sorted(recipes['domains'])
    accepted_families = set(edition['retained_families'] + edition['v4_unit_families'])
    families = {p.parent.name: read_json(p) for p in (ROOT/'labs').glob('*/family.json')}
    new_families = sorted(set(families)-accepted_families)
    modules = {key: parse(ROOT/'curriculum/modules'/f'{key}.md')[0] for key in new_domains}
    projects = [pid for module in modules.values() for pid in module['projects']]
    if len(projects) != len(set(projects)):
        raise ValueError('A new project has multiple module owners')
    for pid in projects:
        meta, _ = parse(ROOT/'curriculum/projects'/f'{pid}.md')
        if meta['module'] not in new_domains:
            raise ValueError('New project belongs to a different domain: '+pid)
    taxonomies = []
    for p in sorted((ROOT/'research/v5').glob('*_taxonomy.json')) + sorted((ROOT/'research/v5/modern').glob('*_taxonomy.json')):
        document = read_json(p)
        taxonomies += document.get('domains', [document] if 'id' in document else [])
    if sorted(d['id'] for d in taxonomies) != new_domains:
        raise ValueError('Authored taxonomy and domain recipes disagree')
    objectives = [o['id'] for d in taxonomies for t in d['subtopics'] for o in t['objectives']]
    if len(objectives) != len(set(objectives)) or set(objectives) & set(base['objectives']):
        raise ValueError('New objective collision')
    units = [u for f in families.values() for u in f.get('units', []) if u['project'] in projects]
    if len({u['id'] for u in units}) != len(units):
        raise ValueError('New unit collision')
    counts = dict(Counter(u['id'].split('-')[1] for u in units if u.get('new_executable')))
    if counts != recipes['new_executable_counts']:
        raise ValueError('Declared executable counts differ from authored units')
    recall = [s for key in new_domains for s in read_json(ROOT/'curriculum/v5/recall'/f'{key}.json')['sets']]
    vocabulary = [s for s in vocabulary_sets() if s['id'] not in base['recall']]
    artifacts = [a['id'] for d in recipes['domains'].values() for a in d.get('artifacts', [])]
    if len(artifacts) != len(set(artifacts)) or set(artifacts) & set(base['artifacts']):
        raise ValueError('New artifact collision')
    added_hours = sum(sum(m['workload'].values()) for m in modules.values())
    impl_verify = sum(m['workload']['implementation']+m['workload']['verification'] for m in modules.values())
    added = dict(domains=len(new_domains), projects=len(projects), objectives=len(objectives), units=len(units),
                 recall_sets=len(recall)+len(vocabulary), recall_variants=sum(len(s['variants']) for s in recall+vocabulary),
                 artifacts=len(artifacts), families=len(new_families), new_executable_units=counts,
                 catalog_hours=added_hours, core_hours=added_hours, core_after_diagnostic_hours=added_hours,
                 implementation_verification_hours=impl_verify, vocabulary_sets=len(vocabulary))
    # Every new project is required core work. A future elective needs explicit accounting.
    for pid in projects:
        if parse(ROOT/'curriculum/projects'/f'{pid}.md')[0]['status'] != 'core':
            raise ValueError('Explicit elective accounting is required: '+pid)
    preserved = edition['expected']['preserved']
    total = {k: preserved.get(k, 0)+v for k, v in added.items() if k not in ('new_executable_units','vocabulary_sets')}
    total['new_executable_units'] = dict(preserved['new_executable_units'])
    for key, value in counts.items():
        total['new_executable_units'][key] = total['new_executable_units'].get(key, 0)+value
    edition['expected']['added'] = added
    edition['expected']['total'] = total
    edition['new_domains'] = new_domains
    edition['new_families'] = new_families
    edition['receipts_dir'] = 'reviews/v5/expansion/integrated'
    expansion = edition['exercise_expansion']
    from research_batches_v5 import discover
    research_batches, author_evidence_batches = discover()
    expansion.update(batch='B01-B08', research_batches=research_batches,
                     author_evidence_batches=author_evidence_batches,
                     required_exercises=len(list((ROOT/'curriculum/v5/exercises').glob('X-*.md'))),
                     validation_receipt='reviews/v5/expansion/integrated/validation.json',
                     status='authored-additions-under-validation',
                     planned_domains_not_yet_authored=sorted({f'T{i}' for i in range(57,77)}-set(new_domains)))
    expansion['fixture_oracles'] = sorted({oracle for f in families.values() if isinstance(f.get('campaign'), dict)
                                         and (oracle:=f['campaign'].get('fixture_oracle')) and oracle.startswith('tools/verify_')}
                                        | {'tools/verify_foundation_depth_fixtures.py'})
    # check_fixtures accepts tool basenames; family-local oracles run in their campaigns.
    expansion['fixture_oracles'] = [p.removeprefix('tools/') for p in expansion['fixture_oracles']]
    for receipt in ('expansion_campaigns.json','modern_coverage.json','authored_content.json'):
        if receipt not in edition['required_receipts']:edition['required_receipts'].append(receipt)
        if receipt not in expansion['required_batch_receipts']:expansion['required_batch_receipts'].append(receipt)
    dump(ROOT/'curriculum/edition.json', edition)
    return dict(added=added, expected=total, planned_domains_not_yet_authored=expansion['planned_domains_not_yet_authored'])


if __name__=='__main__':print(reconcile())
