"""Pure additive T25 renderer hook; each coherent wave declares its own attachment."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4]
def attach(d):
 if d.get('domain')!='T25':return d
 for path in sorted((ROOT/'research/v5/expansion').glob('legacy_thread_*/attachment.json')):
  a=json.loads(path.read_text(encoding='utf8'))
  if d['project']!=a['project']:continue
  for key in ['supplied_artifacts','shared_contracts']:
   d[key]=list(dict.fromkeys(d[key]+a[key]))
  for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:
   d['source_records']=[x for x in d['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=a['source_register'],passages=r['locators'])]
  row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope'])
  existing=d.setdefault('additional_successor_support',[]);existing[:]=[x for x in existing if x.get('path')!=row['path']]+[row]
  d['execution_support'][a['key']]=row
  d['status']['execution']='Current criterion-specific author successors; independent learner and exact-source gaps remain distinct.'
 return d
