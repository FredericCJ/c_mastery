"""Actual M-profile MPU phase control; no peripheral or physical flash claim."""
from pathlib import Path
import datetime,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/firmware_lifecycle/targets';ref=ROOT/'assessor/firmware_lifecycle/targets';build=ROOT/'build/b06-program-mode';build.mkdir(parents=True,exist_ok=True);rows=[];ld=build/'guest.ld';ld.write_text('ENTRY(fp_reset)'+chr(10)+'SECTIONS { . = 0; .vectors : { KEEP(*(.vectors)) } .text : { *(.text*) *(.rodata*) } /DISCARD/ : { *(.comment) *(.ARM.exidx*) *(.ARM.extab*) } }'+chr(10))
 def run(label,cmd,negative=False):
  try:
   p=subprocess.run(list(map(str,cmd)),text=True,capture_output=True,timeout=30);out=p.stdout+p.stderr;ok=p.returncode!=0 and 'FP_BAD:' in out if negative else p.returncode==0;rows.append(dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=ok));print(label,ok,flush=True);return p
  except (OSError,subprocess.TimeoutExpired) as e:rows.append(dict(label=label,command=list(map(str,cmd)),passed=False,error=str(e)));return None
 for cpu,machine in [('cortex-m4','mps2-an386'),('cortex-m7','mps2-an500')]:
  for variant in [0,1,'stub']:
   tag=cpu+'-'+str(variant);elf=build/(tag+'.elf');src=(lab if variant=='stub' else ref)/'boot_program_mode.c';args=[] if variant=='stub' else ['-DFP_MUTANT='+str(variant)];p=run(tag+' build',['arm-none-eabi-gcc','-std=c17','-O2','-mthumb','-mcpu='+cpu,'-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-stack-protector','-I'+str(lab),*args,src,lab/'boot_program_guest.c',lab/'boot_program_guest.S','-nostdlib','-Wl,-T,'+str(ld),'-o',elf])
   if p and p.returncode==0:
    p=run(tag+' execute',['qemu-system-arm','-M',machine,'-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf],variant!=0)
    if variant==0:rows[-1]['passed']=rows[-1]['passed'] and p is not None and 'FP_PASS:' in p.stdout+p.stderr
 files=[Path(__file__),lab/'boot_program_mode.h',lab/'boot_program_mode.c',lab/'boot_program_guest.c',lab/'boot_program_guest.S',ref/'boot_program_mode.c'];d=dict(passed=all(x['passed'] for x in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limits=['Actual Cortex-M4/M7 emulator MPU protection and trusted phase on a32byte SRAM cell; not STM32 flash controller, voltage, reset or physical hardware','Standalone guest tests the same compiled phase helper; full firmware call placement is separately source/link-audited','No independent reviewer or learner inferred']);(ROOT/'research/v5/expansion/b06/program_mode_validation.json').write_text(json.dumps(d,indent=2)+chr(10));print(json.dumps(dict(passed=d['passed'],checks=len(rows))));return 0 if d['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
