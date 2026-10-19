"""Validate exact T24 successor observations, executed modes and retained objects."""
from copy import deepcopy
from pathlib import Path
import argparse,itertools,json
from common import ROOT,read_json,dump,sha
MODULES={'signals':{f'T24.{n:02}.{x}' for n in (1,2) for x in 'abc'},'ipc':{f'T24.{n:02}.{x}' for n in (3,4) for x in 'abc'}}
PUBLIC={'signals':['signals.h','signals.c','signals_tests.c','SIGNALS.md'],
 'ipc':['ipc.h','ipc.c','ipc_tests.c','ipc_budget.c','ipc_budget_tests.c','local.h','local.c','local_tests.c','IPC.md']}
IPC_NEGATIVES={'ipc-early-release':('IPC-CHECK:',None),'ipc-ack-stops':('IPC-CHECK:',None),
 'ipc-reset-budget':('IPC-BUDGET-CHECK:',None),'local-writable-success':('LOCAL-PROBE-CHECK:','candidates'),
 'local-cancel-leak':('LOCAL-PROBE-CHECK:','failures'),'ipc-learner':('IPC-CHECK:',None),
 'ipc-budget-learner':('IPC-BUDGET-CHECK:',None),'local-learner':('LOCAL-CHECK:',None)}
MODES={'coalesce':'COALESCE:generated=2;accepted=1;','realtime':'REALTIME:accepted=4;',
 'restart0':'RESTART:flag=0;return=-1;','restart1':'RESTART:flag=1;return=1;',
 'partial':'PARTIAL:requested=8;completed=2;','lost':'LOST-WAKE:repaired=0;',
 'repaired':'LOST-WAKE:repaired=1;','signalfd':'SIGNALFD:records=2;',
 'threads':'THREADS:workers=2;plugin=1;','bad-plugin':'BAD-PLUGIN:sole_unmasked_worker_received_process_signal=1;',
 'children':'CHILDREN:late=0;reaped=3;','late-child':'CHILDREN:late=1;reaped=3;',
 'deadline':'DEADLINE:real_ppoll_interruptions=3;'}
NEGATIVES=['full','errno','clear-all','restart-budget','one-child','learner']
def validate(d):
 errors=[];selected=d.get('selected_modules',[])
 if not selected or len(selected)!=len(set(selected)) or not set(selected)<=MODULES.keys():return ['Invalid module scope']
 if not d.get('passed') or d.get('error') is not None:errors.append('Failed campaign')
 if d.get('topic_complete') is not False:errors.append('Inflated mastery claim')
 expected={(o,v) for m in selected for o in MODULES[m] for v in (False,True)};obs=d.get('observations',[])
 if len(obs)!=len(expected) or {(r.get('objective'),r.get('variant')) for r in obs}!=expected:errors.append('Missing exact ordinary/changed observation')
 required=['tools/run_events_followthrough.py','assessor/legacy_systems/toolchain/common.py','research/v5/expansion/legacy_events/resources.json','research/v5/expansion/legacy_events/research_gate.json']
 if 'signals' in selected:required += [folder+'/'+name for folder,names in [('labs/hosted_systems/event_depth',['signals.h','signals.c','signals_tests.c','SIGNALS.md','CONTRACTS.md']),('assessor/hosted_systems/event_depth',['signals.c','signals.py','signal_probes.c','signal_failures.c','SIGNALS.md'])] for name in names]
 if 'ipc' in selected:required += [folder+'/'+name for folder,names in [('labs/hosted_systems/event_depth',PUBLIC['ipc']+['signals.h','CONTRACTS.md']),('assessor/hosted_systems/event_depth',['ipc.c','ipc.py','ipc_native.c','ipc_protocol.c','ipc_restart.c','ipc_budget.c','ipc_budget_native.c','ipc_trace.c','ipc_oracle.py','local.c','local_probes.c','local_abstract.c','signals.c','IPC.md'])] for name in names]
 if not set(required)<=d.get('input_sha256',{}).keys():errors.append('Missing required source pins')
 for name,h in d.get('input_sha256',{}).items():
  p=(ROOT/name).resolve()
  if not p.is_relative_to(ROOT.resolve()) or not p.is_file() or sha(p)!=h:errors.append('Stale input: '+name)
 if not d.get('retained_artifacts'):errors.append('Missing retained artifacts')
 for name,row in d.get('retained_artifacts',{}).items():
  p=(ROOT/name).resolve()
  if not p.is_relative_to((ROOT/'build/legacy_events').resolve()) or not p.is_file() or sha(p)!=row.get('sha256') or p.stat().st_size!=row.get('bytes'):errors.append('Stale artifact: '+name)
 commands=d.get('commands',[]);checks=d.get('checks',[])
 if not commands or not checks or any(r.get('passed') is not True for r in checks):errors.append('Missing/failed checks')
 for r in commands:
  rc=r.get('exit');wanted=r.get('expected');ok=isinstance(rc,int) and rc!=0 if wanted=='nonzero' else isinstance(rc,int) and rc==wanted
  if not ok or r.get('passed') is not True or (r.get('diagnostic') and r['diagnostic'] not in r.get('stdout','')+r.get('stderr','')):errors.append('Inconsistent command')
 def runs(name,mode=None):return [r for r in commands if r.get('argv') and 'CM_EVENT_MEASURE' not in r.get('environment',{}) and Path(r['argv'][0]).name==name and (mode is None or r['argv'][1:]==(mode if isinstance(mode,list) else [mode]))]
 def good(name,code=0,marker='',mode=None):
  rows=runs(name,mode)
  if len(rows)!=1 or rows[0]['exit']!=code or marker not in rows[0].get('stdout','')+rows[0].get('stderr',''):return False
  exe=rows[0]['argv'][0]
  builds=[r for r in commands if Path(r['argv'][0]).name in ('gcc','clang') and '-o' in r['argv'] and r['argv'][r['argv'].index('-o')+1]==exe and r['exit']==0]
  return len(builds)==1
 if 'signals' in selected:
  for cc,profile in itertools.product(('gcc','clang'),('debug','release','sanitize')):
   prefix=cc+'-'+profile
   if not good(prefix+'-signals',marker='counted_histories=9841;'):errors.append('Missing public profile: '+prefix)
   for mode,marker in MODES.items():
    if not good(prefix+'-signal-probes',marker=marker,mode=mode):errors.append('Missing actual mode: '+prefix+'/'+mode)
   if not good(prefix+'-signal-failures',marker='SIGNAL-FAILURES:acquisition_cuts=4;'):errors.append('Missing acquisition cleanup: '+prefix)
  for name in NEGATIVES:
   marker='SIGNAL-PROBE-CHECK:' if name=='one-child' else 'SIGNALS-CHECK:'
   if not good('signals-'+name,1,marker):errors.append('Missing semantic rejection: '+name)
  if not good('signals-thread-tsan',marker=MODES['threads'],mode='threads'):errors.append('Missing actual receiver TSan')
  names={r.get('name') for r in checks}
  for cc in ('gcc','clang'):
   if not {'no generated atomic helper '+cc,'handler body exists '+cc,'handler call closure '+cc}<=names:errors.append('Missing handler generated audit: '+cc)
 if 'ipc' in selected:
  for cc,profile in itertools.product(('gcc','clang'),('debug','release','sanitize')):
   prefix=cc+'-'+profile
   for name,marker in [('ipc','queue_capacity=4;bytes=32;wire_sum=643;'),('ipc-restart','independent_execs=3;epochs=2;stale_request_rejected=1;'),('ipc-budget','IPC-BUDGET:independent_cells=81;'),('ipc-budget-native','IPC-BUDGET-NATIVE:sent=8;partial_reply=2;timeout=1;'),('local','LOCAL-PASS:'),('local-abstract','LOCAL-ABSTRACT:embedded_nul_names_distinct=1;')]:
    if not good(prefix+'-'+name,marker=marker):errors.append('Missing IPC profile: '+prefix+'/'+name)
   for mode in range(3):
    if not good(prefix+'-ipc-native',marker=f'IPC-NATIVE:mode={mode};completed=512;',mode=[str(mode),'512']):errors.append('Missing IPC transport: '+prefix+'/'+str(mode))
    if not good(prefix+'-ipc-protocol',marker=f'IPC-PROTOCOL:mode={mode};effect={0 if mode==0 else 1};',mode=str(mode)):errors.append('Missing IPC ownership: '+prefix+'/'+str(mode))
   for mode,marker in [('candidates','LOCAL-CANDIDATES:attempts=2;'),('failures','LOCAL-FAILURES:socket_bind_listen_connect_accept=5;')]:
    if not good(prefix+'-local-probes',marker=marker,mode=mode):errors.append('Missing local case: '+prefix+'/'+mode)
   families=[r for r in runs(prefix+'-local-probes') if len(r['argv'])==3 and r['argv'][1]=='families' and r['exit']==0 and 'LOCAL-FAMILIES:actual_ipv6_loopback=1;actual_private_unix=1;' in r['stdout']]
   if len(families)!=1:errors.append('Missing local families: '+prefix)
   if not good(prefix+'-ipc-trace'):errors.append('Missing IPC trace execution: '+prefix)
   graph=[r for r in checks if r.get('name')=='IPC independent graph '+prefix and r.get('observed')=={'rows':208,'reachable_history_states':26}]
   if len(graph)!=1:errors.append('Missing IPC independent oracle: '+prefix)
  for name,(marker,mode) in IPC_NEGATIVES.items():
   if not good(name,1,marker,mode):errors.append('Missing IPC semantic rejection: '+name)
  for cc,mode in itertools.product(('gcc','clang'),range(3)):
   samples=[r for r in commands if Path(r['argv'][0]).name==cc+'-release-ipc-native' and r['argv'][1:]==[str(mode),'512'] and 'CM_EVENT_MEASURE' in r.get('environment',{})]
   if len(samples)!=8 or {r['environment']['CM_EVENT_MEASURE'] for r in samples}!={str(n) for n in range(8)} or any(f'IPC-NATIVE:mode={mode};completed=512;' not in r['stdout'] for r in samples):errors.append('Missing IPC cost samples: '+cc+'/'+str(mode))
 return errors
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--output',default='reviews/v5/expansion/legacy_events/support_validation.json');a=p.parse_args();d=read_json(ROOT/a.report);errors=validate(d);controls=[]
 if not errors:
  cases=[('missing delayed criterion',lambda x:x['observations'].pop(),'Missing exact ordinary/changed observation'),('inflated topic',lambda x:x.update(topic_complete=True),'Inflated mastery claim'),('missing objects',lambda x:x.update(retained_artifacts={}),'Missing retained artifacts'),('missing pins',lambda x:x.update(input_sha256={}),'Missing required source pins')]
  if 'signals' in d['selected_modules']:
   for name in NEGATIVES:cases.append((name+' crash substitution',lambda x,n=name:[r.update(exit=-11,expected=-11) for r in x['commands'] if Path(r['argv'][0]).name=='signals-'+n],'Missing semantic rejection: '+name))
   cases += [('skip restart profile',lambda x:x.update(commands=[r for r in x['commands'] if not(Path(r['argv'][0]).name=='clang-release-signal-probes' and r['argv'][1:]==['restart1'])]),'Missing actual mode: clang-release/restart1'),('skip TSan',lambda x:x.update(commands=[r for r in x['commands'] if Path(r['argv'][0]).name!='signals-thread-tsan']),'Missing actual receiver TSan'),('skip generated audit',lambda x:x.update(checks=[r for r in x['checks'] if r.get('name')!='handler call closure gcc']),'Missing handler generated audit: gcc')]
  if 'ipc' in d['selected_modules']:
   for name in IPC_NEGATIVES:cases.append((name+' crash substitution',lambda x,n=name:[r.update(exit=-11,expected=-11) for r in x['commands'] if Path(r['argv'][0]).name==n],'Missing IPC semantic rejection: '+name))
   cases += [('skip process restart',lambda x:x.update(commands=[r for r in x['commands'] if Path(r['argv'][0]).name!='clang-release-ipc-restart']),'Missing IPC profile: clang-release/ipc-restart'),('skip history oracle',lambda x:x.update(checks=[r for r in x['checks'] if r.get('name')!='IPC independent graph gcc-debug']),'Missing IPC independent oracle: gcc-debug'),('delete semantic mutant compilation',lambda x:x.update(commands=[r for r in x['commands'] if not('-o' in r['argv'] and Path(r['argv'][-1]).name=='ipc-ack-stops')]),'Missing IPC semantic rejection: ipc-ack-stops'),('skip shared record',lambda x:x.update(commands=[r for r in x['commands'] if not(Path(r['argv'][0]).name=='gcc-release-ipc-native' and r['argv'][1:]==['2','512'])]),'Missing IPC transport: gcc-release/2')]
   cases += [('drop cost samples',lambda x:x.update(commands=[r for r in x['commands'] if 'CM_EVENT_MEASURE' not in r.get('environment',{})]),'Missing IPC cost samples: gcc/0')]
  for title,mutate,wanted in cases:
   bad=deepcopy(d);mutate(bad);assert wanted in validate(bad),title;controls.append(title+' rejected')
  bad=deepcopy(d);bad['input_sha256'][next(iter(bad['input_sha256']))]='0'*64;assert any(e.startswith('Stale input:') for e in validate(bad));controls.append('stale source rejected')
 result=dict(passed=not errors,errors=errors,negative_guards=controls,input_sha256={a.report:sha(ROOT/a.report),'tools/check_events_followthrough.py':sha(Path(__file__))});dump(ROOT/a.output,result);print(json.dumps(result));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
