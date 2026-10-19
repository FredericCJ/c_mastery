"""Versioned additive ownership-premise clarification, with historical bytes retained."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent
CONTRACT='labs/object_followthrough/clinic_contracts/SHARED_RIGHTS_V2.md';SUPPORT='research/v5/expansion/legacy_object_clinic_contract/completion_dispositions.json'
def attach(d):
    if d['project']!='PR-T03-05':return d
    for key in ['shared_contracts','supplied_artifacts']:d[key]=list(dict.fromkeys(d[key]+[CONTRACT]))
    for r in json.loads((HERE/'resources.json').read_text(encoding='utf8'))['resources']:d['source_records']=[x for x in d['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path='research/v5/expansion/legacy_object_clinic_contract/resources.json',passages=r['locators'])]
    row=dict(path=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Current shared-right caller premises; original C campaign retained byte-identically, new public export only.');records=d.setdefault('additional_successor_support',[]);records[:]=[r for r in records if r.get('path')!=SUPPORT]+[row];d['execution_support']['object_clinic_contract_v2']=row;return d
