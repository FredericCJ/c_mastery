"""Pure additive attachment of the exact source-bound framing criteria."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[4]
SUPPORT='research/v5/expansion/legacy_framing/support.json'
PUBLIC=['CONTRACTS.md','STREAMS.md','DATAGRAMS.md','stream.h','stream.c','tests.c']

def attach(record):
 path=ROOT/SUPPORT
 if not path.is_file() or record.get('project')!='PR-T24-03':return record
 support=json.loads(path.read_text(encoding='utf8'));ids=[r['id'] for r in support['criteria']]
 record['supplied_artifacts']=sorted(set(record['supplied_artifacts']+['labs/hosted_systems/stream_depth/'+p for p in PUBLIC]))
 record['shared_contracts']=list(dict.fromkeys(record['shared_contracts']+['labs/hosted_systems/stream_depth/CONTRACTS.md']))
 register='research/v5/expansion/legacy_framing/resources.json';resources=json.loads((ROOT/register).read_text(encoding='utf8'))['resources'];existing={r['id'] for r in record['source_records']}
 record['source_records'] += [dict(id=r['id'],path=register,passages=r['locators']) for r in resources if r['id'] not in existing]
 for row in record['criteria']:
  if row['id'] in ids:row['successor_support']=dict(path=SUPPORT,criterion_id=row['id'])
 previous=record.get('successor_support')
 if previous and previous.get('path')!=SUPPORT and previous not in record.setdefault('prior_successor_supports',[]):record['prior_successor_supports'].append(previous)
 record['successor_support']=dict(path=SUPPORT,criterion_ids=ids,scope='Only six named framing/datagram criteria and their implemented changed conditions; independent and delayed learner assessment remains required.')
 return record
