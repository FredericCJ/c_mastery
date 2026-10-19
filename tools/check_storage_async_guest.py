"""Verify actual guest adapter, source-copy boundaries and strict image closure."""
import copy,json
from pathlib import Path
from check_replicated_replay_depth import ROOT,sha
REPORT='research/v5/expansion/legacy_storage_followthrough/async_guest_validation.json'
def check(d,files=True):
 assert d['passed'] and d['status']=='passed' and d['source_stable'] and d['external_inputs_stable']
 assert d['input_sha256'] and d['external_input_sha256'] and len(d['executions'])==194
 assert len(d['builds'])==4 and {r['mode'] for r in d['builds']}=={'reference','mutant1','mutant3','learner'}
 assert len(d['observations'])==6 and {(r['mode'],r['cpus']) for r in d['observations']}=={('reference',1),('reference',2),('reference',4),('mutant1',2),('mutant3',2),('learner',2)}
 assert {'assessor/kernel_smp/async.inc','labs/kernel_smp/tools/guest.py','labs/kernel_smp/tests/user_v2.c','assessor/storage_followthrough/async/async.c','labs/storage_followthrough/async/guest.c'}<=set(d['input_sha256'])
 assert d['physical_validation']=='not-run' and d['independent_review']=='not-run' and d['learner_mastery']=='not-assessed'
 indexes=set()
 for r in d['observations']:
  assert r['passed'] and r['execution'] not in indexes;indexes.add(r['execution']);x=d['executions'][r['execution']];cmd=x['command'];positive=r['mode']=='reference'
  assert x['exit_code']==r['expected_exit']==(33 if positive else 35)
  assert cmd[0]=='qemu-system-x86_64' and cmd[cmd.index('-smp')+1]==str(r['cpus']) and cmd[cmd.index('-net')+1]=='none' and cmd[cmd.index('-accel')+1]=='tcg,thread=multi' and cmd[cmd.index('-machine')+1]=='pc-i440fx-10.2' and '-enable-kvm' not in cmd
  assert r['kind']==('actual-QEMU-CMK2-user-read-lifetime' if positive else 'compiled-guest-semantic-rejection')
 assert all(r['exit_code']==0 or i in indexes for i,r in enumerate(d['executions']))
 for b in d['builds']:
  assert b['copied_inputs'] and b['compiler_dependencies'] and b['artifacts']
  assert b['original_workload_sha256']==d['input_sha256']['labs/kernel_smp/tests/user_v2.c']
  assert {'tests/sa_impl.c','tests/sa_guest.c','include/async.h','tests/user_v2.c'}<=set(b['copied_inputs'])
  assert any(p.endswith('/async.inc') for p in b['compiler_dependencies']) and any(p.endswith('/sa_impl.c') for p in b['compiler_dependencies'])
 if files:
  for p,h in d['input_sha256'].items():assert sha(ROOT/p)==h,'stale guest input '+p
  for p,h in d['external_input_sha256'].items():assert sha(Path(p))==h
  for x in d['executions']:assert sha(ROOT/x['log'])==x['log_sha256']
  for r in d['observations']:
   body=(ROOT/d['executions'][r['execution']]['log']).read_text()
   if r['mode']=='reference':assert all(s in body for s in ['SA_GUEST_PASS actual-CMK2-read-borrow-completion-before-reuse bytes4096','SA_GUEST retained-after-abandon-before-positive-wait','PASS smp-suite','PASS kernel-suite']) and 'FAIL' not in body and 'SA_GUEST_CHECK' not in body
   else:assert 'SA_GUEST_CHECK' in body and 'SA_GUEST_PASS' not in body
  for b in d['builds']:
   assert sha(ROOT/b['build_report'])==b['build_sha256'];report=json.loads((ROOT/b['build_report']).read_text());assert report['built'] and '-std=c17' in report['flags'] and '-ffreestanding' in report['flags'] and '-fno-builtin' in report['flags'] and not report['runtime_linked'] and not report['storage_linked']
   home=(ROOT/b['build_report']).parent.parent/'lab'
   for p,h in b['copied_inputs'].items():assert sha(home/p)==h
   for p,h in b['compiler_dependencies'].items():assert sha(Path(p))==h
   for p,h in b['artifacts'].items():assert sha(ROOT/p)==h
   original=(ROOT/'labs/kernel_smp/tests/user_v2.c').read_text();changed=(home/'tests/user_v2.c').read_text();expected=original.replace('static void producer(uintptr_t index){','#include "sa_impl.c"\n#include "sa_guest.c"\nstatic void producer(uintptr_t index){').replace('message("PASS v2-process-slot-generation-reuse\\n");die(0);','sa_guest();message("PASS v2-process-slot-generation-reuse\\n");die(0);');assert changed==expected
   for elf in ['kernel','init','worker','fault','shell','spawn','v2']:
    rows=[x for x in d['executions'] if x['command'][:2]==['nm','-u'] and x['command'][-1]==str((ROOT/b['build_report']).parent/(elf+'.elf'))];assert rows and all(not (ROOT/x['log']).read_text().strip() for x in rows)
    rows=[x for x in d['executions'] if x['command'][:2]==['readelf','-d'] and x['command'][-1]==str((ROOT/b['build_report']).parent/(elf+'.elf'))];assert rows and all('NEEDED' not in (ROOT/x['log']).read_text() for x in rows)
 return True
def selftest(d):
 edits=[lambda a:a.update(passed=False),lambda a:a['observations'].pop(),lambda a:a['builds'].pop(),lambda a:a['observations'][0].update(cpus=9),lambda a:a['observations'][0].update(kind='model-only'),lambda a:a['observations'][0].update(expected_exit=0),lambda a:a.update(physical_validation='passed'),lambda a:a.update(input_sha256={}),lambda a:a['builds'][0].update(compiler_dependencies={}),lambda a:a.update(source_stable=False),lambda a:a['builds'][0].update(copied_inputs={}),lambda a:a['executions'][a['observations'][-1]['execution']].update(exit_code=-11)]
 for edit in edits:
  a=copy.deepcopy(d);edit(a)
  try:check(a,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported guest evidence')
 return len(edits)
