"""Pure additive attachment; original tasks, delayed IDs and bindings preserved."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def attach(record):
 if record.get('project')!='PR-T38-10':return record
 a=json.loads((HERE/('attachment_'+record['project']+'.json')).read_text(encoding='utf8'))
 for k in ['supplied_artifacts','shared_contracts']:record[k]=list(dict.fromkeys(record.get(k,[])+a[k]))
 for r in json.loads((ROOT/a['source_register']).read_text(encoding='utf8'))['resources']:
  record['source_records']=[x for x in record['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=a['source_register'],passages=r['locators'])]
 row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope']);records=record.setdefault('additional_successor_support',[]);records[:]=[x for x in records if x.get('path')!=row['path']]+[row];record['execution_support']['fd_successor']=row
 record['status']['execution']='Source-bound author metadata/padding/current-historical synthetic frame/TDC and controller-boundary construction; physical PR11/12 and independent evidence remain separate.'
 return record
