"""Validate lossless obligation origins and dimensional evidence, not mastery."""
from copy import deepcopy
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
from common import ROOT, dump, read_json
from reconcile_implementation_v5 import BASE, DIMENSIONS, discover_registers, extract, read_source, exercise_sources, linked_ids, has_gap_rows


def resolve(document, pointer):
    value = document
    for part in pointer.split('/')[1:]:
        key = part.replace('~1', '/').replace('~0', '~')
        value = value[int(key)] if isinstance(value, list) else value[key]
    return value


def validate(report):
    errors = []
    sources = {}
    for record in report.get('source_registers', []):
        path = ROOT / record['path']
        if not path.is_file():
            errors.append('Missing source: ' + record['path']); continue
        if hashlib.sha256(path.read_bytes()).hexdigest() != record['sha256']:
            errors.append('Stale source: ' + record['path'])
        sources[record['path']] = read_source(path)
    required, _ = discover_registers()
    for path in set(required) - set(sources):
        errors.append('Omitted register: ' + path)
    rows = report.get('obligations', [])
    projects = {p.stem for p in (ROOT/'curriculum/projects').glob('PR-*.md')}
    index = {r['id']: r for r in rows}
    if len(index) != len(rows): errors.append('Duplicate obligation identities')
    def explicit_gap_ids(value):
        if isinstance(value,dict):
            for key,child in value.items():
                if key in ('gaps','remaining','reference_gaps','pending','remaining_author_work',
                           'remaining_cross_scope','required_remaining','required_work',
                           'required_unexecuted_extensions','external_and_experiment_gaps') and isinstance(child,list):
                    for row in child:
                        if isinstance(row,dict) and isinstance(row.get('id'),str):yield row['id']
                if isinstance(child,(dict,list)):yield from explicit_gap_ids(child)
        elif isinstance(value,list):
            for child in value:yield from explicit_gap_ids(child)
    for source,document in required.items():
        for ident in explicit_gap_ids(document):
            if ident not in index:errors.append('Omitted registered work: '+ident+' in '+source)
    # Independent traversal of the canonical criterion/variant format: a missing
    # delayed variant must not vanish merely because extraction forgot a key.
    for path in sorted((BASE/'legacy_assessments').glob('PR-*.json')):
        contract = read_json(path)
        for criterion in contract['criteria']:
            for ident in [criterion['id'], *[v['id'] for v in criterion.get('variants', [])]]:
                if ident not in index: errors.append('Omitted assessment obligation: ' + ident)
                elif contract['project'] not in index[ident]['links']['projects'] or ident not in index[ident]['links']['criteria']:
                    errors.append('Missing exact assessment link: ' + ident)
    for path in exercise_sources():
        contract=read_source(path)
        identities=[contract['id'], *[r['id'] for r in contract['rubric']]]
        for key in ('delayed_transfer', 'delayed_assessment'):
            identities.extend(local if local.startswith('X-') else contract['id']+'-'+local
                              for local in contract.get(key, {}).get('variant_ids', []))
        for ident in identities:
            if ident not in index: errors.append('Omitted exercise obligation: ' + ident)
    for entry in rows:
        ident = entry['id']
        if set(entry.get('links',{}).get('projects',[])) - projects:errors.append('Unknown project link: ' + ident)
        if not entry.get('origins'): errors.append('No originating record: ' + ident)
        for origin in entry.get('origins', []):
            try:
                value = resolve(sources[origin['path']], origin['pointer'])
                if origin.get('role') == 'reference' and value != ident:
                    errors.append('Changed identity reference: ' + ident)
                elif origin.get('role') != 'reference' and not isinstance(value, dict):
                    errors.append('Origin is not an obligation object: ' + ident)
            except (KeyError, IndexError, ValueError, TypeError):
                errors.append('Unresolvable origin: ' + ident + ' ' + str(origin))
        for parent in entry.get('parent_ids', []):
            if parent not in index or parent == ident: errors.append('Invalid parent: ' + ident)
        if set(entry.get('dimensions', {})) != set(DIMENSIONS):
            errors.append('Missing evidence dimension: ' + ident)
        for name, dimension in entry.get('dimensions', {}).items():
            state = dimension.get('state')
            if state in {'complete', 'passed'} and not dimension.get('evidence'):
                errors.append('Unsupported completion: ' + ident + '/' + name)
            for evidence in dimension.get('evidence', []):
                path = (ROOT / evidence.split('#')[0]).resolve()
                if not path.is_relative_to(ROOT.resolve()) or not path.is_file():
                    errors.append('Missing/escaped evidence: ' + ident + '/' + evidence)
    return errors


def regression_controls(report):
    links = linked_ids('AX-PR-CAP-LINUX-01-T20.01.a-DELAYED, AX-PR-T18-01-T18.02.c and X-T70-002-R4.', {'PR-CAP-LINUX-01','PR-T18-01'})
    if links['projects']!=['PR-CAP-LINUX-01','PR-T18-01'] or links['criteria']!=['AX-PR-CAP-LINUX-01-T20.01.a-DELAYED','AX-PR-T18-01-T18.02.c','X-T70-002-R4']:
        raise AssertionError('Project or lowercase leaf identity truncated')
    fixture = {'pending': [{'id': 'P', 'variants': [{'id': 'P-DELAYED', 'minimum_days': 7}]}],
               'required_remaining': [{'id': 'Q'}], 'external_and_experiment_gaps': [{'id': 'R'}],
               'nested_support': {'remaining_author_work': [{'id': 'S', 'task': 'Original mechanism'}],
                                  'remaining_cross_scope': [{'id': 'T'}], 'reference_gaps': [{'id': 'U'}]}}
    actual = {(i, p) for i, _, _, p in extract(fixture, 'fixture.json')}
    if actual != {('P', None), ('P-DELAYED', 'P'), ('Q', None), ('R', None), ('S', None), ('T', None), ('U', None)}:
        raise AssertionError('Nested/hidden obligation regression')
    if not has_gap_rows({'wrapper': fixture['nested_support']}) or has_gap_rows({'rows': [{'id':'result-only'}]}):
        raise AssertionError('Content-based gap discovery confused work with result rows')
    registered={row['path'] for row in report['source_registers']}
    if 'research/v5/expansion/legacy_realtime_server/support.json' not in registered or not any(
            row['id']=='LRS-CLASSICAL-LONG-ACTIVE' and any(origin['path']=='research/v5/expansion/legacy_realtime_server/support.json' for origin in row['origins'])
            for row in report['obligations']):
        raise AssertionError('Support-envelope original author obligation missing')
    lost_author=deepcopy(report);lost_author['obligations']=[r for r in lost_author['obligations'] if r['id']!='LRS-CLASSICAL-LONG-ACTIVE']
    if 'Omitted registered work: LRS-CLASSICAL-LONG-ACTIVE in research/v5/expansion/legacy_realtime_server/support.json' not in validate(lost_author):
        raise AssertionError('Omitted support-envelope author work was not rejected')
    victim = next(r for r in report['obligations'] if r['id'].startswith('AX-') and r['id'].endswith('-DELAYED'))
    missing = deepcopy(report)
    missing['obligations'] = [r for r in missing['obligations'] if r['id'] != victim['id']]
    if not any('Omitted assessment obligation: ' + victim['id'] == e for e in validate(missing)):
        raise AssertionError('Missing variant was not rejected')
    unsupported = deepcopy(report)
    unsupported['obligations'][0]['dimensions']['implementation'] = {'state': 'complete', 'evidence': []}
    if not any(e.startswith('Unsupported completion:') for e in validate(unsupported)):
        raise AssertionError('Unsupported completion was not rejected')
    stale = deepcopy(report); stale['source_registers'][0]['sha256'] = '0'*64
    if not any(e.startswith('Stale source:') for e in validate(stale)):
        raise AssertionError('Changed source was not rejected')
    omitted=deepcopy(report)
    omitted['obligations']=[r for r in omitted['obligations'] if r['id'] != 'X-T70-002-DELAYED']
    if 'Omitted exercise obligation: X-T70-002-DELAYED' not in validate(omitted):
        raise AssertionError('Missing Markdown delayed variant was not rejected')
    local_a=next(r for r in report['obligations'] if r['id']=='X-T56-002-V1')
    local_b=next(r for r in report['obligations'] if r['id']=='X-T56-003-V1')
    if local_a['parent_ids']==local_b['parent_ids'] or local_a.get('original_local_id')!='V1':
        raise AssertionError('Exercise-local variant identities conflated')
    wrong=deepcopy(report);wrong['obligations'][0]['links']['projects'].append('PR-CAP-LINUX-01-T20')
    if not any(e.startswith('Unknown project link:') for e in validate(wrong)):raise AssertionError('Greedy project fragment accepted')
    return ['complete project/criterion identities preserved; greedy fragment rejected', 'nested pending and required_remaining retained', 'omitted delayed variant rejected',
            'unsupported implementation completion rejected', 'stale origin digest rejected',
            'omitted Markdown delayed variant rejected', 'reused V1 labels remain scoped to original exercises',
            'content-defined remaining_author_work/cross-scope/reference gaps retained; result rows are not work',
            'omitted support-envelope author work rejected independently']


def active_pin_errors(audit):
    """Require the current active-row audit without changing any dimension."""
    errors = []
    canonical = 'curriculum/v5/expansion/implementation_updates.json'
    canonical_bytes = (ROOT/canonical).read_bytes()
    expected = hashlib.sha256(canonical_bytes).hexdigest()
    actual_ids = [row['id'] for row in json.loads(canonical_bytes)['updates']]
    if audit.get('input') != {'path': canonical, 'sha256': expected}:
        errors.append('Active source audit does not bind current implementation updates')
    if audit.get('passed') is not True or audit.get('accepted') is not False:
        errors.append('Active source audit failed or improperly claims acceptance')
    for key in ('stale', 'missing', 'errors', 'unmatched_active_ids'):
        if audit.get(key) != []:
            errors.append('Active source audit '+key+': '+str(audit.get(key)))
    summary = audit.get('summary', {})
    if (len(actual_ids) != len(set(actual_ids))
            or not isinstance(summary.get('active_rows'), int) or summary.get('active_rows', 0) <= 0
            or summary.get('active_rows') != len(actual_ids)
            or summary.get('matched_rows') != summary.get('active_rows')
            or summary.get('stale') != 0 or summary.get('missing') != 0):
        errors.append('Active source audit lacks complete exact-row coverage')
    return errors


def active_pin_controls():
    canonical = 'curriculum/v5/expansion/implementation_updates.json'
    count = len(json.loads((ROOT/canonical).read_text(encoding='utf8'))['updates'])
    good = dict(input=dict(path=canonical, sha256=hashlib.sha256((ROOT/canonical).read_bytes()).hexdigest()),
                passed=True, accepted=False, stale=[], missing=[], errors=[], unmatched_active_ids=[],
                summary=dict(active_rows=count, matched_rows=count, stale=0, missing=0))
    assert not active_pin_errors(good)
    changes = [lambda d: d.update(passed=False), lambda d: d.update(accepted=True),
               lambda d: d['input'].update(sha256='0'*64),
               lambda d: d['stale'].append({'path': 'changed-current-provider.c'}),
               lambda d: d['missing'].append('required-source.c'),
               lambda d: d['errors'].append('malformed canonical provider'),
               lambda d: d['unmatched_active_ids'].append('unbound-active-obligation'),
               lambda d: d['summary'].update(matched_rows=0),
               lambda d: d['summary'].update(active_rows=count-1, matched_rows=count-1)]
    for change in changes:
        bad = deepcopy(good); change(bad)
        assert active_pin_errors(bad), 'Current active-source defect accepted by inventory validator'
    return len(changes)


if __name__ == '__main__':
    report = read_json(BASE/'work_inventory.json')
    errors = validate(report)
    controls = regression_controls(report)
    from audit_active_disposition_pins import audit, selftest
    source_audit = audit()
    errors.extend(active_pin_errors(source_audit))
    active_controls = active_pin_controls()
    successor_controls = selftest()
    result = {'passed': not errors, 'errors': errors, 'controls': controls,
              'active_disposition_audit': source_audit,
              'active_pin_negative_controls': active_controls,
              'historical_successor_negative_controls': successor_controls,
              'observed_utc': datetime.now(timezone.utc).isoformat(),
              'input_sha256': {str(p.relative_to(ROOT)).replace('\\', '/'):
                  hashlib.sha256(p.read_bytes()).hexdigest() for p in
                  (BASE/'work_inventory.json', Path(__file__), ROOT/'tools/reconcile_implementation_v5.py',
                   ROOT/'tools/audit_active_disposition_pins.py', ROOT/'tools/check_active_source_projections.py',
                   ROOT/'tools/check_expansion_campaigns.py', ROOT/'tools/rebind_modern_publication.py',
                   ROOT/'research/v5/modern/publication_utf8/current_binding.json')},
              'obligations': len(report['obligations']),
              'scope': 'Inventory traceability, evidence schema and exact current active disposition/source bindings; unresolved and external work remains unresolved.'}
    dump(ROOT/'reviews/v5/expansion/followthrough/inventory_check.json', result)
    display = dict(result)
    display['active_disposition_audit'] = dict(passed=source_audit['passed'], summary=source_audit['summary'])
    print(json.dumps(display, indent=2))
    raise SystemExit(bool(errors))
