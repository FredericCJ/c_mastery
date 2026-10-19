"""Additive security attachment; original criterion objects are immutable."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4]
def attach(d):
 if d.get('domain')!='T35':return d
 for path in sorted((ROOT/'research/v5/expansion').glob('legacy_security_*/attachment*.json')):
  a=json.loads(path.read_text(encoding='utf8'))
  if a['project']!=d['project']:continue
  assert a['domain']==d['domain'] and set(a['criterion_ids'])<={x['id'] for x in d['criteria']}
  for key in ['supplied_artifacts','shared_contracts']:d[key]=list(dict.fromkeys(d[key]+a[key]))
  for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:
   d['source_records']=[x for x in d['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=a['source_register'],passages=r['locators'])]
  row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope'])
  rows=d.setdefault('additional_successor_support',[]);rows[:]=[x for x in rows if x.get('path')!=row['path']]+[row]
  d['execution_support'][a['key']]=row
  d['status']['execution']='Current scoped author successors; original historical evidence and independent learner/source/target gaps remain separate.'
 return d
