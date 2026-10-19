"""Idempotently attach exact T02 qualified-view successors to retained assessment cards."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[4]
HERE=Path(__file__).resolve().parent
def attach(d):
 p=HERE/('attachment_'+d['project']+'.json')
 if not p.is_file():return d
 a=json.loads(p.read_text(encoding='utf8'))
 assert a['domain']==d['domain'] and set(a['criterion_ids'])<={c['id'] for c in d['criteria']}
 for key in ['shared_contracts','supplied_artifacts']:d[key]=list(dict.fromkeys(d[key]+a[key]))
 for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:
  d['source_records']=[x for x in d['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=a['source_register'],passages=r['locators'])]
 row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope'])
 records=d.setdefault('additional_successor_support',[]);records[:]=[r for r in records if r.get('path')!=row['path']]+[row]
 d['execution_support'][a['key']]=row
 return d
