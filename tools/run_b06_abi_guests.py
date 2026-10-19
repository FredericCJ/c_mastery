"""Disposable no-network/no-disk ABI and privileged translation guests."""
from pathlib import Path
import subprocess,hashlib,json,datetime,tempfile,argparse,shutil
ROOT=Path(__file__).resolve().parents[1]
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--tool-root',type=Path,default=Path.home()/'cmastery-b06-tools');ap.add_argument('--report',default='research/v5/expansion/b06/abi_guest_validation.json');a=ap.parse_args();records=[];pins={}
 def run(cmd,label,expected=0,marker=None):
  try:
   r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30);v=dict(label=label,command=list(map(str,cmd)),returncode=r.returncode,stdout=r.stdout,stderr=r.stderr,passed=r.returncode==expected and (marker is None or marker in r.stdout+r.stderr),expected=expected,marker=marker)
  except (OSError,subprocess.TimeoutExpired) as e:v=dict(label=label,command=list(map(str,cmd)),passed=False,error=str(e))
  records.append(v);return v
 for p in list((ROOT/'assessor/architecture_ports/guests').glob('*'))+list((ROOT/'assessor/architecture_ports/targets').glob('*.S'))+[Path(__file__)]:pins[p.relative_to(ROOT).as_posix()]=hashlib.sha256(p.read_bytes()).hexdigest()
 lld=a.tool_root/'usr/bin/ld.lld-21'
 run(['clang','--version'],'Clang identity');run([lld,'--version'],'linker identity')
 with tempfile.TemporaryDirectory(prefix='cm-b06-ports-') as td:
  w=Path(td)
  for arch,target in [('rv64','riscv64-none-elf'),('aa64','aarch64-none-elf')]:
   src=ROOT/'assessor/architecture_ports/guests'/(arch+'.S');ld=src.with_suffix('.ld');context=ROOT/'assessor/architecture_ports/targets'/(arch+'_context.S');qemu=(a.tool_root/'usr/bin/qemu-system-riscv64') if arch=='rv64' else Path(shutil.which('qemu-system-aarch64') or 'qemu-system-aarch64');run([qemu,'--version'],arch+' emulator identity')
   for mutation in ([0,1,2,3,4,5,6] if arch=='aa64' else [0,1,2,4]):
    c=w/'context.S';s=w/'guest.S';c.write_text(context.read_text().replace('ld s0,16(a0)','li s0,0') if mutation==1 and arch=='rv64' else context.read_text().replace('ldp x19,x20,[x0,#0]','ldp x19,x20,[x0,#0]\n mov x19,#0') if mutation==1 else context.read_text());s.write_text(src.read_text().replace('la t1,page_b','la t1,page_a') if mutation==2 and arch=='rv64' else src.read_text().replace('ldr x1,=page_b','ldr x1,=page_a') if mutation==2 else src.read_text());elf=w/(arch+str(mutation)+'.elf');label=arch+' mutation='+str(mutation)
    cmd=['clang','--target='+target,'-ffreestanding','-nostdlib','-fuse-ld='+str(lld),'-Wl,-T,'+str(ld),'-Wl,-Map,'+str(w/'target.map')]
    if arch=='rv64':cmd+=['-march=rv64imac','-mabi=lp64']
    extra=[]
    aggregate=w/'aggregate.S';aggregate.write_text((src.parent/(arch+'_aggregate.S')).read_text().replace('add x8,sp,#16','add x0,sp,#16') if mutation==4 and arch=='aa64' else (src.parent/(arch+'_aggregate.S')).read_text().replace('addi a0,sp,8','addi a7,sp,8') if mutation==4 else (src.parent/(arch+'_aggregate.S')).read_text());extra += [aggregate,src.parent/'aggregate.c']
    if arch=='aa64':
     simd=w/'simd.S';simd.write_text((src.parent/'aa64_simd.S').read_text().replace('ldp q8,q9,[sp,#128]','ldp d8,d9,[sp,#128]') if mutation==3 else (src.parent/'aa64_simd.S').read_text());extra.append(simd)
     tasks=w/'tasks.S';original=(src.parent/'aa64_tasks.S').read_text();tasks.write_text(original.replace('ldr x1,=task_b_top','ldr x1,=task_a_top') if mutation==5 else original.replace('cbnz x2,task_refuse','nop') if mutation==6 else original);extra.append(tasks)
    if not run(cmd+[s,c,*extra,'-o',elf],label+' link')['passed']:continue
    r=run(['nm','-u',elf],label+' undefined symbols');records.append(dict(label=label+' closure',passed=r['passed'] and not r.get('stdout','').strip()))
    run(['readelf','-h','-l',elf],label+' ELF mapping')
    command=[qemu,'-M','virt','-nographic','-monitor','none','-nic','none','-m','128M','-kernel',elf]
    if arch=='rv64':command+=['-bios','none']
    else:command+=['-cpu','cortex-a53','-semihosting-config','enable=on,target=native']
    run(command,label+' execution',0 if mutation==0 else 1,('B06_RV_' if arch=='rv64' else 'B06_AA_')+('PASS' if mutation==0 else 'FAIL'))
 report=dict(passed=all(r['passed'] for r in records),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256=pins,checks=records,scope='Author guest execution; actual original assembly on QEMU virt; no host kernel changes, disks or network',limitations=['One hart/core; AArch64 low64 ABI versus all32 fixed128-bit vector registers plus FPCR/FPSR tested. No SVE/SME or asynchronous full integer-frame proof','Two page-table roots, one remapped data page, synchronous service and two deliberate page faults; not a complete operating system','Emulator architectural observations do not validate physical cache/timing behavior','Learner delayed transfer and independent assessment not performed']);out=ROOT/a.report;out.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(records),failed=[r for r in records if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
