"""Pure additive attachment; keep accepted project, tasks and baseline identity."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def upsert(rows,key,value):
 for i,old in enumerate(rows):
  if old.get(key)==value[key]:rows[i]=value;return
 rows.append(value)

def attach(record):
 if record.get('project')!='PR-T39-04':return record
 a=json.loads((HERE/'attachment_PR-T39-04.json').read_text(encoding='utf8'))
 for k in ['supplied_artifacts','shared_contracts']:record[k]=list(dict.fromkeys(record.get(k,[])+a[k]))
 for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:
  upsert(record['source_records'],'id',dict(id=r['id'],path=a['source_register'],passages=r['locators']))
 row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope']);records=record.setdefault('additional_successor_support',[]);upsert(records,'path',row);record['execution_support']['ethernet_phy_successor']=row
 record['status']['execution']='Actual bounded author MDIO/reset/link implementation, native and Thumb RAM-effect observations; normal descriptor ring, physical PHY/board observations and independent assessment remain separate required evidence.'
 return record
