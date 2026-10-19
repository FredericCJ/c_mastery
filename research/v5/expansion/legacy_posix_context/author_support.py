"""Pure final renderer hook; preserve all existing criterion objects and observations."""
import json,pathlib
from .bindings import BINDINGS
ROOT=pathlib.Path(__file__).resolve().parents[4]
def attach(d):
 if d['project'] not in ['PR-T23-01','PR-T23-05','PR-T23-06']:return d
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/posix_context_depth').rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 register='research/v5/expansion/legacy_posix_context/resources.json';resources=json.loads((ROOT/register).read_text(encoding='utf8'))['resources']
 d['supplied_artifacts']=sorted(set(d['supplied_artifacts']+public));contract='labs/posix_context_depth/CONTRACTS.md'
 if contract not in d['shared_contracts']:d['shared_contracts'].append(contract)
 for r in resources:d['source_records']=[x for x in d['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=register,passages=r['locators'])]
 ids=[c['id'] for c in d['criteria'] if c['objective'] in BINDINGS]
 row=dict(path='research/v5/expansion/legacy_posix_context/completion_dispositions.json',project=d['project'],criterion_ids=ids,scope='Only these remaining original criteria and exact delayed author tasks; other existing successor pointers and independent learner obligations remain unchanged.')
 existing=d.setdefault('additional_successor_support',[]);existing[:]=[x for x in existing if x.get('path')!=row['path']]+[row]
 d['execution_support']['posix_context_successor']=row
 if d['project']=='PR-T23-01':d['execution_support']['current_preserved_launcher_binding']='research/v5/expansion/legacy_posix_context/launcher_binding_successor.json'
 d['status']['execution']='Per-criterion current successors, including posix_context_depth; historical observations retained and independent learner/review not inferred.'
 return d
