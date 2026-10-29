"""Actual guarded storage/signature/record composition and MPS2 handoff."""
from pathlib import Path
import datetime,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/firmware_lifecycle/targets';ref=ROOT/'assessor/firmware_lifecycle/targets';tool=Path.home()/'cmastery-b06-tools/monocypher-4.0.3';manifest=ROOT/'research/v5/expansion/b06/monocypher_provider.json';build=ROOT/'build/b06-storage';build.mkdir(parents=True,exist_ok=True);rows=[];external=[]
 for item in json.loads(manifest.read_text())['files']:
  path=tool/item['path'];assert path.is_file() and hashlib.sha256(path.read_bytes()).hexdigest()==item['sha256'];external.append(dict(path=str(path),sha256=item['sha256']))
 def run(label,cmd,negative=None,expected=None):
  try:
   p=subprocess.run(list(map(str,cmd)),text=True,capture_output=True,timeout=90);out=p.stdout+p.stderr;ok=p.returncode==0 and (not expected or expected in out) if negative is None else p.returncode!=0 and negative in out;r=dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=ok)
  except (OSError,subprocess.TimeoutExpired) as e:r=dict(label=label,command=list(map(str,cmd)),passed=False,error=str(e));p=None
  rows.append(r);print(label,r['passed'],flush=True);return p
 includes=['-I'+str(lab),'-I'+str(tool/'src'),'-I'+str(tool/'src/optional')];common=[ref/'boot_pair.c',ref/'ecc_guard.c',ref/'flash_maintenance.c',ref/'h745_flash.c',tool/'src/monocypher.c',tool/'src/optional/monocypher-ed25519.c']
 for cc,flags,tag in [('gcc',['-O0'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O0'],'clang-debug'),('clang',['-O3'],'clang-release'),('gcc',['-O1','-g','-fsanitize=address,undefined'],'gcc-sanitize'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]:
  base=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-Wno-misleading-indentation','-fno-pie',*flags,*includes];objects=[]
  for i,src in enumerate(common):
   obj=build/(tag+'-dependency-'+str(i)+'.o');objects.append(obj);run(tag+' actual dependency compile '+str(i),[*base,'-c',src,'-o',obj])
  for mutant in [0,1,2,'stub']:
   exe=build/(tag+'-'+str(mutant));src=(lab if mutant=='stub' else ref)/'boot_storage.c';cmd=[*base,'-no-pie',src,lab/'boot_storage_test.c',*objects,'-o',exe]
   if mutant!='stub':cmd.append('-DBS_MUTANT='+str(mutant))
   p=run(tag+' build '+str(mutant),cmd)
   if p and p.returncode==0:run(tag+' execute '+str(mutant),[exe],None if mutant==0 else 'BS_CHECK:')
 for cpu,machine in [('cortex-m4','mps2-an386'),('cortex-m7','mps2-an500')]:
  for opt in ['-O0','-O2']:
   tag=cpu+opt;elf=build/(tag+'.elf');sources=[ref/'boot_storage.c',*common,ref/'h745_cache.c',ref/'ecc_guard_port.c',ref/'ecc_guard_port.S',ref/'boot_handoff.c',ref/'boot_handoff.S',lab/'boot_storage_guest.c',lab/'boot_storage_guest.S',lab/'boot_runtime.c'];p=run(tag+' complete struct lr_cpu image build',['arm-none-eabi-gcc','-std=c17',opt,'-mthumb','-mcpu='+cpu,'-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-DFB_CODE_BASE=0',*includes,*sources,'-nostdlib','-Wl,--gc-sections','-Wl,-T,'+str(lab/'boot_storage_guest.ld'),'-Wl,-Map,'+str(build/(tag+'.map')),'-o',elf])
   if p and p.returncode==0:
    p=run(tag+' actual struct lr_cpu storage-to-handoff',['qemu-system-arm','-M',machine,'-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf],expected='BS_GUEST_PASS:');rows.append(dict(label=tag+' admitted reset entry observed',passed=p is not None and p.returncode==0 and 'FB_GUEST_HANDOFF_PASS:' in p.stdout+p.stderr));p=run(tag+' undefined',['arm-none-eabi-nm','-u',elf]);rows.append(dict(label=tag+' helper closure',passed=p is not None and p.returncode==0 and not p.stdout.strip()));run(tag+' disassembly',['arm-none-eabi-objdump','-d',elf]);run(tag+' section sizes',['arm-none-eabi-size','-A',elf]);su='\n'.join(p.read_text() for p in build.glob(tag+'*.su'));rows.append(dict(label=tag+' stack records',stdout=su,passed=bool(su) and 'dynamic' not in su))
 files=[Path(__file__),manifest,*[lab/n for n in ['boot_storage.h','boot_storage.c','boot_storage_test.c','boot_storage_guest.c','boot_storage_guest.S','boot_storage_guest.ld','BOOT_STORAGE.md','boot_runtime.c','boot_pair.h','ecc_guard.h','flash_maintenance.h','h745_flash.h','h745_cache.h']],*[ref/n for n in ['boot_storage.c','BOOT_STORAGE_ANALYSIS.md','boot_pair.c','ecc_guard.c','ecc_guard_port.c','ecc_guard_port.S','flash_maintenance.c','h745_flash.c','h745_cache.c','boot_handoff.c','boot_handoff.S']]]
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},external_provider=external,checks=rows,limits=['MPS2 RAM-backed teaching slots and actual struct lr_cpu faults/handoff, not STM32 flash/clock/other-core physical execution','Fallback allowed only within trusted current floors/revocations; no CRC-based antirollback claim','External signature/hash primitive follows explicit FW-BOOT profile; no crypto-internals mastery claim','No independent learner, external reviewer or physical evidence inferred']);(ROOT/'research/v5/expansion/b06/boot_storage_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
