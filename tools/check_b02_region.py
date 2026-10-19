"""Reject unsupported arena enumeration, adapter, export and target claims."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
BASE='research/v5/expansion/b02_followthrough'
REPORT=BASE+'/region_validation.json';TARGET=BASE+'/region_target_validation.json';EXPORT=BASE+'/region_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
ANCHORS={'region','cohort','slice','growth','children','retry'}
CONTROL={1:'region',2:'region',3:'cohort',4:'slice',5:'growth',6:'retry'}
TRACE='c32a2f872b7f4a08ae45bfea640a53df3d3f530da76b9c5ab32bae782b3c2404'

def check(d,t,e,files=True):
 base(d);base(t)
 assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
 for r in d['profiles']:
  assert r['passed'] and set(r['anchors'])==ANCHORS and len(r['controls'])==6
  assert {v['test'] for v in r['controls']}=={f'b02_followthrough.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(v['semantic_rejection'] for v in r['controls'])
 assert len(d['learner_stub_rejections'])==6 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
 oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
 assert len(oracle)==6 and len(metrics)==6 and {r['profile'] for r in oracle}==PROFILES and {r['profile'] for r in metrics}==PROFILES
 for r in oracle:
  q=r['result'];assert q['passed'] and (q['ordinary_admissions'],q['extra_admissions'],q['slice_cases'],q['growth_histories'],q['growth_transitions'],q['size_max'],q['trace_sha256'])==(4620,462,20664,37324,179160,2**64-1,TRACE)
 for r in metrics:
  q=r['result'];assert q['region_bytes']>=32 and q['object_align']==32 and q['object_bytes']%q['object_align']==0 and q['cohort_bytes']>=4*q['object_bytes']+q['region_bytes']
  assert q['growth_bytes']>0 and q['slice_bytes']>0 and q['chunk_bytes']>32 and q['retry_bytes']>64
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 required={'assessor/b02_depth/depth.c','labs/b02_depth/include/depth.h','assessor/b02_followthrough/region/region.c','tools/verify_b02_region.py','assessor/modern_bridge/bridge.c','assessor/b02_followthrough/arena/arena.c'}
 assert required<=set(d['input_sha256'])
 for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256']) and r['external_inputs']
 rows=d['executions'];mutants=[r for r in rows if 'b02-region-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==127 and len(mutants)==36 and all(r['exit_code']==1 for r in mutants) and len(learners)==6 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 assert len(t['profiles'])==5 and {r['profile'] for r in t['profiles']}=={'gcc-O0','gcc-O3','clang-O3','mutant-slice','learner'} and len(t['executions'])==152
 for r in t['profiles']:
  negative=r['profile'] in ['mutant-slice','learner'];assert r['passed'] and r['actual_execution'] and r['semantic_rejection']==negative and r['compiler_dependencies']
  run=t['executions'][r['execution']];assert run['command'][0]=='qemu-system-arm' and run['exit_code']==int(negative)
  assert len(r['object_symbols'])==8 and all(set(s)<={'U','_sdata','_edata','_sidata','_sbss','_ebss','bf_target','b2_arena_init','b2_reserve','b2_mark','b2_rollback','bf_arena_init','bf_arena_reserve','bf_arena_get','bf_arena_set','bf_arena_mark','bf_arena_register','bf_arena_rollback','bf_arena_commit','mb_region_init','mb_region_alloc','mb_region_read','mb_region_reset','mb_region_cleanup','bf_cohort_init','bf_cohort_batch','bf_cohort_reset','bf_tree_init','bf_tree_child_reset','bf_tree_reset','bf_slice_init','bf_slice_alloc','bf_slice_read','bf_slice_reset','bf_growth_init','bf_growth_alloc','bf_growth_read','bf_growth_reset','bf_retry_init','bf_retry_reserve','bf_retry_register','bf_retry_reset','memcpy','memset','__aeabi_memclr4','__aeabi_memcpy4'} for s in r['object_symbols'].values())
  assert not any(p.endswith('/host.c') for p in r['compiler_dependencies'])
 assert len([r for r in t['executions'] if r['exit_code']!=0])==2
 for r in t['executions']:
  if '-c' in r['command']:assert '-std=c17' in r['command'] and '-ffreestanding' in r['command'] and '-fno-builtin' in r['command']
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and not e['private_sources_copied'] and e['exercise']=='X-T16-003' and e['original_family']=='modern_bridge'
 assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed'
 assert e['input_sha256'] and e['exported_artifacts'] and len(e['executions'])==8
 assert set(e['explicit_actual_providers'])==required-{'labs/b02_depth/include/depth.h','tools/verify_b02_region.py'}
 failed=[r for r in e['executions'] if r['exit_code']!=0];assert len(failed)==2 and {r['required_marker'] for r in failed}=={'BF_CHECK','Supply the explicit completed region, MB, arena and B02 implementations'}
 assert all(r['expected_exit']==0 and r['exit_code']==0 or r['expected_exit'] is None and r['exit_code'] not in [None,0] for r in e['executions'])
 if files:
  bindings(d);bindings(t)
  for r in mutants+learners:
   text=(ROOT/r['log']).read_text(encoding='utf8');assert 'BF_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
  for r in ctests:
   text=(ROOT/r['log']).read_text(encoding='utf8');assert '100% tests passed' in text and all('BF '+a+' validated' in text for a in ANCHORS)
  for r in d['generated_artifacts']:
   if r['path'].endswith('compile_commands.json'):assert all('-std=c17' in c['command'] for c in json.loads((ROOT/r['path']).read_text()))
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
  resource=json.loads((ROOT/BASE/'region_resources.json').read_text())['resources'][0];assert sha(ROOT/resource['url'])==resource['inspection']['local_sha256']
  gate=json.loads((ROOT/BASE/'region_research_gate.json').read_text());assert gate['status']=='passage-inspected-author-only'
  for p,h in gate['input_sha256'].items():assert sha(ROOT/p)==h,'stale passage '+p
 return True

def selftest(d,t,e):
 edits=[lambda a,b,c:a.update(passed=False),lambda a,b,c:a['profiles'].pop(),lambda a,b,c:a['profiles'][0]['controls'].pop(),lambda a,b,c:a['learner_stub_rejections'].pop(),lambda a,b,c:a['observations'][0]['result'].update(growth_histories=1),lambda a,b,c:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b,c:a.update(input_sha256={}),lambda a,b,c:a.update(learner_mastery='passed'),lambda a,b,c:a['compiler_dependency_closures'].pop(),lambda a,b,c:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b,c:a.update(source_stable=False),lambda a,b,c:b['profiles'].pop(),lambda a,b,c:b['profiles'][0].update(actual_execution=False),lambda a,b,c:b['profiles'][0]['object_symbols'].update(a=['unknown_helper']),lambda a,b,c:b.update(physical_validation='passed'),lambda a,b,c:b['executions'][b['profiles'][0]['execution']].update(exit_code=1),lambda a,b,c:c.update(private_sources_copied=True),lambda a,b,c:c['executions'].pop(),lambda a,b,c:c.update(exported_artifacts={}),lambda a,b,c:c.update(learner_mastery='passed'),lambda a,b,c:c.update(original_family='replacement'),lambda a,b,c:c.update(explicit_actual_providers=[]),lambda a,b,c:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(object_align=1),lambda a,b,c:next(r for r in a['observations'] if r['kind']=='independent-oracle')['result'].update(slice_cases=1)]
 for edit in edits:
  a,b,c=copy.deepcopy(d),copy.deepcopy(t),copy.deepcopy(e);edit(a,b,c)
  try:check(a,b,c,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported arena evidence')
 return len(edits)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());t=json.loads((ROOT/TARGET).read_text());e=json.loads((ROOT/EXPORT).read_text());check(d,t,e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,t,e) if a.self_test else 0,report_sha256=sha(ROOT/REPORT),target_sha256=sha(ROOT/TARGET),export_sha256=sha(ROOT/EXPORT))))
