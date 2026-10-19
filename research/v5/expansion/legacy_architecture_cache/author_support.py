"""Pure additive attachment, preserving original criteria and delayed tasks."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def attach(d):
 if d.get('project')!='PR-T37-05':return d
 a=json.loads((HERE/'attachment_PR-T37-05.json').read_text(encoding='utf8'));assert set(a['criterion_ids'])=={c['id'] for c in d['criteria']}
 for key in ['supplied_artifacts','shared_contracts']:d[key]=list(dict.fromkeys(d[key]+a[key]))
 for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:d['source_records']=[x for x in d['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=a['source_register'],passages=r['locators'])]
 row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope']);rows=d.setdefault('additional_successor_support',[]);rows[:]=[x for x in rows if x.get('path')!=row['path']]+[row];d['execution_support']['architecture_cache_successor']=row;d['status']['execution']='Actual cache/set/shadow, rational cost and overlapping-service author execution; no physical cache/timing, external review or delayed learner evidence inferred.';return d
