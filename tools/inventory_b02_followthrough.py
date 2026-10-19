"""Keep each original exercise/rubric/delayed ID, including prose variants."""
import json
from common import ROOT,dump,sha,read_json
IDS=['X-T12-002','X-T14-002','X-T15-002','X-T16-002','X-T16-003','X-T17-002','X-T19-002','X-T21-002']
def main():
 rows=[];inputs={}
 for identity in IDS:
  path='curriculum/v5/exercises/'+identity+'.md';s=(ROOT/path).read_text(encoding='utf8');inputs[path]=sha(ROOT/path);d=json.loads(s.split('```json\n',1)[1].split('\n```',1)[0]);variant=s.split('## Variants\n',1)[1].split('\n## ',1)[0]
  rows.append(dict(id=identity,required=True,kind='exercise',requirements=[s.split('## Problem\n',1)[1].split('\n## ',1)[0],s.split('## Verification\n',1)[1].split('\n## ',1)[0],variant],status='exact-author-support-pending',sources=[path,d['assessor_analysis']]))
  inputs[d['assessor_analysis']]=sha(ROOT/d['assessor_analysis'])
  for r in d['rubric']:rows.append(dict(id=r['id'],required=True,kind='criterion',requirements=[r['criterion'],r['levels']['3']],status='exact-author-support-pending',sources=[path,d['assessor_analysis']]))
  for v in d['delayed_transfer']['variant_ids']:rows.append(dict(id=v,required=True,kind='delayed',requirements=[variant],status='exact-author-support-pending',sources=[path,d['assessor_analysis']]))
 base=ROOT/'research/v5/expansion/b02_followthrough';resolved={}
 for p in sorted(base.glob('*_completion_dispositions.json')):
  d=read_json(p);pins=d.get('input_sha256',d.get('source_sha256',{}))
  if pins and all((ROOT/k).is_file() and sha(ROOT/k)==h for k,h in pins.items()):
   for r in d['updates']:resolved[r['id']]=(p.relative_to(ROOT).as_posix(),r)
 for r in rows:
  if r['id'] in resolved:
   p,d=resolved[r['id']];r.update(status='ready-bounded-author-support',resolved_descendants=[p],scope=d['closure_rationale'])
 dump(base/'remaining.json',dict(schema_version=1,required=True,complete=False,rows=rows,input_sha256=inputs,counts=dict(exercises=8,rubrics=26,named_delayed=8,total=len(rows),ready_bounded=sum(r['status']=='ready-bounded-author-support' for r in rows)),limits=['All original prose variants and criterion level3 demands remain required, not only named delayed IDs.','Existing B02 anchors do not execute all enumerations or required adapters.','Precise actual newer legacy providers are inspected/reused where contracts match; related headings are not evidence.','Physical observations, published-edition access, human-independent review and learner assessment remain separate.']))
 print(dict(original_ids=len(rows),ready_bounded=sum(r['status']=='ready-bounded-author-support' for r in rows),additional_exact_ids=len(set(resolved)-{r['id'] for r in rows})))
if __name__=='__main__':main()
