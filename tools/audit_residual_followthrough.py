"""Collect exact residual origins and current child evidence without editing inventory.

This is an inspection input, never a completion producer. A separate reviewed
clause map is required before any additive parent dimension can be emitted.
"""
from __future__ import annotations
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEST = ROOT / 'research/v5/expansion/residual_followthrough'
DOMAINS = {f'T{i:02}' for i in [2,3,4,5,6,9,18,21,23,28,*range(36,51)]}

def read(path):
    return json.loads((ROOT/path).read_text(encoding='utf-8'))

def sha(path):
    return hashlib.sha256((ROOT/path).read_bytes()).hexdigest()

def original(origin):
    value = read(origin['path'])
    for token in origin['pointer'].strip('/').split('/'):
        token = token.replace('~1','/').replace('~0','~')
        value = value[int(token)] if isinstance(value,list) else value[token]
    return value

def main():
    inventory = read('curriculum/v5/expansion/work_inventory.json')
    updates = {r['id']:r for r in read('curriculum/v5/expansion/implementation_updates.json')['updates']}
    providers = {}
    for path in sorted((ROOT/'research/v5/expansion').glob('**/*.json')):
        if path.name not in ('support.json','completion_dispositions.json') and not path.name.endswith('_completion_dispositions.json'):
            continue
        if 'residual_followthrough' in path.parts:
            continue
        value = json.loads(path.read_text(encoding='utf-8'))
        if not isinstance(value,dict):
            continue
        for row in value.get('updates',[]):
            if not isinstance(row,dict) or not isinstance(row.get('id'),str):
                continue
            if row == updates.get(row['id']):
                providers.setdefault(row['id'],[]).append(path.relative_to(ROOT).as_posix())
    assessments = {}
    for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T*.json')):
        value = json.loads(path.read_text(encoding='utf-8'))
        for criterion in value['criteria']:
            assessments[criterion['id']] = (path.relative_to(ROOT).as_posix(),criterion)
    records = []
    for row in inventory['obligations']:
        if row['dimensions']['implementation']['state'] != 'unreconciled':
            continue
        if row['id'].startswith(('AX-','X-')):
            continue
        if not (set(row['links']['domains']) & DOMAINS or row['id']=='G-FOLLOWTHROUGH-T52-08-STOPPED'):
            continue
        origins=[]
        for origin in row['origins']:
            try:
                body=original(origin)
            except (KeyError,ValueError,TypeError):
                # Some registers additionally point to a named dictionary key.
                # Preserve that origin verbatim; do not invent a replacement.
                body=None
            origins.append({**origin,'record':body})
        objectives=set(row['links']['objectives'])
        topics=set()
        for origin in origins:
            body=origin['record']
            if isinstance(body,dict) and isinstance(body.get('topic'),str):topics.add(body['topic'])
        children=[]
        for criterion_id,(path,criterion) in assessments.items():
            objective=criterion.get('objective')
            if objective not in objectives and not (not objectives and any(objective and objective.startswith(t+'.') for t in topics)):
                continue
            selected=[(criterion_id,criterion.get('task'))]+[(v['id'],v.get('task')) for v in criterion.get('variants',[])]
            for identity,task in selected:
                support=updates.get(identity)
                children.append({'id':identity,'original_source':path,'original_task':task,
                    'provider_candidates':providers.get(identity,[]),'current_support':support})
        records.append({'id':row['id'],'links':row['links'],'origins':origins,'requirements':row['requirements'],
                        'candidate_children':children})
    census={}
    for domain in sorted(DOMAINS):
        selected=[r for r in inventory['obligations'] if domain in r['links']['domains'] and not r['id'].startswith(('AX-','X-'))]
        counts={}
        for r in selected:
            state=r['dimensions']['implementation']['state'];counts[state]=counts.get(state,0)+1
        census[domain]=counts
    result={'schema_version':1,'scope':'Inspection input only; candidate children are not proof of aggregate closure.',
            'inventory_snapshot_sha256':sha('curriculum/v5/expansion/work_inventory.json'),
            'updates_snapshot_sha256':sha('curriculum/v5/expansion/implementation_updates.json'),
            'domain_census':census,'records':records}
    DEST.mkdir(parents=True,exist_ok=True)
    (DEST/'inspection_input.json').write_text(json.dumps(result,indent=2,ensure_ascii=False)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'records':len(records),'children':sum(len(r['candidate_children']) for r in records),
        'without_provider':sorted({c['id'] for r in records for c in r['candidate_children'] if not c['provider_candidates']})}))

if __name__=='__main__':main()
