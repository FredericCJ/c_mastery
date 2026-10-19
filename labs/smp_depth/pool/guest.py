"""Compose a completed pool and lock implementation with the original CMK guest.

Only temporary copies change. The retained kernel and its public ABI stay exact.
The runner stops at the explicit pool marker; it does not certify later baseline
stages. Four caller-owned stack allocations are reused only after actual joins.
"""
import argparse,hashlib,importlib.util,json,os,select,shutil,subprocess,time
from pathlib import Path
PUBLIC=Path(__file__).resolve().parent

def prepare(lab,kernel,lock_public,lock_completed,implementation,work,mutant=0,compiler='gcc',optimization='-O2',observer=None):
 if mutant not in (0,1):raise ValueError('Unknown pool experiment')
 work.mkdir(parents=True,exist_ok=False);copied=work/'lab';shutil.copytree(lab,copied,ignore=shutil.ignore_patterns('__pycache__'));adapter=work/'adapter';adapter.mkdir()
 for name in ('pool.c','pool_guest.c'):shutil.copyfile(implementation/name,adapter/name)
 shutil.copyfile(lock_completed/'locks.c',adapter/'locks.c')
 text='#include "'+str(adapter/'pool_guest.c').replace('\\','/')+'"\n'
 (copied/'tests/user_v2.c').write_text(text,encoding='utf-8',newline='\n')
 spec=importlib.util.spec_from_file_location('pool_cmk_guest',copied/'tools/guest.py');g=importlib.util.module_from_spec(spec);spec.loader.exec_module(g)
 extra=['-DSPG_MUTANT='+str(mutant),'-I'+str(PUBLIC),'-I'+str(lock_public)]
 def run(command,**kwargs):
  command=list(command)
  if command[0]=='gcc':
   own=any(str(p) in map(str,command) for p in (copied/'tests/user_v2.c',adapter/'locks.c',adapter/'pool.c'))
   if own:command[0]=compiler
   command=[optimization if x=='-O2' else x for x in command];command[1:1]=extra
   if own:command[1:1]=['-Werror','-Wmisleading-indentation','-Wunused-function']
  if command[0]=='ld' and any(str(p).endswith('/v2.elf') for p in command):
   for name in ('pool.c','locks.c'):
    obj=work/'image'/(name+'.o');run(['gcc',*g.FLAGS,'-I'+str(copied/'include'),'-c',adapter/name,'-o',obj]);command.append(obj)
  if observer:return observer(command,**kwargs)
  result=subprocess.run(list(map(str,command)),capture_output=True,text=True,**kwargs)
  if result.returncode:raise ValueError(str(command)+'\n'+result.stdout+result.stderr)
  return result.stdout
 g.run=run;g.build(copied,kernel,work/'image')
 return g,{'mutant':mutant,'compiler':compiler,'optimization':optimization,'compiler_scope':'Selected compiler for pool, lock and original user adapter as separate translation units; unchanged GNU assembly/kernel prerequisite remains GCC-built.','scope':'Actual original CMK kernel, copied standalone pool target; stack arena recycling after join, not a nonexistent free syscall.','extra_flags':extra,'composed_sha256':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in adapter.iterdir()}}

def execute(g,folder,cpus,expected,timeout=90):
 disk=folder/('pool-'+str(cpus)+'.raw')
 with disk.open('wb') as f:f.truncate(4*1024*1024)
 command=[x.replace('disk.raw',disk.name) for x in g.command(folder,'write')];command[command.index('-smp')+1]=str(cpus)
 start=time.monotonic();p=subprocess.Popen(command,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);output=bytearray();observed=False
 try:
  while time.monotonic()-start<timeout:
   ready,_,_=select.select([p.stdout],[],[],0.2)
   if ready:
    chunk=os.read(p.stdout.fileno(),4096)
    if not chunk:break
    output.extend(chunk)
    expected_at=output.find(expected.encode())
    if expected_at>=0 and b'\n' in output[expected_at:]:observed=True;break
    failure_at=output.find(b'SP_GUEST_CHECK:')
    if failure_at>=0 and b'\n' in output[failure_at:] or b'FAIL' in output:break
   if p.poll() is not None:break
 finally:
  if p.poll() is None:p.terminate()
  try:p.wait(timeout=3)
  except subprocess.TimeoutExpired:p.kill();p.wait()
 text=output.decode('utf-8','replace');passed=observed and 'FAIL' not in text and (expected.startswith('SP_GUEST_CHECK:') or 'SP_GUEST_CHECK:' not in text)
 return {'command':command,'passed':passed,'marker':expected,'observed_marker':observed,'exit_code_after_deliberate_stop':p.returncode,'elapsed_seconds':time.monotonic()-start,'scope':'Emulator deliberately stopped at observed marker, not normal image exit or whole baseline acceptance.'},text

if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('lab','kernel','lock-public','lock-completed','implementation','work'):p.add_argument('--'+name,type=Path,required=True)
 p.add_argument('--mutant',type=int,choices=(0,1),default=0);p.add_argument('--cpus',type=int,choices=(1,2,4),default=4);a=p.parse_args()
 g,info=prepare(a.lab,a.kernel,a.lock_public,a.lock_completed,a.implementation,a.work,a.mutant)
 observation,text=execute(g,a.work/'image',a.cpus,'SP_GUEST_PASS:' if not a.mutant else 'SP_GUEST_CHECK:')
 (a.work/'execution.log').write_text(text,encoding='utf-8',newline='\n');(a.work/'execution.json').write_text(json.dumps(observation,indent=2)+'\n',encoding='utf-8',newline='\n');print(text);print(json.dumps(observation));raise SystemExit(0 if observation['passed'] else 1)
