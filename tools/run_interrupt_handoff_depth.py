"""T29.03–.06: actual queue/mask/target composition; explicit finite evidence."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/interrupt_depth/handoff';REF=ROOT/'assessor/interrupt_depth/handoff';FRAME=ROOT/'assessor/interrupt_depth/frame/frame.c';HELP=ROOT/'assessor/execution/support.c';DMA=ROOT/'assessor/peripheral_depth/dma/dma.c';GENMC=Path('/home/frede/cmastery-modern-tools/genmc/build/bin/genmc')
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
MUTANTS={1:'xi_take(&q, &out) == XI_EMPTY',2:'xi_push(&q, &r) == XI_FULL',3:'service.dropped == UINT32_MAX',4:'xi_reserve(&q) == XI_BUSY',5:'mask.value == 1',6:'mask.value == 0x40'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 return [Path(__file__),FRAME,HELP,DMA,ROOT/'labs/peripheral_depth/dma/dma.h',ROOT/'assessor/execution/concurrent.c',ROOT/'labs/execution/include/cmx.h',ROOT/'labs/interrupt_depth/frame/frame.h',ROOT/'labs/interrupt_depth/frame/target.ld',*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/interrupt-handoff-depth'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1',TSAN_OPTIONS='halt_on_error=1')
 def run(label,cmd,expected=0,marker=None,empty=False):
  cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log')
  with log.open('w') as f:
   try:rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=120,env=env).returncode
   except (OSError,subprocess.TimeoutExpired) as ex:f.write(str(ex));rc=None
  text=log.read_text(errors='replace');ok=rc==expected and (not marker or marker in text) and (not empty or not text.strip()) and not any(x in text for x in ['runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer']);rows.append(dict(label=label,command=cmd,expected=expected,returncode=rc,marker=marker,empty=empty,passed=bool(ok),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(ok),flush=True);return ok,log
 for t in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','qemu-system-arm']:run(t+' version',[t,'--version'])
 run('environment',['uname','-a']);run('GenMC version',[GENMC,'--version'])
 includes=['-I'+str(LAB),'-I'+str(ROOT/'labs/interrupt_depth/frame')]
 for cc,opt,tag in PROFILES:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*includes]+(['-O1','-g'] if 'asan' in tag else [])
  for v in [0,1,2,3,4,5,6,'stub']:
   exe=w/(tag+'-'+str(v));label=tag+' '+str(v);ok,_=run(label+' compile',[cc,*flags,'-DXI_MUTANT='+str(0 if v=='stub' else v),(LAB if v=='stub' else REF)/'handoff.c',FRAME,LAB/'tests.c','-o',exe])
   if ok:run(label+' execute',[exe],0 if v==0 else 1,'XI_PASS' if v==0 else ('xi_init(&q, 0)' if v=='stub' else MUTANTS[v]))
  for case,source,marker in [('oracle','probe.c',None),('threads','threads.c','XI_THREADS_PASS owners=2 records=4000'),('locks','locks.c','XI_LOCK_PASS observed=5000')]:
   exe=w/(tag+'-'+case);ok,_=run(tag+' '+case+' compile',[cc,*flags,'-pthread',REF/'handoff.c',FRAME,REF/source,'-o',exe])
   if ok:
    ok,log=run(tag+' '+case+' execute',[exe],marker=marker)
    if ok and case=='oracle':run(tag+' independent oracle',[sys.executable,REF/'oracle.py',log],marker='XI_ORACLE_PASS queue=49152 mask=4096')
  exe=w/(tag+'-cmx');ok,_=run(tag+' full capacity compile',[cc,*flags,'-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+str(ROOT/'labs/execution/include'),ROOT/'assessor/execution/concurrent.c',REF/'full_capacity.c','-o',exe])
  if ok:run(tag+' full capacity execute',[exe],marker='XI_CMX_PASS full_capacity=1,2,4,8,16 records=1984')
  for mutant in [0,6]:
   exe=w/(tag+'-dma-'+str(mutant));label=tag+' DMA '+str(mutant);ok,_=run(label+' compile',[cc,*flags,'-I'+str(ROOT/'labs/peripheral_depth/dma'),'-DPD_MUTANT='+str(mutant),DMA,REF/'handoff.c',FRAME,REF/'dma_handoff.c','-o',exe])
   if ok:run(label+' execute',[exe],int(bool(mutant)),'out.sequence == 42' if mutant else 'XI_DMA_PASS')
  exe=w/(tag+'-wide');ok,_=run(tag+' wide host compile',[cc,*flags,LAB/'wide_atomic.c',LAB/'wide_test.c','-o',exe])
  if ok:run(tag+' wide host execute',[exe],marker='XI_WIDE_HOST_PASS records=3000 index_bits=64 capacity=3')
 for cc in ['gcc','clang']:
  for case,marker in [('threads','XI_THREADS_PASS owners=2 records=4000'),('locks','XI_LOCK_PASS observed=5000')]:
   exe=w/(cc+'-tsan-'+case);label=cc+' tsan '+case;ok,_=run(label+' compile',[cc,'-std=c17','-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie','-pthread',*includes,REF/'handoff.c',FRAME,REF/(case+'.c'),'-o',exe])
   if ok:run(label+' execute',[exe],marker=marker)
 model=w/'model.c';model.write_text('#include "'+str(REF/'queue.c')+'"\n#include "'+str(REF/'genmc.c')+'"\n')
 for m in [0,1,7,8]:run('GenMC '+str(m),[GENMC,'-rc11','-mode=verify','-disable-mm-detector','-disable-estimation','--','-std=c17','-O0',*includes,'-DXI_MUTANT='+str(m),model],42 if m else 0,'Error: Non-atomic race!' if m else 'Number of complete executions explored: 20')
 timing=[]
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   for instrument in [0,1]:
    for mutant in ([0,1,2] if cc=='arm-none-eabi-gcc' and opt=='0' and instrument==1 else [0]):
     label=cc+'-O'+opt+'-i'+str(instrument)+'-v'+str(mutant);dest=w/label
     ok,_=run(label+' build',[sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--source',REF/'handoff.c','--port',REF/'port.c','--wide',REF/'wide.c','--frame',FRAME,'--driver',REF/'target.c','--boot',REF/'target.S','--runtime',HELP,'--port-mutant',mutant,'--instrument',instrument],marker='XI_TARGET_BUILD_PASS')
     if ok:
      elf=dest/'image.elf';run(label+' closure',['arm-none-eabi-nm','-u',elf],empty=True);run(label+' assembly',['arm-none-eabi-objdump','-drwC',elf]);run(label+' size',['arm-none-eabi-size','-A',elf])
      ok,log=run(label+' execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-icount','shift=0,align=off,sleep=off','-kernel',elf],1 if mutant else 0,{0:'XI_TARGET_PASS',1:'primask() == 1',2:'basepri() == stricter'}[mutant])
      if ok and not mutant:
       interval_ok,interval_log=run(label+' interval oracle',[sys.executable,REF/'target_oracle.py',log,'--instrument',instrument],marker='"passed": true');timing.append(interval_log)
       run(label+' object audit',[sys.executable,REF/'audit_target.py',dest],marker='"passed": true')
  dest=w/(cc+'-wide.o');ok,_=run(cc+' widened compile',[cc,*(['--target=arm-none-eabi'] if cc=='clang' else []),'-std=c17','-O3','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-c',LAB/'wide_atomic.c','-o',dest])
  if ok:
   run(cc+' widened imports',['arm-none-eabi-nm','-u',dest],marker='__atomic_load_8')
   run(cc+' widened store imports',['arm-none-eabi-nm','-u',dest],marker='__atomic_store_8')
   run(cc+' widened link rejection',['arm-none-eabi-ld','-e','xi_wide_queue_push',dest,'-o',w/(cc+'-wide.elf')],1,'undefined reference to `__atomic_load_8')
 run('instrumentation comparison',[sys.executable,REF/'compare_timing.py',*timing],marker='"passed": true')
 run('interference model',[sys.executable,REF/'interference.py'],marker='"passed": true')
 for stub in [False,True]:run('public native '+('stub' if stub else 'reference'),[sys.executable,LAB/'run.py','--build',w/('public-stub' if stub else 'public-reference'),'--frame',FRAME,*([] if stub else ['--source',REF/'handoff.c'])],int(stub),'XI_CHECK:' if stub else 'XI_PASS')
 dest=w/'public-target-stub';ok,_=run('public target stub build',[sys.executable,LAB/'target_build.py','--build',dest,'--runtime',HELP],marker='XI_TARGET_BUILD_PASS')
 if ok:run('public target stub execute',['qemu-system-arm','-M','mps2-an386','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',dest/'image.elf'],1,'XI_TARGET_UNIMPLEMENTED')
 rows.append(dict(label='sources stable',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 result=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_input_sha256={str(GENMC):sha(GENMC)},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Author references and finite profiles; no external review, independent learner or physical observation.','GenMC RC11 selected O0 actual queue core, one producer/consumer, three attempted operations each; optimized IR promotion incompatibility is retained in development evidence, not silently counted.','QEMU virtual timer observations are not physical latency, WCET or real DMA/cache behavior.','Actual Pd/CMX/frame providers imported unchanged; learner artifact integration remains later assessment.'])
 out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(result,indent=2)+'\n');print(dict(passed=result['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not result['passed'])
if __name__=='__main__':raise SystemExit(main())
