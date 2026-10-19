"""Pure attachment of only source-bound event criteria; no new coverage by glob."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[4]
SUPPORT='research/v5/expansion/legacy_events/support.json'

def attach(record):
 path=ROOT/SUPPORT
 if not path.is_file() or record.get('domain')!='T24':return record
 support=json.loads(path.read_text(encoding='utf8'))
 ids=[r['id'] for r in support['criteria'] if r['project']==record['project']]
 if not ids:return record
 public=[p for p in support['source_sha256'] if p.startswith('labs/hosted_systems/event_depth/')]
 record['supplied_artifacts']=sorted(set(record['supplied_artifacts']+public))
 contract='labs/hosted_systems/event_depth/CONTRACTS.md'
 record['shared_contracts']=list(dict.fromkeys(record['shared_contracts']+[contract]))
 register='research/v5/expansion/legacy_events/resources.json';resources=json.loads((ROOT/register).read_text(encoding='utf8'))['resources']
 existing={r['id'] for r in record['source_records']}
 record['source_records'] += [dict(id=r['id'],path=register,passages=r['locators']) for r in resources if r['id'] not in existing]
 for row in record['criteria']:
  if row['id'] in ids:row['successor_support']=dict(path=SUPPORT,criterion_id=row['id'])
 previous=record.get('successor_support')
 if previous and previous.get('path')!=SUPPORT and previous not in record.setdefault('prior_successor_supports',[]):record['prior_successor_supports'].append(previous)
 record['successor_support']=dict(path=SUPPORT,criterion_ids=ids,scope='Only explicitly named validated criteria; original independent and delayed assessment remains required.')
 return record
