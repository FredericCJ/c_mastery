"""Attach validated T21 successor support without rewriting original tasks/receipts."""
from pathlib import Path
import argparse
from common import ROOT,read_json,dump,sha
from check_failure_followthrough import validate,MODULES
from expansion_evidence import require_batch_research
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
 folder=ROOT/'research/v5/expansion/legacy_failure';require_batch_research(folder);report=read_json(ROOT/a.report);errors=validate(report)
 if set(report.get('selected_modules',[]))!=set(MODULES):errors.append('Full seven-module campaign required for closure')
 if errors:raise SystemExit(errors)
 guardpath='reviews/v5/expansion/legacy_failure/support_validation.json';guard=read_json(ROOT/guardpath)
 assert guard['passed'] and guard['input_sha256'][a.report]==sha(ROOT/a.report) and guard['input_sha256']['tools/check_failure_followthrough.py']==sha(ROOT/'tools/check_failure_followthrough.py')
 support='research/v5/expansion/legacy_failure/support.json';observations={(r['objective'],r['variant']):r for r in report['observations']};resources=read_json(folder/'resources.json')['resources'];public=[f.relative_to(ROOT).as_posix() for folder in ('labs/api_depth/failure','labs/api_depth/contracts','labs/api_depth/abstractions') for f in sorted((ROOT/folder).glob('*')) if f.is_file()]+['labs/api_depth/include/api_depth.h','labs/foundations/include/foundation.h'];criteria=[];updates=[];canonical=[]
 for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T21-*.json')):
  record=read_json(path);bound=[r for r in record['criteria'] if (r['objective'],False) in observations]
  if not bound:continue
  origin=path.relative_to(ROOT).as_posix();canonical.append(origin);record['shared_contracts']=list(dict.fromkeys(record['shared_contracts']+['labs/api_depth/failure/CONTRACTS.md']));record['supplied_artifacts']=list(dict.fromkeys(record['supplied_artifacts']+public));existing={r['id'] for r in record['source_records']};record['source_records'] += [dict(id=r['id'],path='research/v5/expansion/legacy_failure/resources.json',passages=r['locators']) for r in resources if r['id'] not in existing]
  for row in bound:
   oid=row['objective'];module=next(m for m in report['selected_modules'] if oid in MODULES[m]);teaching='labs/api_depth/failure/'+module.upper()+'.md';analysis='assessor/api_depth/failure/'+module.upper()+'.md';row['successor_support']=dict(path=support,criterion_id=row['id']);criteria.append(dict(id=row['id'],objective=oid,project=record['project'],original_source=origin,ordinary=observations[oid,False],changed=observations[oid,True],teaching=teaching,assessor_analysis=analysis,receipt=a.report))
   for ident,variant in [(row['id'],False),*[(v['id'],True) for v in row['variants']]]:
    updates.append(dict(id=ident,original_source=origin,original_objective=oid,project=record['project'],closure_rationale=observations[oid,variant]['claim'],dimensions={'teaching':dict(state='complete-for-declared-scope-with-exact-source-gaps',evidence=[teaching,origin,'research/v5/expansion/legacy_failure/source_access.json']),'implementation':dict(state='complete-for-declared-reference-contract',evidence=['assessor/api_depth/failure/'+module+'.c','assessor/api_depth/failure/'+module+'.py',analysis]),'author_validation':dict(state='passed-with-stated-scope',evidence=[a.report,guardpath]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed; original minimum7days unchanged',evidence=[]),'physical_observation':dict(state='not-observed; native-host and target-link evidence only',evidence=[])},required_environment='C17 GCC/Clang hosted Linux, POSIX threads and binutils; Cortex-M4/x86-64 strict generated audits; no board or persistence observation inferred.',validation_method='Criterion-specific ordinary/changed executions or explicit invalid-lifetime reasoning, independent oracles, compiled semantic and translation controls.',evidence_limits=report['limits']))
  record['successor_support']=dict(path=support,criterion_ids=[r['id'] for r in bound],scope='Only explicitly named validated criteria; original independent and delayed assessment remains required.');dump(path,record)
 remaining=sorted({f'T21.{i:02}.{c}' for i in range(1,11) for c in 'abc'}-{r['objective'] for r in criteria})
 if not remaining:
  parent=dict(updates[0],id='G-LDEPTH-T21',original_source='research/v5/expansion/legacy_api/gap_ledger.json',original_objective='T21.01–10',project='PR-T21-01..04',closure_rationale='All30 original ordinary and30 original changed criteria have separately mapped implementation or rigorous reasoning packets; exact source access and external dimensions remain open.');parent['dimensions']={'teaching':dict(state='complete-for-registered-criteria-with-source-gaps',evidence=['research/v5/expansion/legacy_failure/AUDIT.md',*canonical]),'implementation':dict(state='complete-for-30-ordinary-and-30-changed-contracts',evidence=[support]),'author_validation':dict(state='passed-bounded-source-bound-campaign',evidence=[a.report,guardpath]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed; original minimum7days retained',evidence=[]),'physical_observation':dict(state='not-observed; hosted execution and target-build evidence separate',evidence=[])};updates.append(parent)
 pins=[a.report,guardpath,*public,*canonical,'research/v5/expansion/legacy_failure/resources.json','research/v5/expansion/legacy_failure/source_access.json']
 dump(folder/'support.json',dict(schema_version=1,required=True,scope='Exact registered T21 ordinary/changed criteria; source access and external dimensions separately open; strict generated closure and hosted ports kept separate',source_sha256={x:sha(ROOT/x) for x in pins},criteria=criteria,updates=updates,remaining_author_objectives=remaining))
 statepath=ROOT/'curriculum/v5/expansion/implementation_updates.json';state=read_json(statepath);index={r['id']:r for r in state['updates']}
 for row in updates:index[row['id']]=row
 state['updates']=list(index.values());dump(statepath,state);print(dict(criteria=len(criteria),dispositions=len(updates),total=len(index),whole_topic_complete=False))
if __name__=='__main__':main()
