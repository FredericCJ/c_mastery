"""Explicit DAG composition with completed pool/lock and original CMK providers."""
import argparse,hashlib,importlib.util,json,shutil,subprocess
from pathlib import Path
PUBLIC=Path(__file__).resolve().parent
def prepare(lab,kernel,lock_public,lock_completed,pool_public,pool_completed,implementation,work,mutant=0,compiler='gcc',optimization='-O2',observer=None):
 if mutant not in (0,1):raise ValueError('Unknown DAG control')
 work.mkdir(parents=True,exist_ok=False);adapter=work/'adapter';adapter.mkdir();shutil.copyfile(pool_completed/'pool.c',adapter/'pool.c');shutil.copyfile(implementation/'dag_guest.c',adapter/'pool_guest.c');shutil.copyfile(implementation/'dag.c',adapter/'dag.c')
 spec=importlib.util.spec_from_file_location('dag_pool_guest',pool_public/'guest.py');pool=importlib.util.module_from_spec(spec);spec.loader.exec_module(pool);template=[]
 def actual(command):
  if observer:return observer(command)
  result=subprocess.run(list(map(str,command)),capture_output=True,text=True)
  if result.returncode:raise ValueError(str(command)+'\n'+result.stdout+result.stderr)
  return result.stdout
 def observe(command,**kwargs):
  if kwargs:raise ValueError('Unexpected guest build options')
  command=list(command)
  if command[0] in ('gcc','clang') and '-c' in command:
   command[1:1]=['-I'+str(PUBLIC),'-DDGG_MUTANT='+str(mutant)]
   if any(str(p).endswith('/tests/user_v2.c') for p in command):template[:]=command
  if command[0]=='ld' and any(str(p).endswith('/v2.elf') for p in command):
   if not template:raise ValueError('Missing original target compile recipe')
   for source,name in ((adapter/'dag.c','dag'),(PUBLIC/'target_runtime.c','target_runtime')):
    extra=list(template);extra[extra.index('-c')+1]=source;obj=work/(name+'.o');extra[extra.index('-o')+1]=obj;actual(extra);command.append(obj)
  return actual(command)
 g,info=pool.prepare(lab,kernel,lock_public,lock_completed,adapter,work/'composed',0,compiler,optimization,observe)
 return pool,g,{'mutant':mutant,'compiler':compiler,'optimization':optimization,'pool':info,'composed_sha256':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in adapter.iterdir()},'scope':'Actual completed pool launcher and lock, separate new DAG and user adapter objects, unchanged GCC-built CMK prerequisite. Caller stack arena is reused only after successful pool joins.'}
def execute(pool,g,folder,cpus,expected):
 observation,text=pool.execute(g,folder,cpus,expected)
 if expected.startswith('DG_GUEST_PASS:') and 'DG_GUEST_CHECK:' in text:observation['passed']=False
 return observation,text
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('lab','kernel','lock-public','lock-completed','pool-public','pool-completed','implementation','work'):p.add_argument('--'+name,type=Path,required=True)
 p.add_argument('--cpus',type=int,choices=(1,2,4),default=4);p.add_argument('--mutant',type=int,choices=(0,1),default=0);a=p.parse_args()
 pool,g,info=prepare(a.lab,a.kernel,a.lock_public,a.lock_completed,a.pool_public,a.pool_completed,a.implementation,a.work,a.mutant)
 expected='DG_GUEST_PASS:validated-arbitrary-DAG-pool-lifecycle-and-failure-propagation' if not a.mutant else 'DG_GUEST_CHECK:worker-placement'
 observation,text=execute(pool,g,a.work/'composed/image',a.cpus,expected);(a.work/'execution.log').write_text(text,encoding='utf-8',newline='\n');(a.work/'execution.json').write_text(json.dumps(observation,indent=2)+'\n',encoding='utf-8',newline='\n');print(text);print(json.dumps(observation));raise SystemExit(0 if observation['passed'] else 1)
