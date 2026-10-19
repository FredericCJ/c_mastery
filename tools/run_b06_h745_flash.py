"""Selected H745 program-word register model, joint record and target code audit."""
from pathlib import Path
import subprocess,json,hashlib,datetime,struct,zlib
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/firmware_lifecycle/targets';ref=ROOT/'assessor/firmware_lifecycle/targets/h745_flash.c';build=ROOT/'build/b06-h745-flash';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(label,cmd,negative=False):
  p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=40);out=p.stdout+p.stderr;ok=p.returncode==0 if not negative else p.returncode!=0 and 'B06_FLASH_CHECK:' in out;rows.append(dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=ok));return p
 for cc,flags,tag in [('gcc',['-O0','-g'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]:
  for m in [0,1,2,3,'stub']:
   exe=build/(tag+'-'+str(m));src=lab/'h745_flash.c' if m=='stub' else ref;cmd=[cc,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(lab),src,lab/'h745_flash_test.c','-o',exe]
   if m!='stub':cmd+=['-DHF_MUTANT='+str(m)]
   if run(tag+' build '+str(m),cmd).returncode==0:
    run(tag+' execute '+str(m),[exe],m!=0)
    if m==0:
     p=run(tag+' record bytes',[exe,'--dump']);body=struct.pack('<IIII',0x314A3642,19,7,0)+bytes((i*13)%256 for i in range(64))+struct.pack('<IIII',0x08040000,0x08140000,9,12);expected=body+struct.pack('<IIIII',0x31433642,19,7,(~19)&0xffffffff,zlib.crc32(body))+bytes(12);rows.append(dict(label=tag+' independent struct/zlib oracle',passed=p.stdout.strip()==expected.hex()))
 for cpu in ['cortex-m7','cortex-m4']:
  for opt in ['-O0','-O3']:
   name=cpu+opt;objects=[];flags=['arm-none-eabi-gcc','-std=c17','-Wall','-Wextra','-Werror','-mthumb','-mcpu='+cpu,opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-I'+str(lab)]
   for src in [ref,lab/'h745_flash_port.c']:
    obj=build/(name+'-'+src.stem+'.o');objects.append(obj);run(name+' compile '+src.stem,[*flags,'-c',src,'-o',obj])
   elf=build/(name+'.elf');run(name+' freestanding link',[*flags,'-nostdlib','-Wl,-e,fh_start','-Wl,-Ttext=0x08100000',*objects,'-o',elf]);p=run(name+' undefined symbols',['arm-none-eabi-nm','-u',elf]);rows.append(dict(label=name+' no implicit helpers',passed=p.returncode==0 and not p.stdout.strip()));p=run(name+' disassembly',['arm-none-eabi-objdump','-d',elf]);rows.append(dict(label=name+' actual DSB adapter',passed=p.returncode==0 and 'dsb' in p.stdout));run(name+' section sizes',['arm-none-eabi-size','-A',elf]);su='\n'.join(p.read_text() for p in build.glob(name+'-*.su'));rows.append(dict(label=name+' bounded stack records',stdout=su,passed=bool(su) and 'dynamic' not in su))
 files=[Path(__file__),ref,ROOT/'assessor/firmware_lifecycle/targets/H745_FLASH_ANALYSIS.md',*[lab/x for x in ['h745_flash.h','h745_flash.c','h745_flash_test.c','h745_flash_port.c','H745_FLASH.md','h745_cache.h']]]
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['Host status/register and digital cut models; target code compiled/linked but not executed on H745','No erase, bank swap, clock/voltage setup, complete dual-image boot flow or ECC BusFault containment implemented here','Physical power-loss behavior and silicon errata not established','Old committed record must remain intact; uncertain programmed words are quarantined, never retried in place','Independent learner and external review not claimed']);(ROOT/'research/v5/expansion/b06/h745_flash_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
