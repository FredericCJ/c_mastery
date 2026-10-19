"""Exact native mechanisms, counterexamples and original ARM image campaign."""
from pathlib import Path
import argparse,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():
 paths=[p for base in ('labs/amp_followthrough','assessor/amp_followthrough') for p in (ROOT/base).rglob('*') if p.is_file() and p.suffix in ('.c','.h','.S','.inc','.py','.json')]
 provider=json.loads((ROOT/'assessor/amp_followthrough/providers.json').read_text());paths += [ROOT/v for v in provider.values()]
 paths += [ROOT/p for p in ('labs/execution/include/cmx.h','labs/execution/include/cmx_port.h','labs/peripheral_depth/dma/dma.h','labs/interrupt_depth/lease/lease.h','labs/interrupt_depth/lease/bridge.h','labs/firmware_lifecycle/targets/h745_cache.h')]
 return sorted(set(paths+[Path(__file__).resolve()]))
def main():
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);a=p.parse_args();work=a.work.resolve();work.mkdir(parents=True,exist_ok=False);rows=[];checks=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()}
 def run(name,cmd,expected=0,marker=None,timeout=60):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=timeout);out=r.stdout+r.stderr;log=work/(name+'.txt');log.write_text(out);ok=r.returncode==expected and (marker is None or marker in out) and 'runtime error:' not in out and 'ERROR: AddressSanitizer' not in out
  rows.append(dict(name=name,command=cmd,returncode=r.returncode,expected=expected,marker=marker,passed=ok,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));print(name,ok,flush=True)
  if not ok:raise RuntimeError(name+'\n'+out[-4000:])
  return out
 inc=['-I'+str(ROOT/x) for x in ('labs/amp_followthrough/core','labs/interrupt_depth/lease','labs/peripheral_depth/dma')];lab=ROOT/'labs/amp_followthrough';ref=ROOT/'assessor/amp_followthrough';provider=json.loads((ref/'providers.json').read_text())
 units={'core':[lab/'core/tests.c',ref/'core/amp.c'],'doorbell':[lab/'core/doorbell_tests.c',ref/'core/doorbell.c',ref/'core/amp.c'],'buffers':[lab/'core/buffer_tests.c',ref/'core/buffers.c',*[ROOT/provider[k] for k in ('dma','lease','bridge')]],'recovery':[lab/'core/recovery_tests.c',ref/'core/amp.c'],'threaded':[lab/'core/thread_tests.c',ref/'core/amp.c']}
 try:
  for cc in ('gcc','clang'):
   run(cc+'-version',[cc,'--version'])
   for opt in ('O0','O3','sanitized'):
    tag=cc+'-'+opt;flags=['-std=c17','-Wall','-Wextra','-Werror','-pthread',*inc,*(['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'] if opt=='sanitized' else ['-'+opt])]
    for kind,files in units.items():
     exe=work/(tag+'-'+kind);run(tag+'-'+kind+'-compile',[cc,*flags,*files,'-o',exe])
     for group in ('images','boot','ring','cache','receipt') if kind=='core' else (kind,):run(tag+'-'+group,[exe,*([group] if kind=='core' else [])],marker='AF_PASS:')
   for macro,number,kind,group,mark in [('AF_MUTANT',1,'core','boot','AF_CHECK:'),('AF_DOOR_MUTANT',1,'doorbell',None,'AF_DOOR_CHECK:'),('AF_DOOR_MUTANT',2,'doorbell',None,'AF_DOOR_CHECK:'),('AF_DOOR_MUTANT',3,'doorbell',None,'AF_DOOR_CHECK:'),('PD_MUTANT',3,'buffers',None,'AF_BUFFER_CHECK:'),('PD_MUTANT',6,'buffers',None,'AF_BUFFER_CHECK:')]:
    name=cc+'-'+macro+str(number);exe=work/name;run(name+'-compile',[cc,'-std=c17','-O2','-Wall','-Wextra','-Werror','-pthread',*inc,'-D'+macro+'='+str(number),*units[kind],'-o',exe]);run(name,[exe,*([group] if group else [])],1,mark)
   for opt in ('O0','O3'):
    tag=cc+'-'+opt
    def target(name,mode,extra=(),expected=0):
     return run(name,[sys.executable,lab/'target/build.py','--source',ref,'--providers',ref/'providers.json','--build',work/name,'--cc',cc,'--optimization',opt,'--mode',mode,*extra],expected, '"passed": true' if not expected else 'FAIL CMX Cortex-M image',timeout=90)
    target('transport-'+tag,'transport',('--shared','sram4') if opt=='O3' else ())
    target('dual-pipeline-'+tag,'pipeline',('--cached','--burst','4','--interference',*(['--local'] if opt=='O3' else [])))
    target('qemu-pipeline-'+tag,'pipeline',('--qemu',))
    target('qemu-burst-'+tag,'pipeline',('--qemu','--burst','4','--interference'))
    target('qemu-local-'+tag,'pipeline',('--qemu','--burst','4','--local'))
    if opt=='O3':
     for mutant in (1,2):target('qemu-mutant'+str(mutant)+'-'+cc,'pipeline',('--qemu','--define','AF_PIPE_MUTANT='+str(mutant)),1)
    compiler=['arm-none-eabi-gcc'] if cc=='gcc' else ['clang','--target=arm-none-eabi'];obj=work/('dependency-'+tag+'.o')
    run('dependency-'+tag,[*compiler,'-std=c17','-'+opt,'-mcpu=cortex-m4','-mthumb','-ffreestanding','-c',lab/'target/dependency.c','-o',obj]);imports=run('dependency-'+tag+'-imports',['arm-none-eabi-nm','-u',obj]);checks.append(dict(name='64-bit helper '+tag,passed='__aeabi_uldivmod' in imports));run('dependency-'+tag+'-reject',['arm-none-eabi-ld','-e','af_changed_division',obj,'-o',work/('rejected-'+tag+'.elf')],1,'__aeabi_uldivmod')
  run('analyses',[sys.executable,ref/'models/analysis.py','--output',work/'analysis.json'],marker='AF_PASS:analysis')
  checks.append(dict(name='providers and source unchanged',passed=all(sha(ROOT/p)==h for p,h in pins.items())))
  if not all(x['passed'] for x in checks):raise RuntimeError('identity/dependency checks')
 except Exception as e:passed=False;error=str(e)
 else:passed=True;error=None
 out=dict(passed=passed,error=error,rows=rows,checks=checks,source_sha256=pins,artifacts={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},limits=['Native and finite event/cache models are distinct from actual AN386 machine execution.','AN386 executes one-core task composition; it does not emulate H745 dual-core, DMA or cache.','H745 images audited, physical measurements not-run.','Author execution does not establish independent learner, delayed or external review completion.'])
 (work/'result.json').write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(dict(passed=passed,error=error,commands=len(rows),checks=len(checks))));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
