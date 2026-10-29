"""Fifth-edition structure: v4 inspection with manifest counts, accepted-identity preservation and manifest arithmetic."""
from copy import deepcopy
from common import ROOT, dump, load, read_json, sha
from check_v4 import inspect
from v5_common import RECEIPTS, V5, baseline, digest, identity, manifest

COUNT_KEYS = ['domains', 'projects', 'objectives', 'units', 'recall_sets', 'recall_variants', 'artifacts', 'families',
              'catalog_hours', 'core_hours', 'core_after_diagnostic_hours', 'implementation_verification_hours']
SUMMARY_KEYS = ['domains', 'projects', 'objectives', 'units', 'recall_sets', 'recall_variants', 'artifacts', 'families']


def inspection_expectations(total):
    return {'new_executable_units': total['new_executable_units'],
            'implementation_verification_hours': total['implementation_verification_hours'],
            'catalog_hours': total['catalog_hours'], 'core_hours': total['core_hours'],
            'recall_sets': total['recall_sets']}


def check_arithmetic(edition):
    errors = []
    expected = edition['expected']
    for key in COUNT_KEYS:
        if expected['preserved'].get(key, 0) + expected['added'].get(key, 0) != expected['total'].get(key):
            errors.append('Manifest arithmetic differs: ' + key)
    merged = dict(expected['preserved'].get('new_executable_units', {}))
    for prefix, count in expected['added'].get('new_executable_units', {}).items():
        merged[prefix] = merged.get(prefix, 0) + count
    if merged != expected['total'].get('new_executable_units'):
        errors.append('Manifest arithmetic differs: new_executable_units')
    new_domains = edition['new_domains']
    if len(set(new_domains)) != len(new_domains) or len(new_domains) != expected['added'].get('domains', 0):
        errors.append('Manifest new_domains disagree with the added domain count')
    return errors


def ledger_covers(path, expected_sha):
    """A teaching file may differ from its accepted bytes only through reviewed terminology hunks."""
    ledger_path = V5 / 'terminology_ledger.json'
    if not ledger_path.is_file():
        return False
    try:
        from check_terminology_ledger import reverse_applies
    except ImportError:
        return False
    return reverse_applies(path, expected_sha, read_json(ledger_path))


def publication_successor_covers(path, expected_sha):
    """Qualify only the two exact B06 public-asset attachments.

    The predecessor remains the accepted teaching artifact. This separate
    document proof does not reclassify the attachment as terminology, rewrite
    the accepted hash, or establish fresh source inspection or execution.
    """
    from native_teaching_revision import qualifies
    if qualifies(path, expected_sha, root=ROOT):
        return True
    if path not in {'curriculum/projects/PR-T28-06.md',
                    'curriculum/projects/PR-T29-04.md'}:
        return False
    from check_legacy_assessments import baseline_inspection_matches
    return baseline_inspection_matches({'path': path, 'sha256': expected_sha}, ROOT / path)


def check_preserved(base, data, artifacts, recall, projects, resources, taxonomy, *, publication_proof=None):
    # Public reproduction supplies a bounded document-only proof. The default
    # retains the complete historical author-evidence successor requirements.
    successor_covers = publication_proof if publication_proof is not None else publication_successor_covers
    errors = []
    for path, expected_sha in base['files'].items():
        file = ROOT / path
        if not file.is_file() or sha(file) != expected_sha:
            errors.append('Accepted fourth-edition file changed: ' + path)
    for path, expected_sha in base['teaching'].items():
        file = ROOT / path
        if not file.is_file():
            errors.append('Accepted teaching file missing: ' + path)
        elif (sha(file) != expected_sha and not ledger_covers(path, expected_sha)
              and not successor_covers(path, expected_sha)):
            errors.append('Accepted teaching prose changed without a terminology or exact publication-successor proof: ' + path)
    program = read_json(ROOT / 'curriculum/program.json')['order']
    if program[:len(base['program_order'])] != base['program_order']:
        errors.append('Accepted program order prefix changed')
    graph = read_json(ROOT / 'curriculum/project_graph.json')
    rows = {r['id']: r for r in graph['projects']}
    for pid, expected_digest in base['project_graph'].items():
        if pid not in rows or digest(rows[pid]) != expected_digest:
            errors.append('Accepted project graph row changed: ' + pid)
    if [p for p in graph['order'] if p in base['project_graph']] != base['project_order']:
        errors.append('Accepted project route order changed')
    by_resource = {r['id']: r for r in resources}
    for rid, expected_digest in base['resources'].items():
        if rid not in by_resource or digest(by_resource[rid]) != expected_digest:
            errors.append('Accepted resource record changed: ' + rid)
    objectives = {o['id']: o['text'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
    for oid, text in base['objectives'].items():
        if objectives.get(oid) != text:
            errors.append('Accepted objective changed: ' + oid)
    units = {u['id']: u for u in data['units']}
    for uid, expected_digest in base['units'].items():
        if uid not in units or digest(units[uid]) != expected_digest:
            errors.append('Accepted unit contract changed: ' + uid)
    if data['order'][:len(base['order'])] != base['order']:
        errors.append('Accepted unit route prefix changed')
    sets = {s['id']: s for s in recall['sets']}
    for sid, expected_digest in base['recall'].items():
        if sid not in sets or digest(sets[sid]) != expected_digest:
            errors.append('Accepted recall set changed: ' + sid)
    arts = {a['id']: a for a in artifacts['artifacts']}
    for aid, record in base['artifacts'].items():
        art = arts.get(aid)
        if art is None:
            errors.append('Accepted artifact missing: ' + aid)
            continue
        if digest({k: v for k, v in art.items() if k != 'consumers'}) != record['digest']:
            errors.append('Accepted artifact contract changed: ' + aid)
        if art['consumers'][:len(record['consumers'])] != record['consumers']:
            errors.append('Accepted artifact consumers reordered: ' + aid)
    return errors


def main():
    edition = manifest()
    total = edition['expected']['total']
    taxonomy, modules, projects, resources = load()
    data = read_json(V5 / 'units.json'); arts = read_json(V5 / 'artifacts.json')
    recall = read_json(V5 / 'recall.json'); budget = read_json(V5 / 'budget.json')
    errors, counts = inspect(data, arts, recall, budget, projects, taxonomy, resources, expected=inspection_expectations(total))
    errors += check_arithmetic(edition)
    base = baseline()
    errors += check_preserved(base, data, arts, recall, projects, resources, taxonomy)
    summary = read_json(V5 / 'summary.json')
    for key in SUMMARY_KEYS:
        if summary.get(key) != total.get(key):
            errors.append('Catalog summary differs from the manifest: ' + key)
    if summary.get('new_executable_units') != sum(total['new_executable_units'].values()):
        errors.append('Catalog summary differs from the manifest: new_executable_units')
    families = {p.parent.name for p in (ROOT / 'labs').glob('*/family.json')}
    if set(edition['retained_families']) | set(edition['v4_unit_families']) | set(edition['new_families']) != families:
        errors.append('Manifest family sets differ from labs/*/family.json')
    if set(edition['new_families']) & (set(edition['retained_families']) | set(edition['v4_unit_families'])):
        errors.append('A new family reuses an accepted family name')
    guards = []
    for label, change in [
        ('tampered-unit-contract', lambda d: d['units'][0].update(title=d['units'][0]['title'] + ' (changed)')),
        ('reordered-route-prefix', lambda d: d['order'].reverse()),
        ('missing-accepted-unit', lambda d: (d['units'].pop(0), d['order'].pop(0))),
    ]:
        sample = deepcopy(data); change(sample)
        rejected = bool(check_preserved(base, sample, arts, recall, projects, resources, taxonomy))
        guards.append({'fault': label, 'rejected': rejected})
        if not rejected:
            errors.append('Guard accepted ' + label)
    broken = deepcopy(edition); broken['expected']['total']['units'] += 1
    rejected = bool(check_arithmetic(broken)); guards.append({'fault': 'manifest-arithmetic', 'rejected': rejected})
    if not rejected:
        errors.append('Guard accepted manifest-arithmetic')
    report = {'schema_version': 5, 'status': 'passed' if not errors else 'failed', 'passed': not errors,
              'input_sha256': identity()['sha256'], 'counts': counts, 'baseline_commit': base['commit'],
              'preserved': base['counts'], 'negative_controls': guards, 'errors': errors,
              'scope': 'Authored progression, exact accounting, manifest arithmetic and preserved fourth-edition '
                       'identities. Exact document-only publication successors retain their accepted predecessor '
                       'hashes and claims; execution and learner evidence are separate.'}
    dump(ROOT / RECEIPTS / 'structure.json', report)
    print({**report, 'errors': errors[:25], 'error_count': len(errors)})
    if errors:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
