"""Require actual schema associations, type boundaries and strict closure."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
BASE='research/v5/expansion/legacy_portability_followthrough';REPORT=BASE+'/schema_validation.json';COMPANION=BASE+'/schema_companion_validation.json';EXPORT=BASE+'/schema_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']};ANCHORS={'wire','identity','boundary'};TRACE='fb699ac7f86c15c0a2da6062f5561146537710e0ba4e99e73a6183da474ec037'
def check(d,c,e,files=True):
 base(d);base(c);assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
 for r in d['profiles']:assert r['passed'] and set(r['anchors'])==ANCHORS and len(r['controls'])==3 and {x['test'] for x in r['controls']}=={'portability.mutation.wire.1','portability.mutation.identity.2','portability.mutation.boundary.3'} and all(x['semantic_rejection'] for x in r['controls'])
 assert len(d['learner_stub_rejections'])==3 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
 oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics'];assert len(oracle)==len(metrics)==6
 for r in oracle:assert r['result']==dict(passed=True,cases=1024,trace_sha256=TRACE)
 for r in metrics:assert r['result']['record_bytes']==12 and r['result']['fields_bytes']==12 and r['result']['carrier_bits']==16 and r['result']['allocations']==0
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 for r in d['compiler_dependency_closures']:
  assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256']) and r['external_inputs']
  assert len([v for v in r['generated_link_inputs'].values() if 'recipe_contract' in v])==5
 rows=d['executions'];mutants=[r for r in rows if 'portability-schema-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==106 and len(mutants)==18 and all(r['exit_code']==1 for r in mutants) and len(learners)==3 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 assert len(c['executions'])==239 and all(r['exit_code']==r['expected_exit'] for r in c['executions'])
 assert len(c['variants'])==8 and {r['kind'] for r in c['variants']}=={'ordinary','fast','none','raw','framed','datagram','reserved-v2','semantic'}
 v={r['kind']:r['manifest'] for r in c['variants']}
 for key in ['wire_identity','abi_identity']:assert v['ordinary'][key]==v['fast'][key]==v['reserved-v2'][key]
 assert len({v[n]['build_identity'] for n in ['ordinary','fast','reserved-v2']})==3
 assert v['raw']['wire_identity']!=v['ordinary']['wire_identity'] and v['raw']['abi_identity']!=v['ordinary']['abi_identity'] and v['ordinary']['enabled']==['crc','framed']
 for r in c['variants']:assert r['expected_masks']==dict(tag=7,length=248,flags=768)
 assert len(c['feature_matrices'])==4 and {(r['compiler'],r['features']) for r in c['feature_matrices']}=={(a,b) for a in ['gcc','clang'] for b in [3,4]}
 for r in c['feature_matrices']:assert r['admitted']==([0,1,2,6] if r['features']==3 else [0,1,2,6,8]) and set(r['rejected'])==set(range(2**r['features']))-set(r['admitted'])
 assert set(c['admission_rejections'])=={'overlap','duplicate','zero-width','wide','negative','count','name','reserved-v1','missing','semantic','feature','unknown-key'}
 assert len(c['python_versions'])==2 and {r.split()[1].split('.')[0]+'.'+r.split()[1].split('.')[1] for r in c['python_versions']}=={'3.12','3.14'}
 for key in ['reproducible_clean_paths','internal_policy_identity_preserved','old_consumer_executed','incompatible_consumer_rejected','override_rejected','shared_wrong_independent_rejected','mask_mutant_rejected']:assert c[key]
 assert c['shared_wrong_roundtrips']==1024
 assert len(c['rebuilds'])==2
 for r in c['rebuilds']:assert r['initial_steps']==r['changed_steps']==(3 if r['kind']=='external' else 2) and r['unchanged_steps']==0 and r['diff']
 expected={a+'-'+b for a in ['gcc','clang'] for b in ['O0','O3']};assert len(c['strict_targets'])==4 and {r['profile'] for r in c['strict_targets']}==expected
 for r in c['strict_targets']:assert r['executed'] and r['undefined']==r['needed']==[] and not r['component_library_calls']
 assert len(c['compiler_dependencies'])==44 and len([r for r in c['executions'] if r['command'][0]=='ldd' and '--version' not in r['command']])==24
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and e['input_sha256'] and e['exported_artifacts'] and not e['private_sources_copied'] and e['project']=='PR-T06-03'
 assert e['explicit_actual_providers']==['assessor/language_portability/schema/schema.c','assessor/language_portability/schema/generate.py'] and e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed'
 assert len(e['executions'])==8;failed=[r for r in e['executions'] if r['exit_code']!=0];assert len(failed)==2 and {r['required_marker'] for r in failed}=={'LP_CHECK','Supply the explicit completed schema implementation'}
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
     if 'producer_source' in v:assert all(p in d['input_sha256'] for p in [v['producer_source'],v['input_source'],v['recipe_contract']])
  for p,h in e['input_sha256'].items():assert sha(ROOT/p)==h,'stale export '+p
  for p,h in e['exported_artifacts'].items():assert e['input_sha256'][p]==h and 'assessor' not in p.split('/')
  for r in e['executions']:
   assert sha(ROOT/r['log'])==r['log_sha256']
   if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text()
  gate=json.loads((ROOT/BASE/'schema_research_gate.json').read_text());assert gate['status']=='passage-inspected-author-only'
  for p,h in {**gate['input_sha256'],**gate['external_source_sha256']}.items():assert sha(ROOT/p)==h
  for r in json.loads((ROOT/BASE/'schema_resources.json').read_text())['resources']:
   if 'local_sha256' in r['inspection']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
 return True
def selftest(d,c,e):
 changes=[lambda a,b,c:a.update(passed=False),lambda a,b,c:a['profiles'].pop(),lambda a,b,c:a['profiles'][0]['controls'].pop(),lambda a,b,c:a['learner_stub_rejections'].pop(),lambda a,b,c:a['observations'][0]['result'].update(cases=1),lambda a,b,c:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b,c:a.update(input_sha256={}),lambda a,b,c:a.update(learner_mastery='passed'),lambda a,b,c:a['compiler_dependency_closures'].pop(),lambda a,b,c:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b,c:a.update(source_stable=False),lambda a,b,c:a['compiler_dependency_closures'][0].update(generated_link_inputs={}),lambda a,b,c:b['variants'].pop(),lambda a,b,c:b['variants'][1]['manifest'].update(wire_identity='wrong'),lambda a,b,c:b['feature_matrices'][0].update(admitted=[0]),lambda a,b,c:b['admission_rejections'].pop(),lambda a,b,c:b.update(python_versions=['Python 3.14.4','Python 3.14.4']),lambda a,b,c:b.update(old_consumer_executed=False),lambda a,b,c:b.update(shared_wrong_independent_rejected=False),lambda a,b,c:b.update(shared_wrong_roundtrips=1),lambda a,b,c:b['rebuilds'][0].update(changed_steps=0),lambda a,b,c:b['strict_targets'][0].update(executed=False),lambda a,b,c:b['strict_targets'][0].update(undefined=['memcpy']),lambda a,b,c:b['strict_targets'][0].update(component_library_calls=True),lambda a,b,c:b['compiler_dependencies'].pop(),lambda a,b,c:c.update(private_sources_copied=True),lambda a,b,c:c['executions'].pop(),lambda a,b,c:c.update(exported_artifacts={}),lambda a,b,c:c.update(learner_mastery='passed'),lambda a,b,c:c.update(explicit_actual_providers=[])]
 for change in changes:
  a,b,f=copy.deepcopy(d),copy.deepcopy(c),copy.deepcopy(e);change(a,b,f)
  try:check(a,b,f,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported schema evidence')
 return len(changes)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d,c,e=[json.loads((ROOT/q).read_text()) for q in [REPORT,COMPANION,EXPORT]];check(d,c,e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,c,e) if a.self_test else 0,current_reports={q:sha(ROOT/q) for q in [REPORT,COMPANION,EXPORT]})))
