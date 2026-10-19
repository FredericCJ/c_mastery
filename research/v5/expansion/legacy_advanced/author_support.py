"""Pure additive T26–T29 attachment hook; never rewrite original criterion objects."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4]
def attach(d):
 if d.get('domain') not in ['T26','T27','T28','T29']:return d
 for path in sorted((ROOT/'research/v5/expansion').glob('legacy_advanced_*/attachment*.json')):
  a=json.loads(path.read_text(encoding='utf8'))
  if d['project']!=a['project']:continue
  if a['domain']!=d['domain']:raise ValueError('Attachment domain/parent mismatch: '+str(path))
  actual={c['id'] for c in d['criteria']}
  if not set(a['criterion_ids'])<=actual:raise ValueError('Attachment claims absent criteria: '+str(path))
  for key in ['supplied_artifacts','shared_contracts']:d[key]=list(dict.fromkeys(d[key]+a[key]))
  for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:
   d['source_records']=[x for x in d['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=a['source_register'],passages=r['locators'])]
  row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope'])
  existing=d.setdefault('additional_successor_support',[]);existing[:]=[x for x in existing if x.get('path')!=row['path']]+[row]
  d['execution_support'][a['key']]=row
  d['status']['execution']='Current criterion-specific author successors; unlinked criteria, independent learner and source/target gaps remain distinct.'
 return d
