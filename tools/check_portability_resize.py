"""Source-bound gate for normalized resize and bounded correction analyses."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
BASE='research/v5/expansion/legacy_portability_followthrough'
REPORT=BASE+'/resize_validation.json';COMPANION=BASE+'/resize_companion_validation.json';EXPORT=BASE+'/resize_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
ANCHORS={'zero','failure','drop','growth'}
CONTROL={1:'zero',2:'failure',3:'drop',4:'growth'}
TRACE='ee80ec0fa6945331b49fe793287872014fd1d3b8ca5840974c8c9b43c5049ecd'

def check(d,c,e,files=True):
 base(d);base(c)
 assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
 for r in d['profiles']:
  assert r['passed'] and set(r['anchors'])==ANCHORS and len(r['controls'])==4
  assert {v['test'] for v in r['controls']}=={f'portability.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(v['semantic_rejection'] for v in r['controls'])
 assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
 oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
 assert len(oracle)==6 and len(metrics)==6 and {r['profile'] for r in oracle}==PROFILES and {r['profile'] for r in metrics}==PROFILES
 for r in oracle:
  q=r['result'];assert q['passed'] and q['trace_sha256']==TRACE and q['histories']==39216 and q['maximum_length']==5 and q['zero_policies']==2
 for r in metrics:assert r['result']['control_bytes']>0 and r['result']['size_limit']==64 and r['result']['zero_policies']==2
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256']) and r['external_inputs']
 assert {'assessor/language_portability/resize/resize.c','labs/language_portability/resize/resize.h','tools/verify_portability_resize.py'}<=set(d['input_sha256'])
 rows=d['executions'];mutants=[r for r in rows if 'portability-resize-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==113 and len(mutants)==24 and all(r['exit_code']==1 for r in mutants) and len(learners)==4 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 for tool in ['nm','objdump']:assert len([r for r in rows if r['command'][0]==tool and r['command'][-1].endswith('/resize.c.o')])==6
 assert len(c['profiles'])==4 and {(r['compiler'],r['optimization']) for r in c['profiles']}=={(a,b) for a in ['gcc','clang'] for b in ['-O0','-O3']} and len(c['executions'])==98
 for p in c['profiles']:
  assert p['nonzero_realloc_modes']==['c11','c17'] and p['temporary_modes']==['c11','c17'] and p['earlier_features']==[2,4]
  ex=[c['executions'][i] for i in p['executions']];assert len(ex)==24 and all(r['exit_code']==0 for r in ex)
  assert len([r for r in ex if '-dM' in r['command']])==2
  assert len([r for r in ex if r['command'][0]=='ldd'])==2
  assert len([r for r in ex if r['markers']==['LP_REALLOC nonzero-only failure-preserves grow-shrink explicit-release']])==2
 assert all(r['exit_code']==0 for r in c['executions'])
 assert c['compiler_dependencies'] and any(p.endswith('/stdlib.h') for r in c['compiler_dependencies'] for p in r['paths'])
 assert len(c['observations'])==8 and all(not r['portable_equality_expected'] and r['output'].startswith(('temporary_address_equal=0 ','temporary_address_equal=1 ')) and 'returned_value=42' in r['output'] for r in c['observations'])
 assert c['synthetic_applicability']==dict(introduced=[True,False],correction_applies=[True,False])
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and e['input_sha256'] and e['exported_artifacts'] and not e['private_sources_copied']
 assert e['project']=='PR-T05-03' and e['original_family']=='legacy-assessment' and e['explicit_actual_providers']==['assessor/language_portability/resize/resize.c']
 assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed' and len(e['executions'])==8
 failed=[r for r in e['executions'] if r['exit_code']!=0];assert len(failed)==2 and {r['required_marker'] for r in failed}=={'LP_CHECK','Supply the explicit completed resize implementation'}
 assert all(r['exit_code']==r['expected_exit']==0 or r['expected_exit'] is None and r['exit_code'] not in [None,0] for r in e['executions'])
 if files:
  bindings(d);bindings(c)
  for r in mutants+learners:
   text=(ROOT/r['log']).read_text(encoding='utf8');assert 'LP_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
  for r in ctests:
   text=(ROOT/r['log']).read_text(encoding='utf8');assert '100% tests passed' in text and all('LP '+a+' validated' in text for a in ANCHORS)
  for r in d['generated_artifacts']:
   if r['path'].endswith('compile_commands.json'):assert all('-std=c17' in q['command'] for q in json.loads((ROOT/r['path']).read_text()))
  for r in c['executions']:
   text=(ROOT/r['log']).read_text(encoding='utf8');assert all(m in text for m in r['markers'])
   if '-dM' in r['command']:assert ('__STDC_VERSION__ 201710L' if '-std=c17' in r['command'] else '__STDC_VERSION__ 201112L') in text
  assert any('/libc.so.' in p for p in c['external_input_sha256'])
  for p,h in e['input_sha256'].items():assert sha(ROOT/p)==h,'stale export '+p
  for p,h in e['exported_artifacts'].items():assert e['input_sha256'][p]==h and 'assessor' not in p.split('/')
  for r in e['executions']:
   assert sha(ROOT/r['log'])==r['log_sha256']
   if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
  gate=json.loads((ROOT/BASE/'resize_research_gate.json').read_text());assert gate['status']=='passage-inspected-author-only'
  for p,h in {**gate['input_sha256'],**gate['external_source_sha256']}.items():assert sha(ROOT/p)==h,'stale inspection '+p
  for r in json.loads((ROOT/BASE/'resize_resources.json').read_text())['resources']:
   if 'local_sha256' in r['inspection']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
 return True

def selftest(d,companion,e):
 edits=[lambda a,b,c:a.update(passed=False),lambda a,b,c:a['profiles'].pop(),lambda a,b,c:a['profiles'][0]['controls'].pop(),lambda a,b,c:a['learner_stub_rejections'].pop(),lambda a,b,c:a['observations'][0]['result'].update(histories=1),lambda a,b,c:a['observations'][0]['result'].update(zero_policies=1),lambda a,b,c:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b,c:a.update(input_sha256={}),lambda a,b,c:a.update(learner_mastery='passed'),lambda a,b,c:a['compiler_dependency_closures'].pop(),lambda a,b,c:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b,c:a.update(source_stable=False),lambda a,b,c:b['profiles'].pop(),lambda a,b,c:b['profiles'][0].update(nonzero_realloc_modes=['c17']),lambda a,b,c:b['profiles'][0].update(earlier_features=[]),lambda a,b,c:b.update(physical_validation='passed'),lambda a,b,c:b['observations'][0].update(portable_equality_expected=True),lambda a,b,c:b.update(compiler_dependencies=[]),lambda a,b,c:b['synthetic_applicability'].update(correction_applies=[True,True]),lambda a,b,c:c.update(private_sources_copied=True),lambda a,b,c:c['executions'].pop(),lambda a,b,c:c.update(exported_artifacts={}),lambda a,b,c:c.update(learner_mastery='passed'),lambda a,b,c:c.update(explicit_actual_providers=[])]
 for edit in edits:
  a,b,c=copy.deepcopy(d),copy.deepcopy(companion),copy.deepcopy(e);edit(a,b,c)
  try:check(a,b,c,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported correction/ownership evidence')
 return len(edits)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d,c,e=[json.loads((ROOT/p).read_text()) for p in [REPORT,COMPANION,EXPORT]];check(d,c,e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,c,e) if a.self_test else 0,current_reports={p:sha(ROOT/p) for p in [REPORT,COMPANION,EXPORT]})))
