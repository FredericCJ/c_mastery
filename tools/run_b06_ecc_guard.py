"""Precise-load policy, staged record scan and real Cortex-M fault entry/return."""
from pathlib import Path
import datetime,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/firmware_lifecycle/targets';ref=ROOT/'assessor/firmware_lifecycle/targets';build=ROOT/'build/b06-ecc';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(label,cmd,negative=None,expected=None,timeout=45):
  try:
   p=subprocess.run(list(map(str,cmd)),text=True,capture_output=True,timeout=timeout);out=p.stdout+p.stderr;ok=p.returncode==0 and (not expected or expected in out) if negative is None else p.returncode!=0 and negative in out;r=dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=ok)
  except (OSError,subprocess.TimeoutExpired) as e:r=dict(label=label,command=list(map(str,cmd)),passed=False,error=str(e));p=None
  rows.append(r);print(label,r['passed'],flush=True);return p
 for cc,flags,tag in [('gcc',['-O0'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O0'],'clang-debug'),('clang',['-O3'],'clang-release'),('gcc',['-O1','-g','-fsanitize=address,undefined'],'gcc-sanitize'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]:
  for mutant in [0,1,2,'stub']:
   exe=build/(tag+'-'+str(mutant));src=(lab if mutant=='stub' else ref)/'ecc_guard.c';cmd=[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-pie','-no-pie',*flags,'-I'+str(lab),src,ref/'flash_maintenance.c',ref/'h745_flash.c',lab/'ecc_guard_test.c','-o',exe]
   if mutant!='stub':cmd.append('-DEG_MUTANT='+str(mutant))
   p=run(tag+' build '+str(mutant),cmd)
   if p and p.returncode==0:run(tag+' execute '+str(mutant),[exe],None if mutant==0 else 'EG_CHECK:')
 for cpu,machine in [('cortex-m4','mps2-an386'),('cortex-m7','mps2-an500')]:
  for opt in ['-O0','-O2']:
   tag=cpu+opt;flags=['arm-none-eabi-gcc','-std=c17','-mthumb','-mcpu='+cpu,opt,'-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-I'+str(lab)]
   for variant in [0,1,2]:
    name=tag+'-'+str(variant);elf=build/(name+'.elf');sources=[ref/'ecc_guard.c',ref/'ecc_guard_port.c',ref/'ecc_guard_port.S',ref/'flash_maintenance.c',ref/'h745_flash.c',lab/'ecc_guard_guest.c',lab/'ecc_guard_guest.S',lab/'boot_runtime.c'];p=run(name+' build',[*flags,'-DEG_OUTSIDE='+str(variant),*sources,'-nostdlib','-Wl,--gc-sections','-Wl,-T,'+str(lab/'boot_guest.ld'),'-Wl,-e,EGReset','-Wl,-Map,'+str(build/(name+'.map')),'-o',elf])
    if p and p.returncode==0:
     run(name+' actual CPU exception execution',['qemu-system-arm','-M',machine,'-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf],expected='EG_GUEST_PASS:' if variant==0 else 'EG_GUEST_REFUSED:');p=run(name+' undefined',['arm-none-eabi-nm','-u',elf]);rows.append(dict(label=name+' helper closure',passed=p is not None and p.returncode==0 and not p.stdout.strip()));run(name+' handler disassembly',['arm-none-eabi-objdump','-d',elf]);run(name+' section sizes',['arm-none-eabi-size','-A',elf])
 files=[Path(__file__),*[lab/n for n in ['ecc_guard.h','ecc_guard.c','ecc_guard_test.c','ecc_guard_guest.c','ecc_guard_guest.S','ECC_GUARD.md','boot_guest.ld','boot_runtime.c','flash_maintenance.h','h745_flash.h','h745_cache.h']],*[ref/n for n in ['ecc_guard.c','ecc_guard_port.c','ecc_guard_port.S','ECC_GUARD_ANALYSIS.md','flash_maintenance.c','h745_flash.c']]]
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limits=['Actual Cortex-M fault entry/return in original MPS2 guest; faulting address is not STM32 ECC-controller emulation','Only basic MSP thread frame and exact precise data-load fault supported; unrelated faults deliberately fail-stop','Fault-contained snapshots and two-candidate recovery are original C implementations; physical H745 ECC/cache/prefetch/errata require separate board qualification','No external review, independent learner or physical claims']);(ROOT/'research/v5/expansion/b06/ecc_guard_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
