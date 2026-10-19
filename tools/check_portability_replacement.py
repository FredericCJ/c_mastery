"""Source-bound gate for declaration probes and explicit C17 transfer."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
BASE='research/v5/expansion/legacy_portability_followthrough'
REPORT=BASE+'/replacement_validation.json';COMPANION=BASE+'/replacement_companion_validation.json';EXPORT=BASE+'/replacement_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
ANCHORS={'failure','order','transfer'}
CONTROL={1:'failure',2:'order',3:'transfer'}
TRACE='cb69b9069c532e6dbd83bf443c69dafcb38f3e5cbc0994e27d6d500540189eb0'

def check(d,c,e,files=True):
 base(d);base(c)
 assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
 for r in d['profiles']:
  assert r['passed'] and set(r['anchors'])==ANCHORS and len(r['controls'])==3
  assert {v['test'] for v in r['controls']}=={f'portability.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(v['semantic_rejection'] for v in r['controls'])
 assert len(d['learner_stub_rejections'])==3 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
 oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
 assert len(oracle)==6 and len(metrics)==6 and {r['profile'] for r in oracle}==PROFILES and {r['profile'] for r in metrics}==PROFILES
 for r in oracle:
  q=r['result'];assert q['passed'] and q['trace_sha256']==TRACE and q['cases']==16
 for r in metrics:assert r['result']['owner_bytes']>0 and r['result']['resource_bytes']==24 and r['result']['maximum_acquisitions']==2
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256']) and r['external_inputs']
 assert {'assessor/language_portability/replacement/replacement.c','labs/language_portability/replacement/replacement.h','tools/verify_portability_replacement.py'}<=set(d['input_sha256'])
 rows=d['executions'];mutants=[r for r in rows if 'portability-replacement-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==106 and len(mutants)==18 and all(r['exit_code']==1 for r in mutants) and len(learners)==3 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 for tool in ['nm','objdump']:assert len([r for r in rows if r['command'][0]==tool and r['command'][-1].endswith('/replacement.c.o')])==6
 assert len(c['profiles'])==6 and {r['profile'] for r in c['profiles']}==PROFILES and len(c['executions'])==174
 for p in c['profiles']:
  assert p['variants']==['baseline','extension'] and p['strict_rejection'] and p['extension_free_removal'] and p['live_only']
  ex=[c['executions'][i] for i in p['executions']];assert len(ex)==28 and len([r for r in ex if r['expected_exit'] is None])==1
  assert len([r for r in ex if r['command'][0]=='ldd'])==2
 assert len(c['observations'])==12 and {(r['profile'],r['variant']) for r in c['observations']}=={(p,v) for p in PROFILES for v in ['baseline','extension']}
 for r in c['observations']:assert r['result']['passed'] and r['result']['cases']==16 and r['result']['trace_sha256']==TRACE
 assert c['expired_case_executed'] is False and c['synthetic_fixture_ready']==[True,False,False]
 for r in c['executions']:
  assert r['exit_code']==r['expected_exit'] if r['expected_exit'] is not None else r['exit_code'] not in [0,None]
  assert not any('expired_case.c.txt' in t for t in r['command'])
 assert len([r for r in c['executions'] if r['expected_exit'] is None])==6
 assert c['compiler_dependencies'] and any(p.endswith('/stdint.h') for r in c['compiler_dependencies'] for p in r['paths'])
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and e['input_sha256'] and e['exported_artifacts'] and not e['private_sources_copied']
 assert e['project']=='PR-T05-05' and e['original_family']=='legacy-assessment' and e['explicit_actual_providers']==['assessor/language_portability/replacement/replacement.c']
 assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed' and len(e['executions'])==8
 failed=[r for r in e['executions'] if r['exit_code']!=0];assert len(failed)==2 and {r['required_marker'] for r in failed}=={'LP_CHECK','Supply the explicit completed replacement implementation'}
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
   if r['expected_exit'] is None:assert 'error:' in text and 'internal compiler error' not in text
  assert any('/libc.so.' in p for p in c['external_input_sha256'])
  for p,h in e['input_sha256'].items():assert sha(ROOT/p)==h,'stale export '+p
  for p,h in e['exported_artifacts'].items():assert e['input_sha256'][p]==h and 'assessor' not in p.split('/')
  for r in e['executions']:
   assert sha(ROOT/r['log'])==r['log_sha256']
   if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
  gate=json.loads((ROOT/BASE/'replacement_research_gate.json').read_text());assert gate['status']=='passage-inspected-author-only'
  for p,h in {**gate['input_sha256'],**gate['external_source_sha256']}.items():assert sha(ROOT/p)==h,'stale inspection '+p
  for r in json.loads((ROOT/BASE/'replacement_resources.json').read_text())['resources']:
   if 'local_sha256' in r['inspection']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
 return True

def selftest(d,companion,e):
 edits=[lambda a,b,c:a.update(passed=False),lambda a,b,c:a['profiles'].pop(),lambda a,b,c:a['profiles'][0]['controls'].pop(),lambda a,b,c:a['learner_stub_rejections'].pop(),lambda a,b,c:a['observations'][0]['result'].update(cases=1),lambda a,b,c:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b,c:a.update(input_sha256={}),lambda a,b,c:a.update(learner_mastery='passed'),lambda a,b,c:a['compiler_dependency_closures'].pop(),lambda a,b,c:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b,c:a.update(source_stable=False),lambda a,b,c:b['profiles'].pop(),lambda a,b,c:b['profiles'][0].update(variants=['baseline']),lambda a,b,c:b['profiles'][0].update(extension_free_removal=False),lambda a,b,c:b.update(physical_validation='passed'),lambda a,b,c:b.update(expired_case_executed=True),lambda a,b,c:b.update(compiler_dependencies=[]),lambda a,b,c:b['observations'].pop(),lambda a,b,c:b.update(synthetic_fixture_ready=[True,True,False]),lambda a,b,c:c.update(private_sources_copied=True),lambda a,b,c:c['executions'].pop(),lambda a,b,c:c.update(exported_artifacts={}),lambda a,b,c:c.update(learner_mastery='passed'),lambda a,b,c:c.update(explicit_actual_providers=[])]
 for edit in edits:
  a,b,c=copy.deepcopy(d),copy.deepcopy(companion),copy.deepcopy(e);edit(a,b,c)
  try:check(a,b,c,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported declaration evidence')
 return len(edits)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d,c,e=[json.loads((ROOT/p).read_text()) for p in [REPORT,COMPANION,EXPORT]];check(d,c,e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,c,e) if a.self_test else 0,current_reports={p:sha(ROOT/p) for p in [REPORT,COMPANION,EXPORT]})))
