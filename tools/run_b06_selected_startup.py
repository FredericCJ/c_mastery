"""Cold-reset model and complete selected-address H745 bootstrap link audit."""
from pathlib import Path
import datetime,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/firmware_lifecycle/targets';ref=ROOT/'assessor/firmware_lifecycle/targets';tool=Path.home()/'cmastery-b06-tools/monocypher-4.0.3';manifest=ROOT/'research/v5/expansion/b06/monocypher_provider.json';build=ROOT/'build/b06-startup';build.mkdir(parents=True,exist_ok=True);rows=[];external=[]
 for item in json.loads(manifest.read_text())['files']:
  p=tool/item['path'];assert hashlib.sha256(p.read_bytes()).hexdigest()==item['sha256'];external.append(dict(path=str(p),sha256=item['sha256']))
 def run(label,cmd,negative=None):
  try:
   p=subprocess.run(list(map(str,cmd)),text=True,capture_output=True,timeout=90);out=p.stdout+p.stderr;ok=(p.returncode==0) if negative is None else (p.returncode!=0 and negative in out);rows.append(dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=ok))
  except (OSError,subprocess.TimeoutExpired) as e:rows.append(dict(label=label,command=list(map(str,cmd)),passed=False,error=str(e)));p=None
  print(label,rows[-1]['passed'],flush=True);return p
 for cc,flags,tag in [('gcc',['-O0'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O0'],'clang-debug'),('clang',['-O3'],'clang-release'),('gcc',['-O1','-g','-fsanitize=address,undefined'],'gcc-sanitize'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]:
  for mutant in [0,1,2,'stub']:
   exe=build/(tag+'-'+str(mutant));src=(lab if mutant=='stub' else ref)/'selected_startup.c';cmd=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-Wno-misleading-indentation','-fno-pie','-no-pie',*flags,'-I'+str(lab),src,lab/'selected_startup_test.c','-o',exe]
   if mutant!='stub':cmd.append('-DST_MUTANT='+str(mutant))
   p=run(tag+' compile '+str(mutant),cmd)
   if p and p.returncode==0:run(tag+' execute '+str(mutant),[exe],None if mutant==0 else 'ST_CHECK:')
 includes=['-I'+str(lab),'-I'+str(tool/'src'),'-I'+str(tool/'src/optional')]
 common=[ref/'selected_startup.c',lab/'selected_startup_port.c',lab/'selected_reset.S',lab/'boot_runtime.c',ref/'ecc_guard.c',ref/'ecc_guard_port.c',ref/'ecc_guard_port.S']
 actual=[ref/'selected_boot.c',ref/'boot_storage.c',ref/'boot_pair.c',ref/'h745_flash.c',ref/'flash_maintenance.c',ref/'h745_cache.c',lab/'h745_port.c',ref/'boot_handoff.c',ref/'boot_handoff.S',tool/'src/monocypher.c',tool/'src/optional/monocypher-ed25519.c']
 for name,cpu,core,ram in [('h745_m7','cortex-m7',0,'0x20000000'),('h745_m4','cortex-m4',1,'0x30000000'),('f439','cortex-m4',0,'0x20000000')]:
  for opt in ['-O0','-O2']:
   tag=name+opt;elf=build/(tag+'.elf');sources=[*common,*([lab/'selected_f439_probe.c'] if name=='f439' else actual)];p=run(tag+' complete selected address link',['arm-none-eabi-gcc','-std=c17',opt,'-mcpu='+cpu,'-mthumb','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-DST_CORE='+str(core),'-DFW_H745_M7='+str(1-core),'-DFB_RAM_BASE='+ram,*includes,*sources,'-nostdlib','-Wl,--gc-sections','-Wl,-T,'+str(lab/('selected_'+name+'.ld')),'-Wl,-Map,'+str(build/(tag+'.map')),'-o',elf])
   if p and p.returncode==0:
    p=run(tag+' unresolved symbols',['arm-none-eabi-nm','-u',elf]);rows.append(dict(label=tag+' helper closure',passed=p is not None and not p.stdout.strip()));p=run(tag+' symbols',['arm-none-eabi-nm',elf]);rows.append(dict(label=tag+' excludes signing',passed=p is not None and not any(x in p.stdout for x in ['crypto_ed25519_sign','crypto_ed25519_key_pair'])));p=run(tag+' disassembly',['arm-none-eabi-objdump','-d',elf]);rows.append(dict(label=tag+' barriers and reset',passed=p is not None and all(x in p.stdout for x in ['<st_reset>','dsb','isb'])));run(tag+' sizes',['arm-none-eabi-size','-A',elf]);su='\n'.join(x.read_text() for x in build.glob(tag+'*.su'));rows.append(dict(label=tag+' stack records',stdout=su,passed=bool(su) and 'dynamic' not in su))
 files=[Path(__file__),manifest,*common,*[x for x in actual if x.is_relative_to(ROOT)],*[lab/n for n in ['selected_startup.h','selected_startup.c','selected_startup_test.c','selected_boot.c','selected_f439_probe.c','selected_h745_m7.ld','selected_h745_m4.ld','selected_f439.ld','SELECTED_STARTUP.md','boot_storage.h','boot_pair.h','ecc_guard.h','flash_maintenance.h','h745_flash.h','h745_cache.h']],ref/'SELECTED_STARTUP_ANALYSIS.md']
 report=dict(passed=all(x['passed'] for x in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},external_provider=external,checks=rows,limits=['Selected target images linked and disassembled, not run on STM32; register model is not actual RCC/dual-core timing','Cold reset/provisioned BOOT pin and fixed boot options required; no option programming performed','F439 probe is single-core startup only, not complete update deployment','No recovery transport or new persistent commit in this boot-only wrapper','No independent reviewer, learner or physical observation inferred']);(ROOT/'research/v5/expansion/b06/selected_startup_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[x['label'] for x in rows if not x['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
