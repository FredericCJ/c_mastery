"""Source-bound gate for exact prototype/adapter ordinary and delayed tasks."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
BASE='research/v5/expansion/legacy_portability_followthrough'
REPORT=BASE+'/migration_validation.json';COMPANION=BASE+'/migration_companion_validation.json';EXPORT=BASE+'/migration_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
ANCHORS={'sum','adapter','reverse','stable'}
CONTROL={1:'sum',2:'adapter',3:'reverse',4:'stable'}
TRACE='b9bb81e06fadd130bff73370c1a6dd0340f052765c3875b84cc829cc77b398fd'

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
  q=r['result'];assert q['passed'] and q['trace_sha256']==TRACE and q['sort_cases']==39062 and q['sum_cases']==65618 and q['max_length']==6 and q['orders']==2 and q['keys']==5
 for r in metrics:assert r['result']['record_bytes']>0 and r['result']['int_bits']==32 and r['result']['capacity']==16
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256']) and r['external_inputs']
 assert {'assessor/language_portability/migration/migration.c','labs/language_portability/migration/migration.h','tools/verify_portability_migration.py'}<=set(d['input_sha256'])
 rows=d['executions'];mutants=[r for r in rows if 'portability-migration-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==113 and len(mutants)==24 and all(r['exit_code']==1 for r in mutants) and len(learners)==4 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 for tool in ['nm','objdump']:assert len([r for r in rows if r['command'][0]==tool and r['command'][-1].endswith('/migration.c.o')])==6
 assert len(c['profiles'])==4 and {(r['compiler'],r['optimization']) for r in c['profiles']}=={(a,b) for a in ['gcc','clang'] for b in ['-O0','-O3']} and len(c['executions'])==106
 for p in c['profiles']:
  assert p['legacy_cases']==65618 and p['c17_float_promoted'] and p['c23_empty_list_rejected'] and p['mismatch_never_executed']
  ex=[c['executions'][i] for i in p['executions']];old=[r for r in ex if '-c' in r['command'] and '-std=c90' in r['command'] and not r['expected_rejection']]
  assert len(old)==2 and {r['command'][-3].split('/')[-1] for r in old}=={'old_sum.c','old_consumer.c'}
  assert len([r for r in ex if r['markers']==['LP_OLD_MODERN 65618']])==1
  assert len([r for r in ex if 'LP_CASE=3' in ' '.join(r['command'])])==1
  assert not any(r['command'][0].endswith('cast.o') for r in ex)
  assert len([r for r in ex if r['expected_rejection']])==(5 if p['compiler']=='clang' else 4)
  assert any('-std=c23' in r['command'] and r['expected_rejection'] and r['markers']==['lp_promoted','argument'] for r in ex)
  assert any('-DLP_CASE=1' in r['command'] and r['expected_rejection'] and r['markers']==['lp_sum','conflicting'] for r in ex)
  assert len([r for r in ex if r['command'][0]=='nm'])==4 and len([r for r in ex if r['command'][0]=='objdump'])==4
 for r in c['executions']:
  assert (r['exit_code'] not in [None,0] if r['expected_rejection'] else r['exit_code']==0)
  cmd=r['command']
  if '-pedantic' in cmd:assert cmd[0]=='clang' and ('-std=c90' in cmd or '-std=c17' in cmd and any(x.endswith('promotion_consumer.c') for x in cmd) and '-DLP_PROTOTYPE' not in cmd)
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and e['input_sha256'] and e['exported_artifacts'] and not e['private_sources_copied']
 assert e['project']=='PR-T05-01' and e['original_family']=='legacy-assessment' and e['explicit_actual_providers']==['assessor/language_portability/migration/migration.c']
 assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed' and len(e['executions'])==8
 failed=[r for r in e['executions'] if r['exit_code']!=0];assert len(failed)==2 and {r['required_marker'] for r in failed}=={'LP_CHECK','Supply the explicit completed migration implementation'}
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
  gate=json.loads((ROOT/BASE/'migration_research_gate.json').read_text());assert gate['status']=='passage-inspected-author-only'
  for p,h in {**gate['input_sha256'],**gate['external_source_sha256']}.items():assert sha(ROOT/p)==h,'stale inspection '+p
  for r in json.loads((ROOT/BASE/'migration_resources.json').read_text())['resources']:
   if 'local_sha256' in r['inspection']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
 return True

def selftest(d,companion,e):
 edits=[lambda a,b,c:a.update(passed=False),lambda a,b,c:a['profiles'].pop(),lambda a,b,c:a['profiles'][0]['controls'].pop(),lambda a,b,c:a['learner_stub_rejections'].pop(),lambda a,b,c:a['observations'][0]['result'].update(sort_cases=1),lambda a,b,c:a['observations'][0]['result'].update(sum_cases=1),lambda a,b,c:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b,c:a.update(input_sha256={}),lambda a,b,c:a.update(learner_mastery='passed'),lambda a,b,c:a['compiler_dependency_closures'].pop(),lambda a,b,c:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b,c:a.update(source_stable=False),lambda a,b,c:b['profiles'].pop(),lambda a,b,c:b['profiles'][0].update(mismatch_never_executed=False),lambda a,b,c:b['profiles'][0].update(legacy_cases=1),lambda a,b,c:b.update(physical_validation='passed'),lambda a,b,c:next(r for r in b['executions'] if r['expected_rejection']).update(exit_code=0),lambda a,b,c:next(r for r in b['executions'] if r['markers']==['LP_OLD_MODERN 65618']).update(markers=[]),lambda a,b,c:c.update(private_sources_copied=True),lambda a,b,c:c['executions'].pop(),lambda a,b,c:c.update(exported_artifacts={}),lambda a,b,c:c.update(learner_mastery='passed'),lambda a,b,c:c.update(original_family='replacement'),lambda a,b,c:c.update(explicit_actual_providers=[])]
 for edit in edits:
  a,b,c=copy.deepcopy(d),copy.deepcopy(companion),copy.deepcopy(e);edit(a,b,c)
  try:check(a,b,c,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported prototype evidence')
 return len(edits)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d,c,e=[json.loads((ROOT/p).read_text()) for p in [REPORT,COMPANION,EXPORT]];check(d,c,e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,c,e) if a.self_test else 0,current_reports={p:sha(ROOT/p) for p in [REPORT,COMPANION,EXPORT]})))
