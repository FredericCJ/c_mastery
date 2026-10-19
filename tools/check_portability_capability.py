"""Reject layer conflation, missing variants, stale pins and unobserved controls."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
BASE='research/v5/expansion/legacy_portability_followthrough'
REPORT=BASE+'/capability_validation.json';COMPANION=BASE+'/capability_companion_validation.json';EXPORT=BASE+'/capability_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
ANCHORS={'record','transform','matrix','trace'};LEARNERS=ANCHORS-{'trace'}
CONTROL={1:'transform',2:'matrix',3:'trace',4:'record'}
TRACE='65514ba382ab1ec680b8936fb25408915bc7e82ff892a44d4c51a1d2f38a146f'

def check(d,c,e,files=True):
 base(d);base(c)
 assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
 for r in d['profiles']:
  assert r['passed'] and set(r['anchors'])==ANCHORS and len(r['controls'])==4
  assert {v['test'] for v in r['controls']}=={f'portability.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(v['semantic_rejection'] for v in r['controls'])
 assert len(d['learner_stub_rejections'])==3 and {r['test'] for r in d['learner_stub_rejections']}==LEARNERS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
 oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
 assert len(oracle)==len(metrics)==6 and {r['profile'] for r in oracle}=={r['profile'] for r in metrics}==PROFILES
 for r in oracle:
  q=r['result'];assert q['passed'] and q['trace_sha256']==TRACE and q['cases']==1655 and q['mask_cases']==324 and q['transform_cases']==1331
 for r in metrics:assert r['result']['record_bytes']>0 and r['result']['array_limit']==16 and r['result']['int_min']==-2147483648 and r['result']['int_max']==2147483647
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256']) and r['external_inputs']
 assert {'assessor/language_portability/capability/capability.c','labs/language_portability/capability/capability.h','tools/verify_portability_capability.py'}<=set(d['input_sha256'])
 rows=d['executions'];mutants=[r for r in rows if 'portability-capability-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==112 and len(mutants)==24 and all(r['exit_code']==1 for r in mutants) and len(learners)==3 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 for tool in ['nm','objdump']:assert len([r for r in rows if r['command'][0]==tool and r['command'][-1].endswith('/capability.c.o')])==6
 assert len(c['profiles'])==4 and {(r['compiler'],r['optimization']) for r in c['profiles']}=={(a,b) for a in ['gcc','clang'] for b in ['-O0','-O3']} and len(c['executions'])==902
 for p in c['profiles']:
  assert p['changed_layouts']==2 and p['wide_atomic_helper'] and p['explicit_original_helper']
  assert len(p['features'])==12 and {(r['mode'],r['feature']) for r in p['features']}=={(m,f) for m in ['c11','c17'] for f in range(1,7)}
  for r in p['features']:
   ex=[c['executions'][i] for i in r['steps']];assert len(ex)==5 and all(q['exit_code']==0 for q in ex)
   assert '-E' in ex[0]['command'] and '-fsyntax-only' in ex[1]['command'] and '-c' in ex[2]['command'] and '-pthread' in ex[3]['command'] and len(ex[4]['command'])==1
  assert len(p['inline'])==7 and sum(r.get('intended_duplicate_rejection',False) for r in p['inline'])==1
  assert {(r['mode'],r['kind']) for r in p['inline']}=={('c99','static'),('c17','static'),('gnu89','static'),('c99','external'),('c17','external'),('gnu89','external'),('gnu89','external-extern')}
  for r in p['inline']:
   if 'link' in r:assert c['executions'][r['link']]['expected_rejection']==r['intended_duplicate_rejection']
  ex=[c['executions'][i] for i in p['executions']];assert len(ex)==211+(p['compiler']=='clang')
  assert len([r for r in ex if '-dM' in r['command']])==2
  assert len([r for r in ex if r['expected_rejection']])==3+(p['compiler']=='clang')
 assert len(c['observations'])==8 and all(r['identical'] and r['output'].startswith('empty\n') for r in c['observations']) and {r['layout'] for r in c['observations']}=={1,2}
 assert c['synthetic_fixture_usable']==[False,True]
 assert len(c['target_profiles'])==3 and {r['profile'] for r in c['target_profiles']}=={'gcc-O0','gcc-O3','clang-O3'}
 for p in c['target_profiles']:
  assert p['actual_execution'] and p['zero_undefined'] and p['wide_runtime_rejected']
  ex=[c['executions'][i] for i in p['executions']];assert len(ex)==15 and sum(r['expected_rejection'] for r in ex)==1
  assert len([r for r in ex if r['command'][0]=='qemu-system-arm' and r['exit_code']==0])==1
  assert any('-nostdlib' in r['command'] and not r['expected_rejection'] for r in ex)
 for r in c['executions']:assert (r['exit_code'] not in [None,0] if r['expected_rejection'] else r['exit_code']==0)
 assert sum(r['expected_rejection'] for r in c['executions'])==18
 assert c['compiler_dependencies'] and any(p.endswith('/threads.h') for r in c['compiler_dependencies'] for p in r['paths']) and any(p.endswith('/stdatomic.h') for r in c['compiler_dependencies'] for p in r['paths'])
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and e['input_sha256'] and e['exported_artifacts'] and not e['private_sources_copied']
 assert e['project']=='PR-T05-02' and e['original_family']=='legacy-assessment' and e['explicit_actual_providers']==['assessor/language_portability/capability/capability.c']
 assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed' and len(e['executions'])==8
 failed=[r for r in e['executions'] if r['exit_code']!=0];assert len(failed)==2 and {r['required_marker'] for r in failed}=={'LP_CHECK','Supply the explicit completed capability implementation'}
 assert all(r['exit_code']==r['expected_exit']==0 or r['expected_exit'] is None and r['exit_code'] not in [None,0] for r in e['executions'])
 if files:
  bindings(d);bindings(c)
  for r in mutants+learners:
   text=(ROOT/r['log']).read_text(encoding='utf8');assert 'LP_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
  for r in ctests:
   text=(ROOT/r['log']).read_text(encoding='utf8');assert '100% tests passed' in text and all('LP '+a+' validated' in text for a in ANCHORS)
  for r in d['generated_artifacts']:
   if r['path'].endswith('compile_commands.json'):assert all('-std=c17' in q['command'] for q in json.loads((ROOT/r['path']).read_text()))
  for r in c['executions']:assert all(m in (ROOT/r['log']).read_text(encoding='utf8') for m in r['markers'])
  assert any('libatomic' in p for p in c['external_input_sha256'])
  for p,h in e['input_sha256'].items():assert sha(ROOT/p)==h,'stale export '+p
  for p,h in e['exported_artifacts'].items():assert e['input_sha256'][p]==h and 'assessor' not in p.split('/')
  for r in e['executions']:
   assert sha(ROOT/r['log'])==r['log_sha256']
   if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
  gate=json.loads((ROOT/BASE/'capability_research_gate.json').read_text());assert gate['status']=='passage-inspected-author-only'
  for p,h in {**gate['input_sha256'],**gate['external_source_sha256']}.items():assert sha(ROOT/p)==h,'stale inspection '+p
  for r in json.loads((ROOT/BASE/'capability_resources.json').read_text())['resources']:
   if 'local_sha256' in r['inspection']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
 return True

def selftest(d,companion,e):
 edits=[lambda a,b,c:a.update(passed=False),lambda a,b,c:a['profiles'].pop(),lambda a,b,c:a['profiles'][0]['controls'].pop(),lambda a,b,c:a['learner_stub_rejections'].pop(),lambda a,b,c:a['observations'][0]['result'].update(cases=1),lambda a,b,c:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b,c:a.update(input_sha256={}),lambda a,b,c:a.update(learner_mastery='passed'),lambda a,b,c:a['compiler_dependency_closures'].pop(),lambda a,b,c:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b,c:b['profiles'].pop(),lambda a,b,c:b['profiles'][0]['features'].pop(),lambda a,b,c:b['profiles'][0]['features'][0]['steps'].pop(),lambda a,b,c:b['profiles'][0].update(wide_atomic_helper=False),lambda a,b,c:b['profiles'][0]['inline'].pop(),lambda a,b,c:b.update(synthetic_fixture_usable=[True,True]),lambda a,b,c:b['target_profiles'].pop(),lambda a,b,c:b['target_profiles'][0].update(actual_execution=False),lambda a,b,c:b.update(physical_validation='passed'),lambda a,b,c:next(r for r in b['executions'] if r['expected_rejection']).update(exit_code=0),lambda a,b,c:c.update(private_sources_copied=True),lambda a,b,c:c['executions'].pop(),lambda a,b,c:c.update(exported_artifacts={}),lambda a,b,c:c.update(explicit_actual_providers=[])]
 for edit in edits:
  a,b,c=copy.deepcopy(d),copy.deepcopy(companion),copy.deepcopy(e);edit(a,b,c)
  try:check(a,b,c,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported capability evidence')
 return len(edits)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d,c,e=[json.loads((ROOT/p).read_text()) for p in [REPORT,COMPANION,EXPORT]];check(d,c,e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,c,e) if a.self_test else 0,current_reports={p:sha(ROOT/p) for p in [REPORT,COMPANION,EXPORT]})))
