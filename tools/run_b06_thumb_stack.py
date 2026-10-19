"""Actual Cortex-M0 stack-admission experiment with semantically failing controls."""
from pathlib import Path
import subprocess,json,hashlib,datetime
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/architecture_ports/targets/thumb_stack';ref=ROOT/'assessor/architecture_ports/targets/thumb_stack/probe.S';build=ROOT/'build/b06-thumb-stack';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(label,cmd,expected=0,marker=None):
  p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30);rows.append(dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=p.returncode==expected and (marker is None or marker in p.stdout+p.stderr)));return p
 run('compiler identity',['arm-none-eabi-gcc','--version']);run('emulator identity',['qemu-system-arm','--version'])
 for name,src,defines,expect in [('reference',ref,[],0),('alignment4',ref,['-DTS_MUTANT=1'],1),('excludes-exact',ref,['-DTS_MUTANT=2'],1),('learner-stub',lab/'probe.S',[],1)]:
  elf=build/(name+'.elf');cmd=['arm-none-eabi-gcc','-mcpu=cortex-m0','-mthumb','-std=c17','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-nostdlib','-Wl,-T,'+str(lab/'link.ld'),'-Wl,-Map,'+str(build/(name+'.map')),*defines,src,lab/'start.c','-o',elf]
  if run(name+' link',cmd).returncode==0:
   p=run(name+' undefined references',['arm-none-eabi-nm','-u',elf]);rows.append(dict(label=name+' helper closure',passed=not p.stdout.strip()));run(name+' execute',['qemu-system-arm','-M','microbit','-cpu','cortex-m0','-nographic','-monitor','none','-serial','none','-nic','none','-semihosting-config','enable=on,target=native','-kernel',elf],expected=expect,marker='B06_THUMB_STACK_'+('PASS' if expect==0 else 'FAIL'))
   if name=='reference':run('reference disassembly',['arm-none-eabi-objdump','-d',elf]);run('reference section sizes',['arm-none-eabi-size','-A',elf]);run('reference stack addresses',['arm-none-eabi-nm','-n',elf])
 files=[Path(__file__),ref,*[p for p in lab.iterdir() if p.is_file()]];report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['Maskable interrupts disabled; exact exhaustion provides no exception or nested-call frame headroom; leaf alignment check uses no stack storage','Guard sentinels observe bounded test writes, not MPU isolation or detection of every transient corruption','QEMU Cortex-M0 microbit execution, not a physical board or timing measurement','Original learner stub remains unsolved; author reference results are not learner acceptance']);out=ROOT/'research/v5/expansion/b06/thumb_stack_validation.json';out.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
