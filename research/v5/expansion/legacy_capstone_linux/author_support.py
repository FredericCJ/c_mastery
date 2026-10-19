"""Pure Linux capstone publication attachment; original assessment is immutable."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent
def attach(d):
    p=HERE/('attachment_'+d['project']+'.json')
    if not p.is_file():return d
    a=json.loads(p.read_text(encoding='utf8'));assert a['domain']==d['domain'] and set(a['criterion_ids'])<={c['id'] for c in d['criteria']}
    for k in ['supplied_artifacts','shared_contracts']:d[k]=list(dict.fromkeys(d[k]+a[k]))
    for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:d['source_records']=[x for x in d['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=a['source_register'],passages=r['locators'])]
    row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope']);items=d.setdefault('additional_successor_support',[]);items[:]=[r for r in items if r.get('path')!=row['path']]+[row];d['execution_support'][a['key']]=row
    return d
