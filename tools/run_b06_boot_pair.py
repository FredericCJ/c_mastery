"""Source-pinned permitted crypto boundary, actual Cortex-M guest and target audit."""
from pathlib import Path
import argparse,datetime,hashlib,json,subprocess,urllib.request
ROOT=Path(__file__).resolve().parents[1]
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--fetch-provider',action='store_true');args=ap.parse_args();lab=ROOT/'labs/firmware_lifecycle/targets';ref=ROOT/'assessor/firmware_lifecycle/targets';build=ROOT/'build/b06-boot';build.mkdir(parents=True,exist_ok=True);tool=Path.home()/'cmastery-b06-tools/monocypher-4.0.3';manifest=ROOT/'research/v5/expansion/b06/monocypher_provider.json';provider=json.loads(manifest.read_text());rows=[];external=[]
 for row in provider['files']:
  dest=tool/row['path'];url='https://raw.githubusercontent.com/LoupVaillant/Monocypher/4.0.3/'+row['path']
  if not dest.exists() and args.fetch_provider:dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(urllib.request.urlopen(url,timeout=30).read())
  if not dest.exists() or hashlib.sha256(dest.read_bytes()).hexdigest()!=row['sha256']:raise ValueError('Missing/mismatched pinned provider '+str(dest))
  external.append(dict(path=str(dest),url=url,sha256=row['sha256']))
 def run(label,cmd,negative=None,expected=None,timeout=90):
  try:
   p=subprocess.run(list(map(str,cmd)),text=True,capture_output=True,timeout=timeout);out=p.stdout+p.stderr;ok=(p.returncode==0 and (not expected or expected in out)) if negative is None else p.returncode!=0 and negative in out;r=dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=ok)
  except (OSError,subprocess.TimeoutExpired) as e:r=dict(label=label,command=list(map(str,cmd)),passed=False,error=str(e));p=None
  rows.append(r);print(label,r['passed'],flush=True);return p
 for cc in ['gcc','clang','arm-none-eabi-gcc','qemu-system-arm']:run(cc+' version',[cc,'--version'])
 includes=['-I'+str(lab),'-I'+str(tool/'src'),'-I'+str(tool/'src/optional')];crypto=[tool/'src/monocypher.c',tool/'src/optional/monocypher-ed25519.c'];core=[ref/'h745_flash.c',ref/'h745_cache.c'];profiles=[('gcc',['-O0'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O0'],'clang-debug'),('clang',['-O3'],'clang-release'),('gcc',['-O1','-g','-fsanitize=address,undefined'],'gcc-sanitize'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]
 for cc,flags,tag in profiles:
  base=[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-pie','-no-pie',*flags,*includes]
  for mutant in [0,1,2,3,'stub']:
   exe=build/(tag+'-'+str(mutant));src=(lab if mutant=='stub' else ref)/'boot_pair.c';cmd=[*base,src,*core,lab/'boot_pair_test.c',*crypto,'-o',exe]
   if mutant!='stub':cmd.append('-DFB_MUTANT='+str(mutant))
   p=run(tag+' build '+str(mutant),cmd)
   if p and p.returncode==0:run(tag+' execute '+str(mutant),[exe],None if mutant==0 else 'FB_CHECK:')
  exe=build/(tag+'-composition');p=run(tag+' actual pair composition build',[*base,ref/'boot_pair.c',*core,ref/'flash_maintenance.c',lab/'boot_recovery_test.c',*crypto,'-o',exe])
  if p and p.returncode==0:run(tag+' actual pair composition execute',[exe],expected='FB_COMPOSITION_PASS:')
 for cpu,machine in [('cortex-m4','mps2-an386'),('cortex-m7','mps2-an500')]:
  for opt in ['-O0','-O2']:
   tag=cpu+opt;flags=['arm-none-eabi-gcc','-std=c17','-mthumb','-mcpu='+cpu,opt,'-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections',*includes]
   objects=[]
   for i,src in enumerate([ref/'boot_pair.c',*core,ref/'boot_handoff.c',ref/'boot_handoff.S',lab/'boot_runtime.c',*crypto]):
    obj=build/(tag+'-verifier-'+str(i)+'.o');objects.append(obj);run(tag+' target compile '+str(i),[*flags,'-c',src,'-o',obj])
   elf=build/(tag+'-verifier.elf');run(tag+' verifier-only target link',[*flags,'-nostdlib','-Wl,--gc-sections','-Wl,-e,fb_admit','-Wl,-u,fb_handoff','-Wl,-u,fb_joint_record','-Wl,-u,fb_record_matches','-Wl,-Ttext=0x08000000',*objects,'-o',elf]);p=run(tag+' target undefined',['arm-none-eabi-nm','-u',elf]);rows.append(dict(label=tag+' target helper closure',passed=p is not None and p.returncode==0 and not p.stdout.strip()));p=run(tag+' target symbols',['arm-none-eabi-nm',elf]);rows.append(dict(label=tag+' verifier excludes signing and includes declared memset',passed=p is not None and p.returncode==0 and 'crypto_ed25519_sign' not in p.stdout and 'crypto_ed25519_key_pair' not in p.stdout and ' memset' in p.stdout));run(tag+' target disassembly',['arm-none-eabi-objdump','-d',elf]);run(tag+' target sizes',['arm-none-eabi-size','-A',elf]);su='\n'.join(p.read_text() for p in build.glob(tag+'-verifier-*.su'));rows.append(dict(label=tag+' target stack records',stdout=su,passed=bool(su) and 'dynamic' not in su))
   sources=[ref/'boot_pair.c',*core,lab/'boot_pair_test.c',ref/'boot_handoff.c',ref/'boot_handoff.S',lab/'boot_guest.c',lab/'boot_guest.S',lab/'boot_runtime.c',*crypto]
   for variant in [0,1,2]:
    name=tag+'-guest-'+str(variant);elf=build/(name+'.elf');p=run(name+' build',[*flags,'-DFB_GUEST','-DFB_CODE_BASE=0','-DFB_FAULT_VARIANT='+str(variant),*sources,'-nostdlib','-Wl,--gc-sections','-Wl,-T,'+str(lab/'boot_guest.ld'),'-Wl,-Map,'+str(build/(name+'.map')),'-o',elf]);
    if p and p.returncode==0:
     run(name+' actual struct lr_cpu execute',['qemu-system-arm','-M',machine,'-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf],expected='FB_GUEST_HANDOFF_PASS:' if variant==0 else 'FB_GUEST_MPU_FAULT:',timeout=120)
     p=run(name+' undefined',['arm-none-eabi-nm','-u',elf]);rows.append(dict(label=name+' guest helper closure',passed=p is not None and p.returncode==0 and not p.stdout.strip()))
 files=[Path(__file__),manifest,*[lab/n for n in ['boot_pair.h','boot_pair.c','boot_pair_test.c','boot_recovery_test.c','boot_handoff.c','boot_handoff.S','boot_guest.c','boot_guest.S','boot_guest.ld','boot_runtime.c','BOOT_PAIR.md','flash_maintenance.h','flash_maintenance_test.c','h745_flash.h','h745_cache.h']],*[ref/n for n in ['boot_pair.c','boot_handoff.c','boot_handoff.S','BOOT_PAIR_ANALYSIS.md','flash_maintenance.c','h745_flash.c','h745_cache.c']]]
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},external_provider=external,checks=rows,limits=['MPS2 Cortex-M struct lr_cpu execution is not STM32 board/flash/dual-core physical behavior','Monocypher external primitives explicitly permitted for original admission/handoff mechanism; not a first-principles crypto implementation or full provider audit','Original emulator fixture includes deliberately public signing seed; verifier-only target ELF excludes signing/key-generation functions','Torn H745 ECC BusFault containment, platform shutdown/other-core startup and physical supply/errata/retention remain separate obligations','No external reviewer, independent learner or physical completion inferred']);(ROOT/'research/v5/expansion/b06/boot_pair_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
