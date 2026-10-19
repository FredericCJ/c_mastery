"""Pure additive successor; never rewrite accepted task/variant objects."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def upsert(rows,key,row):
 for i,current in enumerate(rows):
  if current.get(key)==row[key]:rows[i]=row;return
 rows.append(row)

def attach(record):
 if record.get('project')!='PR-T38-09':return record
 a=json.loads((HERE/('attachment_'+record['project']+'.json')).read_text(encoding='utf8'))
 for k in ['supplied_artifacts','shared_contracts']:record[k]=list(dict.fromkeys(record.get(k,[])+a[k]))
 for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:
  upsert(record['source_records'],'id',dict(id=r['id'],path=a['source_register'],passages=r['locators']))
 row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope']);records=record.setdefault('additional_successor_support',[]);upsert(records,'path',row);record['execution_support']['classic_can_successor']=row
 record['status']['execution']='Current-source Classic CAN/FCE and selected CAN1 mailbox/FIFO implementation; native/Thumb RAM-effects evidence separate from physical bus and independent learner work.'
 return record
