"""Add CAN successor while preserving the two existing terminal criteria."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[4]
SUPPORT='research/v5/expansion/legacy_can/support.json'
PUBLIC=['frames.h','frames.c','tests.c','CONTRACTS.md','CAN.md','BENCH.md','bench_plan.json','check_bench.py']
def public_paths():return ['labs/hosted_systems/can_depth/'+p for p in PUBLIC]
def attach(record):
 path=ROOT/SUPPORT
 if not path.is_file() or record.get('project')!='PR-T24-06':return record
 support=json.loads(path.read_text(encoding='utf8'));ids=[r['id'] for r in support['criteria']]
 record['supplied_artifacts']=sorted(set(record['supplied_artifacts']+public_paths()))
 record['shared_contracts']=list(dict.fromkeys(record['shared_contracts']+['labs/hosted_systems/can_depth/CONTRACTS.md']))
 register='research/v5/expansion/legacy_can/resources.json';resources=json.loads((ROOT/register).read_text(encoding='utf8'))['resources'];existing={r['id'] for r in record['source_records']}
 record['source_records'] += [dict(id=r['id'],path=register,passages=r['locators']) for r in resources if r['id'] not in existing]
 for row in record['criteria']:
  if row['id'] in ids:row['successor_support']=dict(path=SUPPORT,criterion_id=row['id'])
 previous=record.get('successor_support')
 if previous and previous.get('path')!=SUPPORT and previous not in record.setdefault('prior_successor_supports',[]):record['prior_successor_supports'].append(previous)
 record['successor_support']=dict(path=SUPPORT,criterion_ids=ids,scope='Only CAN/frame-adapter and authored two-node transfer packet; earlier terminal successor remains on its own criteria. Physical and independent/delayed evidence pending.')
 return record
