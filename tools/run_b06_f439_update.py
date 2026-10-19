"""Selected single-core F439 update, port and complete bank2 target evidence."""
from pathlib import Path
import datetime,hashlib,json,subprocess
from audit_b06_f439_stack import analyze,self_test
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/firmware_lifecycle/targets';ref=ROOT/'assessor/firmware_lifecycle/targets';tool=Path.home()/'cmastery-b06-tools/monocypher-4.0.3';manifest=ROOT/'research/v5/expansion/b06/monocypher_provider.json';build=ROOT/'build/b06-f439';build.mkdir(parents=True,exist_ok=True);rows=[];external=[]
 rows.append(dict(label="conservative stack checker controls",**self_test()))
 for item in json.loads(manifest.read_text())['files']:
  path=tool/item['path'];assert path.is_file() and hashlib.sha256(path.read_bytes()).hexdigest()==item['sha256'];external.append(dict(path=str(path),sha256=item['sha256']))
 def run(label,cmd,negative=None,expected=None):
  try:
   p=subprocess.run(list(map(str,cmd)),text=True,capture_output=True,timeout=90);out=p.stdout+p.stderr;ok=p.returncode==0 and (not expected or expected in out) if negative is None else p.returncode!=0 and negative in out;r=dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=ok)
  except (OSError,subprocess.TimeoutExpired) as e:r=dict(label=label,command=list(map(str,cmd)),passed=False,error=str(e));p=None
  rows.append(r);print(label,r['passed'],flush=True);return p
 includes=['-I'+str(lab),'-I'+str(tool/'src'),'-I'+str(tool/'src/optional')];common=[ref/'boot_pair.c',ref/'h745_flash.c',ref/'recovery_transport.c',ref/'boot_storage.c',ref/'flash_maintenance.c',ref/'ecc_guard.c',ref/'h745_cache.c',tool/'src/monocypher.c',tool/'src/optional/monocypher-ed25519.c']
 for cc,flags,tag in [('gcc',['-O0'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O0'],'clang-debug'),('clang',['-O3'],'clang-release'),('gcc',['-O1','-g','-fsanitize=address,undefined'],'gcc-sanitize'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]:
  base=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-Wno-misleading-indentation','-fno-pie',*flags,*includes];objects=[]
  for i,src in enumerate(common):
   obj=build/(tag+'-dependency-'+str(i)+'.o');objects.append(obj);run(tag+' actual dependency compile '+str(i),[*base,'-c',src,'-o',obj])
  for mutant in [0,1,2,3,'stub']:
   exe=build/(tag+'-'+str(mutant));src=(lab if mutant=='stub' else ref)/'f439_update.c';cmd=[*base,'-no-pie',src,lab/'f439_update_test.c',*objects,'-o',exe]
   if mutant!='stub':cmd.append('-DF4_MUTANT='+str(mutant))
   p=run(tag+' build '+str(mutant),cmd)
   if p and p.returncode==0:run(tag+' execute '+str(mutant),[exe],None if mutant==0 else 'F4_CHECK:')

  for mutant in [0,1,2,'stub']:
   exe=build/(tag+'-port-'+str(mutant));src=(lab if mutant=='stub' else ref)/'f439_port.c';cmd=[*base,'-no-pie',src,lab/'f439_port_test.c','-o',exe]
   if mutant!='stub':cmd.append('-DF4_PORT_MUTANT='+str(mutant))
   p=run(tag+' port build '+str(mutant),cmd)
   if p and p.returncode==0:run(tag+' port execute '+str(mutant),[exe],None if mutant==0 else 'F4_PORT_CHECK:')
 for opt in ['-O0','-O2']:
  tag='cortex-m4'+opt;elf=build/(tag+'.elf');sources=[ref/'f439_update.c',ref/'f439_port.c',ref/'f439_boot.c',*common,lab/'f439_mmio.c',lab/'selected_reset.S',lab/'trial_report.S',lab/'boot_runtime.c',ref/'ecc_guard_port.c',ref/'ecc_guard_port.S',ref/'boot_handoff.c',ref/'boot_handoff.S',ref/'boot_program_mode.c'];p=run(tag+' complete single-core lifecycle link',['arm-none-eabi-gcc','-std=c17',opt,'-mthumb','-mcpu=cortex-m4','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections',*includes,*sources,'-nostdlib','-Wl,--gc-sections','-Wl,-T,'+str(lab/'f439_service.ld'),'-Wl,-Map,'+str(build/(tag+'.map')),'-o',elf])
  if p and p.returncode==0:
   p=run(tag+' unresolved symbols',['arm-none-eabi-nm','-u',elf]);rows.append(dict(label=tag+' helper closure',passed=p is not None and p.returncode==0 and not p.stdout.strip()));p=run(tag+' symbols',['arm-none-eabi-nm',elf]);symbols=p.stdout if p else '';placed=[line.split() for line in symbols.splitlines() if len(line.split())==3 and line.split()[1] in 'TtRr'];rows.append(dict(label=tag+' every code and rodata symbol in owned bank2',passed=bool(placed) and all(0x08120000<=int(z[0],16)<0x08160000 for z in placed)));rows.append(dict(label=tag+' exact report entry',passed=any(int(z[0],16)==0x08120400 and z[2]=='th_app_report' for z in placed)));rows.append(dict(label=tag+' excludes signing',passed=p is not None and not any(x in p.stdout for x in ['crypto_ed25519_sign','crypto_ed25519_key_pair'])));dis=run(tag+' disassembly',['arm-none-eabi-objdump','-d',elf]);run(tag+' sizes',['arm-none-eabi-size','-A',elf]);su='\n'.join(x.read_text() for x in build.glob(tag+'*.su'));rows.append(dict(label=tag+' stack records',stdout=su,passed=bool(su) and 'dynamic' not in su))
   try:
    stackrows={}
    for line in su.splitlines():
     if not line.strip():continue
     z=line.split('\t') if '\t' in line else line.split('	');name=z[0].split(':')[-1];stackrows[name]=max(stackrows.get(name,0),int(z[1]))
    bounded=analyze(dis.stdout,stackrows);rows.append(dict(label=tag+' bounded whole call paths',passed=bounded['startup_bound']<=16384 and bounded['report_bound']+256<=16384,**bounded))
   except (ValueError,AttributeError,IndexError) as error:rows.append(dict(label=tag+' bounded whole call paths',passed=False,error=str(error)))
  primary=build/('primary'+opt+'.elf');p=run('primary'+opt+' trusted cold stage branch',['arm-none-eabi-gcc',opt,'-mthumb','-mcpu=cortex-m4',lab/'trial_primary.S','-nostdlib','-Wl,-T,'+str(lab/'trial_primary.ld'),'-o',primary]);
  if p and p.returncode==0:run('primary'+opt+' disassembly',['arm-none-eabi-objdump','-d',primary])
 pack=build/'pack';p=run('host-only image packer build',['gcc','-std=c17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',*includes,lab/'f439_image_pack.c',*[build/('gcc-debug-dependency-'+str(i)+'.o') for i in range(len(common))],'-o',pack])
 for slot in [0,1]:
  tag='application-'+str(slot);base=0x08080200+slot*0x20000;ld=build/(tag+'.ld');ld.write_text('ENTRY(th_app_reset)\nSECTIONS { . = '+hex(base)+'; .vectors : { KEEP(*(.vectors)) } .text : { *(.text*) *(.rodata*) } _stack_top = 0x20020000; ASSERT(. <= '+hex(base+2048)+',"signed payload too large") .forbidden : { *(.data*) *(.bss*) *(COMMON) } ASSERT(SIZEOF(.forbidden)==0,"application requires unimplemented runtime storage") /DISCARD/ : { *(.comment) *(.ARM.exidx*) *(.ARM.extab*) } }\n'.replace('\\n','\n'));ld.write_text(ld.read_text().replace('\\n','\n').replace('\n','\n'));elf=build/(tag+'.elf');binary=build/(tag+'.bin');p=run(tag+' original application link',['arm-none-eabi-gcc','-std=c17','-O2','-mthumb','-mcpu=cortex-m4','-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-DAPP_CORE=0','-DAPP_GENERATION='+str(slot+1),lab/'trial_application.c',lab/'trial_application.S','-nostdlib','-Wl,-T,'+str(ld),'-o',elf]);
  if p and p.returncode==0:
   stacklines=[line.split('	') for f in build.glob(tag+'*.su') for line in f.read_text().splitlines()];rows.append(dict(label=tag+' finite application stack bound',passed=bool(stacklines) and all(z[2]=='static' and int(z[1])<=256 for z in stacklines),rows=stacklines));run(tag+' disassembly',['arm-none-eabi-objdump','-d',elf]);p=run(tag+' undefined',['arm-none-eabi-nm','-u',elf]);rows.append(dict(label=tag+' helper closure',passed=p is not None and p.returncode==0 and not p.stdout.strip()));p=run(tag+' raw payload',['arm-none-eabi-objcopy','-O','binary',elf,binary]);
   if p and p.returncode==0:run(tag+' signature and actual admission',[pack,'0',str(slot),str(slot+1),binary,build/(tag+'.slot'),build/(tag+'.frame')],expected='F4_PACK_PASS:')
 guestld=build/'application-guest.ld';guestld.write_text('ENTRY(th_app_reset)\nSECTIONS { . = 0; .vectors : { KEEP(*(.vectors)) } .text : { *(.text*) *(.rodata*) } .bss 0x20000000 (NOLOAD) : { *(.bss*) *(COMMON) } _stack_top = 0x20020000; /DISCARD/ : { *(.comment) *(.ARM.exidx*) *(.ARM.extab*) } }\n'.replace('\n','\n'))
 for generation in [2,3]:
  for bad in [0,1]:
   tag='cpu-application-'+str(generation)+'-'+str(bad);elf=build/(tag+'.elf');flags=['-DAPP_BAD=1'] if bad else [];p=run(tag+' build',['arm-none-eabi-gcc','-std=c17','-O2','-mthumb','-mcpu=cortex-m4','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-DTH_GUEST=1','-DAPP_CORE=0','-DAPP_GENERATION='+str(generation),*flags,*includes,lab/'trial_application.c',lab/'trial_application.S',lab/'f439_application_guest.c',ref/'f439_update.c',lab/'boot_runtime.c','-nostdlib','-Wl,--gc-sections','-Wl,-T,'+str(guestld),'-o',elf])
   if p and p.returncode==0:run(tag+' actual CPU execution',['qemu-system-arm','-M','mps2-an386','-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf],negative='F4_CPU_BAD:' if bad else None,expected=None if bad else 'F4_CPU_PASS:')
 files=[Path(__file__),ROOT/'tools/audit_b06_f439_stack.py',manifest,*[lab/n for n in ['f439_update.h','f439_update.c','f439_update_test.c','f439_port.h','f439_port.c','f439_port_test.c','f439_mmio.c','f439_boot.c','f439_service.ld','f439_image_pack.c','f439_application_guest.c','F439_UPDATE.md','F439_ERRATA.md','trial_primary.S','trial_primary.ld','trial_report.S','trial_application.c','trial_application.S','selected_reset.S','boot_runtime.c','recovery_transport.h','boot_pair.h','boot_program_mode.h','boot_program_mode.c','boot_storage.h','ecc_guard.h','flash_maintenance.h','h745_flash.h','h745_cache.h']],*[ref/n for n in ['f439_update.c','f439_port.c','f439_boot.c','F439_UPDATE_ANALYSIS.md','recovery_transport.c','boot_storage.c','boot_pair.c','h745_flash.c','flash_maintenance.c','ecc_guard.c','ecc_guard_port.c','ecc_guard_port.S','h745_cache.c','boot_handoff.c','boot_handoff.S','boot_program_mode.c']]]
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},external_provider=external,checks=rows,limits=['Host controller/register model; no physical F439 voltage/flash/UART/reset or power-cut observation','Complete Cortex-M4 service is linked and placement/stack audited, not physical F439 execution','MPS2 executes original application and actual single-core commit gate with modeled controller callbacks; not F439 peripherals','External signature/hash provider follows FW-BOOT permission; no crypto-internals mastery claim','No independent learner, external reviewer or physical evidence inferred']);(ROOT/'research/v5/expansion/b06/f439_update_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
