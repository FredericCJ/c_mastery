"""Pure additive T40 physical/grant bindings; accepted criteria untouched."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def attach(record):
 if record.get('project') not in ['PR-T40-02']:return record
 a=json.loads((HERE/('attachment_'+record['project']+'.json')).read_text(encoding='utf8'))
 record['source_records']=[x for x in record['source_records'] if not (x['id']=='R-EP-RM21' and x['path']==a['source_register'])]
 for k in ['supplied_artifacts','shared_contracts']:record[k]=list(dict.fromkeys(record.get(k,[])+a[k]))
 for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:
  record['source_records']=[x for x in record['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=a['source_register'],passages=r['locators'])]
 row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope']);records=record.setdefault('additional_successor_support',[]);records[:]=[x for x in records if x.get('path')!=row['path']]+[row];record['execution_support']['physical_successor']=row
 record['status']['execution']='Actual EDU7 full-frame UART, two-port and grant author evidence with explicit remaining whole-stack and physical obligations.'
 return record
