"""Source-bound gate for exact prototype/adapter ordinary and delayed tasks."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
BASE='research/v5/expansion/legacy_portability_followthrough'
REPORT=BASE+'/context_validation.json';COMPANION=BASE+'/context_companion_validation.json';EXPORT=BASE+'/context_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
ANCHORS={'strict','partial','locale','combining'}
CONTROL={1:'strict',2:'partial',3:'locale',4:'combining'}
TRACE='70e64601b5a0365fa154f3cd97c10af52a1a53b3de6ac8fef3f466a61d74d4c6'

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
  q=r['result'];assert q['passed'] and q['trace_sha256']==TRACE and q['cases']==213637 and q['complete']==61848 and q['invalid']==118636 and q['incomplete']==33153 and q['canonical_equivalence']
 for r in metrics:assert r['result']['utf_bytes']>0 and r['result']['locale_bytes']>0 and r['result']['byte_limit']==64 and r['result']['wchar_bits']==32
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256']) and r['external_inputs']
 assert {'assessor/language_portability/context/context.c','labs/language_portability/context/context.h','tools/verify_portability_context.py'}<=set(d['input_sha256'])
 rows=d['executions'];mutants=[r for r in rows if 'portability-context-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==113 and len(mutants)==24 and all(r['exit_code']==1 for r in mutants) and len(learners)==4 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 for tool in ['nm','objdump']:assert len([r for r in rows if r['command'][0]==tool and r['command'][-1].endswith('/context.c.o')])==6
 assert len(c['profiles'])==4 and {(r['compiler'],r['optimization']) for r in c['profiles']}=={(a,b) for a in ['gcc','clang'] for b in ['-O0','-O3']} and len(c['executions'])==88
 for p in c['profiles']:
  assert p['bool_modes']==4 and p['constexpr_valid']==2 and p['constexpr_constraints']==3 and p['wrong_mode_rejected'] and p['extension_cleanup']
  ex=[c['executions'][i] for i in p['executions']]
  assert len(ex)==21 and len([r for r in ex if r['expected_rejection']])==5
  assert len([r for r in ex if '-dM' in r['command']])==4
  assert len([r for r in ex if r['markers']==['BOOL_MACRO']])==1 and len([r for r in ex if r['markers']==['BOOL_NONMACRO']])==2
  assert len([r for r in ex if r['markers']==['LC_CLEANUP normal21 early21']])==1
  assert len([r for r in ex if '-std=c23' in r['command'] and r['expected_rejection'] and r['markers']==['constexpr']])==3
  assert len([r for r in ex if '-std=c17' in r['command'] and r['expected_rejection'] and r['markers']==['constexpr']])==1
 for r in c['executions']:assert (r['exit_code'] not in [None,0] if r['expected_rejection'] else r['exit_code']==0)
 assert c['compiler_dependencies'] and any(p.endswith('/stdbool.h') for r in c['compiler_dependencies'] for p in r['paths'])
 assert {r['markers'][0] for r in c['executions'][-2:]}=={'LC_LEDGER preserved_history rejected6 deep_copy','LC_LEDGER_CHECK unfinished'}
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and e['input_sha256'] and e['exported_artifacts'] and not e['private_sources_copied']
 assert e['project']=='PR-T05-01' and e['original_family']=='legacy-assessment' and e['explicit_actual_providers']==['assessor/language_portability/context/context.c']
 assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed' and len(e['executions'])==8
 failed=[r for r in e['executions'] if r['exit_code']!=0];assert len(failed)==2 and {r['required_marker'] for r in failed}=={'LP_CHECK','Supply the explicit completed context implementation'}
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
  for p,h in e['input_sha256'].items():assert sha(ROOT/p)==h,'stale export '+p
  for p,h in e['exported_artifacts'].items():assert e['input_sha256'][p]==h and 'assessor' not in p.split('/')
  for r in e['executions']:
   assert sha(ROOT/r['log'])==r['log_sha256']
   if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
  gate=json.loads((ROOT/BASE/'context_research_gate.json').read_text());assert gate['status']=='passage-inspected-author-only'
  for p,h in {**gate['input_sha256'],**gate['external_source_sha256']}.items():assert sha(ROOT/p)==h,'stale inspection '+p
  for r in json.loads((ROOT/BASE/'context_resources.json').read_text())['resources']:
   if 'local_sha256' in r['inspection']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
 return True

def selftest(d,companion,e):
 edits=[lambda a,b,c:a.update(passed=False),lambda a,b,c:a['profiles'].pop(),lambda a,b,c:a['profiles'][0]['controls'].pop(),lambda a,b,c:a['learner_stub_rejections'].pop(),lambda a,b,c:a['observations'][0]['result'].update(cases=1),lambda a,b,c:a['observations'][0]['result'].update(invalid=1),lambda a,b,c:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b,c:a.update(input_sha256={}),lambda a,b,c:a.update(learner_mastery='passed'),lambda a,b,c:a['compiler_dependency_closures'].pop(),lambda a,b,c:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b,c:a.update(source_stable=False),lambda a,b,c:b['profiles'].pop(),lambda a,b,c:b['profiles'][0].update(wrong_mode_rejected=False),lambda a,b,c:b['profiles'][0].update(bool_modes=1),lambda a,b,c:b.update(physical_validation='passed'),lambda a,b,c:next(r for r in b['executions'] if r['expected_rejection']).update(exit_code=0),lambda a,b,c:b.update(compiler_dependencies=[]),lambda a,b,c:c.update(private_sources_copied=True),lambda a,b,c:c['executions'].pop(),lambda a,b,c:c.update(exported_artifacts={}),lambda a,b,c:c.update(learner_mastery='passed'),lambda a,b,c:c.update(original_family='replacement'),lambda a,b,c:c.update(explicit_actual_providers=[])]
 for edit in edits:
  a,b,c=copy.deepcopy(d),copy.deepcopy(companion),copy.deepcopy(e);edit(a,b,c)
  try:check(a,b,c,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported authority/locale evidence')
 return len(edits)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d,c,e=[json.loads((ROOT/p).read_text()) for p in [REPORT,COMPANION,EXPORT]];check(d,c,e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,c,e) if a.self_test else 0,current_reports={p:sha(ROOT/p) for p in [REPORT,COMPANION,EXPORT]})))
