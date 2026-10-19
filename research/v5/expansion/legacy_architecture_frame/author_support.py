"""Pure additive current support; original criteria/baseline bindings stay exact."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def attach(d):
 if d.get('project')!='PR-T37-07':return d
 a=json.loads((HERE/'attachment_PR-T37-07.json').read_text(encoding='utf8'));assert set(a['criterion_ids'])=={c['id'] for c in d['criteria']}
 for key in ['supplied_artifacts','shared_contracts']:d[key]=list(dict.fromkeys(d[key]+a[key]))
 for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:d['source_records']=[x for x in d['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=a['source_register'],passages=r['locators'])]
 row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope']);rows=d.setdefault('additional_successor_support',[]);rows[:]=[x for x in rows if x.get('path')!=row['path']]+[row];d['execution_support']['architecture_frame_successor']=row;d['status']['execution']='Actual frozen decoder/startup/IRQ composition and original/changed author packets; physical boot, external review and independent delayed learner evidence remain separate.';return d
