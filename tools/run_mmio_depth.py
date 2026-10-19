"""Execute register side effects, debounce transfer, imported GPIO and target audits."""
from pathlib import Path
import argparse,datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
LAB=ROOT/'labs/peripheral_depth/mmio';REF=ROOT/'assessor/peripheral_depth/mmio'
FW=ROOT/'labs/firmware_io/include';PROVIDER=ROOT/'assessor/firmware_io/firmware_io.c'
BOOT=ROOT/'labs/smp_depth/acpi/big_endian.S'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return [Path(__file__),PROVIDER,BOOT,*FW.glob('*.h'),*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args()
 build=ROOT/'build/mmio-depth'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');build.mkdir(parents=True)
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};rows=[]
 qemu=ROOT/'build/checkpoint-tools/qemu-user-local/usr/bin/qemu-ppc64';lld=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21'
 external={str(p):sha(p) for p in [qemu,lld]};includes=['-I'+str(LAB),'-I'+str(FW)]
 def run(label,cmd,expected=0,marker=None,empty=False):
  cmd=list(map(str,cmd));log=build/(str(len(rows))+'.log')
  try:r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=60);out=r.stdout+r.stderr;rc=r.returncode
  except (OSError,subprocess.TimeoutExpired) as e:out=str(e);rc=None
  log.write_text(out);ok=rc==expected and (not marker or marker in out) and (not empty or not out.strip()) and not any(s in out for s in ['runtime error:','ERROR: AddressSanitizer'])
  rows.append(dict(label=label,command=cmd,returncode=rc,expected=expected,marker=marker,empty=empty,passed=ok,raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,ok,flush=True);return ok,out
 for tool in ['gcc','clang','arm-none-eabi-gcc',qemu,lld]:run(str(tool)+' version',[tool,'--version'])
 run('environment',['uname','-a'])
 for cc,opt,tag in [('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*includes]+(['-O1','-g'] if 'asan' in tag else [])+(['-DNDEBUG'] if 'release' in tag else [])
  for variant in [0,1,2,3,4,5,6,7,'stub']:
   src=LAB/'mmio.c' if variant=='stub' else REF/'mmio.c';exe=build/(tag+'-'+str(variant));ok,_=run(tag+' compile '+str(variant),[cc,*flags,'-DPM_MUTANT='+str(0 if variant=='stub' else variant),src,LAB/'tests.c','-o',exe])
   if ok:run(tag+' execute '+str(variant),[exe],0 if variant==0 else 1,'PM_PASS' if variant==0 else 'PM_CHECK:')
  for name,src,marker in [('composition',REF/'composition.c','PM_COMPOSE_PASS'),('oracle',REF/'probe.c',None)]:
   exe=build/(tag+'-'+name);ok,_=run(tag+' '+name+' compile',[cc,*flags,REF/'mmio.c',src,*([PROVIDER] if name=='composition' else []),'-o',exe])
   if ok:
    ok,_=run(tag+' '+name+' execute',[exe],marker=marker)
    if ok and name=='oracle':run(tag+' independent oracle',[sys.executable,REF/'oracle.py',ROOT/rows[-1]['raw']])
 for opt in ['-O0','-O2','-O3']:
  for variant in [0,1,2,3,4,5,6,7,'stub']:
   label='big-endian'+opt+' '+str(variant);exe=build/('be'+opt+str(variant));src=LAB/'mmio.c' if variant=='stub' else REF/'mmio.c'
   flags=['--target=powerpc64-linux-gnu','-mabi=elfv2','-std=c17',opt,'-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-stack-protector','-nostdlib','-static','--ld-path='+str(lld),'-Wl,-e,_start','-DPM_BIG_ENDIAN','-DPM_MUTANT='+str(0 if variant=='stub' else variant),*includes]
   ok,_=run(label+' compile',['clang',*flags,src,LAB/'tests.c',REF/'big_endian.c',BOOT,'-o',exe])
   if ok:
    run(label+' ELF',['readelf','-hW',exe],marker='big endian');run(label+' closure',['nm','-u',exe],empty=True)
    run(label+' execute',[qemu,exe],0 if variant==0 else 1,'PM_PASS' if variant==0 else 'PM_CHECK:')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['-O0','-O3']:
   tag=cc+opt;dest=build/tag;dest.mkdir();objects=[]
   flags=['-std=c17',opt,'-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage',*includes,'-isystem','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib']+(['--target=arm-none-eabi'] if cc=='clang' else [])
   for name,source in [('model',REF/'mmio.c'),('target',REF/'target.c'),('firmware',PROVIDER)]:
    obj=dest/(name+'.o');objects.append(obj);ok,_=run(tag+' '+name+' compile',[cc,*flags,'-c',source,'-o',obj])
    if ok:run(tag+' '+name+' object closure',['arm-none-eabi-nm','-u',obj],empty=True)
   elf=dest/'audit.elf';ok,_=run(tag+' combined audit link',['arm-none-eabi-ld','-e','pm_target_write','-Ttext=0x08000000',*objects,'-Map='+str(dest/'image.map'),'-o',elf])
   if ok:
    run(tag+' image closure',['arm-none-eabi-nm','-u',elf],empty=True);run(tag+' target disassembly',['arm-none-eabi-objdump','-d',dest/'target.o']);run(tag+' cost',['arm-none-eabi-size','-A',elf]);run(tag+' attributes',['arm-none-eabi-readelf','-A',elf])
 rows.append(dict(label='sources stable',passed=all(sha(ROOT/p)==h for p,h in pins.items())))
 out=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tools_sha256=external,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in build.rglob('*') if p.is_file()},limits=['No physical pin/clock/electrical behavior observed.','M4 audit image is linked symbol-closure evidence, not a bootable board program.','Big-endian PPC64 runs the original model through an explicit Linux syscall bootstrap; it is not an STM32 endian mode.','Serialized model, no race-freedom or cross-core cache guarantee.'])
 report=ROOT/a.report;report.parent.mkdir(parents=True,exist_ok=True);report.write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(dict(passed=out['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return int(not out['passed'])
if __name__=='__main__':raise SystemExit(main())
