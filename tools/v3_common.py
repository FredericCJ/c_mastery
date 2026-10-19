"""Third-edition migration against the accepted second-edition identities."""
from common import ROOT, read_json, sha
BASELINE_V2_SHA256='aafe9a1ee77393ee1ef6441d85c0b9f793c7e7fae73487f57dd165c063e8fc7a'

def check_v3_migration(taxonomy, projects, migration=None):
    path=ROOT/'curriculum/baseline_v2.json'
    if sha(path)!=BASELINE_V2_SHA256: return ['Immutable v2 identity snapshot changed']
    base=read_json(path)
    migration=read_json(ROOT/'curriculum/migration_v3.json') if migration is None else migration
    actual={o['id']:o['text'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
    errors=[]
    rows=migration.get('objectives',[])
    if len(rows)!=len(base['objectives']) or {r['old_id'] for r in rows}!=set(base['objectives']):
        errors.append('V2 objective migration identity set differs')
    for row in rows:
        key=row['old_id']
        if base['objectives'].get(key)!=row.get('old_text'): errors.append('V2 objective provenance changed: '+key)
        if row.get('disposition') not in {'retained','revised','split','merged'}: errors.append('Unknown v3 migration disposition')
        if not row.get('rationale') or not row.get('new_ids') or not set(row['new_ids'])<=set(actual):
            errors.append('Unexplained or dangling v3 objective migration: '+key)
        if row.get('disposition')=='retained' and (row.get('new_ids')!=[key] or actual.get(key)!=row.get('old_text')):
            errors.append('Retained v2 objective changed: '+key)
    rows=migration.get('projects',[])
    if len(rows)!=len(base['projects']) or {r['old_id'] for r in rows}!=set(base['projects']):
        errors.append('V2 project migration identity set differs')
    for row in rows:
        if row.get('baseline_sha256')!=base['projects'].get(row['old_id']): errors.append('V2 project fingerprint changed')
        if not row.get('new_ids') or not set(row['new_ids'])<=set(projects): errors.append('Dangling v2 project')
        if row.get('disposition') not in {'retained','revised','split','merged'} or not row.get('rationale'):
            errors.append('Unexplained v3 project migration: '+row['old_id'])
        if row.get('disposition')=='retained':
            project=projects.get(row['old_id'])
            unchanged = project and sha(project['path']) == row['baseline_sha256']
            if project and not unchanged:
                # The accepted inspection remains the exact predecessor. Only
                # the two fully proved public B06 asset attachments qualify;
                # this is not a new baseline inspection or execution claim.
                from check_v5 import publication_successor_covers
                name = 'curriculum/projects/' + row['old_id'] + '.md'
                unchanged = (project['path'].resolve() == (ROOT / name).resolve()
                             and publication_successor_covers(name, row['baseline_sha256']))
            if row.get('new_ids')!=[row['old_id']] or not unchanged:
                errors.append('Retained v2 project changed: '+row['old_id'])
    if not set(base['domains'])<={d['id'] for d in taxonomy}: errors.append('Prior domain omitted')
    return errors
