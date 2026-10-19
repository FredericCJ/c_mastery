"""Read-only, fail-closed discovery of fifth-edition reading registrations.

Evidence sidecars and pinned passage wrappers are retained as evidence, never
promoted into readings. Duplicate reading registrations need exact identity or
an explicit, exact producer copy (only a domain extension is permitted).
"""
from copy import deepcopy
import hashlib
import json
from pathlib import Path

GENERATED = {'work_id', 'edition_id', 'inspection_ids'}


def read(path):
    return json.loads(path.read_text(encoding='utf8'))


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def historical(path):
    return any(p == 'history' or p.startswith('history_') or p.startswith('history-')
               for p in Path(path).parts)


def local(root, name):
    if not isinstance(name, str) or not name or '\\' in name:
        raise ValueError('Invalid resource path: ' + str(name))
    p = Path(name)
    if p.is_absolute() or '..' in p.parts or historical(p):
        raise ValueError('Noncanonical resource path: ' + name)
    target = (root / p).resolve()
    if not target.is_relative_to(root.resolve()) or not target.is_file():
        raise ValueError('Missing or escaping resource path: ' + name)
    return target


def rows(value, name):
    result = value if isinstance(value, list) else value.get('resources') if isinstance(value, dict) else None
    if not isinstance(result, list) or any(not isinstance(x, dict) for x in result):
        raise ValueError('Malformed canonical resource register: ' + name)
    ids = [x.get('id') for x in result]
    if any(not isinstance(x, str) or not x.strip() for x in ids) or len(set(ids)) != len(ids):
        raise ValueError('Missing or duplicate resource ID within: ' + name)
    return result


def canonical(row, name):
    if row.get('reuse') is True:
        if set(row) - {'id', 'reuse', 'reason'}:
            raise ValueError('Reuse alias contains unverified source fields: ' + name)
        return
    for field in ('id', 'title', 'edition'):
        if not isinstance(row.get(field), str) or not row[field].strip():
            raise ValueError('Missing canonical resource ' + field + ': ' + name + ':' + str(row.get('id')))
    if not isinstance(row.get('inspection'), dict):
        raise ValueError('Invalid canonical inspection: ' + name + ':' + row['id'])
    if 'assignable' in row and not isinstance(row['assignable'], bool):
        raise ValueError('Invalid source eligibility: ' + name + ':' + row['id'])


def verify_wrapper(root, name, value, kind):
    if kind == 'retained_registration':
        refs = [value.get('retained_registration')]
        key = 'passages'
    elif kind == 'registered_passages':
        refs = value.get('resources')
        key = 'registered_passages'
    else:
        raise ValueError('Unknown pinned wrapper type: ' + name)
    if not isinstance(refs, list) or not refs:
        raise ValueError('Malformed pinned wrapper: ' + name)
    verified = []
    for ref in refs:
        if not isinstance(ref, dict):
            raise ValueError('Malformed pinned reference: ' + name)
        target = local(root, ref.get('path'))
        if sha(target) != ref.get('sha256'):
            raise ValueError('Stale pinned resource reference: ' + name + ' -> ' + ref['path'])
        source = {r['id']: r for r in rows(read(target), ref['path'])}
        passages = ref.get(key)
        if not isinstance(passages, list) or not passages:
            raise ValueError('Missing pinned passages: ' + name)
        ids = []
        for passage in passages:
            if not isinstance(passage, dict) or passage.get('id') not in source:
                raise ValueError('Unknown pinned passage: ' + name)
            ident = passage['id']; ids.append(ident); original = source[ident]
            locators = passage.get('locators')
            if not isinstance(locators, list) or not locators or any(x not in original.get('locators', []) for x in locators):
                raise ValueError('Unregistered pinned locator: ' + name + ':' + ident)
            for field, val in passage.items():
                # Wrapper scope explicitly describes reuse; it is not a fresh
                # inspection and is never copied into the reading registry.
                if field not in {'scope', 'locators'} and original.get(field) != val:
                    raise ValueError('Conflicting pinned passage field ' + field + ': ' + name + ':' + ident)
        if len(ids) != len(set(ids)):
            raise ValueError('Duplicate pinned passage: ' + name)
        verified.append({'path': ref['path'], 'sha256': ref['sha256'], 'ids': ids})
    return verified


def discover(root, policy=None):
    root = Path(root)
    policy = read(root/'research/v5/resource_merge_policy.json') if policy is None else policy
    if policy.get('schema_version') != 1 or policy.get('history_components') != ['history', 'history_*', 'history-*']:
        raise ValueError('Unsupported resource discovery policy')
    evidence = policy.get('evidence_only', {}); wrappers = policy.get('pinned_wrappers', {})
    if not isinstance(evidence, dict) or not isinstance(wrappers, dict) or set(evidence) & set(wrappers):
        raise ValueError('Ambiguous resource input policy')
    paths = list((root/'research/v5').glob('*resources.json'))
    paths += list((root/'research/v5/expansion').rglob('*resources.json'))
    paths += list((root/'research/v5/modern').glob('*resources.json'))
    report = {'inputs': {}, 'excluded_history': [], 'evidence_only': {}, 'pinned_wrappers': {}, 'registration_provenance': {}}
    definitions = {}
    for path in sorted(set(paths)):
        name = path.relative_to(root).as_posix()
        if historical(name):
            report['excluded_history'].append(name)
            continue
        local(root, name)
        value = read(path); digest = sha(path); report['inputs'][name] = digest
        if name in evidence:
            declared = evidence[name]; records = rows(value, name)
            if declared.get('role') != 'author-evidence-only' or not declared.get('reason') or declared.get('sha256') != digest or declared.get('ids') != [r['id'] for r in records]:
                raise ValueError('Stale or malformed evidence-only classification: ' + name)
            report['evidence_only'][name] = deepcopy(declared)
        elif name in wrappers:
            report['pinned_wrappers'][name] = verify_wrapper(root, name, value, wrappers[name])
        else:
            for row in rows(value, name):
                canonical(row, name)
                definitions.setdefault(row['id'], []).append((name, row))
    if set(evidence) - set(report['evidence_only']) or set(wrappers) - set(report['pinned_wrappers']):
        raise ValueError('Declared resource input disappeared')
    result = []
    for ident, group in definitions.items():
        concrete = [(p, r) for p, r in group if not r.get('reuse')]
        bases = [(p, r) for p, r in concrete if 'reuse_from' not in r]
        if concrete and not bases:
            raise ValueError('Resource producer missing or cyclic: ' + ident)
        normalized = lambda r: {k: v for k, v in r.items() if k not in GENERATED}
        base = deepcopy(bases[0][1] if bases else group[0][1])
        for name, item in bases[1:]:
            if normalized(item) != normalized(base):
                raise ValueError('Conflicting canonical resource identity: ' + ident + ' in ' + ', '.join(p for p, _ in bases))
        domains = set(base.get('domains', []))
        for name, item in concrete:
            if 'reuse_from' not in item:
                continue
            producer_name = item['reuse_from']; local(root, producer_name)
            producer = next((r for p, r in bases if p == producer_name), None)
            if producer is None:
                raise ValueError('Unknown direct resource producer: ' + name + ':' + ident)
            trim = lambda r: {k: v for k, v in normalized(r).items() if k not in {'reuse_from', 'domains'}}
            if trim(item) != trim(producer) or not set(producer.get('domains', [])) <= set(item.get('domains', [])):
                raise ValueError('Conflicting resource reuse scope or identity: ' + name + ':' + ident)
            domains.update(item.get('domains', []))
        if concrete and domains != set(base.get('domains', [])):
            base['domains'] = sorted(domains)
        result.append(base)
        report['registration_provenance'][ident] = [{'path': p, 'sha256': report['inputs'][p], 'role': 'alias' if r.get('reuse') else 'producer-copy' if 'reuse_from' in r else 'registration'} for p, r in group]
    # Versioned, exact metadata additions preserve the original resource files
    # and their technical inspection receipts. Import after definitions to keep
    # the shared identity-correction helpers' dependency one-way at runtime.
    from resource_identity_v5 import add_missing_authors
    result, report['authorship_additions'] = add_missing_authors(
        root, result, report['registration_provenance'])
    return result, report
