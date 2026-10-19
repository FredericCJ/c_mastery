"""Snapshot exact named source/target parents and all selected canonical children."""
import hashlib,json,pathlib
from audit_inventory_aliases import ROOT,read,sha,original
BASE=ROOT/'research/v5/expansion/named_parent_followthrough'
SELECT={'LRUN-14':['T25.01','T25.02'],'LRUN-20':['T27.01','T27.02','T27.08'],'LRUN-21':['T27.03','T27.04','T27.05','T27.06','T27.07','T27.09','T27.10'],'LRUN-22':['T28.%02d'%n for n in range(1,10)],'G-LCON-PROOFS':['T52'],'LSYS-TARGETS':['T07','T08','T09','T10','T11','T29.01','T29.02'],'LSYS-BOOKS':[]}
def children(ident):
 out={}
 for p in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-*.json')):
  for c in read(p)['criteria']:
   if not any(c['objective'].startswith(t+'.') for t in SELECT[ident]):continue
   for v in [None,*c.get('variants',[])]:out[(v or c)['id']]=dict(id=(v or c)['id'],canonical=p.relative_to(ROOT).as_posix(),criterion=c,variant=v,task=(v or c)['task'])
 return out
def main():
 canonical={r['id']:r for r in read(ROOT/'curriculum/v5/expansion/implementation_updates.json')['updates']};inventory=read(ROOT/'curriculum/v5/expansion/work_inventory.json');rows=[]
 for x in inventory['obligations']:
  if x['id'] not in SELECT:continue
  rows.append(dict(id=x['id'],origins=[dict(**o,record=original(o)) for o in x['origins']],links=x['links'],requirements=x['requirements'],selected_topics=SELECT[x['id']],candidate_children=list(children(x['id']).values())))
 assert {r['id'] for r in rows}==set(SELECT);wanted={c['id'] for r in rows for c in r['candidate_children']};providers={k:[] for k in wanted};actual={}
 for tree in [ROOT/'research/v5/expansion',ROOT/'research/v5/modern']:
  for p in sorted(set(tree.rglob('*_dispositions.json'))|set(tree.rglob('support.json'))):
   if 'history' in p.parts or 'named_parent_followthrough' in p.parts:continue
   for x in read(p).get('updates',[]):
    if x.get('id') not in wanted or x!=canonical.get(x['id']):continue
    name=p.relative_to(ROOT).as_posix();providers[x['id']].append(name);actual[(name,x['id'])]=x
 missing=[]
 for row in rows:
  for c in row['candidate_children']:
   choices=providers[c['id']]
   if not choices:missing.append(c['id']);continue
   choice=min(choices,key=lambda p:(len(pathlib.Path(p).parts),len(p),p));c.update(provider=choice,provider_record=actual[(choice,c['id'])])
 BASE.mkdir(parents=True,exist_ok=True);(BASE/'inspection_input.json').write_text(json.dumps(dict(schema_version=1,scope='Exact original records and all canonical criteria/variants under selected original topic scopes; source/target parent does not promote every child to physical evidence.',records=rows,missing_provider_ids=missing),indent=2,ensure_ascii=False)+'\n',encoding='utf8');print(json.dumps(dict(parents=len(rows),children=len(wanted),missing=missing)))
if __name__=='__main__':main()
