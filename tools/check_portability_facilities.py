"""Source-bound gate for declaration probes and explicit C17 transfer."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
BASE='research/v5/expansion/legacy_portability_followthrough'
REPORT=BASE+'/facilities_validation.json';COMPANION=BASE+'/facilities_companion_validation.json';EXPORT=BASE+'/facilities_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
ANCHORS={'add','mixed','bits','divide','select'}
CONTROL={1:'add',2:'mixed',3:'bits',4:'divide',5:'select'}
TRACE='76e7f792a5d442af558d8e2aacd571021eb2dde47a661e7746f562b3c59f8664'

def check(d,c,e,files=True):
 base(d);base(c)
 assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
 for r in d['profiles']:
  assert r['passed'] and set(r['anchors'])==ANCHORS and len(r['controls'])==5
  assert {v['test'] for v in r['controls']}=={f'portability.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(v['semantic_rejection'] for v in r['controls'])
 assert len(d['learner_stub_rejections'])==5 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
 oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
 assert len(oracle)==6 and len(metrics)==6 and {r['profile'] for r in oracle}==PROFILES and {r['profile'] for r in metrics}==PROFILES
 for r in oracle:
  q=r['result'];assert q['passed'] and q['trace_sha256']==TRACE and q['cases']==150657 and q['counts']==dict(a=65657,m=2948,b=65569,d=16384,s=99)
 for r in metrics:assert r['result']==dict(division_steps=64,bit_width=32,observation_bits=5)
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256']) and r['external_inputs']
 assert {'assessor/language_portability/facilities/facilities.c','labs/language_portability/facilities/facilities.h','tools/verify_portability_facilities.py'}<=set(d['input_sha256'])
 rows=d['executions'];mutants=[r for r in rows if 'portability-facilities-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==120 and len(mutants)==30 and all(r['exit_code']==1 for r in mutants) and len(learners)==5 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 for tool in ['nm','objdump']:assert len([r for r in rows if r['command'][0]==tool and r['command'][-1].endswith('/facilities.c.o')])==6
 assert len(c['profiles'])==4 and {(r['compiler'],r['optimization']) for r in c['profiles']}=={(a,b) for a in ['gcc','clang'] for b in ['-O0','-O3']} and len(c['executions'])==482
 for p in c['profiles']:
  assert p['staged_features']==[1,2,3,4,5,6,7,9] and p['strict_extension_modes']==['c17','c23'] and p['extension_modes']==['gnu17','gnu23'] and p['missing_header_rejected']
  ex=[c['executions'][i] for i in p['executions']];assert len(ex)==89 and len([r for r in ex if r['expected_exit'] is None])==4
 assert len(c['observations'])==8 and {(r['compiler'],r['optimization'],r['enhanced']) for r in c['observations']}=={(a,b,e) for a in ['gcc','clang'] for b in ['-O0','-O3'] for e in [False,True]}
 for r in c['observations']:assert r['result']['passed'] and r['result']['trace_sha256']==TRACE and r['result']['cases']==150657
 assert len(c['embed_rebuilds'])==4
 for r in c['embed_rebuilds']:
  assert r['source_unchanged'] and [x['rebuilt'] for x in r['events']]==[True,False,True] and [x['output'] for x in r['events']]==['007fff','007fff','007ffe']
  assert r['events'][0]['compile_command']==r['events'][2]['compile_command'] and r['events'][1]['compile_command'] is None
 assert len(c['compiler_upgrade'])==2 and [r['label'] for r in c['compiler_upgrade']]==['before20','after21']
 assert len({r['executable_sha256'] for r in c['compiler_upgrade']})==2
 for r in c['compiler_upgrade']:assert len(r['executions'])==29 and r['all_selected_observations_unchanged']
 assert len(c['target_profiles'])==3 and {r['profile'] for r in c['target_profiles']}=={'gcc-O0','gcc-O3','clang-O3'}
 for r in c['target_profiles']:assert r['actual_execution'] and r['zero_undefined'] and r['direct_helper_rejected']=='__aeabi_uldivmod' and len(r['executions'])==16
 cross=c['cross_only'];assert cross['compiled'] and cross['linked'] and cross['executed'] is False and cross['observation_mask']==7 and cross['target']=='riscv32-none-elf' and len(cross['executions'])==4
 assert c['synthetic_fixture_usable']==[False,False]
 for r in c['executions']:assert r['exit_code']==r['expected_exit'] if r['expected_exit'] is not None else r['exit_code'] not in [0,None]
 assert len([r for r in c['executions'] if r['expected_exit'] is None])==21
 assert c['compiler_dependencies'] and any(p.endswith('/stdckdint.h') for r in c['compiler_dependencies'] for p in r['paths']) and any(p.endswith('/stdbit.h') for r in c['compiler_dependencies'] for p in r['paths'])
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and e['input_sha256'] and e['exported_artifacts'] and not e['private_sources_copied']
 assert e['project']=='PR-T05-04' and e['original_family']=='legacy-assessment' and e['explicit_actual_providers']==['assessor/language_portability/facilities/facilities.c']
 assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed' and len(e['executions'])==8
 failed=[r for r in e['executions'] if r['exit_code']!=0];assert len(failed)==2 and {r['required_marker'] for r in failed}=={'LP_CHECK','Supply the explicit completed facilities implementation'}
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
   if '-dM' in r['command']:assert '__STDC_VERSION__ 202311L' in text
   if r['expected_exit'] is None:assert ('error:' in text or 'undefined reference' in text) and 'internal compiler error' not in text
  assert any('/libc.so.' in p for p in c['external_input_sha256'])
  for p,h in e['input_sha256'].items():assert sha(ROOT/p)==h,'stale export '+p
  for p,h in e['exported_artifacts'].items():assert e['input_sha256'][p]==h and 'assessor' not in p.split('/')
  for r in e['executions']:
   assert sha(ROOT/r['log'])==r['log_sha256']
   if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
  gate=json.loads((ROOT/BASE/'facilities_research_gate.json').read_text());assert gate['status']=='passage-inspected-author-only'
  for p,h in {**gate['input_sha256'],**gate['external_source_sha256']}.items():assert sha(ROOT/p)==h,'stale inspection '+p
  for r in json.loads((ROOT/BASE/'facilities_resources.json').read_text())['resources']:
   if 'local_sha256' in r['inspection']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
 return True

def selftest(d,companion,e):
 edits=[lambda a,b,c:a.update(passed=False),lambda a,b,c:a['profiles'].pop(),lambda a,b,c:a['profiles'][0]['controls'].pop(),lambda a,b,c:a['learner_stub_rejections'].pop(),lambda a,b,c:a['observations'][0]['result'].update(cases=1),lambda a,b,c:a['observations'][0]['result']['counts'].update(m=1),lambda a,b,c:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b,c:a.update(input_sha256={}),lambda a,b,c:a.update(learner_mastery='passed'),lambda a,b,c:a['compiler_dependency_closures'].pop(),lambda a,b,c:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b,c:a.update(source_stable=False),lambda a,b,c:b['profiles'].pop(),lambda a,b,c:b['profiles'][0].update(staged_features=[1]),lambda a,b,c:b['embed_rebuilds'][0]['events'][2].update(rebuilt=False),lambda a,b,c:b.update(physical_validation='passed'),lambda a,b,c:b['cross_only'].update(executed=True),lambda a,b,c:b.update(compiler_dependencies=[]),lambda a,b,c:b['compiler_upgrade'].pop(),lambda a,b,c:b['target_profiles'][0].update(zero_undefined=False),lambda a,b,c:b['observations'][0]['result'].update(cases=1),lambda a,b,c:b.update(synthetic_fixture_usable=[True,False]),lambda a,b,c:c.update(private_sources_copied=True),lambda a,b,c:c['executions'].pop(),lambda a,b,c:c.update(exported_artifacts={}),lambda a,b,c:c.update(learner_mastery='passed'),lambda a,b,c:c.update(explicit_actual_providers=[])]
 for edit in edits:
  a,b,c=copy.deepcopy(d),copy.deepcopy(companion),copy.deepcopy(e);edit(a,b,c)
  try:check(a,b,c,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported declaration evidence')
 return len(edits)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d,c,e=[json.loads((ROOT/p).read_text()) for p in [REPORT,COMPANION,EXPORT]];check(d,c,e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,c,e) if a.self_test else 0,current_reports={p:sha(ROOT/p) for p in [REPORT,COMPANION,EXPORT]})))
