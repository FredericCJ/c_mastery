"""Add original changed-task public artifacts without replacing parent evidence."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
PROJECTS=['PR-T38-03','PR-T38-04']
def upsert(rows,key,row):
 for i,current in enumerate(rows):
  if current.get(key)==row[key]:rows[i]=row;return
 rows.append(row)
def attach(record):
 if record.get('project') not in PROJECTS:return record
 a=json.loads((HERE/('attachment_'+record['project']+'.json')).read_text(encoding='utf8'))
 for k in ['supplied_artifacts','shared_contracts']:record[k]=list(dict.fromkeys(record.get(k,[])+a[k]))
 for r in json.loads((HERE/'resources.json').read_text(encoding='utf8'))['resources']:upsert(record['source_records'],'id',dict(id=r['id'],path='research/v5/expansion/legacy_serial_retry/resources.json',passages=r['locators']))
 row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope']);upsert(record.setdefault('additional_successor_support',[]),'path',row);record['execution_support']['serial_retry_successor']=row
 return record
