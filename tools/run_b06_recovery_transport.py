"""Actual recovery frame/signature and UART register boundary."""
from pathlib import Path
import datetime,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/firmware_lifecycle/targets';ref=ROOT/'assessor/firmware_lifecycle/targets';tool=Path.home()/'cmastery-b06-tools/monocypher-4.0.3';manifest=ROOT/'research/v5/expansion/b06/monocypher_provider.json';build=ROOT/'build/b06-transport';build.mkdir(parents=True,exist_ok=True);rows=[];external=[]
 for item in json.loads(manifest.read_text())['files']:
  path=tool/item['path'];assert path.is_file() and hashlib.sha256(path.read_bytes()).hexdigest()==item['sha256'];external.append(dict(path=str(path),sha256=item['sha256']))
 def run(label,cmd,negative=None,expected=None):
  try:
   p=subprocess.run(list(map(str,cmd)),text=True,capture_output=True,timeout=90);out=p.stdout+p.stderr;ok=p.returncode==0 and (not expected or expected in out) if negative is None else p.returncode!=0 and negative in out;r=dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=ok)
  except (OSError,subprocess.TimeoutExpired) as e:r=dict(label=label,command=list(map(str,cmd)),passed=False,error=str(e));p=None
  rows.append(r);print(label,r['passed'],flush=True);return p
 includes=['-I'+str(lab),'-I'+str(tool/'src'),'-I'+str(tool/'src/optional')];common=[ref/'boot_pair.c',ref/'h745_flash.c',tool/'src/monocypher.c',tool/'src/optional/monocypher-ed25519.c']
 for cc,flags,tag in [('gcc',['-O0'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O0'],'clang-debug'),('clang',['-O3'],'clang-release'),('gcc',['-O1','-g','-fsanitize=address,undefined'],'gcc-sanitize'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]:
  base=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-Wno-misleading-indentation','-fno-pie',*flags,*includes];objects=[]
  for i,src in enumerate(common):
   obj=build/(tag+'-dependency-'+str(i)+'.o');objects.append(obj);run(tag+' actual dependency compile '+str(i),[*base,'-c',src,'-o',obj])
  for mutant in [0,1,2,'stub']:
   exe=build/(tag+'-'+str(mutant));src=(lab if mutant=='stub' else ref)/'recovery_transport.c';cmd=[*base,'-no-pie',src,lab/'recovery_transport_test.c',*objects,'-o',exe]
   if mutant!='stub':cmd.append('-DRT_MUTANT='+str(mutant))
   p=run(tag+' build '+str(mutant),cmd)
   if p and p.returncode==0:run(tag+' execute '+str(mutant),[exe],None if mutant==0 else 'RT_CHECK:')
 for opt in ['-O0','-O2']:
  tag='h745_m7'+opt;elf=build/(tag+'.elf');sources=[ref/'recovery_transport.c',*common,ref/'selected_startup.c',lab/'selected_startup_port.c',lab/'selected_reset.S',lab/'recovery_probe.c',lab/'boot_runtime.c',ref/'ecc_guard.c',ref/'ecc_guard_port.c',ref/'ecc_guard_port.S'];p=run(tag+' target receive admission link',['arm-none-eabi-gcc','-std=c17',opt,'-mthumb','-mcpu=cortex-m7','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections',*includes,*sources,'-nostdlib','-Wl,--gc-sections','-Wl,-T,'+str(lab/'selected_h745_m7.ld'),'-Wl,-Map,'+str(build/(tag+'.map')),'-o',elf])
  if p and p.returncode==0:
   p=run(tag+' unresolved symbols',['arm-none-eabi-nm','-u',elf]);rows.append(dict(label=tag+' helper closure',passed=p is not None and p.returncode==0 and not p.stdout.strip()));p=run(tag+' symbols',['arm-none-eabi-nm',elf]);rows.append(dict(label=tag+' excludes signing',passed=p is not None and not any(x in p.stdout for x in ['crypto_ed25519_sign','crypto_ed25519_key_pair'])));run(tag+' disassembly',['arm-none-eabi-objdump','-d',elf]);run(tag+' sizes',['arm-none-eabi-size','-A',elf]);su='\n'.join(x.read_text() for x in build.glob(tag+'*.su'));rows.append(dict(label=tag+' stack records',stdout=su,passed=bool(su) and 'dynamic' not in su))
 files=[Path(__file__),manifest,*[lab/n for n in ['recovery_transport.h','recovery_transport.c','recovery_transport_test.c','recovery_probe.c','RECOVERY_TRANSPORT.md','selected_startup.h','selected_startup_port.c','selected_reset.S','selected_h745_m7.ld','boot_runtime.c','boot_pair.h','boot_storage.h','ecc_guard.h','flash_maintenance.h','h745_flash.h','h745_cache.h']],*[ref/n for n in ['recovery_transport.c','RECOVERY_TRANSPORT_ANALYSIS.md','selected_startup.c','boot_pair.c','h745_flash.c','ecc_guard.c','ecc_guard_port.c','ecc_guard_port.S']]]
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},external_provider=external,checks=rows,limits=['Host frame/crypto and UART register execution; linked target probe not actual UART waveform/board operation','No persistent flash writer or trial/health commit in this transport-only campaign','External signature/hash primitive follows explicit FW-BOOT profile; no crypto-internals mastery claim','No independent learner, external reviewer or physical evidence inferred']);(ROOT/'research/v5/expansion/b06/recovery_transport_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
