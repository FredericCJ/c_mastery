"""Attach exact validated successor criteria; never promote unrelated T12 work."""
from pathlib import Path
import argparse,sys
ROOT=Path(__file__).resolve().parents[4];sys.path.insert(0,str(ROOT/'tools'))
from common import read_json,dump,sha
from check_library_followthrough import validate,MODULES
from expansion_evidence import require_batch_research
folder=Path(__file__).parent
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();require_batch_research(folder);report=read_json(ROOT/a.report);errors=validate(report)
 if errors:raise SystemExit(errors)
 guardpath='reviews/v5/expansion/legacy_library/support_validation.json';guard=read_json(ROOT/guardpath)
 assert guard['passed'] and guard['input_sha256'][a.report]==sha(ROOT/a.report) and guard['input_sha256']['tools/check_library_followthrough.py']==sha(ROOT/'tools/check_library_followthrough.py')
 support='research/v5/expansion/legacy_library/support.json';observations={(r['objective'],r['variant']):r for r in report['observations']};resources=read_json(folder/'resources.json')['resources'];public=[f.relative_to(ROOT).as_posix() for f in sorted((ROOT/'labs/foundations/library_depth').glob('*')) if f.is_file()]+['labs/foundations/include/foundation.h'];criteria=[];updates=[];canonical=[]
 for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T12-*.json')):
  record=read_json(path);bound=[r for r in record['criteria'] if (r['objective'],False) in observations]
  if not bound:continue
  origin=path.relative_to(ROOT).as_posix();canonical.append(origin);record['supplied_artifacts']=list(dict.fromkeys(record['supplied_artifacts']+public));existing={r['id'] for r in record['source_records']};record['source_records'] += [dict(id=r['id'],path='research/v5/expansion/legacy_library/resources.json',passages=r['locators']) for r in resources if r['id'] not in existing]
  for row in bound:
   oid=row['objective'];module=next(m for m in report['selected_modules'] if oid in MODULES[m]);teaching='labs/foundations/library_depth/'+module.upper()+'.md';analysis='assessor/legacy_library/'+module.upper()+'.md';row['successor_support']=dict(path=support,criterion_id=row['id']);criteria.append(dict(id=row['id'],objective=oid,project=record['project'],original_source=origin,ordinary=observations[oid,False],changed=observations[oid,True],teaching=teaching,assessor_analysis=analysis,receipt=a.report))
   if oid=='T12.16.b':row['versioned_correction']='research/v5/expansion/legacy_library/corrections.json#LLIB-T12-PICO-2026-01'
   for ident,variant in [(row['id'],False),*[(v['id'],True) for v in row['variants']]]:
    updates.append(dict(id=ident,original_source=origin,original_objective=oid,project=record['project'],closure_rationale=observations[oid,variant]['claim'],dimensions={'teaching':dict(state='complete-with-published-ISO-source-gap',evidence=[teaching,origin]),'implementation':dict(state='complete-for-declared-reference-contract',evidence=['assessor/legacy_library/'+module+'.c','assessor/legacy_library/'+module+'.py',analysis]),'author_validation':dict(state='passed-with-stated-scope',evidence=[a.report,guardpath]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed; original minimum7days unchanged',evidence=[]),'physical_observation':dict(state='not-observed; native-host and target-object evidence only',evidence=[])},required_environment='Declared C17 host profiles, actual installed locales and explicit foreign object targets in receipt; no Windows library or physical target execution inferred.',validation_method='Criterion-specific ordinary/changed executions, independent oracle and compiled semantic/translation controls; retained commands and artifacts.',evidence_limits=report['limits']))
  record['successor_support']=dict(path=support,criterion_ids=[r['id'] for r in bound],scope='Only explicitly named validated criteria; all other original ordinary and delayed tasks remain required.');dump(path,record)
 if 'picolib' in report['selected_modules']:
  original=next(r for r in updates if r['original_objective']=='T12.16.b');correction=dict(original,id='LLIB-T12-PICO-2026-01',original_source='research/v5/expansion/legacy_library/corrections.json',closure_rationale='Actual tagged configuration rejection and supported A/B successor preserve historical task identity; console/file callbacks, closure and changed nonseekable policy executed.');updates.append(correction)
 remaining=sorted({f'T12.{i:02}.{c}' for i in range(1,17) for c in 'abc'}-{r['objective'] for r in criteria})
 if not remaining:
  parent=dict(updates[0],id='G-LLIB-T12',original_source='research/v5/expansion/legacy_library/gap_ledger.json',original_objective='T12.01–16',project='PR-T12-01..05',closure_rationale='All48 original ordinary and48 original changed criteria have separately mapped working implementations or rigorous reasoning packets; exact source access and external dimensions remain open.');parent['dimensions']={'teaching':dict(state='complete-for-registered-criteria-with-published-source-gap',evidence=['research/v5/expansion/legacy_library/AUDIT.md',*canonical]),'implementation':dict(state='complete-for-48-ordinary-and-48-changed-contracts',evidence=[support]),'author_validation':dict(state='passed-bounded-source-bound-campaign',evidence=[a.report,guardpath]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed; original minimum7days retained',evidence=[]),'physical_observation':dict(state='not-observed; hosted execution and target-build evidence separate',evidence=[])};updates.append(parent)
 pins=[a.report,guardpath,*public,*canonical,'research/v5/expansion/legacy_library/corrections.json','research/v5/expansion/legacy_library/resources.json']
 dump(folder/'support.json',dict(schema_version=1,required=True,scope='Exact registered T12 ordinary/changed criteria; source access and external dimensions remain separately open',source_sha256={x:sha(ROOT/x) for x in pins},criteria=criteria,updates=updates,remaining_author_objectives=remaining))
 statepath=ROOT/'curriculum/v5/expansion/implementation_updates.json';state=read_json(statepath);index={r['id']:r for r in state['updates']}
 for row in updates:index[row['id']]=row
 state['updates']=list(index.values());dump(statepath,state);print(dict(criteria=len(criteria),dispositions=len(updates),total=len(index),whole_topic_complete=False))
if __name__=='__main__':main()
