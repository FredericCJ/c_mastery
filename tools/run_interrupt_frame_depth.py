"""T29.01/.02 exact semantic, priority, architectural execution and stack campaign."""
from pathlib import Path
import argparse,datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/interrupt_depth/frame';REF=ROOT/'assessor/interrupt_depth/frame';HELP=ROOT/'assessor/execution/support.c'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():return [Path(__file__),HELP,*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/interrupt-frame-depth'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}
 def run(label,cmd,expected=0,marker=None,empty=False):
  cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log')
  with log.open('w') as f:
   try:rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=120).returncode
   except (OSError,subprocess.TimeoutExpired) as ex:f.write(str(ex));rc=None
  text=log.read_text(errors='replace');ok=rc==expected and (not marker or ''.join(marker.split()) in ''.join(text.split())) and (not empty or not text.strip()) and not any(x in text for x in ['runtime error:','ERROR: AddressSanitizer']);rows.append(dict(label=label,command=cmd,expected=expected,returncode=rc,marker=marker,empty=empty,passed=bool(ok),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(ok),flush=True);return ok,log
 for t in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','qemu-system-arm']:run(t+' version',[t,'--version'])
 run('environment',['uname','-a'])
 mutants={1:'valid ? rc == XF_OK : rc == XF_UNSUPPORTED',2:'xf_frame(words, 8, 0x20000000u',3:'!f.fp_materialized',4:'xf_pick(cfg, irq, 3, 0x60',5:'xf_pick(cfg, irq, 3, 0x60',6:'xf_source(&irq[0], 0)'}
 profiles=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
 for cc,opt,tag in profiles:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie','-I'+str(LAB)]+(['-O1','-g'] if 'asan' in tag else [])
  for v in [0,1,2,3,4,5,6,'stub']:
   exe=w/(tag+'-'+str(v));label=tag+' '+str(v);ok,_=run(label+' compile',[cc,*flags,'-DXF_MUTANT='+str(0 if v=='stub' else v),(LAB if v=='stub' else REF)/'frame.c',LAB/'tests.c','-o',exe])
   if ok:run(label+' execute',[exe],0 if v==0 else 1,'XF_PASS priority_cells=12288' if v==0 else ('xf_frame(words, 8' if v=='stub' else mutants[v]))
  exe=w/(tag+'-oracle');ok,_=run(tag+' oracle compile',[cc,*flags,REF/'frame.c',REF/'probe.c','-o',exe])
  if ok:
   ok,log=run(tag+' oracle data',[exe])
   if ok:run(tag+' independent oracle',[sys.executable,REF/'oracle.py',log],marker='XF_ORACLE_PASS priority=12288 selection=41472')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   for fp in [0,1]:
    for v in ([0,1,2,3] if cc=='arm-none-eabi-gcc' and opt=='0' and fp==0 else [0]):
     label=cc+'-O'+opt+'-fp'+str(fp)+'-v'+str(v);dest=w/label
     ok,_=run(label+' build',[sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--source',REF/'frame.c','--driver',REF/'target.c','--boot',REF/'target.S','--runtime',HELP,'--mutant',v,*(['--fp-callee',REF/'fp_callee.c'] if fp else [])],marker='XF_TARGET_BUILD_PASS')
     if ok:
      elf=dest/'image.elf';run(label+' closure',['arm-none-eabi-nm','-u',elf],empty=True);run(label+' symbols',['arm-none-eabi-nm','-S',elf]);run(label+' assembly',['arm-none-eabi-objdump','-drwC',elf]);run(label+' size',['arm-none-eabi-size','-A',elf])
      ok,log=run(label+' execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-d','int','-D',dest/'interrupts.log','-kernel',elf],0 if v==0 else 1,'XF_TARGET_PASS' if v==0 else {1:'a->frame.pc >= (uint32_t)(uintptr_t)xf_interrupted_begin',2:'a->frame.alignment_word == pad',3:'r->frame.fp_materialized =='}[v])
      if ok and v==0:
       run(label+' architectural oracle',[sys.executable,REF/'target_oracle.py',log,dest/'interrupts.log',fp],marker='XF_TARGET_ORACLE_PASS cases=16 tailchains=24')
       run(label+' stack ledger',[sys.executable,REF/'stack_ledger.py',dest,'--fp',fp],marker='"passed": true')
 run('late arrival model',[sys.executable,REF/'late_arrival.py'],marker='"passed": true')
 for stub in [False,True]:run('public native '+('stub' if stub else 'reference'),[sys.executable,LAB/'run.py','--build',w/('public-stub' if stub else 'public-reference'),*([] if stub else ['--source',REF/'frame.c'])],int(stub),'XF_CHECK:' if stub else 'XF_PASS')
 dest=w/'public-target-stub';ok,_=run('public target stub build',[sys.executable,LAB/'target_build.py','--build',dest,'--runtime',HELP],marker='XF_TARGET_BUILD_PASS')
 if ok:run('public target stub execute',['qemu-system-arm','-M','mps2-an386','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',dest/'image.elf'],1,'XF_TARGET_UNIMPLEMENTED')
 rows.append(dict(label='sources stable',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 result=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Finite profiles/inputs, author references, no independent learner or external review.','MPS2 architectural execution is not STM32 physical timing; late arrival is an explicit architectural phase model, not a timed physical injection.','Static stack ledger covers the declared direct-call graph and two-level experiment; no arbitrary faults or future IRQ additions.'])
 out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(result,indent=2)+'\n');print(dict(passed=result['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not result['passed'])
if __name__=='__main__':raise SystemExit(main())
