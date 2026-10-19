"""Reject overstated host/target equivalence and incomplete source-bound evidence."""
from pathlib import Path
import argparse,copy,hashlib,importlib.util,json
from run_alternate_close_depth import PROFILES,inputs
ROOT=Path(__file__).resolve().parents[1];BASE='research/v5/expansion/alternate_close_depth';REPORT=BASE+'/validation.json'
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def read(p):return json.loads((ROOT/p).read_text())
def validate(d):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 need(d.get('passed') is True and d.get('source_stable') is True,'actual stable host campaign required')
 need(d.get('input_sha256')==inputs(),'complete current source inputs required')
 for p,h in d.get('outputs_sha256',{}).items():need((ROOT/p).is_file() and sha(p)==h,'stale output '+p)
 need(d.get('target_execution')=='not-run','host evidence cannot establish alternate-OS execution')
 for k,state in [('external_review','not-run'),('independent_learner','not-assessed'),('physical_observation','not-run')]:need(d.get(k)==state,'unsupported '+k)
 profiles=d.get('profiles',[]);need(len(profiles)==6 and {p.get('name') for p in profiles}==set(PROFILES),'six C17 profiles')
 for p in profiles:need(p.get('passed') and p.get('oracle',{}).get('passed') and p.get('oracle',{}).get('cases')==1305 and p.get('mutants')==6 and p.get('learner_rejected') is True,'full profile observations')
 commands=d.get('commands',[]);need(len(commands)==133,'133 observed commands')
 for c in commands:need(c.get('passed') is True and c.get('exit_code')==c.get('expected') and (not c.get('marker') or c['marker'] in c.get('stdout','')+c.get('stderr','')),'failed/mislabeled '+c.get('label',''))
 for name in PROFILES:
  expected=['version','reference build','core','native','cancel','oracle build','oracle','learner build','learner reject']+[f'mutant{m} {step}' for m in range(1,7) for step in ['build','reject']]
  for s in expected:need(sum(c.get('label')==name+' '+s for c in commands)==1,'missing/duplicated operation '+name+' '+s)
  for m in range(1,7):
   c=next((c for c in commands if c.get('label')==name+f' mutant{m} reject'),{})
   need(c.get('expected')==1 and c.get('marker')=='QC_CHECK:','compiled semantic rejection required')
  c=next((c for c in commands if c.get('label')==name+' oracle'),{})
  try:need(json.loads(c['stdout']).get('cases')==1305 and json.loads(c['stdout']).get('passed') is True,'actual oracle output')
  except (KeyError,ValueError):errors.append('missing actual oracle output')
 target=d.get('target_build',{});need(target.get('status')=='not-built' and 'qcc unavailable' in target.get('reason',''),'current licensed SDK blocker retained')
 try:
  need(sha(target['path'])==target['sha256'],'target blocker digest');t=read(target['path']);need(t['passed'] is False and t['status']=='not-built' and t['target_execution']=='not-run' and not t['commands'],'no fabricated target commands')
 except (OSError,KeyError):errors.append('missing actual target availability receipt')
 gate=read(BASE+'/research_gate.json')
 need(gate.get('status')=='passage-inspected','bounded passage status')
 for p,h in (gate['research_input_sha256'] | gate['input_sha256']).items():need(sha(p)==h,'stale research input '+p)
 resources=read(BASE+'/resources.json')['resources'];need(len(resources)==25 and all('8.0' in r['url'] or 'qnxeverywhere' in r['url'] for r in resources),'actual QNX8 source scope')
 for r in resources:
  need(bool(r['inspection']['scope']) and r['inspection']['scope']=='; '.join(r['locators']),'exact inspected passages')
  c=r['inspection']['capture'];need(sha(c['path'])==c['sha256'],'inspected capture changed')
 return errors
def capture_controls():
 path=ROOT/'labs/alternate_close_depth/capture.py';spec=importlib.util.spec_from_file_location('qc_capture',path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
 b=dict(passed=True,status='built-not-executed',source_sha256={'example.c':'1'*64},outputs_sha256={'/guest/probe':'2'*64,'/guest/manager':'3'*64,'/guest/signal_probe':'5'*64,'/guest/signal_manager':'6'*64})
 i=dict(os='QNX',release='8.0.0',image_sha256='4'*64,sdk_packages=['synthetic-control-only'],compiler_version='synthetic',guest_command='synthetic',probe_sha256='2'*64,manager_sha256='3'*64)
 p='QC_QNX_FIRST rc=1 error=4 eintr=1 retained=1 calls=1 live=1 same=1\nQC_QNX_FINAL rc=0 state=2 calls=2 ebadf=1\nQC_QNX_ID sysname=QNX release=8.0.0 machine=x86_64\nQC_QNX_OK:resource-manager-error-path\n';m='QC_MANAGER:injected-EINTR-before-default\nQC_MANAGER:default-close\n'
 assert not module.validate(b,p,m,0,i)
 controls=[]
 for name in ['host-only','failed-build','wrong-binary','missing-image','premature-retirement','missing-close','reordered','duplicate-injection','failed-probe','wrong-os']:
  bb,ii=copy.deepcopy((b,i));pp,mm,code=p,m,0
  if name=='host-only':bb['status']='host-passed'
  elif name=='failed-build':bb['passed']=False
  elif name=='wrong-binary':ii['probe_sha256']='0'*64
  elif name=='missing-image':ii.pop('image_sha256')
  elif name=='premature-retirement':pp=pp.replace('retained=1','retained=0')
  elif name=='missing-close':pp=pp.replace('QC_QNX_FINAL','MISSING')
  elif name=='reordered':lines=pp.splitlines();lines[0],lines[1]=lines[1],lines[0];pp='\n'.join(lines)
  elif name=='duplicate-injection':mm+=m
  elif name=='failed-probe':code=1
  else:ii['os']='Linux'
  controls.append(dict(control=name,rejected=bool(module.validate(bb,pp,mm,code,ii))))
 i.update(probe_sha256='5'*64,manager_sha256='6'*64)
 p='QC_SIGNAL_PROBE:handler-installed-before-close pid=100\nQC_SIGNAL_FIRST rc=1 eintr=1 retained=1 calls=1 live=1 same=1 caught=1\nQC_SIGNAL_FINAL rc=0 state=2 calls=2 ebadf=1\nQC_QNX_ID sysname=QNX release=8.0.0 machine=x86_64\nQC_QNX_OK:blocked-close-signal\n'
 m='QC_SIGNAL:held rcvid=55 pid=100 tid=1 coid=4\nQC_SIGNAL:sent-exact-thread\nQC_SIGNAL:unblock-pending rcvid=55 pid=100 tid=1 coid=4\nQC_SIGNAL:kernel-error-replied\nQC_SIGNAL:default-close\n'
 assert not module.validate(b,p,m,0,i,'signal')
 for name in ['signal-injected-substitute','signal-no-handler','signal-wrong-client','signal-reused-rcvid','signal-no-pending','signal-no-reply','signal-reordered','signal-duplicate','signal-failed-send','signal-wrong-binary']:
  bb,ii=copy.deepcopy((b,i));pp,mm=p,m
  if name=='signal-injected-substitute':mm+='QC_MANAGER:injected-EINTR-before-default\n'
  elif name=='signal-no-handler':pp=pp.replace('caught=1','caught=0')
  elif name=='signal-wrong-client':pp=pp.replace('pid=100','pid=101')
  elif name=='signal-reused-rcvid':mm=mm.replace('unblock-pending rcvid=55','unblock-pending rcvid=56')
  elif name=='signal-no-pending':mm=mm.replace('QC_SIGNAL:unblock-pending','MISSING')
  elif name=='signal-no-reply':mm=mm.replace('QC_SIGNAL:kernel-error-replied','MISSING')
  elif name=='signal-reordered':lines=mm.splitlines();lines[1],lines[2]=lines[2],lines[1];mm='\n'.join(lines)
  elif name=='signal-duplicate':mm+=m
  elif name=='signal-failed-send':mm+='QC_SIGNAL:send-failed\n'
  else:ii['probe_sha256']='2'*64
  controls.append(dict(control=name,rejected=bool(module.validate(bb,pp,mm,0,ii,'signal'))))
 return controls
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',required=True);a=p.parse_args();d=read(REPORT);errors=validate(d);controls=[]
 if a.self_test and not errors:
  for name in ['stale-source','missing-profile','lost-mutant','compile-error-as-mutant','lost-oracle','lost-native','invented-qnx','invented-review','invented-learner','invented-physical']:
   x=copy.deepcopy(d)
   if name=='stale-source':x['input_sha256'][next(iter(x['input_sha256']))]='0'*64
   elif name=='missing-profile':x['profiles'].pop()
   elif name=='lost-mutant':x['commands']=[c for c in x['commands'] if c['label']!='gcc-debug mutant1 reject']
   elif name=='compile-error-as-mutant':next(c for c in x['commands'] if c['label']=='gcc-debug mutant1 reject').update(exit_code=2,expected=2,marker=None)
   elif name=='lost-oracle':next(c for c in x['commands'] if c['label']=='gcc-debug oracle')['stdout']='{}'
   elif name=='lost-native':x['commands']=[c for c in x['commands'] if c['label']!='gcc-debug native']
   elif name=='invented-qnx':x['target_execution']='passed'
   else:x[{'invented-review':'external_review','invented-learner':'independent_learner','invented-physical':'physical_observation'}[name]]='passed'
   controls.append(dict(control=name,rejected=bool(validate(x))))
  controls+=capture_controls()
  if not all(c['rejected'] for c in controls):errors.append('accepted evidence mutant')
 result=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(p) for p in [REPORT,'tools/check_alternate_close_depth.py','labs/alternate_close_depth/capture.py']},scope='Host execution/source and synthetic capture-schema guards only; no supplied QNX execution capture')
 dest=ROOT/a.output;dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(result,indent=2)+'\n');print({'passed':not errors,'controls':len(controls),'errors':errors});return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
