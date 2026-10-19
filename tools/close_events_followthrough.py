"""Attach only validated T24 successor criteria, preserving unrelated POSIX support."""
from pathlib import Path
import argparse
from common import ROOT,read_json,dump,sha
from check_events_followthrough import validate,MODULES,PUBLIC
from expansion_evidence import require_batch_research
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();folder=ROOT/'research/v5/expansion/legacy_events';require_batch_research(folder);report=read_json(ROOT/a.report);errors=validate(report)
 if errors:raise SystemExit(errors)
 guardpath='reviews/v5/expansion/legacy_events/support_validation.json';guard=read_json(ROOT/guardpath)
 assert guard['passed'] and guard['input_sha256'][a.report]==sha(ROOT/a.report) and guard['input_sha256']['tools/check_events_followthrough.py']==sha(ROOT/'tools/check_events_followthrough.py')
 support='research/v5/expansion/legacy_events/support.json';observations={(r['objective'],r['variant']):r for r in report['observations']};resources=read_json(folder/'resources.json')['resources'];public=['labs/hosted_systems/event_depth/'+name for name in dict.fromkeys(['CONTRACTS.md',*[n for module in report['selected_modules'] for n in PUBLIC[module]]])];criteria=[];updates=[];canonical=[]
 for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T24-*.json')):
  record=read_json(path);bound=[r for r in record['criteria'] if (r['objective'],False) in observations]
  if not bound:continue
  origin=path.relative_to(ROOT).as_posix();canonical.append(origin);record['shared_contracts']=list(dict.fromkeys(record['shared_contracts']+['labs/hosted_systems/event_depth/CONTRACTS.md']));record['supplied_artifacts']=sorted(set(record['supplied_artifacts']+public));existing={r['id'] for r in record['source_records']};record['source_records'] += [dict(id=r['id'],path='research/v5/expansion/legacy_events/resources.json',passages=r['locators']) for r in resources if r['id'] not in existing]
  for row in bound:
   oid=row['objective'];module=next(m for m in report['selected_modules'] if oid in MODULES[m]);teaching='labs/hosted_systems/event_depth/'+module.upper()+'.md';analysis='assessor/hosted_systems/event_depth/'+module.upper()+'.md';row['successor_support']=dict(path=support,criterion_id=row['id']);criteria.append(dict(id=row['id'],objective=oid,project=record['project'],original_source=origin,ordinary=observations[oid,False],changed=observations[oid,True],teaching=teaching,assessor_analysis=analysis,receipt=a.report))
   for ident,variant in [(row['id'],False),*[(v['id'],True) for v in row['variants']]]:
    updates.append(dict(id=ident,original_source=origin,original_objective=oid,project=record['project'],closure_rationale=observations[oid,variant]['claim'],dimensions={'teaching':dict(state='complete-for-declared-criterion-with-source-gaps',evidence=[teaching,origin,'research/v5/expansion/legacy_events/source_access.json']),'implementation':dict(state='complete-for-declared-reference-contract',evidence=['assessor/hosted_systems/event_depth/'+module+'.c','assessor/hosted_systems/event_depth/'+module+'.py',analysis]),'author_validation':dict(state='passed-with-stated-scope',evidence=[a.report,guardpath]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed; original minimum7days unchanged',evidence=[]),'physical_observation':dict(state='not-observed; hosted OS execution only',evidence=[])},required_environment='C17 GCC/Clang hosted Linux, POSIX threads, procfs own-child enrollment and binutils; no host policy changes.',validation_method='Exact ordinary/changed execution or explicitly scoped unsafe-handler reasoning; independent bounded oracles and compiled semantic negatives.',evidence_limits=report['limits']))
  previous=record.get('successor_support')
  if previous and previous.get('path')!=support:record.setdefault('prior_successor_supports',[]).append(previous)
  record['successor_support']=dict(path=support,criterion_ids=[r['id'] for r in bound],scope='Only explicitly named validated criteria; original independent and delayed assessment remains required.');dump(path,record)
 statepath=ROOT/'curriculum/v5/expansion/implementation_updates.json';state=read_json(statepath);index={r['id']:r for r in state['updates']}
 for row in updates:index[row['id']]=row
 def author_complete(ident):
  dims=index.get(ident,{}).get('dimensions',{})
  return dims.get('teaching',{}).get('state','').startswith('complete') and dims.get('implementation',{}).get('state','').startswith('complete') and dims.get('author_validation',{}).get('state','').startswith('passed')
 allrows=[r for p in (ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T24-*.json') for r in read_json(p)['criteria']];remaining=[r['objective'] for r in allrows if not author_complete(r['id']) or any(not author_complete(v['id']) for v in r['variants'])]
 pins=[a.report,guardpath,*public,*canonical,'research/v5/expansion/legacy_events/resources.json','research/v5/expansion/legacy_events/source_access.json','research/v5/expansion/legacy_events/author_support.py']
 dump(folder/'support.json',dict(schema_version=1,required=True,scope='Only exact named T24 criteria; retained T24.10 terminal support and all unimplemented objectives remain separate',source_sha256={x:sha(ROOT/x) for x in pins},criteria=criteria,updates=updates,remaining_author_objectives=sorted(set(remaining))))
 state['updates']=list(index.values());dump(statepath,state);print(dict(criteria=len(criteria),dispositions=len(updates),total=len(index),remaining_author_objectives=len(set(remaining)),whole_topic_complete=False))
if __name__=='__main__':main()
