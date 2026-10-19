"""Report exact incomplete canonical reading records without changing them."""
import json
from pathlib import Path
from resource_inputs_v5 import historical
ROOT=Path(__file__).resolve().parents[1]
def main():
 policy=json.loads((ROOT/'research/v5/resource_merge_policy.json').read_text());skip=set(policy['evidence_only'])|set(policy['pinned_wrappers']);out=[]
 paths=list((ROOT/'research/v5').glob('*resources.json'))+list((ROOT/'research/v5/expansion').rglob('*resources.json'))+list((ROOT/'research/v5/modern').glob('*resources.json'))
 for p in sorted(paths):
  n=p.relative_to(ROOT).as_posix()
  if n in skip or historical(n):continue
  v=json.loads(p.read_text(encoding='utf8'));rows=v if isinstance(v,list) else v.get('resources',[])
  for r in rows:
   if r.get('reuse'):continue
   missing=[x for x in ('title','edition','inspection') if not r.get(x)]
   inspection=r.get('inspection',{})
   if r.get('assignable') is True and isinstance(inspection,dict):missing+=['inspection.'+x for x in ('date','method','scope','limits') if not inspection.get(x)]
   if missing:out.append(dict(path=n,missing=missing,record=r))
 dest=ROOT/'research/v5/expansion/resource_merge_repair/schema_omissions.json';dest.write_text(json.dumps(out,indent=2)+'\n',encoding='utf8')
 print(json.dumps([dict(path=r['path'],missing=r['missing'],**{k:r['record'].get(k) for k in ('id','title','url')},pins=r['record'].get('inspection',{}).get('source_sha256')) for r in out],indent=2))
if __name__=='__main__':main()
