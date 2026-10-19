"""Pure, idempotent additive attachment; no criterion/history rewriting."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def attach(record):
 if record.get('project')!='PR-T38-02':return record
 a=json.loads((HERE/'attachment_PR-T38-02.json').read_text(encoding='utf8'))
 for key in ['supplied_artifacts','shared_contracts']:
  record[key]=list(dict.fromkeys(record.get(key,[])+a[key]))
 for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:
  record['source_records']=[x for x in record['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=a['source_register'],passages=r['locators'])]
 row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope']);records=record.setdefault('additional_successor_support',[]);records[:]=[x for x in records if x.get('path')!=row['path']]+[row]
 record['execution_support']['serial_sync_successor']=row
 record['status']['execution']='Actual composed synchronous author experiments; physical CK/setup/hold, external review and independent delayed learner evidence remain required.'
 return record
