"""Explicit composition of completed startup and remote-wakeup learner adapters."""
import argparse,hashlib,importlib.util,json,shutil,subprocess
from pathlib import Path
PUBLIC=Path(__file__).resolve().parent
def prepare(lab,kernel,startup_public,startup_completed,implementation,queues_public,queues_completed,locks_public,locks_completed,work,coarse=0,mutant=0,compiler='gcc',optimization='-O2',observer=None):
 if coarse not in (0,1) or mutant not in (0,1,2):raise ValueError('Unknown experiment')
 work.mkdir(parents=True,exist_ok=False);adapter=work/'composed';adapter.mkdir()
 old=(startup_completed/'startup_guest.c').read_text(encoding='utf-8')
 parked='void ss_parked(unsigned index){\n __atomic_store_n(&ss_observed[index].quiescent,1,__ATOMIC_RELEASE);\n for(;;)__asm__ volatile("cli;hlt");\n}'
 if old.count(parked)!=1 or old.count('text("SS_GUEST_PASS:verified-generation-startup-and-quiescence\\n");')!=1:raise ValueError('Startup prerequisite changed; inspect hook before composing')
 old=old.replace(parked,'void ss_parked(unsigned index){\n __atomic_store_n(&ss_observed[index].quiescent,1,__ATOMIC_RELEASE);\n swg_worker(index);for(;;)__asm__ volatile("cli;hlt");\n}')
 old=old.replace('text("SS_GUEST_PASS:verified-generation-startup-and-quiescence\\n");','swg_bsp_enter((uintptr_t)(cpus[0].boot+sizeof cpus[0].boot));text("SW_GUEST_CHECK:unexpected-return\\n");')
 (adapter/'startup_guest.c').write_text('static void swg_worker(unsigned);void swg_bsp(void);void swg_bsp_enter(uintptr_t);\n'+old+'\n'+(implementation/'wakeup_guest.c').read_text(encoding='utf-8'),encoding='utf-8',newline='\n')
 (adapter/'startup_trampoline.S').write_text((startup_completed/'startup_trampoline.S').read_text(encoding='utf-8')+'\n'+(implementation/'wakeup_frames.S').read_text(encoding='utf-8'),encoding='utf-8',newline='\n');shutil.copyfile(startup_completed/'startup.c',adapter/'startup.c');shutil.copyfile(locks_completed/'locks.c',adapter/'locks.c');shutil.copyfile(queues_completed/'queues.c',adapter/'queues.c');shutil.copyfile(implementation/'wakeup.c',adapter/'wakeup.c')
 spec=importlib.util.spec_from_file_location('wakeup_startup',startup_public/'guest.py');startup=importlib.util.module_from_spec(spec);spec.loader.exec_module(startup)
 extra=['-DSW_GUEST_MUTANT='+str(mutant),'-DSW_MUTANT='+str(1 if mutant==2 else 0),'-I'+str(adapter),'-I'+str(PUBLIC),'-I'+str(locks_public),'-I'+str(queues_public)]
 def run(command,**kwargs):
  command=list(command)
  if command[0] in ('gcc','clang') and '-c' in command:command[1:1]=extra
  if observer:return observer(command,**kwargs)
  result=subprocess.run(list(map(str,command)),capture_output=True,text=True,**kwargs)
  if result.returncode:raise ValueError(str(command)+'\n'+result.stdout+result.stderr)
  return result.stdout
 guest,instrumentation=startup.prepare(lab,kernel,adapter,work/'image',0x8000,1,0,compiler,optimization,run)
 return startup,guest,{'startup':instrumentation,'coarse':coarse,'mutant':mutant,'extra_flags':extra,'scope':'Explicit completed startup prerequisite, original controlled C/assembly worker and exact known hook; no source change to prerequisite.','composed_sha256':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in adapter.iterdir()}}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('lab','kernel','startup-public','startup-completed','implementation','queues-public','queues-completed','locks-public','locks-completed','work'):p.add_argument('--'+name,type=Path,required=True)
 p.add_argument('--coarse',type=int,choices=(0,1),default=0);p.add_argument('--mutant',type=int,choices=(0,1,2),default=0);p.add_argument('--cpus',type=int,choices=(2,4),default=4);a=p.parse_args()
 startup,g,info=prepare(a.lab,a.kernel,a.startup_public,a.startup_completed,a.implementation,a.queues_public,a.queues_completed,a.locks_public,a.locks_completed,a.work,a.coarse,a.mutant)
 r=subprocess.run(startup.command(g,a.work/'image',a.cpus,'qemu64'),capture_output=True,text=True,timeout=60);(a.work/'execution.log').write_text(r.stdout+r.stderr,encoding='utf-8',newline='\n');print(r.returncode);print(r.stdout+r.stderr);raise SystemExit(0 if r.returncode==33 and 'SW_GUEST_PASS:' in r.stdout else 1)
