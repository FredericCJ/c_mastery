"""Snapshot original unresolved IW bodies and exact canonical child candidates.

Read-only with respect to inventory, updates, original sources and providers.
This intake is not a completion decision.
"""
import hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1];BASE=ROOT/'research/v5/expansion/inventory_alias_followthrough'
def read(p):return json.loads(p.read_text(encoding='utf8'))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def original(o):
    d=read(ROOT/o['path'])
    for k in o['pointer'].strip('/').split('/'):
        k=k.replace('~1','/').replace('~0','~');d=d[int(k)] if isinstance(d,list) else d[k]
    return d
def children(record):
    out=[];project=record.get('project');objective=record.get('objective')
    if not project and not objective:return out
    paths=[ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json')] if project else sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-*.json'))
    for p in paths:
        for c in read(p)['criteria']:
            if objective and c['objective']!=objective:continue
            for v in [None,*c.get('variants',[])]:out.append(dict(id=(v or c)['id'],canonical=p.relative_to(ROOT).as_posix(),criterion=c,variant=v,task=(v or c)['task']))
    return out
def main():
    inventory=read(ROOT/'curriculum/v5/expansion/work_inventory.json');canonical={r['id']:r for r in read(ROOT/'curriculum/v5/expansion/implementation_updates.json')['updates']};old=[]
    # Once discovered, this package keeps its exact original scope even after
    # root merges its dispositions into the global update register.
    prior=BASE/'inspection_input.json';frozen={r['id'] for r in read(prior)['records']} if prior.exists() else None
    for row in inventory['obligations']:
        if frozen is not None:
            if row['id'] not in frozen:continue
        elif not row['id'].startswith('IW-') or row['id'] in canonical:continue
        origins=[dict(**o,record=original(o)) for o in row['origins']];assert len(origins)==1
        old.append(dict(id=row['id'],origins=origins,links=row['links'],requirements=row['requirements'],candidate_children=children(origins[0]['record'])))
    if frozen is not None:assert {r['id'] for r in old}==frozen,'Original scope disappeared from inventory'
    wanted={c['id'] for r in old for c in r['candidate_children']};providers={k:[] for k in wanted};unique={}
    trees=[ROOT/'research/v5/expansion',ROOT/'research/v5/modern']
    for tree in trees:
        paths=set(tree.rglob('*_dispositions.json'))|set(tree.rglob('support.json'))
        for p in sorted(paths):
            if 'history' in p.parts or 'inventory_alias_followthrough' in p.parts:continue
            for c in read(p).get('updates',[]):
                if c.get('id') not in wanted:continue
                if c['id'] in canonical and c!=canonical[c['id']]:continue
                name=p.relative_to(ROOT).as_posix();providers[c['id']].append(name);unique[(name,c['id'])]=c
    missing=[]
    for row in old:
        for c in row['candidate_children']:
            c['provider_candidates']=providers[c['id']]
            if not c['provider_candidates']:missing.append(c['id'])
            else:
                chosen=min(c['provider_candidates'],key=lambda n:(len(pathlib.Path(n).parts),len(n),n));c['selected_provider']=chosen;c['provider_record']=unique[(chosen,c['id'])]
    BASE.mkdir(parents=True,exist_ok=True)
    value=dict(schema_version=1,scope='Intake only: original source bodies, canonical ordinary/delayed tasks, exact current-row provider candidates; no completion from titles.',inventory_snapshot_sha256=sha(ROOT/'curriculum/v5/expansion/work_inventory.json'),updates_snapshot_sha256=sha(ROOT/'curriculum/v5/expansion/implementation_updates.json'),records=old,missing_provider_ids=sorted(set(missing)))
    (BASE/'inspection_input.json').write_text(json.dumps(value,indent=2,ensure_ascii=False)+'\n',encoding='utf8');print(json.dumps(dict(origins=len(old),children=len(wanted),missing=sorted(set(missing)))))
if __name__=='__main__':main()
