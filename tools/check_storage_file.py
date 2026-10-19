"""Require actual original-adapter imports and exact changed-operation evidence."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/expansion/legacy_storage_followthrough/file_validation.json'
EXPORT='research/v5/expansion/legacy_storage_followthrough/file_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
ANCHORS={'original_transfer','nonblocking','readiness','native','fresh','amplification'}
LEARNERS={'nonblocking','readiness','fresh'}
CONTROLS={1:'nonblocking',2:'nonblocking',3:'fresh',4:'fresh'}
TRACE='2e9bc9877d427079e76ea2718843982d4007a1ff906d03fc4ff27c4686f7c956'
IMPORTS={'assessor/storage/store.c','labs/storage/include/store.h','labs/storage/include/store_file.h','labs/storage/src/store_file.c'}
TRAFFIC=dict(commit=[0,34,3],accepted_commit_bytes=139264,checkpoint=[65,34,3],accepted_checkpoint_read=266240,accepted_checkpoint_write=139264,useful_changed_bytes=1)

def check_export(e,files=True):
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and not e['private_sources_copied']
 assert e['input_sha256'] and e['exported_artifacts'] and e['explicit_actual_provider']=='assessor/storage/store.c'
 assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed'
 assert len(e['executions'])==8 and e['projects']==['PR-T47-11']
 failed=[r for r in e['executions'] if r['exit_code']!=0];assert len(failed)==2 and {r['required_marker'] for r in failed}=={'SFI_CHECK','Supply completed storage and original file provider'}
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
  assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==4
  assert {r['test'] for r in p['controls']}=={f'storage_followthrough.mutation.{a}.{i}' for i,a in CONTROLS.items()} and all(r['semantic_rejection'] for r in p['controls'])
 assert len(d['learner_stub_rejections'])==3 and {r['test'] for r in d['learner_stub_rejections']}==LEARNERS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
 assert len(d['observations'])==6 and {r['profile'] for r in d['observations']}==PROFILES
 for row in d['observations']:
  r=row['result'];assert row['kind']=='independent-oracle' and r['passed'] and (r['short_transfer_histories'],r['nonblocking_rows'],r['trace_sha256'])==(40954,8,TRACE) and r['traffic']==TRAFFIC
 needed=IMPORTS|{'assessor/storage_followthrough/file/file.c','labs/storage_followthrough/file/CONTRACTS.md','labs/storage_followthrough/file/tests.c','tools/verify_storage_file.py'};assert needed<=set(d['input_sha256'])
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 for c in d['compiler_dependency_closures']:assert IMPORTS<=set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
 rows=d['executions'];mutants=[r for r in rows if 'storage-file-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==106 and len(mutants)==24 and all(r['exit_code']==1 for r in mutants) and len(learners)==3 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 if files:
  bindings(d)
  for r in mutants+learners:
   body=(ROOT/r['log']).read_text(encoding='utf8');assert 'SFI_CHECK' in body and not any(s in body for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
  for r in ctests:
   body=(ROOT/r['log']).read_text(encoding='utf8');assert '100% tests passed' in body and all('SFI '+a+' validated' in body for a in ANCHORS-{'amplification'})
   assert 'histories=40954' in body and 'off_t=8' in body and 'actual-nonblocking-pipe-poll-prefix-and-hangup' in body and 'file-before-directory-sync uncertain-name-retained close-not-retried' in body
  for r in d['generated_artifacts']:
   if r['path'].endswith('compile_commands.json'):
    commands=json.loads((ROOT/r['path']).read_text());assert commands and all('-std=c17' in c['command'] for c in commands)
  for r in json.loads((ROOT/'research/v5/expansion/legacy_storage_followthrough/file_resources.json').read_text())['resources']:
   if 'local_sha256' in r['inspection']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
 return True

def selftest(d,e):
 edits=[lambda a:a.update(passed=False),lambda a:a['profiles'].pop(),lambda a:a['profiles'][0]['controls'].pop(),lambda a:a['learner_stub_rejections'].pop(),lambda a:a['observations'][0]['result'].update(short_transfer_histories=1),lambda a:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a:a.update(input_sha256={}),lambda a:a.update(learner_mastery='passed'),lambda a:a['compiler_dependency_closures'].pop(),lambda a:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a:a['observations'].pop(),lambda a:a.update(source_stable=False),lambda a:a.update(physical_validation='passed'),lambda a:a['observations'][0]['result']['traffic'].update(commit=[0,1,0]),lambda a:a['profiles'][0].update(anchors=['native']),lambda a:a.update(external_inputs_stable=False),lambda a:a['profiles'][0]['controls'][0].update(semantic_rejection=False),lambda a:a.update(independent_review='passed'),lambda a:a['compiler_dependency_closures'][0]['repository_inputs'].remove('labs/storage/src/store_file.c'),lambda a:a['learner_stub_rejections'].append(dict(test='original_transfer',semantic_rejection=True))]
 for edit in edits:
  a=copy.deepcopy(d);edit(a)
  try:check(a,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported file evidence')
 for edit in [lambda a:a.update(private_sources_copied=True),lambda a:a['executions'].pop(),lambda a:a.update(exported_artifacts={}),lambda a:a.update(learner_mastery='passed')]:
  a=copy.deepcopy(e);edit(a)
  try:check_export(a,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported export')
 return len(edits)+4
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());e=json.loads((ROOT/EXPORT).read_text());check(d);check_export(e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,e) if a.self_test else 0,report_sha256=sha(ROOT/REPORT),export_sha256=sha(ROOT/EXPORT))))
