"""Reject unsupported result enumeration, adapter, export and target claims."""
import argparse,copy,json,shlex
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
BASE='research/v5/expansion/b02_followthrough'
REPORT=BASE+'/result_validation.json';TARGET=BASE+'/result_target_validation.json';EXPORT=BASE+'/result_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
ANCHORS={'render','construct','owning','wire'}
CONTROL={1:'render',2:'construct',3:'owning',4:'owning',5:'wire',6:'wire'}
TRACE='a38bfe6dcbe25ab10f03cb2e603db106ea13b3d0e4250b62bdeccc3af40361ff'

def check(d,t,e,files=True):
 base(d);base(t)
 assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
 for r in d['profiles']:
  assert r['passed'] and set(r['anchors'])==ANCHORS and len(r['controls'])==6
  assert {v['test'] for v in r['controls']}=={f'b02_followthrough.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(v['semantic_rejection'] for v in r['controls'])
 assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
 oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
 assert len(oracle)==6 and len(metrics)==6 and {r['profile'] for r in oracle}==PROFILES and {r['profile'] for r in metrics}==PROFILES
 for r in oracle:
  q=r['result'];assert q['passed'] and q['trace_sha256']==TRACE
  expected=dict(numeric_cases=279986,integers=19999,tags=2,capacities=7,wire_cases=80512,text_lengths=257,versions=2)
  assert all(q[k]==v for k,v in expected.items())
 for r in metrics:
  q=r['result'];assert q['result_bytes']>0 and q['value_bytes']>=q['result_bytes'] and q['wire_record_bytes']>=256 and q['max_text']==256 and q['max_wire']==260
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 required={'assessor/b02_depth/depth.c','labs/b02_depth/include/depth.h','assessor/b02_followthrough/result/result.c','tools/verify_b02_result.py'}
 assert required<=set(d['input_sha256'])
 for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256']) and r['external_inputs']
 rows=d['executions'];mutants=[r for r in rows if 'b02-result-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==125 and len(mutants)==36 and all(r['exit_code']==1 for r in mutants) and len(learners)==4 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 assert len(t['profiles'])==5 and {r['profile'] for r in t['profiles']}=={'gcc-O0','gcc-O3','clang-O3','mutant-boolean','learner'} and len(t['executions'])==137
 for r in t['profiles']:
  negative=r['profile'] in ['mutant-boolean','learner'];assert r['passed'] and r['actual_execution'] and r['semantic_rejection']==negative and r['compiler_dependencies']
  run=t['executions'][r['execution']];assert run['command'][0]=='qemu-system-arm' and run['exit_code']==int(negative)
  assert len(r['object_symbols'])==7 and all(set(s)<={'U','_sdata','_edata','_sidata','_sbss','_ebss','bf_target','b2_render','bf_number','bf_text','bf_value_init','bf_clone','bf_move','bf_take','bf_bytes_drop','bf_reset','bf_value_render','bf_encode','bf_decode','bf_old_render_wire','memcpy','memset','__aeabi_memclr4','__aeabi_memcpy4'} for s in r['object_symbols'].values())
  assert not any(p.endswith('/host.c') for p in r['compiler_dependencies'])
 assert len([r for r in t['executions'] if r['exit_code']!=0])==2
 for r in t['executions']:
  if '-c' in r['command']:assert '-std=c17' in r['command'] and '-ffreestanding' in r['command'] and '-fno-builtin' in r['command']
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and not e['private_sources_copied'] and e['exercise']=='X-T19-002' and e['original_family']=='b02_depth'
 assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed'
 assert e['input_sha256'] and e['exported_artifacts'] and len(e['executions'])==8
 assert set(e['explicit_actual_providers'])==required-{'labs/b02_depth/include/depth.h','tools/verify_b02_result.py'}
 failed=[r for r in e['executions'] if r['exit_code']!=0];assert len(failed)==2 and {r['required_marker'] for r in failed}=={'BF_CHECK','Supply the explicit completed result and B02 implementations'}
 assert all(r['expected_exit']==0 and r['exit_code']==0 or r['expected_exit'] is None and r['exit_code'] not in [None,0] for r in e['executions'])
 if files:
  bindings(d);bindings(t)
  for r in mutants+learners:
   text=(ROOT/r['log']).read_text(encoding='utf8');assert 'BF_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
  for r in ctests:
   text=(ROOT/r['log']).read_text(encoding='utf8');assert '100% tests passed' in text and all('BF '+a+' validated' in text for a in ANCHORS)
  for r in d['generated_artifacts']:
   if r['path'].endswith('compile_commands.json'):
    commands=json.loads((ROOT/r['path']).read_text());assert all('-std=c17' in c['command'] for c in commands);old=[c for c in commands if c['file'].endswith('/old_consumer.c')];assert len(old)==1;includes=[Path(token[2:]).resolve() for token in shlex.split(old[0]['command']) if token.startswith('-I')];assert includes==[(ROOT/'labs/b02_depth/include').resolve()]
  for r in t['profiles']:
   text=(ROOT/t['executions'][r['execution']]['log']).read_text(encoding='utf8');assert ('BF_TARGET_CHECK' if r['semantic_rejection'] else 'BF_TARGET_PASS') in text
  for r in t['executions']:
   cmd=r['command'];text=(ROOT/r['log']).read_text(encoding='utf8')
   if cmd[0]=='arm-none-eabi-nm' and cmd[-1].endswith('image.elf'):assert not text.strip()
   if cmd[0]=='arm-none-eabi-readelf' and '-d' in cmd:assert 'NEEDED' not in text
   if cmd[0]=='arm-none-eabi-readelf' and '-h' in cmd:assert 'ELF32' in text and 'ARM' in text
  for p,h in e['input_sha256'].items():assert sha(ROOT/p)==h,'stale export '+p
  for p,h in e['exported_artifacts'].items():assert e['input_sha256'][p]==h and 'assessor' not in p.split('/')
  for r in e['executions']:
   assert sha(ROOT/r['log'])==r['log_sha256']
   if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
  resource=json.loads((ROOT/BASE/'result_resources.json').read_text())['resources'][0];assert sha(ROOT/resource['url'])==resource['inspection']['local_sha256']
  gate=json.loads((ROOT/BASE/'result_research_gate.json').read_text());assert gate['status']=='passage-inspected-author-only'
  for p,h in gate['input_sha256'].items():assert sha(ROOT/p)==h,'stale passage '+p
 return True

def selftest(d,t,e):
 edits=[lambda a,b,c:a.update(passed=False),lambda a,b,c:a['profiles'].pop(),lambda a,b,c:a['profiles'][0]['controls'].pop(),lambda a,b,c:a['learner_stub_rejections'].pop(),lambda a,b,c:a['observations'][0]['result'].update(numeric_cases=1),lambda a,b,c:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b,c:a.update(input_sha256={}),lambda a,b,c:a.update(learner_mastery='passed'),lambda a,b,c:a['compiler_dependency_closures'].pop(),lambda a,b,c:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b,c:a.update(source_stable=False),lambda a,b,c:b['profiles'].pop(),lambda a,b,c:b['profiles'][0].update(actual_execution=False),lambda a,b,c:b['profiles'][0]['object_symbols'].update(a=['unknown_helper']),lambda a,b,c:b.update(physical_validation='passed'),lambda a,b,c:b['executions'][b['profiles'][0]['execution']].update(exit_code=1),lambda a,b,c:c.update(private_sources_copied=True),lambda a,b,c:c['executions'].pop(),lambda a,b,c:c.update(exported_artifacts={}),lambda a,b,c:c.update(learner_mastery='passed'),lambda a,b,c:c.update(original_family='replacement'),lambda a,b,c:c.update(explicit_actual_providers=[]),lambda a,b,c:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(max_wire=1),lambda a,b,c:next(r for r in a['observations'] if r['kind']=='independent-oracle')['result'].update(wire_cases=1)]
 for edit in edits:
  a,b,c=copy.deepcopy(d),copy.deepcopy(t),copy.deepcopy(e);edit(a,b,c)
  try:check(a,b,c,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported result evidence')
 return len(edits)
WARNINGS=BASE+'/result_warnings_validation.json'
def warnings(d,files=True):
 base(d);assert len(d['executions'])==12 and len(d['profiles'])==4 and len(d['generated_artifacts'])==8
 assert {(r['compiler'],r['optimization']) for r in d['profiles']}=={(c,o) for c in ['gcc','clang'] for o in ['-O0','-O3']}
 for r in d['profiles']:
  assert r['ordinary_omission_accepted'] and r['exhaustive_omission_rejected'] and r['complete_accepted'] and len(r['executions'])==3
  records=[d['executions'][i] for i in r['executions']];assert [x['expected_rejection'] for x in records]==[False,True,False]
  assert records[0]['exit_code']==records[2]['exit_code']==0 and records[1]['exit_code'] not in [0,None]
  assert '-Wswitch' in records[0]['command'] and '-Wswitch-enum' in records[1]['command'] and '-DBF_COMPLETE' in records[2]['command']
 for r in d['executions']:assert '-std=c17' in r['command'] and '-Werror' in r['command']
 if files:
  bindings(d)
  for r in d['executions']:
   if r['expected_rejection']:
    text=(ROOT/r['log']).read_text();assert 'ERROR' in text and 'switch' in text
 return True

def warnings_selftest(d):
 edits=[lambda d:d.update(passed=False),lambda d:d['profiles'].pop(),lambda d:d['executions'].pop(),lambda d:d['generated_artifacts'].pop(),lambda d:d['profiles'][0].update(ordinary_omission_accepted=False),lambda d:d['executions'][1].update(exit_code=0),lambda d:d['executions'][0]['command'].remove('-std=c17'),lambda d:d.update(learner_mastery='passed')]
 for edit in edits:
  x=copy.deepcopy(d);edit(x)
  try:warnings(x,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported switch diagnostic evidence')
 return len(edits)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d,t,e,w=[json.loads((ROOT/r).read_text()) for r in [REPORT,TARGET,EXPORT,WARNINGS]];check(d,t,e);warnings(w);print(json.dumps(dict(passed=True,negative_controls=selftest(d,t,e)+warnings_selftest(w) if a.self_test else 0,report_sha256=sha(ROOT/REPORT),target_sha256=sha(ROOT/TARGET),export_sha256=sha(ROOT/EXPORT),warnings_sha256=sha(ROOT/WARNINGS))))
