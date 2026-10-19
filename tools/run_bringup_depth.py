"""T27 author campaign: hosted state mechanisms and real linked M4 images."""
from pathlib import Path
import argparse,datetime,hashlib,json,subprocess,sys,re
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--pilot',action='store_true');a=p.parse_args()
 stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');build=ROOT/'build/legacy_advanced_bringup'/stamp;build.mkdir(parents=True)
 lab=ROOT/'labs/bringup_depth';ref=ROOT/'assessor/bringup_depth';rows=[]
 def pins_now():return {f.relative_to(ROOT).as_posix():hashlib.sha256(f.read_bytes()).hexdigest() for f in [Path(__file__),*lab.rglob('*'),*ref.rglob('*')] if f.is_file() and '__pycache__' not in str(f)}
 start_pins=pins_now()
 def run(label,cmd,expected=0,needle=None,timeout=90):
  command=list(map(str,cmd));i=len(rows);log=build/(str(i)+'.txt')
  try:
   r=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=timeout);out=r.stdout+r.stderr;rc=r.returncode;passed=rc==expected and (needle is None or needle in out)
  except (OSError,subprocess.TimeoutExpired) as e:out=str(e);rc=None;passed=False
  log.write_text(out,encoding='utf8');rows.append(dict(label=label,command=command,returncode=rc,expected_returncode=expected,required_text=needle,passed=passed,raw=log.relative_to(ROOT).as_posix(),raw_sha256=hashlib.sha256(log.read_bytes()).hexdigest()));print(label,passed,flush=True);return out,passed
 for tool in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','qemu-system-arm']:run(tool+' version',[tool,'--version'])
 run('environment',['uname','-a'])
 stack_observations=[]
 profiles=[('gcc','-O0','gcc-debug'),('gcc','-O2','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-sanitize'),('clang','-fsanitize=address,undefined','clang-sanitize')]
 if a.pilot:profiles=profiles[:1]
 for cc,opt,tag in profiles:
  for mut in ([0] if a.pilot else [0,1,2,3,4,5,'stub']):
   src=lab/'bringup.c' if mut=='stub' else ref/'bringup.c';exe=build/(tag+'-'+str(mut));cmd=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',opt,'-I'+str(lab),src,lab/'tests.c','-o',exe]
   if mut!='stub':cmd+=['-DBU_MUTANT='+str(mut)]
   if 'release' in tag:cmd+=['-DNDEBUG']
   if 'sanitize' in tag:cmd+=['-O1','-g']
   _,ok=run(tag+' compile '+str(mut),cmd)
   if ok:run(tag+' execute '+str(mut),[exe],0 if mut==0 else 1,'BU_PASS' if mut==0 else 'BU_CHECK:')
  for mut in ([0] if a.pilot else [0,1,2,3,'stub']):
   src=lab/'lifecycle.c' if mut=='stub' else ref/'lifecycle.c';exe=build/(tag+'-lifecycle-'+str(mut));cmd=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',opt,'-I'+str(lab),src,lab/'lifecycle_tests.c','-o',exe]
   if mut!='stub':cmd+=['-DBU_LIFECYCLE_MUTANT='+str(mut)]
   if 'release' in tag:cmd+=['-DNDEBUG']
   if 'sanitize' in tag:cmd+=['-O1','-g']
   _,ok=run(tag+' lifecycle compile '+str(mut),cmd)
   if ok:run(tag+' lifecycle execute '+str(mut),[exe],0 if mut==0 else 1,'BU_LIFECYCLE_PASS' if mut==0 else 'BU_LIFECYCLE_CHECK:')
  if not a.pilot and 'sanitize' not in tag:
   exe=build/(tag+'oracle');_,ok=run(tag+' oracle compile',[cc,'-std=c17',opt,'-I'+str(lab),ref/'bringup.c',ref/'oracle_probe.c','-o',exe])
   if ok:
    _,ok=run(tag+' independent observations',[exe]);raw=ROOT/rows[-1]['raw']
    if ok:run(tag+' independent set oracle',[sys.executable,ref/'oracle.py',raw])
   exe=build/(tag+'-lifecycle-oracle');_,ok=run(tag+' lifecycle oracle compile',[cc,'-std=c17',opt,'-I'+str(lab),ref/'lifecycle.c',ref/'lifecycle_probe.c','-o',exe])
   if ok:
    _,ok=run(tag+' lifecycle observations',[exe]);raw=ROOT/rows[-1]['raw']
    if ok:run(tag+' lifecycle event oracle',[sys.executable,ref/'lifecycle_oracle.py',raw])
 includes=['-I'+str(lab),'-isystem','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib']
 for cc,opt,tag in ([('arm-none-eabi-gcc','-O0','arm-gcc-debug')] if a.pilot else [('arm-none-eabi-gcc','-O0','arm-gcc-debug'),('arm-none-eabi-gcc','-O2','arm-gcc-release'),('clang','-O0','arm-clang-debug'),('clang','-O3','arm-clang-release')]):
  for target,code,bank in [('mps2','0x00000000','0x20010000'),('f439','0x08000000','0x10000000')]:
   modes=[('ordinary',{}),('software-reset',{'BU_SOFTWARE_RESET':1}),('ram-markers',{'BU_QUIET':1}),('guard',{'BU_GUARD_FAULT':1}),('guard-masked',{'BU_GUARD_FAULT':1,'BU_MASK_GUARD':1}),('bad-lma',{'BU_BAD_LMA':1}),('bad-zero',{'BU_NO_ZERO':1}),('two-defects',{'BU_BAD_LMA':1,'BU_NO_ZERO':1}),('fp',{'BU_FP':1}),('ram-vectors',{'BU_RAM_VECTORS':1}),('boot-offset',{})] if target=='mps2' else [('cold-cross',{'BU_F439':1}),('watchdog-cross',{'BU_F439':1,'BU_WATCHDOG':1})]
   if a.pilot:modes=modes[:1]
   for mode,defs in modes:
    dest=build/(tag+'-'+target+'-'+mode);dest.mkdir();script=dest/'image.ld';image_code='0x4000' if mode=='boot-offset' else code
    text=(lab/'target/image.ld.in').read_text().replace('@CODE@',image_code).replace('@BANK@',bank)
    if mode=='boot-offset':text=text.replace('ENTRY(bu_reset)','ENTRY(bu_bootloader)').replace('MEMORY {','MEMORY { BOOT(rx) : ORIGIN = 0, LENGTH = 16K').replace('SECTIONS {','SECTIONS { .bootstrap : { KEEP(*(.bootstrap)) } >BOOT')
    script.write_text(text);objects=[]
    flags=['-std=c17',opt,'-mcpu=cortex-m4','-mthumb','-mfloat-abi='+('hard' if defs.get('BU_FP') else 'soft'),'-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-fstack-usage','-Wall','-Wextra','-Werror',*includes]
    if defs.get('BU_FP'):flags+=['-mfpu=fpv4-sp-d16']
    if cc=='clang':flags+=['--target=arm-none-eabi']
    flags+=['-D'+k+'='+str(v) for k,v in defs.items()]
    for source in [ref/'target/reset.S',ref/'target/guest.c',*([ref/'target/bootloader.S'] if mode=='boot-offset' else [])]:
     obj=dest/(source.stem+'.o');_,ok=run(tag+' '+target+' '+mode+' compile '+source.name,[cc,*flags,'-c',source,'-o',obj]);objects.append(obj)
     if not ok:break
    else:
     elf=dest/'image.elf';_,ok=run(tag+' '+target+' '+mode+' link',['arm-none-eabi-ld','--gc-sections','-T',script,'-Map='+str(dest/'image.map'),*objects,'-o',elf])
     if not ok:continue
     image_output,image_ok=run(tag+' '+target+' '+mode+' independent image',[sys.executable,lab/'check_image.py',elf,'--code',image_code,'--bank',bank])
     if not image_ok:continue
     out,ok=run(tag+' '+target+' '+mode+' closure',['arm-none-eabi-nm','-u',elf]);rows[-1]['passed']&=not out.strip()
     run(tag+' '+target+' '+mode+' disassembly',['arm-none-eabi-objdump','-d',elf]);run(tag+' '+target+' '+mode+' attributes',['arm-none-eabi-readelf','-A',elf]);run(tag+' '+target+' '+mode+' sections',['arm-none-eabi-objdump','-h',elf]);run(tag+' '+target+' '+mode+' size',['arm-none-eabi-size','-A',elf])
     if target=='mps2':
      bad=mode in ['bad-lma','bad-zero','two-defects','guard-masked'];needle='BU_CHECK:copy' if mode in ['bad-lma','two-defects'] else 'BU_CHECK:zero' if mode=='bad-zero' else None if mode=='ram-markers' else 'BU_GUARD_PASS' if mode=='guard' else 'BU_FAULT' if mode=='guard-masked' else 'BU_RETURN_PASS'
      binary=dest/'image.bin'
      _,ok=run(tag+' '+target+' '+mode+' raw ROM image',['arm-none-eabi-objcopy','-O','binary',elf,binary])
      if not ok:continue
      loaders=[]
      for section in ['.bss','.bank_bss','.data','.bank_data']:
       s=json.loads(image_output)['sections'][section];seed=dest/(section[1:]+'.seed');seed.write_bytes(bytes([165])*s['size']);loaders+=['-device',f'loader,file={seed},addr={s["address"]},force-raw=on']
      output,passed=run(tag+' '+target+' '+mode+' execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',binary,*loaders],1 if bad else 0,needle,20)
      if mode in ['ordinary','fp'] and passed:
       su=(dest/'guest.su').read_text();frames={line.split('\t')[0].rsplit(':',1)[1]:int(line.split('\t')[1]) for line in su.splitlines()};path=['bu_main','middle','leaf','bu_svc','bu_tick'];basic=2*(32+4);extended=2*(72+64) if mode=='fp' else 0;selected={n:[k for k in frames if k==n or k.startswith(n+'.')] for n in path};frame_ok=all(len(v)==1 for v in selected.values());bound=sum(frames[v[0]] for v in selected.values() if len(v)==1)+basic+extended
       observed=int(re.search(r'BU_STACK (\d+)',output)[1]);stack_observations.append(dict(profile=tag,mode=mode,compiler_frames=frames,selected_symbols=selected,path=path,basic_plus_alignment=basic,extended_and_conservative_software=extended,path_budget=bound,observed_written_bytes=observed,passed=frame_ok and 'dynamic' not in su and observed<=bound<2048,limits='Declared straight-line path only; watermark is a lower bound on writes; conservative FP software allowance, not actual forced s16..s31 spill.'));rows.append(dict(label=tag+' '+mode+' stack argument',passed=stack_observations[-1]['passed']))
 pins=pins_now();rows.append(dict(label='source stable during execution',passed=pins==start_pins,initial_sha256=start_pins))
 report=dict(schema_version=1,passed=all(r['passed'] for r in rows),observed_utc=stamp,scope='pilot' if a.pilot else 'hosted-and-emulator-author-campaign',checks=rows,stack_observations=stack_observations,input_sha256=pins,outputs_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in build.rglob('*') if p.is_file()},limits=['No physical STM32 observation; F439 cross-build uses original conservative HSI profile and retained stage only.','Host finite state tests are not hardware simulations; MPS2 models a different board.','No independent learner, delayed attempt, or non-author review.']);rp=ROOT/a.report;rp.parent.mkdir(parents=True,exist_ok=True);rp.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
