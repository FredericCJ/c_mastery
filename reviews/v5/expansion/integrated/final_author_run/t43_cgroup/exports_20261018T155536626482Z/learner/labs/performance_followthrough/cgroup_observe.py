"""Read permitted existing cgroup files only. No membership/policy writes."""
import argparse,json,os,subprocess,time
from pathlib import Path
from certificates import keyed,devices,deltas
FILES=['cpu.max','cpu.max.burst','cpu.stat','memory.current','memory.peak','io.stat','cpu.pressure','memory.pressure','io.pressure','cgroup.controllers','cgroup.subtree_control','cgroup.type']
def read(p):
 try:return {'status':'read','text':p.read_text()}
 except OSError as e:return {'status':'unavailable','errno':e.errno,'text':None}
def snapshot(p):return {name:read(p/name) for name in FILES}
def main(binary):
 membership=Path('/proc/self/cgroup').read_text();mountinfo=Path('/proc/self/mountinfo').read_text();base=Path('/sys/fs/cgroup');path=next((s.split(':',2)[2] for s in membership.splitlines() if s.startswith('0::')),None)
 candidate=(base/(path or '').lstrip('/')).resolve();exact=bool(path and candidate.is_relative_to(base) and candidate.is_dir());current=candidate if exact else base
 ancestors=[];p=current
 while p.is_relative_to(base):
  ancestors.append({'path':str(p),'policy':{f:read(p/f) for f in ('cpu.max','cpu.max.burst','memory.max','cgroup.type')}})
  if p==base:break
  p=p.parent
 before=snapshot(current);start=time.monotonic_ns();run=subprocess.run([binary],capture_output=True,text=True,timeout=30);elapsed=time.monotonic_ns()-start;after=snapshot(current);assert run.returncode==0
 parsed={}
 if before['cpu.stat']['status']==after['cpu.stat']['status']=='read':parsed['cpu.stat']=deltas(keyed(before['cpu.stat']['text']),keyed(after['cpu.stat']['text']))
 for f in ('memory.current','memory.peak'):
  if before[f]['status']==after[f]['status']=='read':parsed[f]={'before':int(before[f]['text']),'after':int(after[f]['text']),'fresh_peak':False};parsed[f]['delta']=parsed[f]['after']-parsed[f]['before']
 if before['io.stat']['status']==after['io.stat']['status']=='read':
  a,b=devices(before['io.stat']['text']),devices(after['io.stat']['text']);parsed['io.stat']={k:deltas(a.get(k,{}),b.get(k,{})) for k in a.keys()|b.keys()}
 usage=parsed.get('cpu.stat',{}).get('usage_usec',{}).get('value')
 print(json.dumps({'passed':True,'membership':membership,'mountinfo':mountinfo,'observed_path':str(current),'exact_membership_path_resolved':exact,'scope':'existing hierarchy;shared descendants and competing tasks;notisolated','policy_writes':0,'membership_writes':0,'delegated_experiment':'unavailable:no already-prepared owned subtree selected','same_descriptor_peak_reset':False,'ancestors':ancestors,'before':before,'after':after,'delta':parsed,'elapsed_ns':elapsed,'workload_command':[binary],'workload_output':run.stdout,'workload_stderr':run.stderr,'workload_code':run.returncode,'object_ledger_bytes':[0,1048576,0],'cpu_fraction':None if usage is None else usage*1000/elapsed,'scope_limit':'aggregateCPUfraction may exceed1;PSI overlaps;IO inode writeback shared;payload bytes differ frommemory.current'},indent=2))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--binary',required=True);main(p.parse_args().binary)
