"""Explicit composition of completed startup and per-CPU learner adapters."""
import argparse,hashlib,importlib.util,json,re,shutil,subprocess
from pathlib import Path
PUBLIC=Path(__file__).resolve().parent
def spans(text,anchor):
 pattern=r'\s*'.join(re.escape(t) for t in re.findall(r'[A-Za-z_]\w*|\d+|[^\s]',anchor));return [m.span() for m in re.finditer(pattern,text)]
def swap(text,anchor,replacement):
 found=spans(text,anchor)
 if len(found)!=1:raise ValueError('Startup prerequisite changed; inspect hook before composing')
 return text[:found[0][0]]+replacement+text[found[0][1]:]
def prepare(lab,kernel,startup_public,startup_completed,implementation,work,simd=0,mutant=0,compiler='gcc',optimization='-O2',observer=None):
 if simd not in (0,1) or mutant not in (0,1,2,3):raise ValueError('Unknown experiment')
 work.mkdir(parents=True,exist_ok=False);adapter=work/'composed';adapter.mkdir()
 old=(startup_completed/'startup_guest.c').read_text(encoding='utf-8')
 parked='void ss_parked(unsigned index){\n __atomic_store_n(&ss_observed[index].quiescent,1,__ATOMIC_RELEASE);\n for(;;)__asm__ volatile("cli;hlt");\n}'
 if len(spans(old,parked))!=1 or len(spans(old,'text("SS_GUEST_PASS:verified-generation-startup-and-quiescence\\n");'))!=1:raise ValueError('Startup prerequisite changed; inspect hook before composing')
 old=swap(old,parked,'void ss_parked(unsigned index){\n __atomic_store_n(&ss_observed[index].quiescent,1,__ATOMIC_RELEASE);\n sp_worker(index);for(;;)__asm__ volatile("cli;hlt");\n}')
 old=swap(old,'text("SS_GUEST_PASS:verified-generation-startup-and-quiescence\\n");','sp_bsp_enter((uintptr_t)(cpus[0].boot+sizeof cpus[0].boot));text("SP_GUEST_CHECK:unexpected-return\\n");')
 (adapter/'startup_guest.c').write_text('static void sp_worker(unsigned);void sp_bsp(void);void sp_bsp_enter(uintptr_t);\n'+old+'\n'+(implementation/'percpu_guest.c').read_text(encoding='utf-8'),encoding='utf-8',newline='\n')
 (adapter/'startup_trampoline.S').write_text((startup_completed/'startup_trampoline.S').read_text(encoding='utf-8')+'\n'+(implementation/'percpu_frames.S').read_text(encoding='utf-8'),encoding='utf-8',newline='\n');shutil.copyfile(startup_completed/'startup.c',adapter/'startup.c');shutil.copyfile(implementation/'percpu.c',adapter/'percpu.c')
 spec=importlib.util.spec_from_file_location('percpu_startup',startup_public/'guest.py');startup=importlib.util.module_from_spec(spec);spec.loader.exec_module(startup)
 extra=['-DSP_SIMD='+str(simd),'-DSP_GUEST_MUTANT='+str(mutant),'-I'+str(adapter),'-I'+str(PUBLIC)]
 def run(command,**kwargs):
  command=list(command)
  if command[0] in ('gcc','clang') and '-c' in command:command[1:1]=extra
  if observer:return observer(command,**kwargs)
  result=subprocess.run(list(map(str,command)),capture_output=True,text=True,**kwargs)
  if result.returncode:raise ValueError(str(command)+'\n'+result.stdout+result.stderr)
  return result.stdout
 guest,instrumentation=startup.prepare(lab,kernel,adapter,work/'image',0x8000,1,0,compiler,optimization,run)
 return startup,guest,{'startup':instrumentation,'simd':simd,'mutant':mutant,'extra_flags':extra,'scope':'Explicit completed startup prerequisite, original controlled C/assembly worker and exact known hook; no source change to prerequisite.','composed_sha256':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in adapter.iterdir()}}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('lab','kernel','startup-public','startup-completed','implementation','work'):p.add_argument('--'+name,type=Path,required=True)
 p.add_argument('--simd',type=int,choices=(0,1),default=0);p.add_argument('--mutant',type=int,choices=(0,1,2,3),default=0);p.add_argument('--cpus',type=int,choices=(1,2,4),default=4);a=p.parse_args()
 startup,g,info=prepare(a.lab,a.kernel,a.startup_public,a.startup_completed,a.implementation,a.work,a.simd,a.mutant)
 r=subprocess.run(startup.command(g,a.work/'image',a.cpus,'qemu64'),capture_output=True,text=True,timeout=60);(a.work/'execution.log').write_text(r.stdout+r.stderr,encoding='utf-8',newline='\n');print(r.returncode);print(r.stdout+r.stderr);raise SystemExit(0 if r.returncode==33 and 'SP_GUEST_PASS:' in r.stdout else 1)
