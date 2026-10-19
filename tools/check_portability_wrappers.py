"""Evidence gate for defined evaluation counts and stage-specific failures."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
BASE='research/v5/expansion/legacy_portability_followthrough';REPORT=BASE+'/wrappers_validation.json';COMPANION=BASE+'/wrappers_companion_validation.json';EXPORT=BASE+'/wrappers_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']};ANCHORS={'trace','minimum','statement'};TRACE='6a272e240b761a084678d36e833590fa0584615b5cf730289a5b568979827a40'
def check(d,c,e,files=True):
 base(d);base(c);assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
 for r in d['profiles']:assert r['passed'] and set(r['anchors'])==ANCHORS and len(r['controls'])==3 and {x['test'] for x in r['controls']}=={'portability.mutation.trace.1','portability.mutation.minimum.2','portability.mutation.statement.3'} and all(x['semantic_rejection'] for x in r['controls'])
 assert len(d['learner_stub_rejections'])==3 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
 oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics'];assert len(oracle)==len(metrics)==6
 for r in oracle:assert r['result']==dict(passed=True,cases=16,trace_sha256=TRACE)
 for r in metrics:assert r['result']['observation_bytes']>=48 and r['result']['text_capacity']==32 and r['result']['allocations']==0
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256']) and r['external_inputs']
 rows=d['executions'];mutants=[r for r in rows if 'portability-wrappers-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==106 and len(mutants)==18 and all(r['exit_code']==1 for r in mutants) and len(learners)==3 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 assert len(c['profiles'])==4 and {r['profile'] for r in c['profiles']}=={a+'-'+b for a in ['gcc','clang'] for b in ['O0','O3']} and len(c['executions'])==87
 for p in c['profiles']:
  assert p['passed'] and p['strict_vaopt_rejected'] and len(p['executions'])==21 and set(p['observed'])==set(map(str,range(1,9)))
  for n in ['1','6','7']:assert p['observed'][n]['mode']=='c17' and p['observed'][n]['preprocess_exit']==0 and p['observed'][n]['compile_exit']==1
  assert p['observed']['2']['preprocess_exit']==1 and p['observed']['2']['mode']=='c17'
  assert p['observed']['3']['stdout']=='1 1 1 1 3\n' and p['observed']['3']['mode']=='c23'
  assert ''.join(p['observed']['4']['preprocessed'].split())=='target(0)target(0,)target(0,)target(0,)' and p['observed']['4']['mode']=='gnu17' and 'stdout' not in p['observed']['4']
  for n in ['5','8']:assert p['observed'][n]['mode']=='gnu17' and p['observed'][n]['stdout']==''
 assert len(c['compiler_dependencies'])==12 and any(p.endswith('/stdio.h') for r in c['compiler_dependencies'] for p in r['paths'])
 assert len([r for r in c['executions'] if r['command'][0]=='ldd' and '--version' not in r['command']])==12 and all(r['exit_code']==r['expected_exit'] for r in c['executions'])
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and e['input_sha256'] and e['exported_artifacts'] and not e['private_sources_copied'] and e['project']=='PR-T06-01'
 assert e['explicit_actual_providers']==['assessor/language_portability/wrappers/wrappers.c'] and e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed'
 assert len(e['executions'])==8;failed=[r for r in e['executions'] if r['exit_code']!=0];assert len(failed)==2 and {r['required_marker'] for r in failed}=={'LP_CHECK','Supply the explicit completed wrappers implementation'}
 assert all(r['exit_code']==r['expected_exit']==0 or r['expected_exit'] is None and r['exit_code'] not in [None,0] for r in e['executions'])
 if files:
  bindings(d);bindings(c)
  for r in mutants+learners:
   text=(ROOT/r['log']).read_text();assert 'LP_CHECK' in text and not any(s in text for s in ['AddressSanitizer','runtime error:','ThreadSanitizer'])
  for r in ctests:
   text=(ROOT/r['log']).read_text();assert '100% tests passed' in text and all('LP '+a+' validated' in text for a in ANCHORS)
  for r in d['generated_artifacts']:
   if r['path'].endswith('compile_commands.json'):assert all('-std=c17' in q['command'] for q in json.loads((ROOT/r['path']).read_text()))
  for r in c['executions']:
   if r['exit_code']:assert 'error:' in (ROOT/r['log']).read_text()
  for p,h in e['input_sha256'].items():assert sha(ROOT/p)==h,'stale export '+p
  for p,h in e['exported_artifacts'].items():assert e['input_sha256'][p]==h and 'assessor' not in p.split('/')
  for r in e['executions']:
   assert sha(ROOT/r['log'])==r['log_sha256']
   if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text()
  gate=json.loads((ROOT/BASE/'wrappers_research_gate.json').read_text());assert gate['status']=='passage-inspected-author-only'
  for p,h in {**gate['input_sha256'],**gate['external_source_sha256']}.items():assert sha(ROOT/p)==h
  for r in json.loads((ROOT/BASE/'wrappers_resources.json').read_text())['resources']:
   if 'local_sha256' in r['inspection']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
 return True
def selftest(d,c,e):
 changes=[lambda a,b,c:a.update(passed=False),lambda a,b,c:a['profiles'].pop(),lambda a,b,c:a['profiles'][0]['controls'].pop(),lambda a,b,c:a['learner_stub_rejections'].pop(),lambda a,b,c:a['observations'][0]['result'].update(cases=1),lambda a,b,c:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b,c:a.update(input_sha256={}),lambda a,b,c:a.update(learner_mastery='passed'),lambda a,b,c:a['compiler_dependency_closures'].pop(),lambda a,b,c:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b,c:a.update(source_stable=False),lambda a,b,c:b['profiles'].pop(),lambda a,b,c:b['profiles'][0]['observed']['3'].update(stdout='wrong'),lambda a,b,c:b['profiles'][0]['observed']['2'].update(preprocess_exit=0),lambda a,b,c:b['profiles'][0]['observed']['6'].update(compile_exit=0),lambda a,b,c:b['profiles'][0]['observed']['4'].update(stdout='executed'),lambda a,b,c:b['profiles'][0].update(strict_vaopt_rejected=False),lambda a,b,c:b['profiles'][0]['observed']['8'].update(mode='c17'),lambda a,b,c:b['compiler_dependencies'].pop(),lambda a,b,c:c.update(private_sources_copied=True),lambda a,b,c:c['executions'].pop(),lambda a,b,c:c.update(exported_artifacts={}),lambda a,b,c:c.update(learner_mastery='passed'),lambda a,b,c:c.update(explicit_actual_providers=[])]
 for change in changes:
  a,b,f=copy.deepcopy(d),copy.deepcopy(c),copy.deepcopy(e);change(a,b,f)
  try:check(a,b,f,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported wrapper evidence')
 return len(changes)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d,c,e=[json.loads((ROOT/q).read_text()) for q in [REPORT,COMPANION,EXPORT]];check(d,c,e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,c,e) if a.self_test else 0,current_reports={q:sha(ROOT/q) for q in [REPORT,COMPANION,EXPORT]})))
