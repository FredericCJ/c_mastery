"""Reject absent stages, stale evidence and claims beyond answer comparison."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
from run_portability_lexical import observation
BASE='research/v5/expansion/legacy_portability_followthrough';REPORT=BASE+'/lexical_validation.json';EXPORT=BASE+'/lexical_export_validation.json'
PROFILES={c+'-'+m+'-'+o for c in ['gcc','clang'] for m in ['c17','c23'] for o in ['O0','O3']}
def check(d,e,files=True):
 base(d);assert len(d['profiles'])==8 and {r['profile'] for r in d['profiles']}==PROFILES
 assert len(d['executions'])==247 and len(d['judgments'])==13
 assert d['judgments'][0]['kind']=='unfinished' and not d['judgments'][0]['result']['passed'] and len(d['judgments'][0]['result']['failed_fields'])==11
 assert d['judgments'][1]['kind']=='reference' and d['judgments'][1]['result']['passed']
 assert all(r['result']['reasoning_assessment']=='not-performed' for r in d['judgments'])
 assert len({r['field'] for r in d['judgments'][2:]})==11
 for r in d['judgments'][2:]:assert not r['result']['passed'] and r['result']['failed_fields']==[r['field']]
 for r in d['profiles']:
  assert r['passed'] and r['counter_controls']==dict(no_splice=9,merged_diagnostic=True,stale=15)
  mode=r['profile'].split('-')[1];observation(dict(mode=mode,observed=r['observed']),stage_records=False)
 for r in d['executions']:
  if 'expected_exit' in r:assert r['exit_code']==r['expected_exit']
 assert sum(r['command'][0] in ['gcc','clang'] and '-fsyntax-only' in r['command'] and r['exit_code']==1 for r in d['executions'])==16
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and e['input_sha256'] and e['exported_artifacts'] and not e['private_sources_copied']
 assert e['project']=='PR-T06-01' and e['explicit_answer_key']=='assessor/language_portability/lexical/answers.json'
 assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed'
 assert len(e['executions'])==6 and [r['exit_code'] for r in e['executions']]==[0,0,0,1,0,2] and all(r['exit_code']==r['expected_exit'] for r in e['executions'])
 assert len(e['observations'])==2 and {(r['compiler'],r['mode']) for r in e['observations']}=={('gcc','c17'),('clang','c23')}
 if files:
  bindings(d)
  for p,h in e['input_sha256'].items():assert sha(ROOT/p)==h,'stale export '+p
  for row in e['generated_artifacts']:assert sha(ROOT/row['path'])==row['sha256']
  for r in d['profiles']+e['observations']:
   assert sha(ROOT/r['report'])==r['report_sha256'];q=json.loads((ROOT/r['report']).read_text());observation(q)
   for a in q['artifacts']:assert sha(__import__('pathlib').Path(a['path']))==a['sha256']
   for row in q['executions']:
    assert sha(__import__('pathlib').Path(row['log']))==row['log_sha256']
    if '-fsyntax-only' in row['command']:assert 'error:' in __import__('pathlib').Path(row['log']).read_text()
   home=(ROOT/r['report']).parent/'dependency'
   for i,ev in enumerate(q['observed']['dependency_events']):
    for n,h in ev['object_sha256'].items():assert sha(home/(n+str(i)+'.o'))==h
  for r in e['executions']:assert sha(ROOT/r['log'])==r['log_sha256']
  assert 'required' in (ROOT/e['executions'][-1]['log']).read_text()
  for p,h in e['exported_artifacts'].items():assert e['input_sha256'][p]==h and 'assessor' not in p.split('/')
  gate=json.loads((ROOT/BASE/'lexical_research_gate.json').read_text());assert gate['status']=='passage-inspected-author-only'
  for p,h in {**gate['input_sha256'],**gate['external_source_sha256']}.items():assert sha(ROOT/p)==h,'stale inspection '+p
  for r in json.loads((ROOT/BASE/'lexical_resources.json').read_text())['resources']:
   if 'local_sha256' in r['inspection']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
 return True
def selftest(d,e):
 changes=[lambda a,b:a.update(passed=False),lambda a,b:a.update(source_stable=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'].append(a['profiles'][0]),lambda a,b:a['profiles'][0]['observed'].update(splice_exit=9),lambda a,b:a['profiles'][0]['observed'].update(trigraph_exit=9),lambda a,b:a['profiles'][0]['observed'].update(bad_preprocess_exit=1),lambda a,b:a['profiles'][0]['observed'].update(bad_compile_exit=0),lambda a,b:a['profiles'][0]['observed']['dependency_events'][2].update(compiled=[]),lambda a,b:a['profiles'][0]['observed']['dependency_events'][2].update(exit_code=15),lambda a,b:a['profiles'][0]['counter_controls'].update(stale=19),lambda a,b:a['judgments'].pop(),lambda a,b:a['judgments'][0]['result'].update(passed=True),lambda a,b:a['judgments'][1]['result'].update(reasoning_assessment='passed'),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a.update(input_sha256={}),lambda a,b:a['executions'].pop(),lambda a,b:b.update(private_sources_copied=True),lambda a,b:b.update(exported_artifacts={}),lambda a,b:b['observations'].pop(),lambda a,b:b['executions'][-1].update(exit_code=0),lambda a,b:b.update(physical_validation='passed')]
 for change in changes:
  a,b=copy.deepcopy(d),copy.deepcopy(e);change(a,b)
  try:check(a,b,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported stage evidence')
 return len(changes)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d,e=[json.loads((ROOT/q).read_text()) for q in [REPORT,EXPORT]];check(d,e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,e) if a.self_test else 0,current_reports={q:sha(ROOT/q) for q in [REPORT,EXPORT]})))
