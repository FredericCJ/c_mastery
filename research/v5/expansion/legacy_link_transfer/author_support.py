"""Additive public variant artifacts; preserve original tasks and identifiers."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
PROJECTS=['PR-T39-01','PR-T39-03','PR-T39-04','PR-T40-05']
def attach(record):
 if record.get('project') not in PROJECTS:return record
 a=json.loads((HERE/('attachment_'+record['project']+'.json')).read_text(encoding='utf8'))
 for k in ['supplied_artifacts','shared_contracts']:record[k]=list(dict.fromkeys(record.get(k,[])+a[k]))
 for r in json.loads((HERE/'resources.json').read_text(encoding='utf8'))['resources']:
  record['source_records']=[x for x in record['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path='research/v5/expansion/legacy_link_transfer/resources.json',passages=r['locators'])]
 row=dict(path=a['support'],criterion_ids=a['criterion_ids'],scope=a['scope']);rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=row['path']]+[row];record['execution_support']['link_transfer_successor']=row
 return record
