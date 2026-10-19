"""Pure additive attachment; original criterion and delayed objects remain unchanged."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def attach(d):
 if d.get('project')!='PR-T37-02':return d
 a=json.loads((HERE/'attachment_PR-T37-02.json').read_text(encoding='utf8'));assert set(a['criterion_ids'])=={c['id'] for c in d['criteria']}
 d['source_records']=[x for x in d['source_records'] if not (x['id']=='R-AB-ARM' and x['path']==a['source_register'])]
 for key in ['supplied_artifacts','shared_contracts']:d[key]=list(dict.fromkeys(d[key]+a[key]))
 for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:d['source_records']=[x for x in d['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=a['source_register'],passages=r['locators'])]
 row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope']);rows=d.setdefault('additional_successor_support',[]);rows[:]=[x for x in rows if x.get('path')!=row['path']]+[row];d['execution_support']['architecture_bytes_successor']=row;d['status']['execution']='Actual register, decoder, typed-storage and paired dependency author execution; external, delayed learner and physical evidence remain separate.';return d
