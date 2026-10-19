"""H745 cache ownership host controls and actual M7/M4 generated-code audit."""
from pathlib import Path
import subprocess,json,hashlib,datetime
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/firmware_lifecycle/targets';ref=ROOT/'assessor/firmware_lifecycle/targets/h745_cache.c';build=ROOT/'build/b06-h745-cache';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(label,cmd,negative=False):
  p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=40);out=p.stdout+p.stderr;ok=p.returncode==0 if not negative else p.returncode!=0 and 'B06_H745_CHECK:' in out;rows.append(dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=ok));return p
 for cc,flags,tag in [('gcc',['-O0','-g'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]:
  for m in [0,1,2,3,'stub']:
   exe=build/(tag+'-'+str(m));src=lab/'h745_cache.c' if m=='stub' else ref
   cmd=[cc,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(lab),src,lab/'h745_cache_test.c','-o',exe]
   if m!='stub':cmd+=['-DHC_MUTANT='+str(m)]
   if run(tag+' build '+str(m),cmd).returncode==0:run(tag+' execute '+str(m),[exe],m!=0)
 for cpu,m7 in [('cortex-m7',1),('cortex-m4',0)]:
  for opt in ['-O0','-O3']:
   name=cpu+opt;objects=[];flags=['arm-none-eabi-gcc','-std=c17','-Wall','-Wextra','-Werror','-mthumb','-mcpu='+cpu,opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-I'+str(lab),'-DFW_H745_M7='+str(m7)]
   for src in [ref,lab/'h745_port.c']:
    obj=build/(name+'-'+src.stem+'.o');objects.append(obj);run(name+' compile '+src.stem,[*flags,'-c',src,'-o',obj])
   elf=build/(name+'.elf');run(name+' freestanding link',[*flags,'-nostdlib','-Wl,-e,fw_h745_step','-Wl,-Ttext=0x08000000',*objects,'-o',elf])
   p=run(name+' undefined symbols',['arm-none-eabi-nm','-u',elf]);rows.append(dict(label=name+' no implicit helpers',passed=p.returncode==0 and not p.stdout.strip()))
   p=run(name+' disassembly',['arm-none-eabi-objdump','-d',elf]);code=p.stdout.lower();rows.append(dict(label=name+' mask and completion instructions',passed=all(x in code for x in ['dsb','mrs','primask','cpsid','msr']) and (not m7 or 'isb' in code)))
   run(name+' section sizes',['arm-none-eabi-size','-A',elf]);su='\n'.join(p.read_text() for p in build.glob(name+'-*.su'));rows.append(dict(label=name+' bounded stack records',stdout=su,passed=bool(su) and 'dynamic' not in su))
 files=[Path(__file__),ref,ROOT/'assessor/firmware_lifecycle/targets/H745_CACHE_ANALYSIS.md',*[lab/x for x in ['h745_cache.h','h745_cache.c','h745_cache_test.c','h745_port.c','H745_CACHE.md']]]
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['Host trace/state tests and actual M7/M4 compilation/disassembly, not target execution','Selected target requires clock/MPU/linker/cold-start integration; this adapter does not configure those or persist a joint image commit','No physical H745 cache, DMA, power-cut, asynchronous fault or independent learner evidence','No NMI/fault reentrancy, arbitrary same-core HSEM users or dirty aliases permitted']);(ROOT/'research/v5/expansion/b06/h745_cache_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
