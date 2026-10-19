"""Criterion-specific native and actual target campaign; immutable attempt dirs."""
from __future__ import annotations
import argparse,hashlib,json,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);a=p.parse_args();work=a.work.resolve();work.mkdir(parents=True,exist_ok=False)
 rows=[];checks=[]
 def run(name,command,expected=0,marker=None,timeout=60):
  command=list(map(str,command));r=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=timeout);out=r.stdout+r.stderr;(work/(name+'.txt')).write_text(out)
  passed=(r.returncode==expected if expected==0 else r.returncode!=0) and (marker is None or marker in out)
  rows.append({'name':name,'command':command,'returncode':r.returncode,'expected':expected,'marker':marker,'passed':passed,'log':(work/(name+'.txt')).relative_to(ROOT).as_posix()})
  if not passed:raise RuntimeError(name+'\n'+out[-5000:])
  print('PASS '+name,flush=True);return out
 def target(name,mode='direct',cc='gcc',opt='O2',defines=(),fp=False,extra=(),marker=None):
  command=[sys.executable,ROOT/'labs/mcu_followthrough/target/build.py','--source',ROOT/'assessor/mcu_followthrough/target','--runtime-source',ROOT/'assessor/execution','--core-source',ROOT/'assessor/mcu_followthrough/core/mcu.c','--mode',mode,'--cc',cc,'--optimization',opt,'--build',work/name,'--run']
  for d in defines:command+=['--define',d]
  if fp:command+=['--fp']
  command+=extra;return run(name,command,1 if marker else 0,marker,60)
 try:
  for cc in ('gcc','clang'):
   san=work/('sanitized-'+cc)
   run('compile-sanitized-'+cc,[cc,'-std=c17','-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-Wall','-Wextra','-Werror','-I'+str(ROOT/'labs/mcu_followthrough/core'),'-I'+str(ROOT/'labs/execution/include'),ROOT/'labs/mcu_followthrough/core/tests.c',ROOT/'assessor/mcu_followthrough/core/mcu.c',ROOT/'assessor/execution/cmx.c','-o',san])
   for group in ('admission','lifecycle','scheduling','faults','timers','timing','masks-sleep','helper'):run('sanitized-'+cc+'-'+group,[san,group])
   for opt in ('O0','O2'):
    name=cc+'-'+opt;run('core-'+name,[sys.executable,ROOT/'labs/mcu_followthrough/run.py','--source',ROOT/'assessor/mcu_followthrough/core/mcu.c','--cmx',ROOT/'assessor/execution/cmx.c','--cc',cc,'--optimization',opt,'--build',work/('core-'+name)])
    exe=work/('variant-'+name);run('compile-variant-'+name,[cc,'-std=c17','-'+opt,'-Wall','-Wextra','-Werror','-I'+str(ROOT/'labs/mcu_followthrough/core'),'-I'+str(ROOT/'labs/execution/include'),ROOT/'labs/mcu_followthrough/core/variants.c',ROOT/'assessor/mcu_followthrough/core/order.c',ROOT/'assessor/mcu_followthrough/core/mcu.c',ROOT/'assessor/execution/cmx.c','-o',exe]);run('variant-'+name,[exe],marker='MF_PASS:variants')
    for mode in ('direct','preemptive','sleep','integrated'):target(mode+'-'+name,mode,cc,opt)
    target('direct-fp-'+name,'direct',cc,opt,fp=True,extra=['--frame-pointer','--move-data'])
    target('preempt-fp-'+name,'preemptive',cc,opt,defines=['MF_NESTED',*(['MF_EAGER'] if opt=='O0' else [])],fp=True)
   for mode in ('direct','preemptive','integrated'):
    target(mode+'-m7-'+cc,mode,cc,fp=mode!='integrated',defines=['MF_NESTED'] if mode=='preemptive' else ['MF_BORROWED'] if mode=='integrated' else [],extra=['--target','m7'])
   target('spill-'+cc,cc=cc,fp=True,defines=['MF_SPILL'],extra=['--frame-pointer'])
   target('long-isr-'+cc,'preemptive',cc,fp=True,defines=['MF_NESTED','MF_LONG_ISR'])
   target('log-mask-'+cc,'preemptive',cc,defines=['MF_LOG_MASK'])
   target('mpu-'+cc,'preemptive',cc,defines=['MF_MPU_FAULT'])
   target('uart-borrowed-'+cc,'integrated',cc,defines=['MF_UART','MF_BORROWED'])
   target('missing-helper-'+cc,cc=cc,extra=['--without-helper'],marker='undefined reference')
   controls=[('bad-lma','direct','MF_BAD_LMA','00000001',False),('bad-bss','direct','MF_BAD_BSS','00000003',False),('bad-helper','direct','MF_BAD_DIVISION','00000005',False),('direct-r8','direct','MF_BAD_REGISTER','0000002c',False),('direct-return','direct','MF_BAD_RETURN','FAIL MF fault',False),('preempt-r8','preemptive','MF_BAD_REGISTER','00000086',False),('preempt-exc','preemptive','MF_BAD_EXC_RETURN','FAIL MF',False),('preempt-s31','preemptive','MF_BAD_HIGH','0000009b',True),('bad-mask','preemptive','MF_BAD_MASK','000000a0',False),('canary','preemptive','MF_BAD_CANARY','0000006a',False),('stack-bound','preemptive','MF_BAD_STACK_BOUND','00000069',False),('owner-fault','preemptive','MF_OWNER_FAULT','MF unsafe owner fault retained resources',False),('sleep-stopped','sleep','MF_STOP_CLOCK','000000d5',False)]
   for name,mode,define,marker,fp in controls:target(name+'-'+cc,mode,cc,defines=[define],fp=fp,marker=marker)
   # Width/optimization/ISA dependency observations are compile-only objects.
   compiler=['arm-none-eabi-gcc'] if cc=='gcc' else ['clang','--target=arm-none-eabi']
   for cpu in ('cortex-m0','cortex-m4'):
    for opt in ('O0','O2'):
     name='dependencies-'+cc+'-'+cpu+'-'+opt;obj=work/(name+'.o')
     run(name,[*compiler,'-std=c17','-mcpu='+cpu,'-mthumb','-'+opt,'-ffreestanding','-fno-builtin','-c',ROOT/'labs/mcu_followthrough/target/dependencies.c','-o',obj]);undef=run(name+'-symbols',['arm-none-eabi-nm','-u',obj]);checks.append({'name':name+' 64-bit helpers explicit','passed':'__aeabi_uldivmod' in undef and '__atomic_fetch_add_8' in undef})
   obj=work/('forbidden-'+cc+'.o');run('compile-forbidden-'+cc,[*compiler,'-mcpu=cortex-m4','-mthumb','-ffreestanding','-c',ROOT/'labs/mcu_followthrough/target/forbidden.c','-o',obj]);run('reject-forbidden-'+cc,['arm-none-eabi-ld','-e','forbidden_dependency',obj,'-o',work/('forbidden-'+cc+'.elf')],1,'puts')
  run('retained-stage1',[sys.executable,ROOT/'labs/execution/targets/build.py','--target','cortex-m4-qemu','--source',ROOT/'assessor/execution','--lab',ROOT/'labs/execution','--build',work/'retained-stage1','--stage','cooperative','--run'],marker='PASS CMX Cortex-M cooperative',timeout=90)
  if not all(x['passed'] for x in checks):raise RuntimeError('dependency observation mismatch')
 except Exception as e:passed=False;error=str(e)
 else:passed=True;error=None
 paths=[p for base in ('labs/mcu_followthrough','assessor/mcu_followthrough') for p in (ROOT/base).rglob('*') if p.is_file() and p.suffix in ('.c','.h','.S','.py')]
 paths += [ROOT/'assessor/execution/cmx.c',ROOT/'assessor/execution/support.c',ROOT/'labs/execution/include/cmx.h',Path(__file__).resolve()]
 result={'passed':passed,'error':error,'rows':rows,'checks':checks,'source_sha256':{p.relative_to(ROOT).as_posix():sha(p) for p in paths},'artifacts':{p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},'scope':'Author execution only; explicit model/native/emulator boundaries, no physical timing or independent learner claim.'}
 (work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':passed,'error':error,'commands':len(rows)}));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
