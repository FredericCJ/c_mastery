"""Derive the completion/continuation ledger from concrete canonical work."""
from collections import Counter

from common import ROOT, dump, load_taxonomy, read_json
from v5_common import baseline

BASE = ROOT / 'curriculum/v5/expansion'


def generate():
    taxonomy = load_taxonomy()
    leaves = {d['id']: {o['id'] for t in d['subtopics'] for o in t['objectives']} for d in taxonomy}
    supplements = [read_json(p) for p in (BASE / 'legacy_assessments').glob('PR-*.json')]
    criteria = [c for record in supplements for c in record['criteria']]
    assessed_projects = {record['project'] for record in supplements}
    accepted_projects = set(baseline()['projects'])
    integration_projects = {pid for pid in accepted_projects if not pid.startswith('PR-T')}
    # Count retained domain criteria separately from the cross-domain integration
    # criteria. Multiple criteria can assess the same leaf under distinct conditions.
    domain_criteria = Counter(c['objective'] for record in supplements
                              if record['project'] in accepted_projects - integration_projects
                              for c in record['criteria'])
    missing_projects = sorted(accepted_projects - assessed_projects)
    audited = {c['objective'] for c in criteria}
    fragments = sorted(p.stem for p in (ROOT / 'curriculum/v5/domains').glob('T*.json'))
    old = read_json(BASE / 'remaining_work.json')
    if not any(batch['id'] == 'B08' for batch in old['batches']):
        old['batches'].append({'id': 'B08',
            'scope': 'All twelve original checkpoints and eight Linux/MCU capstone projects: composition, failure, containment and independent transfer.',
            'exit': 'Every retained integration project has source-bound supplemental criteria, concrete inputs, separate assessor analysis and explicit target evidence limits.'})
    ranges = {'B02': (1, 22), 'B03': (23, 35), 'B04': (36, 45), 'B05': (46, 56)}
    for batch in old['batches']:
        bid = batch['id']
        if bid in ranges:
            low, high = ranges[bid]
            scope = set().union(*(v for k, v in leaves.items() if low <= int(k[1:]) <= high))
            missing = sorted(scope - audited)
            batch.update(status='assessment-authored-execution-obligations-open' if not missing else 'substantive-depth-review-in-progress',
                         legacy_leaf_count=len(scope), assessed_leaf_count=len(scope & audited),
                         explicit_assessment_criteria=sum(domain_criteria[oid] for oid in scope),
                         remaining_leaf_ids=missing,
                         evidence='curriculum/v5/expansion/legacy_assessment_catalog.json',
                         execution_limit='Existing and added laboratory anchors assess named mechanisms only. '
                                         'Each criterion retains its own execution and transfer obligations.')
        elif bid in {'B06', 'B07'}:
            low, high = (57, 64) if bid == 'B06' else (65, 76)
            owned = [f'T{i}' for i in range(low, high + 1)]
            batch.update(status='authored-laboratories-with-scoped-campaigns' if set(owned) <= set(fragments)
                         else 'authoring-in-progress', authored_domains=[d for d in owned if d in fragments],
                         evidence='reviews/v5/expansion/integrated/expansion_campaigns.json',
                         execution_limit='Current scoped pins, controls and captured results must pass '
                                         'tools/check_expansion_campaigns.py. Written variants and target adapters '
                                         'beyond those anchors remain separate required work.')
        elif bid == 'B08':
            missing = sorted(integration_projects - assessed_projects)
            batch.update(status='assessment-authored-execution-obligations-open' if not missing else 'authoring-in-progress',
                         projects=sorted(integration_projects), remaining_project_ids=missing,
                         explicit_assessment_criteria=sum(len(r['criteria']) for r in supplements if r['project'] in integration_projects),
                         evidence='curriculum/v5/expansion/legacy_assessment_catalog.json',
                         execution_limit='Composition criteria require the actual learner components and target evidence. Partial integration anchors do not establish the complete capstone.')
    old['current_batch'] = 'integrated; full legacy depth and cross-domain composition'
    dump(BASE / 'remaining_work.json', old)
    remaining = [{'domain': key, 'leaf_count': len(value), 'assessed_leaf_count': len(value & audited),
                  'explicit_criteria': sum(domain_criteria[oid] for oid in value),
                  'remaining_leaf_ids': sorted(value - audited)}
                 for key, value in leaves.items() if int(key[1:]) <= 56]
    gap_registers = sorted(p.relative_to(ROOT).as_posix() for p in (ROOT / 'research/v5').rglob('*.json')
                           if any(word in p.stem for word in ('gap_ledger', 'remaining_work', 'depth_dispositions')))
    dump(BASE / 'completion_ledger.json', {
        'schema_version': 1,
        'policy': 'Whole scope retained; no leaf is closed by a heading or test name. Authored work, '
                  'scoped author execution, independent assessment and physical validation are distinct.',
        'authored_new_domains': fragments, 'legacy_assessment_criteria': len(criteria),
        'legacy_project_dispositions': {'accepted_projects': len(accepted_projects),
                                        'projects_with_assessments': len(accepted_projects & assessed_projects),
                                        'remaining_project_ids': missing_projects},
        'legacy_leaf_dispositions': remaining, 'required_gap_registers': gap_registers,
        'execution_validation': {'producer': 'tools/check_expansion_campaigns.py',
                                 'report': 'reviews/v5/expansion/integrated/expansion_campaigns.json'},
        'whole_plan_status': 'in-progress' if missing_projects or any(r['remaining_leaf_ids'] for r in remaining)
                             else 'authored-with-explicit-execution-and-review-obligations',
        'independent_assessment': 'not-inferred', 'physical_validation': 'not-inferred',
        'historical_evidence': 'curriculum/v5/expansion/historical_receipts.json'})
    gaps = read_json(BASE / 'gaps.json')
    by_id = {g['id']: g for g in gaps['gaps']}
    by_id['G-BASE-DEPTH'].update(scope='Every retained leaf and project, including checkpoints and capstones, without a substantive disposition in completion_ledger.json',
                               evidence='curriculum/v5/expansion/legacy_assessment_catalog.json',
                               status='in-progress' if missing_projects or any(r['remaining_leaf_ids'] for r in remaining) else 'authored-assessments')
    by_id['G-NEW-DOMAINS'].update(status='authored-scoped-laboratories; target-and-variant-gaps-remain',
                                evidence=['curriculum/v5/domains.json', *gap_registers],
                                resolution='Retain and complete each required implementation/target/variant gap in the linked registers. '
                                           'All20 domain contracts now exist; current author campaigns must validate scoped source pins.')
    by_id['G-MODERN-DEPENDENCY'].update(status='authored-and-executed-scoped-composition',
                                      evidence=['labs/dod/CONTRACTS.md', 'labs/dispatch/family.json',
                                                'labs/modern_composition/CONTRACTS.md'],
                                      resolution='Revalidate actual imported source pins and composition campaigns after changes; '
                                                 'no compiler-enforced ownership or universal abstraction equivalence is claimed.')
    by_id['G-V5-DOMAIN-TOOLS'].update(status='implemented; integrated-validation-required',
                                   evidence=['tools/v5_common.py', 'tools/integrate_v5.py',
                                             'tools/check_authored_v5.py', 'tools/check_v5_authoring_contracts.py'],
                                   resolution='Run the current full build and current source-bound validation; preserve accepted baseline records.')
    by_id['G-CORPUS']['evidence'] = ['research/v5/modern/coverage', 'curriculum/v5/expansion/completion_ledger.json']
    dump(BASE / 'gaps.json', gaps)
    return {'legacy_criteria': len(criteria), 'remaining_legacy_leaves': sum(len(r['remaining_leaf_ids']) for r in remaining),
            'remaining_legacy_projects': len(missing_projects),
            'new_domains': len(fragments), 'gap_registers': len(gap_registers)}


if __name__ == '__main__':
    print(generate())
