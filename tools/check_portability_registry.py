"""Require actual registry associations, type boundaries and strict closure."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
BASE='research/v5/expansion/legacy_portability_followthrough';REPORT=BASE+'/registry_validation.json';COMPANION=BASE+'/registry_companion_validation.json';EXPORT=BASE+'/registry_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']};ANCHORS={'unknown','association','generic'};TRACE='7f45fbfc111b6f6d38b624a31bce4a5107dc7710e65e72f040a48845f9abb457'
def check(d,c,e,files=True):
 base(d);base(c);assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
 for r in d['profiles']:assert r['passed'] and set(r['anchors'])==ANCHORS and len(r['controls'])==3 and {x['test'] for x in r['controls']}=={'portability.mutation.unknown.1','portability.mutation.association.2','portability.mutation.generic.3'} and all(x['semantic_rejection'] for x in r['controls'])
 assert len(d['learner_stub_rejections'])==3 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
 oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics'];assert len(oracle)==len(metrics)==6
 for r in oracle:assert r['result']==dict(passed=True,cases=73,trace_sha256=TRACE)
 for r in metrics:assert r['result']['entries']==3 and r['result']['pointer_bytes']==8 and r['result']['generic_bytes']>=20 and r['result']['allocations']==0
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 for r in d['compiler_dependency_closures']:
  assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256']) and r['external_inputs']
  if r['profile']!='learner':assert len([v for v in r['generated_link_inputs'].values() if 'canonical_path' in v])==6
 rows=d['executions'];mutants=[r for r in rows if 'portability-registry-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==118 and len(mutants)==18 and all(r['exit_code']==1 for r in mutants) and len(learners)==3 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 assert len(c['executions'])==171 and all(r['exit_code']==r['expected_exit'] for r in c['executions'])
 assert len(c['variants'])==5 and {r['kind'] for r in c['variants']}=={'ordinary','added','reordered','aliases','sparse'}
 v={r['kind']:r for r in c['variants']};assert v['ordinary']['generated']==v['reordered']['generated'] and v['added']['rows']==4 and v['sparse']['maximum_id']==2147483647 and v['sparse']['search_entries']==3 and v['sparse']['direct_index_slots']==2147483648
 assert set(c['admission_rejections'])=={'duplicate-id','duplicate-name','missing-handler','unknown-handler','no-rows','too-many','id-range','alias-collision'} and c['stale_generated_rejected'] and c['synthetic_fixture_checked']
 expected={a+'-'+b for a in ['gcc','clang'] for b in ['O0','O3']};assert len(c['probes'])==4 and {r['profile'] for r in c['probes']}==expected and len(c['strict_targets'])==4 and {r['profile'] for r in c['strict_targets']}==expected
 for r in c['probes']:
  assert r['negative_cases']==[1,2,3,4] and r['no_runtime_selection_branch'] and r['accepted_new_long'] and len(r['executions'])==12
  for function,target in [('generic_int','lg_int'),('explicit_int','lg_int'),('generic_double','lg_double'),('explicit_double','lg_double')]:assert r['assembly_paths'][function][0]==function and r['assembly_paths'][function][-1]==target
 for r in c['strict_targets']:assert r['executed'] and r['undefined']==r['needed']==[] and not r['component_library_calls'] and r['iterations']==32
 assert len(c['compiler_dependencies'])==40 and len([r for r in c['executions'] if r['command'][0]=='ldd' and '--version' not in r['command']])==19
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and e['input_sha256'] and e['exported_artifacts'] and not e['private_sources_copied'] and e['project']=='PR-T06-02'
 assert e['explicit_actual_providers']==['assessor/language_portability/registry/registry.c','assessor/language_portability/registry/generic.c','assessor/language_portability/registry/generic.h'] and e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed'
 assert len(e['executions'])==8;failed=[r for r in e['executions'] if r['exit_code']!=0];assert len(failed)==2 and {r['required_marker'] for r in failed}=={'LP_CHECK','Supply the explicit completed registry implementation'}
 assert all(r['exit_code']==r['expected_exit']==0 or r['expected_exit'] is None and r['exit_code'] not in [None,0] for r in e['executions'])
 if files:
  bindings(d);bindings(c)
  for r in mutants+learners:
   text=(ROOT/r['log']).read_text();assert 'LP_CHECK' in text and not any(s in text for s in ['AddressSanitizer','runtime error:','ThreadSanitizer'])
  for r in ctests:
   text=(ROOT/r['log']).read_text();assert '100% tests passed' in text and all('LP '+a+' validated' in text for a in ANCHORS)
  for r in d['generated_artifacts']:
   if r['path'].endswith('compile_commands.json'):assert all('-std=c17' in q['command'] for q in json.loads((ROOT/r['path']).read_text()))
  for r in d['compiler_dependency_closures']:
   for v in r['generated_link_inputs'].values():
    if 'retained_path' in v:
     assert sha(ROOT/v['retained_path'])==v['sha256']
     if 'canonical_path' in v:assert sha(ROOT/v['canonical_path'])==v['sha256'] and all(p in d['input_sha256'] for p in [v['producer_source'],v['input_source'],v['recipe_contract']])
  for p,h in e['input_sha256'].items():assert sha(ROOT/p)==h,'stale export '+p
  for p,h in e['exported_artifacts'].items():assert e['input_sha256'][p]==h and 'assessor' not in p.split('/')
  for r in e['executions']:
   assert sha(ROOT/r['log'])==r['log_sha256']
   if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text()
  gate=json.loads((ROOT/BASE/'registry_research_gate.json').read_text());assert gate['status']=='passage-inspected-author-only'
  for p,h in {**gate['input_sha256'],**gate['external_source_sha256']}.items():assert sha(ROOT/p)==h
  for r in json.loads((ROOT/BASE/'registry_resources.json').read_text())['resources']:
   if 'local_sha256' in r['inspection']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
 return True
def selftest(d,c,e):
 changes=[lambda a,b,c:a.update(passed=False),lambda a,b,c:a['profiles'].pop(),lambda a,b,c:a['profiles'][0]['controls'].pop(),lambda a,b,c:a['learner_stub_rejections'].pop(),lambda a,b,c:a['observations'][0]['result'].update(cases=1),lambda a,b,c:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b,c:a.update(input_sha256={}),lambda a,b,c:a.update(learner_mastery='passed'),lambda a,b,c:a['compiler_dependency_closures'].pop(),lambda a,b,c:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b,c:a.update(source_stable=False),lambda a,b,c:a['compiler_dependency_closures'][0].update(generated_link_inputs={}),lambda a,b,c:b['variants'].pop(),lambda a,b,c:b['variants'][2].update(generated={}),lambda a,b,c:b['variants'][-1].update(search_entries=2147483648),lambda a,b,c:b['admission_rejections'].pop(),lambda a,b,c:b.update(stale_generated_rejected=False),lambda a,b,c:b['probes'][0].update(no_runtime_selection_branch=False),lambda a,b,c:b['probes'][0].update(accepted_new_long=False),lambda a,b,c:b['strict_targets'][0].update(executed=False),lambda a,b,c:b['strict_targets'][0].update(undefined=['memcpy']),lambda a,b,c:b['strict_targets'][0].update(component_library_calls=True),lambda a,b,c:b['compiler_dependencies'].pop(),lambda a,b,c:c.update(private_sources_copied=True),lambda a,b,c:c['executions'].pop(),lambda a,b,c:c.update(exported_artifacts={}),lambda a,b,c:c.update(learner_mastery='passed'),lambda a,b,c:c.update(explicit_actual_providers=[])]
 for change in changes:
  a,b,f=copy.deepcopy(d),copy.deepcopy(c),copy.deepcopy(e);change(a,b,f)
  try:check(a,b,f,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported registry evidence')
 return len(changes)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d,c,e=[json.loads((ROOT/q).read_text()) for q in [REPORT,COMPANION,EXPORT]];check(d,c,e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,c,e) if a.self_test else 0,current_reports={q:sha(ROOT/q) for q in [REPORT,COMPANION,EXPORT]})))
