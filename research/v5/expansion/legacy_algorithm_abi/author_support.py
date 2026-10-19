"""Pure maintained attachment; original criterion and baseline objects stay intact."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def attach(d):
 if d.get('project')!='PR-T36-02':return d
 a=json.loads((HERE/'attachment_PR-T36-02.json').read_text(encoding='utf8'))
 assert set(a['criterion_ids'])=={c['id'] for c in d['criteria']}
 for key in ['supplied_artifacts','shared_contracts']:d[key]=list(dict.fromkeys(d[key]+a[key]))
 for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:d['source_records']=[x for x in d['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=a['source_register'],passages=r['locators'])]
 row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope']);rows=d.setdefault('additional_successor_support',[]);rows[:]=[x for x in rows if x.get('path')!=row['path']]+[row];d['execution_support']['algorithm_abi_successor']=row;d['status']['execution']='Current source-bound paired ABI/callback/stride author campaign; independent learner, external review and physical evidence remain separate.';return d
