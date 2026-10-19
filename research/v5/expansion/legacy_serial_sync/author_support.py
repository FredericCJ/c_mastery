"""Pure, idempotent additive attachment; no criterion/history rewriting."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def upsert(rows,key,row):
 for i,current in enumerate(rows):
  if current.get(key)==row[key]:rows[i]=row;return
 rows.append(row)

def attach(record):
 if record.get('project')!='PR-T38-02':return record
 a=json.loads((HERE/'attachment_PR-T38-02.json').read_text(encoding='utf8'))
 for key in ['supplied_artifacts','shared_contracts']:
  record[key]=list(dict.fromkeys(record.get(key,[])+a[key]))
 for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:
  upsert(record['source_records'],'id',dict(id=r['id'],path=a['source_register'],passages=r['locators']))
 row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope']);records=record.setdefault('additional_successor_support',[]);upsert(records,'path',row)
 record['execution_support']['serial_sync_successor']=row
 record['status']['execution']='Actual composed synchronous author experiments; physical CK/setup/hold, external review and independent delayed learner evidence remain required.'
 return record
