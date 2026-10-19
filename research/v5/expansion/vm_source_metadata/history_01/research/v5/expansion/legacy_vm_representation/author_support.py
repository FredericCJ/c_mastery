"""Pure additive current T46 support; original assessment objects stay unchanged."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4]
def attach(d):
 if d.get('domain')!='T46':return d
 for p in sorted((ROOT/'research/v5/expansion').glob('legacy_vm_*/attachment*.json')):
  a=json.loads(p.read_text(encoding='utf8'))
  if d['project']!=a['project']:continue
  if a['domain']!=d['domain'] or not set(a['criterion_ids'])<={c['id'] for c in d['criteria']}:raise ValueError('VM attachment mismatch')
  for key in ['supplied_artifacts','shared_contracts']:d[key]=list(dict.fromkeys(d[key]+a[key]))
  for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:
   d['source_records']=[x for x in d['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=a['source_register'],passages=r['locators'])]
  row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope'])
  existing=d.setdefault('additional_successor_support',[]);existing[:]=[x for x in existing if x.get('path')!=row['path']]+[row]
  d['execution_support'][a['key']]=row
 return d
