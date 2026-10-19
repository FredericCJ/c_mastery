"""Source-bound T28.11 layers, actual UART composition, ARM vendor/model comparison."""
from pathlib import Path
import argparse,datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/peripheral_depth/layers';REF=ROOT/'assessor/peripheral_depth/layers'
UART=ROOT/'assessor/peripheral_depth/uart';MMIO=ROOT/'assessor/peripheral_depth/mmio';SERIAL=ROOT/'assessor/serial_depth/serial_depth.c';HELPERS=ROOT/'assessor/execution/support.c';BOOT=ROOT/'labs/smp_depth/acpi/big_endian.S';BE=MMIO/'big_endian.c'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return [Path(__file__),SERIAL,HELPERS,BOOT,BE,*[p for base in [LAB,REF,ROOT/'labs/peripheral_depth/uart',UART,ROOT/'labs/peripheral_depth/mmio',MMIO] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts],ROOT/'labs/serial_depth/include/serial_depth.h',ROOT/'labs/firmware_io/include/firmware_io.h']
def target_units():return [('service',REF/'service.c'),('adapter',REF/'uart_adapter.c'),('target',REF/'target.c'),('uart',UART/'uart.c'),('port',UART/'target.c'),('serial',SERIAL),('helpers',HELPERS)]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();build=ROOT/'build/layers-depth'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');build.mkdir(parents=True);pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};rows=[];external={}
 inc=['-I'+str(p) for p in [LAB,ROOT/'labs/peripheral_depth/uart',ROOT/'labs/peripheral_depth/mmio',ROOT/'labs/serial_depth/include',ROOT/'labs/firmware_io/include']]
 def run(label,cmd,expected=0,marker=None,empty=False):
  cmd=list(map(str,cmd));log=build/(str(len(rows))+'.log')
  try:r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=180);out=r.stdout+r.stderr;rc=r.returncode
  except (OSError,subprocess.TimeoutExpired) as e:out=str(e);rc=None
  log.write_text(out);ok=rc==expected and (not marker or marker in out) and (not empty or not out.strip()) and not any(s in out for s in ['runtime error:','ERROR: AddressSanitizer'])
  rows.append(dict(label=label,command=cmd,returncode=rc,expected=expected,marker=marker,empty=empty,passed=ok,raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,ok,flush=True);return ok,out
 qemu=ROOT/'build/checkpoint-tools/qemu-user-local/usr/bin/qemu-ppc64';lld=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21'
 for tool in ['gcc','clang','arm-none-eabi-gcc','qemu-system-arm',qemu,lld]:run(str(tool)+' version',[tool,'--version'])
 run('environment',['uname','-a']);run('generated register identity',[sys.executable,LAB/'generate_registers.py','--check'])
 for cc,opt,tag in PROFILES:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*inc]+(['-O1','-g'] if 'asan' in tag else [])+(['-DNDEBUG'] if 'release' in tag else [])
  def cases(kind,variants,filename,test,extras,defs,mutant,passed,failed):
   for v in variants:
    exe=build/(tag+'-'+kind+'-'+str(v));src=(LAB if v=='stub' else REF)/filename
    ok,_=run(tag+' '+kind+' compile '+str(v),[cc,*flags,*defs,'-D'+mutant+'='+str(0 if v=='stub' else v),src,test,*extras,'-o',exe])
    if ok:run(tag+' '+kind+' execute '+str(v),[exe],0 if v==0 else 1,passed if v==0 else failed)
  cases('service',[0,1,2,3,'stub'],'service.c',LAB/'tests.c',[],[],'PL_MUTANT','PL_PASS','PL_CHECK:')
  cases('posted',[0,1,2,3,'stub'],'posted.c',REF/'posted_test.c',[MMIO/'mmio.c'],[],'PL_POST_MUTANT','PL_POST_PASS','PL_POST_CHECK:')
  cases('compare',[0,1,2,'stub'],'compare.c',REF/'compare_test.c',[],['-DPL_NATIVE'],'PL_COMPARE_MUTANT','PL_COMPARE_PASS','PL_COMPARE_CHECK:')
  cases('status',[0,1],'uart_adapter.c',REF/'adapter_status_test.c',[REF/'service.c'],[],'PL_ADAPTER_MUTANT','PL_STATUS_PASS','PL_STATUS_CHECK:')
  cases('uart',[0,'stub'],'uart_adapter.c',REF/'uart_test.c',[REF/'service.c',UART/'uart.c',SERIAL],[],'PL_UNUSED','PL_UART_PASS','PU_CHECK:')
  cases('target',[0,1,'stub'],'target.c',REF/'target_test.c',[REF/'service.c',REF/'uart_adapter.c',UART/'uart.c',UART/'target.c',SERIAL],['-DPU_TARGET_MODEL'],'PL_TARGET_MUTANT','PL_TARGET_PASS','PU_TARGET_CHECK:')
  exe=build/(tag+'-oracle');ok,_=run(tag+' oracle compile',[cc,*flags,REF/'service.c',REF/'probe.c','-o',exe])
  if ok:
   ok,_=run(tag+' oracle execute',[exe])
   if ok:run(tag+' independent oracle',[sys.executable,REF/'oracle.py',ROOT/rows[-1]['raw']],marker='PL_ORACLE_PASS 65536')
 for opt in ['-O0','-O2','-O3']:
  for v in [0,1,2,3,'stub']:
   label='big-endian'+opt+' '+str(v);exe=build/('be'+opt+str(v));src=LAB/'service.c' if v=='stub' else REF/'service.c'
   flags=['--target=powerpc64-linux-gnu','-mabi=elfv2','-std=c17',opt,'-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-stack-protector','-nostdlib','-static','--ld-path='+str(lld),'-Wl,-e,_start','-DPL_BIG_ENDIAN','-DPL_MUTANT='+str(0 if v=='stub' else v),*inc]
   ok,_=run(label+' compile',['clang',*flags,src,LAB/'tests.c',BE,BOOT,'-o',exe])
   if ok:run(label+' ELF',['readelf','-hW',exe],marker='big endian');run(label+' closure',['nm','-u',exe],empty=True);run(label+' execute',[qemu,exe],0 if v==0 else 1,'PL_PASS' if v==0 else 'PL_CHECK:')
 vendor=Path('/home/frede/cmastery-architecture-tools');hal=vendor/'stm32f4xx-hal-driver-1.8.3';device=vendor/'cmsis-device-f4-2.6.10';cmsis=vendor/'CMSIS_5-5.9.0'
 vinc=['-I'+str(p) for p in [hal/'Inc',device/'Include',cmsis/'CMSIS/Core/Include']]
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['-O0','-O3']:
   tag=cc+opt;dest=build/tag;dest.mkdir();objs=[]
   flags=['-std=c17',opt,'-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections',*inc,'-isystem','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib']+(['--target=arm-none-eabi'] if cc=='clang' else [])
   for name,src in target_units():
    obj=dest/(name+'.o');objs.append(obj);ok,_=run(tag+' '+name+' compile',[cc,*flags,'-c',src,'-o',obj])
    if ok:run(tag+' '+name+' imports',['arm-none-eabi-nm','-u',obj])
   keep=['pl_send','pl_cancel','pl_poll','pl_feed','pl_f439_open','pl_f439_transport','pu_f439_pins','pu_f439_io'];args=[v for name in keep for v in ['-u',name]];elf=dest/'audit.elf'
   ok,_=run(tag+' combined audit link',['arm-none-eabi-ld','--gc-sections','-e','pl_f439_open','-Ttext=0x08000000',*args,*objs,'-Map='+str(dest/'image.map'),'-o',elf])
   if ok:
    run(tag+' image closure',['arm-none-eabi-nm','-u',elf],empty=True);run(tag+' image symbols',['arm-none-eabi-nm',elf]);run(tag+' target disassembly',['arm-none-eabi-objdump','-dr',dest/'port.o']);run(tag+' cost',['arm-none-eabi-size','-A',elf]);run(tag+' attributes',['arm-none-eabi-readelf','-A',elf])
   for suite,mut in [('compare',0),('compare',1),('compare',2),('callback',0),('frozen_tick',0)]:
    gd=dest/(suite+str(mut));ok,_=run(tag+' vendor '+suite+' '+str(mut),[sys.executable,REF/'guest_compare.py','--directory',gd.relative_to(ROOT),'--cc',cc,'--opt='+opt,'--suite',suite,'--mutant',mut])
    if (gd/'report.json').is_file():
     child=json.loads((gd/'report.json').read_text());external.update({k:v for k,v in child['input_sha256'].items() if not Path(k).is_relative_to(ROOT)})
   for mut in [0,2]:
    gd=dest/('g474'+str(mut));run(tag+' G474 transfer '+str(mut),[sys.executable,REF/'guest_compare.py','--directory',gd.relative_to(ROOT),'--cc',cc,'--opt='+opt,'--g474','--mutant',mut])
    if (gd/'report.json').is_file():
     child=json.loads((gd/'report.json').read_text());external.update({k:v for k,v in child['input_sha256'].items() if not Path(k).is_relative_to(ROOT)})
   cost_objects=[]
   for name,src in [('comparison',REF/'compare.c'),('wrapper',REF/'cost.c'),('vendor',hal/'Src/stm32f4xx_hal_uart.c')]:
    obj=dest/('cost-'+name+'.o');cost_objects.append(obj);run(tag+' cost '+name+' compile',[cc,*flags,*vinc,'-DSTM32F439xx','-c',src,'-o',obj])
   for kind in ['direct','generated','vendor']:
    image=dest/('cost-'+kind+'.elf');ok,_=run(tag+' cost '+kind+' link',['arm-none-eabi-ld','--gc-sections','-e','pl_cost_'+kind,'-Ttext=0x08000000',*cost_objects,dest/'helpers.o','-Map='+str(dest/('cost-'+kind+'.map')),'-o',image])
    if ok:
     run(tag+' cost '+kind+' closure',['arm-none-eabi-nm','-u',image],empty=True);run(tag+' cost '+kind+' symbols',['arm-none-eabi-nm','-S',image]);run(tag+' cost '+kind+' size',['arm-none-eabi-size','-A',image]);run(tag+' cost '+kind+' assembly',['arm-none-eabi-objdump','-d',image])
 rows.append(dict(label='sources stable',passed=all(sha(ROOT/p)==h for p,h in pins.items())))
 out=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_source_sha256=external,external_tools_sha256={str(p):sha(p) for p in [qemu,lld]},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in build.rglob('*') if p.is_file()},limits=['No physical UART, DMA, bus ordering or interrupt latency observation.','Actual HAL execution occurs only in separate ARM RAM-register-model guests; no HAL dependency in the original target image.','Frozen-clock witness is bounded; no-return conclusion additionally requires the explicit unchanged-predicate argument.','PPC64 checks parser/service endian and data-model portability, not STM32 endian support.','G474 changed-register map is a preconfigured model, not a complete G474 board port.','Author reference implementations are not learner work or external review.'])
 report=ROOT/a.report;report.parent.mkdir(parents=True,exist_ok=True);report.write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(dict(passed=out['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return int(not out['passed'])
if __name__=='__main__':raise SystemExit(main())
