"""Edition-independent identities and explicit coverage migration contracts."""
from __future__ import annotations
import hashlib
import json
from common import ROOT, read_json, sha

SOURCE_ROOTS = ['curriculum', 'fixtures', 'references', 'research', 'site', 'tools', 'cmastery', 'labs', 'assessor', '.github']
BASELINE_SHA256 = 'f2f3588d96032292d7dfb956ee49a6847299f1e4c17d999b08265a93b030226d'

def baseline_v1():
    path = ROOT/'curriculum/baseline_v1.json'
    if sha(path) != BASELINE_SHA256:
        raise ValueError('First-edition identity snapshot changed')
    return read_json(path)

def source_identity():
    files = {}
    for folder in SOURCE_ROOTS:
        for path in sorted((ROOT/folder).rglob('*')):
            relative = path.relative_to(ROOT)
            if path.is_file() and not {'private', '__pycache__', 'build', '.git'} & set(relative.parts):
                files[relative.as_posix()] = sha(path)
    for name in ['README.md', 'build.ps1', 'requirements-lock.txt', 'requirements.in', '.gitattributes', '.gitignore']:
        if (ROOT/name).is_file(): files[name] = sha(ROOT/name)
    digest = hashlib.sha256(json.dumps(files, sort_keys=True).encode()).hexdigest()
    return {'sha256': digest, 'files': files}

def check_migration(taxonomy, migration=None):
    migration = read_json(ROOT/'curriculum/migration.json') if migration is None else migration
    errors = []
    baseline = baseline_v1()
    dispositions = {'retained', 'revised', 'split', 'merged'}
    domains, topics, objectives = [], [], {}
    for domain in taxonomy:
        domains.append(domain['id'])
        for topic in domain['subtopics']:
            topics.append(topic['id'])
            if not topic['id'].startswith(domain['id']+'.'):
                errors.append('Topic outside owning domain: '+topic['id'])
            if not topic['objectives']:
                errors.append('Empty objective set: '+topic['id'])
            for objective in topic['objectives']:
                key=objective['id']
                if key in objectives: errors.append('Duplicate taxonomy objective IDs: '+key)
                if not key.startswith(topic['id']+'.') or not objective.get('text','').strip():
                    errors.append('Malformed objective: '+key)
                objectives[key]=objective
    for kind, keys in [('domain', domains), ('topic', topics)]:
        if len(keys)!=len(set(keys)): errors.append('Duplicate taxonomy '+kind+' IDs')
    previous=set()
    for row in migration['objectives']:
        if row['old_id'] in previous: errors.append('Duplicate migration source: '+row['old_id'])
        previous.add(row['old_id'])
        if baseline['objectives'].get(row['old_id']) != row.get('old_text'):
            errors.append('Prior objective identity/text differs from baseline: '+row['old_id'])
        if row.get('disposition') not in dispositions:
            errors.append('Unknown objective migration disposition: '+row['old_id'])
        if not row.get('new_ids') or not row.get('rationale'):
            errors.append('Unexplained or empty objective migration: '+row['old_id'])
        for key in row.get('new_ids',[]):
            if key not in objectives: errors.append('Unmapped prior objective: '+row['old_id']+' -> '+key)
        if row.get('disposition')=='retained':
            if row.get('new_ids') != [row['old_id']] or objectives.get(row['old_id'],{}).get('text')!=row['old_text']:
                errors.append('Retained objective changed without explicit migration: '+row['old_id'])
    if previous != set(baseline['objectives']):
        errors.append('Baseline objective migration identity set differs')
    project_rows=migration.get('projects',[])
    if len(project_rows)!=len(baseline['projects']) or {r['old_id'] for r in project_rows}!=set(baseline['projects']):
        errors.append('Baseline project migration identity set differs')
    for row in project_rows:
        if baseline['projects'].get(row['old_id']) != row.get('baseline_sha256'):
            errors.append('Prior project fingerprint differs from baseline: '+row['old_id'])
        if row.get('disposition') not in dispositions:
            errors.append('Unknown project migration disposition: '+row['old_id'])
    return errors

def receipt_current(record, identity):
    return record.get('status')=='passed' and record.get('input_sha256')==identity['sha256']
