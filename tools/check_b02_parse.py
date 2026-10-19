"""Reject unsupported parser enumeration, adapter, export and target claims."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
BASE='research/v5/expansion/b02_followthrough'
REPORT=BASE+'/parse_validation.json';TARGET=BASE+'/parse_target_validation.json';EXPORT=BASE+'/parse_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
ANCHORS={'baseline','grammar','range','lifecycle','adapter'}
CONTROL={1:'range',2:'grammar',3:'range',4:'adapter',5:'lifecycle'}
TRACE='497872f8e24aec323c4864c48f4f8d5a25c75ab4a4719fea44afa4360bb546bc'

def check(d,t,e,files=True):
 base(d);base(t)
 assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
 for r in d['profiles']:
  assert r['passed'] and set(r['anchors'])==ANCHORS and len(r['controls'])==5
  assert {v['test'] for v in r['controls']}=={f'b02_followthrough.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(v['semantic_rejection'] for v in r['controls'])
 assert len(d['learner_stub_rejections'])==5 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
 assert len(d['observations'])==6 and {r['profile'] for r in d['observations']}==PROFILES
 for r in d['observations']:
  q=r['result'];assert r['kind']=='independent-oracle' and q['passed'] and (q['rows'],q['short_split_rows'],q['radix_boundary_rows'],q['long_precedence_rows'],q['trace_sha256'])==(111751,109672,2031,48,TRACE)
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 required={'assessor/b02_depth/depth.c','labs/b02_depth/include/depth.h','assessor/b02_followthrough/parse/parse.c','assessor/b02_followthrough/parse/host.c','tools/verify_b02_parse.py'}
 assert required<=set(d['input_sha256'])
 for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256']) and r['external_inputs']
 rows=d['executions'];mutants=[r for r in rows if 'b02-parse-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==126 and len(mutants)==30 and all(r['exit_code']==1 for r in mutants) and len(learners)==5 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 assert len(t['profiles'])==5 and {r['profile'] for r in t['profiles']}=={'gcc-O0','gcc-O3','clang-O3','mutant-precedence','learner'} and len(t['executions'])==92
 for r in t['profiles']:
  negative=r['profile'] in ['mutant-precedence','learner'];assert r['passed'] and r['actual_execution'] and r['semantic_rejection']==negative and r['compiler_dependencies']
  run=t['executions'][r['execution']];assert run['command'][0]=='qemu-system-arm' and run['exit_code']==int(negative)
  assert len(r['object_symbols'])==4 and all(not any(h in s for h in ['memcpy','memset','__aeabi']) for s in r['object_symbols'].values())
  assert not any(p.endswith('/host.c') for p in r['compiler_dependencies'])
 assert len([r for r in t['executions'] if r['exit_code']!=0])==2
 for r in t['executions']:
  if '-c' in r['command']:assert '-std=c17' in r['command'] and '-ffreestanding' in r['command'] and '-fno-builtin' in r['command']
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and not e['private_sources_copied'] and e['exercise']=='X-T12-002' and e['original_family']=='b02_depth'
 assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed'
 assert e['input_sha256'] and e['exported_artifacts'] and len(e['executions'])==8
 assert set(e['explicit_actual_providers'])==required-{'labs/b02_depth/include/depth.h','tools/verify_b02_parse.py'}
 failed=[r for r in e['executions'] if r['exit_code']!=0];assert len(failed)==2 and {r['required_marker'] for r in failed}=={'BF_CHECK','Supply the explicit completed parser and B02 implementations'}
 assert all(r['expected_exit']==0 and r['exit_code']==0 or r['expected_exit'] is None and r['exit_code'] not in [None,0] for r in e['executions'])
 if files:
  bindings(d);bindings(t)
  for r in mutants+learners:
   text=(ROOT/r['log']).read_text(encoding='utf8');assert 'BF_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
  for r in ctests:
   text=(ROOT/r['log']).read_text(encoding='utf8');assert '100% tests passed' in text and all('BF '+a+' validated' in text for a in ANCHORS) and 'LONG_BITS=64' in text and 'locales=C,C.UTF-8' in text
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
  resource=json.loads((ROOT/BASE/'parse_resources.json').read_text())['resources'][0];assert sha(ROOT/resource['url'])==resource['inspection']['local_sha256']
  gate=json.loads((ROOT/BASE/'parse_research_gate.json').read_text());assert gate['status']=='passage-inspected-author-only'
  for p,h in gate['input_sha256'].items():assert sha(ROOT/p)==h,'stale passage '+p
 return True

def selftest(d,t,e):
 edits=[lambda a,b,c:a.update(passed=False),lambda a,b,c:a['profiles'].pop(),lambda a,b,c:a['profiles'][0]['controls'].pop(),lambda a,b,c:a['learner_stub_rejections'].pop(),lambda a,b,c:a['observations'][0]['result'].update(rows=1),lambda a,b,c:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b,c:a.update(input_sha256={}),lambda a,b,c:a.update(learner_mastery='passed'),lambda a,b,c:a['compiler_dependency_closures'].pop(),lambda a,b,c:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b,c:a.update(source_stable=False),lambda a,b,c:b['profiles'].pop(),lambda a,b,c:b['profiles'][0].update(actual_execution=False),lambda a,b,c:b['profiles'][0]['object_symbols'].update(a=['memcpy']),lambda a,b,c:b.update(physical_validation='passed'),lambda a,b,c:b['executions'][b['profiles'][0]['execution']].update(exit_code=1),lambda a,b,c:c.update(private_sources_copied=True),lambda a,b,c:c['executions'].pop(),lambda a,b,c:c.update(exported_artifacts={}),lambda a,b,c:c.update(learner_mastery='passed'),lambda a,b,c:c.update(original_family='replacement'),lambda a,b,c:c.update(explicit_actual_providers=[])]
 for edit in edits:
  a,b,c=copy.deepcopy(d),copy.deepcopy(t),copy.deepcopy(e);edit(a,b,c)
  try:check(a,b,c,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported parser evidence')
 return len(edits)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());t=json.loads((ROOT/TARGET).read_text());e=json.loads((ROOT/EXPORT).read_text());check(d,t,e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,t,e) if a.self_test else 0,report_sha256=sha(ROOT/REPORT),target_sha256=sha(ROOT/TARGET),export_sha256=sha(ROOT/EXPORT))))
