"""Pure source-bound reactor attachment; original criterion bodies remain intact."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[4]
SUPPORT='research/v5/expansion/legacy_reactor/support.json'
PUBLIC=['CONTRACTS.md','READINESS.md','LIFETIME.md','SHUTDOWN.md','reactor.h','reactor.c','tests.c','waiters.h','waiters.c','waiters_tests.c']
PROVIDER=['stream.h','stream.c','CONTRACTS.md','STREAMS.md']
def public_paths():return ['labs/hosted_systems/reactor_depth/'+p for p in PUBLIC]+['labs/hosted_systems/stream_depth/'+p for p in PROVIDER]
def attach(record):
 path=ROOT/SUPPORT
 if not path.is_file() or record.get('project')!='PR-T24-04':return record
 support=json.loads(path.read_text(encoding='utf8'));ids=[r['id'] for r in support['criteria']]
 record['supplied_artifacts']=sorted(set(record['supplied_artifacts']+public_paths()))
 record['shared_contracts']=list(dict.fromkeys(record['shared_contracts']+['labs/hosted_systems/reactor_depth/CONTRACTS.md','labs/hosted_systems/stream_depth/CONTRACTS.md']))
 register='research/v5/expansion/legacy_reactor/resources.json';resources=json.loads((ROOT/register).read_text(encoding='utf8'))['resources'];existing={r['id'] for r in record['source_records']}
 record['source_records'] += [dict(id=r['id'],path=register,passages=r['locators']) for r in resources if r['id'] not in existing]
 for row in record['criteria']:
  if row['id'] in ids:row['successor_support']=dict(path=SUPPORT,criterion_id=row['id'])
 previous=record.get('successor_support')
 if previous and previous.get('path')!=SUPPORT and previous not in record.setdefault('prior_successor_supports',[]):record['prior_successor_supports'].append(previous)
 record['successor_support']=dict(path=SUPPORT,criterion_ids=ids,scope='Only nine named readiness/lifetime/shutdown criteria and their implemented changed conditions; independent/delayed learner work remains required.')
 return record
