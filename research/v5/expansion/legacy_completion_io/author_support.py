"""Pure source-bound attachment preserving the original criterion bodies."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[4]
SUPPORT='research/v5/expansion/legacy_completion_io/support.json'
PUBLIC=['completion.h','completion.c','tests.c','native.c','workload.c','stall.c','run.py','seal.py','CONTRACTS.md','COMPLETION.md']
PROVIDER=['raw_ring.h','raw_ring.c']
def public_paths():return ['labs/hosted_systems/completion_depth/'+p for p in PUBLIC]+['labs/linux_io_depth/targets/'+p for p in PROVIDER]
def attach(record):
 path=ROOT/SUPPORT
 if not path.is_file() or record.get('project')!='PR-T24-07':return record
 support=json.loads(path.read_text(encoding='utf8'));ids=[r['id'] for r in support['criteria']]
 record['supplied_artifacts']=sorted(set(record['supplied_artifacts']+public_paths()))
 record['shared_contracts']=list(dict.fromkeys(record['shared_contracts']+['labs/hosted_systems/completion_depth/CONTRACTS.md']))
 register='research/v5/expansion/legacy_completion_io/resources.json';resources=json.loads((ROOT/register).read_text(encoding='utf8'))['resources'];existing={r['id'] for r in record['source_records']}
 record['source_records'] += [dict(id=r['id'],path=register,passages=r['locators']) for r in resources if r['id'] not in existing]
 for row in record['criteria']:
  if row['id'] in ids:row['successor_support']=dict(path=SUPPORT,criterion_id=row['id'])
 record['successor_support']=dict(path=SUPPORT,criterion_ids=ids,scope='Three exact completion-I/O criteria and implemented changed contracts; no learner or independent delayed credit.')
 return record
