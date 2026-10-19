"""Preserve each original T05/T06 ordinary and delayed task independently."""
from common import ROOT,dump,read_json,sha
PROJECTS=[f'PR-T05-{i:02}' for i in range(1,6)]+[f'PR-T06-{i:02}' for i in range(1,4)]
BASE='research/v5/expansion/legacy_portability_followthrough'
def main():
 rows=[];inputs={};resolved={}
 for p in sorted((ROOT/BASE).glob('*_completion_dispositions.json')):
  d=read_json(p);pins=d.get('input_sha256',{})
  if pins and all((ROOT/k).is_file() and sha(ROOT/k)==h for k,h in pins.items()):
   for r in d['updates']:resolved[r['id']]=(p.relative_to(ROOT).as_posix(),r)
 for project in PROJECTS:
  path=f'curriculum/v5/expansion/legacy_assessments/{project}.json';d=read_json(ROOT/path);inputs[path]=sha(ROOT/path)
  analysis=f'assessor/legacy_depth/{project}.md';inputs[analysis]=sha(ROOT/analysis)
  for c in d['criteria']:
   tasks=[(c['id'],'ordinary',c['task'],c['verification']['expected'])]+[(v['id'],'delayed',v['task'],c['rubric']['3']) for v in c['variants']]
   for identity,kind,task,expected in tasks:
    row=dict(id=identity,required=True,project=project,objective=c['objective'],kind=kind,task=task,expected=expected,sources=[path,analysis],status='exact-author-support-pending')
    if identity in resolved:
     p,r=resolved[identity];row.update(status='ready-bounded-author-support',resolved_descendants=[p],scope=r['closure_rationale'])
    rows.append(row)
 dump(ROOT/BASE/'remaining.json',dict(schema_version=1,required=True,complete=False,rows=rows,input_sha256=inputs,counts=dict(total=len(rows),ordinary=sum(r['kind']=='ordinary' for r in rows),delayed=sum(r['kind']=='delayed' for r in rows),ready_bounded=sum(r['status']=='ready-bounded-author-support' for r in rows)),limits=['All exact variants remain required, including formerly elective T05.10.','Previous source inspections and authored analyses are retained; they are not actual capability campaigns.','Published-edition access, external review and delayed learner assessment remain distinct from author implementation.']))
 print(dict(original_tasks=len(rows),ready_bounded=sum(r['status']=='ready-bounded-author-support' for r in rows)))
if __name__=='__main__':main()
