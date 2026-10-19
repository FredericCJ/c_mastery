"""Reject missing scope, unintended failures and stale durability evidence."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/expansion/legacy_storage_followthrough/durability_validation.json'
EXPORT='research/v5/expansion/legacy_storage_followthrough/durability_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan','tsan']}
ANCHORS={'commit','checkpoint','repeated','lies','invalid','successive','metadata','late_ack','third_selection','third_cuts','third_maximum'}
CONTROLS={1:'commit',2:'checkpoint',3:'invalid',4:'commit',5:'selection'}
TRACE='0e316ee2877d00fd20ec08f0d9fe21035aa5fac60f716be5047f4d05dc20f035'
ORIGINAL='ff8f3b35057cfbd4e8834cd32c4b630cd66975289abe882ef4f9d0df2c549626'

def check_export(e,files=True):
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and not e['private_sources_copied']
 assert e['input_sha256'] and e['exported_artifacts'] and e['explicit_actual_provider']=='assessor/storage/store.c'
 assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed'
 assert len(e['executions'])==32 and e['projects']==[f'PR-T47-{n:02}' for n in range(7,11)]
 failed=[r for r in e['executions'] if r['exit_code']!=0]
 assert len(failed)==8 and {r['required_marker'] for r in failed}=={'DUR_CHECK','Supply completed storage provider using CMASTERY_STORAGE_ROOT'}
 assert all(r['expected_exit']==0 and r['exit_code']==0 or r['expected_exit'] is None and r['exit_code'] not in [0,None] for r in e['executions'])
 if files:
  for p,h in e['input_sha256'].items():assert sha(ROOT/p)==h,'stale export '+p
  for p,h in e['exported_artifacts'].items():assert p in e['input_sha256'] and h==e['input_sha256'][p] and 'assessor' not in p.split('/')
  for r in e['executions']:
   assert sha(ROOT/r['log'])==r['log_sha256']
   if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
 return True

def check(d,files=True):
 base(d);assert len(d['profiles'])==8 and {p['profile'] for p in d['profiles']}==PROFILES
 for p in d['profiles']:
  assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==5
  assert {r['test'] for r in p['controls']}=={f'storage_followthrough.mutation.{a}.{i}' for i,a in CONTROLS.items()} and all(r['semantic_rejection'] for r in p['controls'])
 assert len(d['learner_stub_rejections'])==11 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
 assert len(d['observations'])==8 and {r['profile'] for r in d['observations']}==PROFILES
 for row in d['observations']:
  r=row['result'];assert row['kind']=='independent-oracle' and r['passed'] and r['trace_sha256']==TRACE
  assert (r['commit_schedules'],r['medium_cases'],r['actual_commit_events'],r['actual_checkpoint_events'])==(259,12,37,37)
  collision=r['crc_collision'];assert (collision['rank'],collision['first_value'],collision['second_value'],collision['checksum'])==(32,165,164,2329055228) and collision['page_sha256']!=collision['changed_sha256']
  m=r['model_checker'];assert m['passed'] and m['positive']['passed'] and (m['positive']['states'],m['positive']['transitions'],m['positive']['terminal_states'],m['positive']['max_pending'])==(135,332,16,2)
  assert len(m['controls'])==3 and {c['mutant'] for c in m['controls']}=={'no_data_flush','no_commit_flush','early_reclaim'} and all(not c['passed'] and c['witness'] for c in m['controls'])
 needed={'assessor/storage/store.c','labs/storage/include/store.h','labs/storage/FORMAT.md','assessor/storage_followthrough/durability/medium.c','assessor/storage_followthrough/durability/guard.c','labs/storage_followthrough/durability/CONTRACTS.md','labs/storage_followthrough/durability/tests.c','labs/storage_followthrough/durability/third/tests.c','labs/storage_followthrough/durability/instrument_store.py','tools/verify_storage_durability.py','tools/model_storage_protocol.py'}
 assert needed<=set(d['input_sha256']) and d['input_sha256']['assessor/storage/store.c']==ORIGINAL
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 for c in d['compiler_dependency_closures']:
  assert set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs'] and 'labs/storage/include/store.h' in c['repository_inputs']
  generated=[r for r in c['generated_link_inputs'].values() if 'metadata_slots' in r]
  assert len(generated)==2 and {r['metadata_slots'] for r in generated}=={2,3} and all(r['producer_source']=='labs/storage_followthrough/durability/instrument_store.py' and r['original_source']=='assessor/storage/store.c' for r in generated)
 rows=d['executions'];mutants=[r for r in rows if 'storage-durability-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==186 and len(mutants)==40 and all(r['exit_code']==1 for r in mutants) and len(learners)==11 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==8 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 if files:
  bindings(d)
  for r in mutants+learners:
   body=(ROOT/r['log']).read_text(encoding='utf8');assert 'DUR_CHECK' in body and not any(s in body for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
  for r in ctests:
   body=(ROOT/r['log']).read_text(encoding='utf8');assert '100% tests passed' in body and all('DUR '+a+' validated' in body for a in ANCHORS)
   assert 'DUR invalid validated structural-before-home=0' in body and 'DUR invalid validated structural-before-home=1' in body
   assert 'double-cuts=1369 canonical-homes-and-whole-image-idempotent' in body and 'retained-until-join-then-reconcile-new' in body
  for r in d['generated_artifacts']:
   if r['path'].endswith('compile_commands.json'):
    commands=json.loads((ROOT/r['path']).read_text());assert commands and all('-std=c17' in c['command'] for c in commands)
    if '-tsan-' in r['path']:assert all('-fsanitize=thread' in c['command'] for c in commands)
  for r in json.loads((ROOT/'research/v5/expansion/legacy_storage_followthrough/durability_resources.json').read_text())['resources']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
 return True

def selftest(d,e):
 edits=[lambda a:a.update(passed=False),lambda a:a['profiles'].pop(),lambda a:a['profiles'][0]['controls'].pop(),lambda a:a['learner_stub_rejections'].pop(),lambda a:a['observations'][0]['result'].update(commit_schedules=1),lambda a:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a:a.update(input_sha256={}),lambda a:a.update(learner_mastery='passed'),lambda a:a['compiler_dependency_closures'].pop(),lambda a:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a:a['observations'].pop(),lambda a:a.update(source_stable=False),lambda a:a.update(physical_validation='passed'),lambda a:a['observations'][0]['result']['model_checker']['positive'].update(states=1),lambda a:a['profiles'][0].update(anchors=['commit']),lambda a:a.update(external_inputs_stable=False),lambda a:a['profiles'][0]['controls'][0].update(semantic_rejection=False),lambda a:a.update(independent_review='passed'),lambda a:a['observations'][0]['result']['model_checker']['controls'].pop(),lambda a:a['observations'][0]['result']['crc_collision'].update(second_value=165),lambda a:next(r for r in a['compiler_dependency_closures'][0]['generated_link_inputs'].values() if 'metadata_slots' in r).update(metadata_slots=4)]
 for edit in edits:
  a=copy.deepcopy(d);edit(a)
  try:check(a,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported durability evidence')
 for edit in [lambda a:a.update(private_sources_copied=True),lambda a:a['executions'].pop(),lambda a:a.update(exported_artifacts={}),lambda a:a.update(learner_mastery='passed')]:
  a=copy.deepcopy(e);edit(a)
  try:check_export(a,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported export')
 return len(edits)+4
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());e=json.loads((ROOT/EXPORT).read_text());check(d);check_export(e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,e) if a.self_test else 0,report_sha256=sha(ROOT/REPORT),export_sha256=sha(ROOT/EXPORT))))
