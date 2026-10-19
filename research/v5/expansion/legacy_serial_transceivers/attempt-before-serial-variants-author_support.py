"""Pure additive successor; preserve exact original tasks, variants and bindings."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def attach(record):
 if record.get('project') not in ['PR-T38-06','PR-T38-07']:return record
 a=json.loads((HERE/('attachment_'+record['project']+'.json')).read_text(encoding='utf8'))
 for k in ['supplied_artifacts','shared_contracts']:record[k]=list(dict.fromkeys(record.get(k,[])+a[k]))
 for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:
  record['source_records']=[x for x in record['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=a['source_register'],passages=r['locators'])]
 row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope']);records=record.setdefault('additional_successor_support',[]);records[:]=[x for x in records if x.get('path')!=row['path']]+[row];record['execution_support']['transceiver_successor']=row
 record['status']['execution']='Source-bound selected-device core, actual UART composition and Thumb synthetic-sample execution; physical line captures and independent attempts remain separately required.'
 return record
