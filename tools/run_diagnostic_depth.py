"""T28.12 actual provider compositions, hostile inputs and changed transports."""
from pathlib import Path
import argparse,datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/peripheral_depth/diagnostic';REF=ROOT/'assessor/peripheral_depth/diagnostic';FD=ROOT/'assessor/foundations/foundation.c';ED=ROOT/'assessor/ethernet_depth/ethernet_depth.c';HELP=ROOT/'assessor/execution/support.c';BE=ROOT/'assessor/peripheral_depth/mmio/big_endian.c';BOOT=ROOT/'labs/smp_depth/acpi/big_endian.S'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():return [Path(__file__),FD,ED,HELP,BE,BOOT,ROOT/'labs/foundations/include/foundation.h',ROOT/'labs/ethernet_depth/include/ethernet_depth.h',*[p for b in [LAB,REF] for p in b.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);p.add_argument('--skip-guests',action='store_true',help='Focused development only; guard refuses closure without guests');a=p.parse_args();w=ROOT/'build/diagnostic-depth'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}
 def run(label,cmd,expected=0,marker=None,empty=False,timeout=180):
  cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log')
  with log.open('w') as f:
   try:rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=timeout).returncode
   except (OSError,subprocess.TimeoutExpired) as ex:f.write(str(ex));rc=None
  text=log.read_text(errors='replace');ok=rc==expected and (not marker or marker in text) and (not empty or not text.strip()) and not any(s in text for s in ['runtime error:','ERROR: AddressSanitizer']);rows.append(dict(label=label,command=cmd,returncode=rc,expected=expected,marker=marker,empty=empty,passed=ok,raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,ok,flush=True);return ok,log
 inc=['-I'+str(p) for p in [LAB,ROOT/'labs/foundations/include',ROOT/'labs/ethernet_depth/include']]
 qemu=ROOT/'build/checkpoint-tools/qemu-user-local/usr/bin/qemu-ppc64';lld=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21'
 for t in ['gcc','clang','arm-none-eabi-gcc','qemu-system-arm','qemu-system-x86_64',qemu,lld]:run(str(t)+' version',[t,'--version'])
 run('environment',['uname','-a'])
 def sources(kind,v):
  if kind=='core':return [(LAB if v=='stub' else REF)/'diagnostic.c',(LAB if v=='stub' else REF)/'ethernet.c',LAB/'tests.c',FD,ED]
  return [(LAB if v=='stub' else REF)/(kind+'.c'),REF/(kind+'_test.c'),REF/'diagnostic.c',FD]
 profiles=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
 for cc,opt,tag in profiles:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*inc]+(['-O1','-g'] if 'asan' in tag else [])+(['-DNDEBUG'] if 'release' in tag else [])
  for kind,variants,macro,marker in [('core',[0,1,2,3,4,5,'stub'],'DG_MUTANT','DG'),('usb',[0,1,2,3,4,5,'stub'],'DG_USB_MUTANT','DG_USB'),('host',[0,1,'stub'],'DG_HOST_MUTANT','DG_HOST')]:
   for v in variants:
    exe=w/(tag+'-'+kind+'-'+str(v));label=tag+' '+kind+' '+str(v);ok,_=run(label+' compile',[cc,*flags,'-D'+macro+'='+str(0 if v=='stub' else v),*sources(kind,v),'-o',exe])
    if ok:run(label+' execute',[exe],0 if v==0 else 1,marker+('_PASS' if v==0 else '_CHECK:'))
  exe=w/(tag+'-oracle');ok,_=run(tag+' oracle compile',[cc,*flags,REF/'diagnostic.c',FD,REF/'probe.c','-o',exe])
  if ok:
   ok,log=run(tag+' oracle data',[exe])
   if ok:run(tag+' independent oracle',[sys.executable,REF/'oracle.py',log],marker='DG_ORACLE_PASS sequences=65536 lengths=33')
 for opt in ['-O0','-O3']:
  for kind,variants,macro,marker in [('core',[0,1,2,3,4,5],'DG_MUTANT','DG'),('usb',[0,1,2,3,4,5],'DG_USB_MUTANT','DG_USB')]:
   for v in variants:
    label='PPC64'+opt+' '+kind+' '+str(v);exe=w/label.replace(' ','-')
    flags=['--target=powerpc64-linux-gnu','-mabi=elfv2','-std=c17',opt,'-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-stack-protector','-nostdlib','-static','--ld-path='+str(lld),'-Wl,-e,_start','-DDG_BIG_ENDIAN','-D'+macro+'='+str(v),*inc]
    ok,_=run(label+' compile',['clang',*flags,*sources(kind,v),BE,BOOT,'-o',exe])
    if ok:run(label+' ELF',['readelf','-hW',exe],marker='big endian');run(label+' closure',['nm','-u',exe],empty=True);run(label+' execute',[qemu,exe],0 if not v else 1,marker+('_PASS' if not v else '_CHECK:'))
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['-O0','-O3']:
   for kind,variants,macro in [('core',[0,3,5],'DG_MUTANT'),('usb',[0,4,5],'DG_USB_MUTANT')]:
    for v in variants:
     label=cc+opt+' '+kind+' '+str(v);dest=w/label.replace(' ','-');dest.mkdir();objs=[]
     flags=['-std=c17',opt,'-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-DDG_STANDALONE','-Dmain=dg_guest_main','-D'+macro+'='+str(v),*inc,'-isystem','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib']+(['--target=arm-none-eabi'] if cc=='clang' else [])
     for i,src in enumerate([*sources(kind,v),HELP,LAB/'thumb_start.c']):
      obj=dest/(str(i)+'.o');objs.append(obj);run(label+' compile '+str(i),[cc,*flags,'-c',src,'-o',obj]);run(label+' imports '+str(i),['arm-none-eabi-nm','-u',obj])
     elf=dest/'image.elf';ok,_=run(label+' link',['arm-none-eabi-ld','--gc-sections','-T',LAB/'link.ld',*objs,'-Map='+str(dest/'image.map'),'-o',elf])
     if ok:
      run(label+' closure',['arm-none-eabi-nm','-u',elf],empty=True);run(label+' symbols',['arm-none-eabi-nm',elf]);run(label+' size',['arm-none-eabi-size','-A',elf]);run(label+' assembly',['arm-none-eabi-objdump','-d',elf]);run(label+' execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf],0 if not v else 1,'DG_TARGET_PASS' if not v else 'DG_TARGET_FAIL')
 run('changed hardware evidence',[sys.executable,REF/'evidence_test.py'],marker='"passed": true')
 for kind in ['core','usb','host']:
  for variant in ['reference','stub']:
   cmd=[sys.executable,LAB/'run.py','--kind',kind,'--build',w/('public-'+kind+'-'+variant)]
   if variant=='reference':
    cmd+=['--source',REF/('diagnostic.c' if kind=='core' else kind+'.c'),'--foundation',FD]
    cmd+=(['--adapter',REF/'ethernet.c','--ethernet',ED] if kind=='core' else ['--payload',REF/'diagnostic.c'])
   marker='DG' if kind=='core' else 'DG_PUBLIC_'+kind.upper()
   run('public '+kind+' '+variant,cmd,0 if variant=='reference' else 1,marker+('_PASS' if variant=='reference' else '_CHECK:'))
 guests=[]
 if not a.skip_guests:
  for v in [0,1,2,'stub']:
   dest=Path('/home/frede/cmastery-diagnostic-tools')/('campaign-'+w.name+'-'+str(v));run('Linux gadget '+str(v),[sys.executable,REF/'guest/run.py','--directory',dest,*(['--stub'] if v=='stub' else ['--mutant',v])],marker='guest passed True',timeout=420)
   if (dest/'report.json').is_file():
    child=json.loads((dest/'report.json').read_text());save=w/('guest'+str(v)+'.json');save.write_text(json.dumps(child,indent=2)+'\n');guests.append(save.relative_to(ROOT).as_posix())
 rows.append(dict(label='sources stable',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 out=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,guest_reports=guests,focused_skip_guests=a.skip_guests,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Exact finite inputs and model/kernel profiles only; no absolute mastery, physical board or cable observation.','Strict core imports are distinct from host libc and selected Linux gadget dependencies.','PPC64 is endian/data-model transfer, MPS2 executes portable logic, not STM32 device hardware.','Reference execution is not an independent learner attempt or external review.'])
 path=ROOT/a.report;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(dict(passed=out['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return int(not out['passed'])
if __name__=='__main__':raise SystemExit(main())
