"""Reject unsupported queued-read ownership and completion claims."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/expansion/legacy_storage_followthrough/async_validation.json'
EXPORT='research/v5/expansion/legacy_storage_followthrough/async_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
ANCHORS={'retained','unknown','admission'}
CONTROLS={1:'retained',2:'unknown',3:'retained'}
TRACE='3057abec3b0991a8c83c576d51df0a6b50b57e0dd19426035735f651fe2028e3'
IMPORTS={'labs/storage_followthrough/async/async.h'}
def check_export(e,files=True):
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and not e['private_sources_copied']
 assert e['input_sha256'] and e['exported_artifacts'] and e['explicit_actual_provider']=='assessor/storage_followthrough/async/async.c'
 assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed'
 assert len(e['executions'])==8 and e['projects']==['PR-T47-12']
 failed=[r for r in e['executions'] if r['exit_code']!=0];assert len(failed)==2 and {r['required_marker'] for r in failed}=={'SA_CHECK','Supply the explicit completed async implementation'}
 assert all(r['expected_exit']==0 and r['exit_code']==0 or r['expected_exit'] is None and r['exit_code'] not in [0,None] for r in e['executions'])
 if files:
  for p,h in e['input_sha256'].items():assert sha(ROOT/p)==h,'stale export '+p
  for p,h in e['exported_artifacts'].items():assert p in e['input_sha256'] and h==e['input_sha256'][p] and 'assessor' not in p.split('/')
  for r in e['executions']:
   assert sha(ROOT/r['log'])==r['log_sha256']
   if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
 return True
def check(d,files=True):
 base(d);assert len(d['profiles'])==6 and {p['profile'] for p in d['profiles']}==PROFILES
 for p in d['profiles']:
  assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==3
  assert {r['test'] for r in p['controls']}=={f'storage_followthrough.mutation.{a}.{i}' for i,a in CONTROLS.items()} and all(r['semantic_rejection'] for r in p['controls'])
 assert len(d['learner_stub_rejections'])==3 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
 assert len(d['observations'])==6 and {r['profile'] for r in d['observations']}==PROFILES
 for row in d['observations']:
  r=row['result'];assert row['kind']=='independent-oracle' and r['passed'] and (r['histories'],r['transitions'],r['trace_sha256'])==(21840,123792,TRACE)
 assert IMPORTS|{'assessor/storage_followthrough/async/async.c','labs/storage_followthrough/async/CONTRACTS.md','labs/storage_followthrough/async/tests.c','tools/verify_storage_async.py'}<=set(d['input_sha256'])
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 for c in d['compiler_dependency_closures']:assert IMPORTS<=set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
 rows=d['executions'];mutants=[r for r in rows if 'storage-async-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==100 and len(mutants)==18 and all(r['exit_code']==1 for r in mutants) and len(learners)==3 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 if files:
  bindings(d)
  for r in mutants+learners:
   body=(ROOT/r['log']).read_text(encoding='utf8');assert 'SA_CHECK' in body and not any(s in body for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
  for r in ctests:
   body=(ROOT/r['log']).read_text(encoding='utf8');assert '100% tests passed' in body and all('SA '+a+' validated' in body for a in ANCHORS)
   assert all(s in body for s in ['abandonment-retains-borrow-until-positive-completion','terminal-error-not-quiescence-no-automatic-reuse','no-request-on-refusal-live-control-reuse-refused'])
  for r in d['generated_artifacts']:
   if r['path'].endswith('compile_commands.json'):
    commands=json.loads((ROOT/r['path']).read_text());assert commands and all('-std=c17' in c['command'] for c in commands)
  resources=json.loads((ROOT/'research/v5/expansion/legacy_storage_followthrough/async_resources.json').read_text())
  for r in resources['resources']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
 return True
def selftest(d,e):
 edits=[lambda a:a.update(passed=False),lambda a:a['profiles'].pop(),lambda a:a['profiles'][0]['controls'].pop(),lambda a:a['learner_stub_rejections'].pop(),lambda a:a['observations'][0]['result'].update(transitions=1),lambda a:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a:a.update(input_sha256={}),lambda a:a.update(learner_mastery='passed'),lambda a:a['compiler_dependency_closures'].pop(),lambda a:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a:a['observations'].pop(),lambda a:a.update(source_stable=False),lambda a:a.update(physical_validation='passed'),lambda a:a['observations'][0]['result'].update(histories=1),lambda a:a['profiles'][0].update(anchors=['retained']),lambda a:a.update(external_inputs_stable=False),lambda a:a['profiles'][0]['controls'][0].update(semantic_rejection=False),lambda a:a.update(independent_review='passed'),lambda a:a['compiler_dependency_closures'][0]['repository_inputs'].remove('labs/storage_followthrough/async/async.h'),lambda a:a['observations'][0]['result'].update(transitions=0)]
 for edit in edits:
  a=copy.deepcopy(d);edit(a)
  try:check(a,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported async evidence')
 for edit in [lambda a:a.update(private_sources_copied=True),lambda a:a['executions'].pop(),lambda a:a.update(exported_artifacts={}),lambda a:a.update(learner_mastery='passed')]:
  a=copy.deepcopy(e);edit(a)
  try:check_export(a,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported export')
 return len(edits)+4
if __name__=='__main__':
 from check_storage_async_guest import check as companion_check,selftest as companion_selftest,REPORT as COMPANION
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());e=json.loads((ROOT/EXPORT).read_text());c=json.loads((ROOT/COMPANION).read_text());check(d);check_export(e);companion_check(c);print(json.dumps(dict(passed=True,negative_controls=selftest(d,e)+companion_selftest(c) if a.self_test else 0,report_sha256=sha(ROOT/REPORT),export_sha256=sha(ROOT/EXPORT),companion_sha256=sha(ROOT/COMPANION))))
