"""Idempotent additive support; original tasks and acceptance remain immutable."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def upsert(rows,key,row):
 for i,old in enumerate(rows):
  if old.get(key)==row[key]:rows[i]=row;return
 rows.append(row)

def attach(record):
 if record.get('project') not in ['PR-T38-03','PR-T38-04']:return record
 a=json.loads((HERE/('attachment_'+record['project']+'.json')).read_text(encoding='utf8'))
 for k in ['supplied_artifacts','shared_contracts']:record[k]=list(dict.fromkeys(record.get(k,[])+a[k]))
 for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:
  upsert(record['source_records'],'id',dict(id=r['id'],path=a['source_register'],passages=r['locators']))
 row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope']);records=record.setdefault('additional_successor_support',[]);upsert(records,'path',row);record['execution_support']['serial_i2c_successor']=row
 record['status']['execution']='Actual original I2C derivation and retained-controller author experiments; physical analog evidence, external review and independent delayed learner evidence remain required.'
 return record
