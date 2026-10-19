"""Selected F439/H745 maintenance model, semantic controls and target audit."""
from pathlib import Path
import datetime,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/firmware_lifecycle/targets';ref=ROOT/'assessor/firmware_lifecycle/targets';build=ROOT/'build/b06-maintenance';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(label,cmd,negative=False):
  p=subprocess.run(list(map(str,cmd)),text=True,capture_output=True,timeout=90);out=p.stdout+p.stderr;ok=p.returncode==0 if not negative else p.returncode!=0 and 'FM_CHECK:' in out;rows.append(dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=ok));print(label,ok,flush=True);return p
 profiles=[('gcc',['-O0'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O0'],'clang-debug'),('clang',['-O3'],'clang-release'),('gcc',['-O1','-g','-fsanitize=address,undefined'],'gcc-sanitize'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]
 for cc,flags,tag in profiles:
  for mutant in [0,1,2,3,'stub']:
   exe=build/(tag+'-'+str(mutant));src=(lab if mutant=='stub' else ref)/'flash_maintenance.c';cmd=[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-pie','-no-pie',*flags,'-I'+str(lab),src,ref/'h745_flash.c',lab/'flash_maintenance_test.c','-o',exe]
   if mutant!='stub':cmd.append('-DFM_MUTANT='+str(mutant))
   if run(tag+' build '+str(mutant),cmd).returncode==0:run(tag+' execute '+str(mutant),[exe],mutant!=0)
 for cpu in ['cortex-m4','cortex-m7']:
  for opt in ['-O0','-O3']:
   tag=cpu+opt;flags=['arm-none-eabi-gcc','-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-mthumb','-mcpu='+cpu,opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-I'+str(lab)];objects=[]
   for src in [ref/'flash_maintenance.c',ref/'h745_flash.c',lab/'flash_maintenance_port.c']:
    obj=build/(tag+'-'+src.stem+'.o');objects.append(obj);run(tag+' compile '+src.stem,[*flags,'-c',src,'-o',obj])
   elf=build/(tag+'.elf');run(tag+' link',[*flags,'-nostdlib','-Wl,-e,fm_erase','-Wl,-Ttext=0x08100000',*objects,'-o',elf]);p=run(tag+' undefined symbols',['arm-none-eabi-nm','-u',elf]);rows.append(dict(label=tag+' no implicit target helpers',passed=p.returncode==0 and not p.stdout.strip()));p=run(tag+' disassembly',['arm-none-eabi-objdump','-d',elf]);rows.append(dict(label=tag+' actual DSB ISB adapters',passed=p.returncode==0 and 'dsb' in p.stdout and 'isb' in p.stdout));run(tag+' sections',['arm-none-eabi-size','-A',elf]);su='\n'.join(p.read_text() for p in build.glob(tag+'-*.su'));rows.append(dict(label=tag+' bounded stack records',stdout=su,passed=bool(su) and 'dynamic' not in su))
 files=[Path(__file__),*[lab/n for n in ['flash_maintenance.h','flash_maintenance.c','flash_maintenance_port.c','flash_maintenance_test.c','FLASH_MAINTENANCE.md','h745_flash.h','h745_cache.h']],ref/'flash_maintenance.c',ref/'h745_flash.c',ref/'FLASH_MAINTENANCE_ANALYSIS.md']
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limits=['Actual target source compiled/linked, not executed on STM32','40 digital interruption histories under sector-local fault model, not physical power loss or exhaustive cell states','Selected last-two-sector profiles only; bank2/DB1M, ECC BusFault containment, signed boot/startup and silicon errata remain separate','Original author assessment; no independent learner, external review or physical evidence inferred']);(ROOT/'research/v5/expansion/b06/flash_maintenance_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
