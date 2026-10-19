"""Lossless obligation index, downstream of existing gap and criterion records.

This does not infer implementation completion from an authored heading. Original
records remain authoritative; explicit, source-bound dispositions add evidence
without rewriting historical snapshots. Generated pointers retain all origins.
"""
from __future__ import annotations
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
from common import ROOT, dump, read_json, parse

BASE = ROOT / 'curriculum/v5/expansion'
DIMENSIONS = ('teaching', 'implementation', 'author_validation', 'external_review',
              'independent_learner', 'physical_observation')
COLLECTIONS = {'gaps', 'remaining', 'criteria', 'subtopics', 'dispositions',
               'objectives', 'required_unexecuted_extensions',
               'external_and_experiment_gaps', 'changed_condition', 'pending',
               'required_remaining', 'required_work', 'variants', 'rows', 'rubric',
               'remaining_author_work', 'remaining_cross_scope', 'reference_gaps'}
GAP_COLLECTIONS = {'gaps', 'remaining', 'reference_gaps', 'pending',
                   'remaining_author_work', 'remaining_cross_scope',
                   'required_remaining', 'required_work',
                   'required_unexecuted_extensions', 'external_and_experiment_gaps'}
WORK = ('required_task', 'remaining_work', 'requirement', 'description', 'work',
        'remaining', 'task', 'resolution', 'scope', 'criterion', 'required', 'missing')
PATH = re.compile(r'(?:curriculum|research|labs|assessor|fixtures|reviews|tools)/[\w./-]+\.(?:json|md|py|c|h)')

def read_source(path):
    """Markdown pointers address the original fenced JSON contract, never prose."""
    return parse(path)[0] if Path(path).suffix == '.md' else read_json(path)

def exercise_sources():
    return sorted((ROOT / 'curriculum/v5/exercises').glob('X-*.md'))

def strings(value):
    if isinstance(value, str):
        yield value
    elif isinstance(value, dict):
        for child in value.values():
            yield from strings(child)
    elif isinstance(value, list):
        for child in value:
            yield from strings(child)

def leaves(value, pointer=''):
    """Retain references as well as defining rows, without minting duplicate work."""
    if isinstance(value, dict):
        for key, child in value.items():
            yield from leaves(child, pointer + '/' + key.replace('~', '~0').replace('/', '~1'))
    elif isinstance(value, list):
        for index, child in enumerate(value):
            yield from leaves(child, pointer + '/' + str(index))
    elif isinstance(value, str):
        yield pointer, value

def discover_registers():
    """Include previously omitted names and follow JSON references recursively."""
    declared = read_json(BASE / 'completion_ledger.json')['required_gap_registers']
    seeds = {ROOT / name for name in declared} | {BASE / 'gaps.json'}
    for folder in (ROOT / 'research/v5', BASE):
        for p in folder.rglob('*.json'):
            # A support/resource envelope can contain original obligations even
            # when its filename says nothing about gaps. Do not seed our own
            # derived inventory or disposition overlays as original evidence.
            if p in (BASE / 'work_inventory.json', BASE / 'implementation_updates.json'):
                continue
            if any(word in p.stem for word in ('gap_ledger', 'remaining', 'depth_disposition',
                                               'depth_audit', 'objective_disposition')) or has_gap_rows(read_json(p)):
                seeds.add(p)
    seeds.discard(BASE / 'remaining_work.json')  # batch accounting, linked separately
    seeds.update(exercise_sources())
    pending, scanned, referenced = list(seeds), {}, set()
    while pending:
        p = pending.pop()
        relative = p.relative_to(ROOT).as_posix()
        if relative in scanned:
            continue
        data = read_source(p)
        scanned[relative] = data
        for value in strings(data):
            for name in PATH.findall(value):
                linked = ROOT / name
                if linked.is_file():
                    referenced.add(name)
                    if linked.suffix == '.json' and linked not in (BASE / 'work_inventory.json', BASE / 'implementation_updates.json') and (any(word in linked.stem for word in
                                                        ('gap', 'remaining', 'disposition')) or has_gap_rows(read_json(linked))):
                        pending.append(linked)
    return scanned, sorted(referenced)


def has_gap_rows(value):
    """Recognize declared work collections, not arbitrary result/coverage rows."""
    if isinstance(value, dict):
        for key, child in value.items():
            if key in GAP_COLLECTIONS and isinstance(child, list) and any(
                    isinstance(row, dict) and any(k in row for k in ('id', 'criterion', 'task', 'work', 'required_task'))
                    for row in child):
                return True
            if isinstance(child, (dict, list)) and has_gap_rows(child):
                return True
    elif isinstance(value, list):
        return any(has_gap_rows(child) for child in value)
    return False

def identifier(record, origin, pointer):
    for key in ('id', 'criterion'):
        if isinstance(record.get(key), str):
            return record[key]
    supplements = record.get('assessment_supplements', [])
    if len(supplements) == 1 and supplements[0].get('criterion'):
        return supplements[0]['criterion']
    # Unnamed rows get content-derived identities, not unstable array numbers.
    text = json.dumps(record, sort_keys=True, ensure_ascii=False)
    return 'IW-' + hashlib.sha256((origin + '\n' + text).encode()).hexdigest()[:16]

def variant_requirement(body, local, siblings):
    """Retain each exact task, including V1/V2 in one prose paragraph."""
    section = re.search(r'(?ms)^## (?:Variants|Changed.condition variants|Delayed transfer)\s*\n(.*?)(?=^## |\Z)', body)
    text = section[1] if section else body
    timing = r'(?:\s+after\s*\d+\s*days)?\s*:'
    marker = re.search(r'(?<![\w-])' + re.escape(local) + timing, text)
    if marker:
        rest = text[marker.end():]
        labels = '|'.join(re.escape(value) for value in siblings)
        end = re.search(r'(?<![\w-])(?:' + labels + r')' + timing + r'|\n\s*\n', rest)
        return text[marker.start():marker.end() + (end.start() if end else len(rest))].strip()
    match = re.search(r'(?m)^' + re.escape(local) + r'\b(.*?)(?=\n\s*\n|\Z)', text, re.S)
    return (local + match[1]).strip() if match else None


def extract(data, origin, pointer='', parent=None):
    if not pointer and origin.endswith('.md') and isinstance(data, dict) and data.get('id', '').startswith('X-'):
        parent = data['id']
        yield parent, '', data, None
        # Both forms are existing contracts. Keep the original variant ID, the
        # original delayed-contract pointer and its actual changed assumptions.
        body = parse(ROOT / origin)[1]
        for key in ('delayed_transfer', 'delayed_assessment'):
            delayed = data.get(key, {})
            for local in delayed.get('variant_ids', []):
                # V1/V2 are local labels reused by many exercises, not global
                # identities. Preserve them explicitly and scope the index key.
                ident=local if local.startswith('X-') else parent+'-'+local
                requirement = variant_requirement(body, local, delayed.get('variant_ids', []))
                row = dict(delayed, id=ident, original_local_id=local, requirement=requirement if requirement else
                           'Implement and assess the named changed-condition variant in ' + origin + '#variants; inspect its assessor analysis before adjudicating completion.')
                yield ident, '/' + key, row, parent
    if isinstance(data, dict):
        for key, value in data.items():
            child_pointer = pointer + '/' + key.replace('~', '~0').replace('/', '~1')
            if key in COLLECTIONS and isinstance(value, list):
                for index, row in enumerate(value):
                    where = child_pointer + '/' + str(index)
                    if not isinstance(row, dict):
                        continue
                    own = identifier(row, origin, where)
                    yield own, where, row, parent
                    yield from extract(row, origin, where, own)
            elif isinstance(value, (dict, list)):
                yield from extract(value, origin, child_pointer, parent)
    elif isinstance(data, list):
        for index, child in enumerate(data):
            yield from extract(child, origin, pointer + '/' + str(index), parent)

def linked_ids(text, project_ids):
    """Recognize complete catalog identities inside criteria, not greedy prefixes."""
    # AX-PR-CAP-LINUX-01-T20.01.a contains project PR-CAP-LINUX-01;
    # an uppercase-only token regex incorrectly returned ...-T20 and dropped a.
    project_pattern = r'\b(?:' + '|'.join(map(re.escape, sorted(project_ids, key=lambda s: (-len(s), s)))) + r')(?=$|[^A-Za-z0-9])'
    return {
        'objectives': sorted(set(re.findall(r'\bT\d{2}\.\d{2}\.[a-z]\b', text))),
        'projects': sorted(set(re.findall(project_pattern, text))),
        'exercises': sorted(set(re.findall(r'\bX-T\d{2}-\d{3}(?:-DELAYED)?', text))),
        'criteria': sorted(set(re.findall(r'\b(?:AX-PR-[A-Z0-9-]+-T\d{2}\.\d{2}\.[a-z](?:-[A-Z0-9-]+)?|X-T\d{2}-\d{3}-R\d+)\b', text))),
        'domains': sorted(set(re.findall(r'\bT\d{2}\b', text)))
    }

def add_source_requirements(entry, row, origin):
    """Retain heterogeneous canonical gap fields without inferring completion."""
    for field in ('status', 'state'):
        if row.get(field):
            state = {'path': origin, 'field': field, 'status': row[field]}
            if state not in entry['source_states']:
                entry['source_states'].append(state)
    for key in WORK:
        if isinstance(row.get(key), str) and row[key] not in entry['requirements']:
            entry['requirements'].append(row[key])
    for field in ('closure', 'completion', 'completion_criteria'):
        for value in strings(row.get(field, [])):
            if value not in entry['completion_criteria']:
                entry['completion_criteria'].append(value)
    for field, aliases in [('prerequisites', ('prerequisites', 'dependency')),
                           ('affected_consumers', ('affected_consumers', 'affected', 'affected_claims')),
                           ('existing_artifact_claims', ('implemented', 'available_support', 'completed')),
                           ('source_access_attempts', ('attempt', 'attempts')),
                           ('evidence_limits', ('limits', 'limitations', 'closure_limit'))]:
        for alias in aliases:
            for value in strings(row.get(alias, [])):
                if value not in entry.setdefault(field, []):
                    entry[field].append(value)


def generate():
    documents, referenced = discover_registers()
    project_ids = {p.stem for p in (ROOT/'curriculum/projects').glob('PR-*.md')}
    supplement_rows = {}
    criteria_by_objective = {}
    for p in sorted((BASE / 'legacy_assessments').glob('PR-*.json')):
        data = read_json(p)
        documents[p.relative_to(ROOT).as_posix()] = data
        for row in data['criteria']:
            supplement_rows[row['id']] = (p.relative_to(ROOT).as_posix(), row, data)
            criteria_by_objective.setdefault(row['objective'], []).append(row['id'])
    updates_path = BASE / 'implementation_updates.json'
    updates = read_json(updates_path) if updates_path.is_file() else {'updates': []}
    explicit = {row['id']: row for row in updates['updates']}
    if len(explicit) != len(updates['updates']):
        raise ValueError('Duplicate implementation disposition')
    inventory = {}
    for origin, document in sorted(documents.items()):
        for ident, pointer, row, parent in extract(document, origin):
            candidates = criteria_by_objective.get(row.get('objective'), [])
            if ident.startswith('IW-') and len(candidates) == 1:
                ident = candidates[0]
            entry = inventory.setdefault(ident, {'id': ident, 'origins': [], 'parent_ids': [],
                'links': {'domains': [], 'objectives': [], 'projects': [], 'exercises': [], 'criteria': []},
                'requirements': [], 'source_states': [], 'artifacts': [],
                'dimensions': {d: {'state': 'unreconciled', 'evidence': []} for d in DIMENSIONS},
                'completion_criteria': [], 'prerequisites': [], 'affected_consumers': [],
                'validation_method': 'Inspect the exact linked contract and criterion, implement its named variant, then run its mechanism-specific positive/negative checks. Do not substitute unrelated anchor tests.',
                'required_environment': 'Resolve from linked project/exercise and target profile before execution.',
                'external_blockers': [], 'evidence_limits': []})
            entry['origins'].append({'path': origin, 'pointer': pointer})
            if origin.endswith('.md'):
                entry['artifacts'] = sorted(set(entry['artifacts']) | {origin})
                if row.get('environment'):
                    entry['required_environment'] = row['environment']
                for field in ('prerequisite_exercises', 'prerequisite_artifacts'):
                    entry['prerequisites'] = sorted(set(entry['prerequisites']) | set(strings(row.get(field, []))))
            declared_parents = row.get('parent_ids', [])
            if not isinstance(declared_parents, list) or any(
                    not isinstance(value, str) or not value.strip()
                    for value in declared_parents):
                raise ValueError('Invalid parent_ids for ' + ident)
            # Flat registers declare the same relation that nested collections
            # supply through traversal. Retain both, including repeated origins.
            for parent_id in ([parent] if parent else []) + declared_parents:
                if parent_id == ident:
                    raise ValueError('Self parent: ' + ident)
                if parent_id not in entry['parent_ids']:
                    entry['parent_ids'].append(parent_id)
            add_source_requirements(entry, row, origin)
            if row.get('minimum_days') is not None:
                entry['minimum_days'] = row['minimum_days']
            if row.get('original_local_id'):
                entry['original_local_id'] = row['original_local_id']
            text = json.dumps(row)
            for key, values in linked_ids(text, project_ids).items():
                entry['links'][key] = sorted(set(entry['links'][key]) | set(values))
            entry['artifacts'] = sorted(set(entry['artifacts']) | {p for p in PATH.findall(text) if (ROOT/p).is_file()})
            for field in ('limits', 'limitations'):
                for limit in strings(row.get(field, [])):
                    if limit not in entry['evidence_limits']:
                        entry['evidence_limits'].append(limit)
            if ident in supplement_rows:
                path, criterion, contract = supplement_rows[ident]
                if path not in entry['artifacts']:
                    entry['artifacts'].append(path)
                entry['criterion_contract'] = {'path': path, 'id': ident}
                entry['links']['projects'] = sorted(set(entry['links']['projects']) | {contract['project']})
                entry['links']['criteria'] = sorted(set(entry['links']['criteria']) | {ident})
                entry['required_environment'] = contract.get('environment', entry['required_environment'])
                entry['validation_method'] = criterion.get('verification', entry['validation_method'])
                entry['rubric'] = criterion.get('rubric', {})
                entry['learner_deliverables'] = criterion.get('deliverables', [])
                for value in strings(criterion.get('prerequisites', [])):
                    if value not in entry['prerequisites']:
                        entry['prerequisites'].append(value)
                if not entry['completion_criteria']:
                    entry['completion_criteria'] = ['Meet the objective-specific rubric in ' + path + ' for ' + ident + '; record author reference validation separately from independent learner performance.']
    for origin, document in sorted(documents.items()):
        for pointer, value in leaves(document):
            if value in inventory:
                item = {'path': origin, 'pointer': pointer, 'role': 'reference'}
                # The identity field is already represented by its defining row.
                if not any(x['path'] == origin and pointer == x['pointer'] + '/id'
                           for x in inventory[value]['origins'] if x.get('role') != 'reference'):
                    inventory[value]['origins'].append(item)
    for entry in inventory.values():
        for parent_id in entry['parent_ids']:
            parent = inventory.get(parent_id)
            if parent is None:
                raise ValueError('Unresolved parent: ' + parent_id)
            for key in entry['links']:
                entry['links'][key] = sorted(set(entry['links'][key]) | set(parent['links'][key]))
            if isinstance(entry['required_environment'], str) and entry['required_environment'].startswith('Resolve from'):
                entry['required_environment'] = parent['required_environment']
            entry['prerequisites'] = sorted(set(entry['prerequisites']) | set(parent['prerequisites']))
    unknown = sorted(set(explicit) - set(inventory))
    if unknown:
        raise ValueError('Disposition lacks originating obligation: ' + str(unknown))
    for ident, change in explicit.items():
        entry = inventory[ident]
        for dimension, state in change.get('dimensions', {}).items():
            if dimension not in DIMENSIONS or not isinstance(state, dict):
                raise ValueError('Invalid dimension for ' + ident)
            if state.get('state') in {'complete', 'passed'} and not state.get('evidence'):
                raise ValueError('Completed dimension requires evidence: ' + ident)
            for name in state.get('evidence', []):
                if not (ROOT / name.split('#')[0]).is_file():
                    raise ValueError('Missing evidence: ' + name)
            entry['dimensions'][dimension] = state
        for key in ('closure_rationale', 'prerequisites', 'affected_consumers', 'required_environment',
                    'validation_method', 'external_blockers', 'child_obligations',
                    'integrated_validation'):
            if key in change:
                entry[key] = change[key]
    report = {'schema_version': 1, 'producer': 'tools/reconcile_implementation_v5.py',
        'scope': 'Existing required gap registers, recursively linked obligation records, named remaining/depth records under research/v5 and curriculum/v5/expansion, all retained project supplemental criteria, and every canonical X exercise with its objective rubric and original delayed variants. Markdown pointers address fenced JSON; changed-condition prose is retained as a requirement. This inventory is not evidence that all corpus passages or implementations were inspected.',
        'canonical_updates': 'curriculum/v5/expansion/implementation_updates.json',
        'batch_accounting': 'curriculum/v5/expansion/remaining_work.json',
        'source_registers': [{'path': path, 'sha256': hashlib.sha256((ROOT/path).read_bytes()).hexdigest()}
                             for path in sorted(documents)],
        'referenced_artifacts': referenced,
        'dimension_counts': {d: dict(Counter(row['dimensions'][d]['state'] for row in inventory.values())) for d in DIMENSIONS},
        'obligations': [inventory[key] for key in sorted(inventory)],
        'limits': ['Unreconciled means no dimension-specific adjudication; it is not an external blocker or completion.',
                   'Source status is retained verbatim and may describe an older snapshot. Explicit dispositions identify corrections.',
                   'An empty remaining_leaf_ids array establishes mapping only.',
                   'Intentionally incomplete learner templates and negative specimens must not be repaired as author defects.']}
    dump(BASE / 'work_inventory.json', report)
    return {'registers': len(documents), 'obligations': len(inventory), 'explicit_dispositions': len(explicit)}

if __name__ == '__main__':
    print(generate())
