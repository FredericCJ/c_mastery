"""Execute source-bound adc arithmetic, preload and actual register adapters."""
from pathlib import Path
import argparse,datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
LAB=ROOT/'labs/peripheral_depth/adc';REF=ROOT/'assessor/peripheral_depth/adc'
CLOCK=ROOT/'assessor/peripheral_depth/uart/uart.c'
SERIAL=ROOT/'assessor/serial_depth/serial_depth.c';FW=ROOT/'assessor/firmware_io/firmware_io.c';TIMER=ROOT/'assessor/peripheral_depth/timer/timer.c'
HELPERS=ROOT/'assessor/execution/support.c';BOOT=ROOT/'labs/smp_depth/acpi/big_endian.S';BE=ROOT/'assessor/peripheral_depth/mmio/big_endian.c'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return [Path(__file__),CLOCK,SERIAL,FW,TIMER,HELPERS,BOOT,BE,ROOT/'labs/peripheral_depth/uart/uart.h',ROOT/'labs/peripheral_depth/timer/timer.h',ROOT/'labs/serial_depth/include/serial_depth.h',ROOT/'labs/firmware_io/include/firmware_io.h',*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();build=ROOT/'build/adc-depth'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');build.mkdir(parents=True);pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};rows=[]
 inc=['-I'+str(p) for p in [LAB,ROOT/'labs/peripheral_depth/uart',ROOT/'labs/peripheral_depth/timer',ROOT/'labs/serial_depth/include',ROOT/'labs/firmware_io/include']]
 def run(label,cmd,expected=0,marker=None,empty=False):
  cmd=list(map(str,cmd));log=build/(str(len(rows))+'.log')
  try:r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=90);out=r.stdout+r.stderr;rc=r.returncode
  except (OSError,subprocess.TimeoutExpired) as e:out=str(e);rc=None
  log.write_text(out);ok=rc==expected and (not marker or marker in out) and (not empty or not out.strip()) and not any(s in out for s in ['runtime error:','ERROR: AddressSanitizer'])
  rows.append(dict(label=label,command=cmd,returncode=rc,expected=expected,marker=marker,empty=empty,passed=ok,raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,ok,flush=True);return ok,out
 qemu=ROOT/'build/checkpoint-tools/qemu-user-local/usr/bin/qemu-ppc64';lld=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21'
 for tool in ['gcc','clang','arm-none-eabi-gcc',qemu,lld]:run(str(tool)+' version',[tool,'--version'])
 run('environment',['uname','-a'])
 for cc,opt,tag in [('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*inc]+(['-O1','-g'] if 'asan' in tag else [])+(['-DNDEBUG'] if 'release' in tag else [])
  for v in [0,1,2,3,4,5,6,7,'stub']:
   exe=build/(tag+'-'+str(v));src=LAB/'adc.c' if v=='stub' else REF/'adc.c';ok,_=run(tag+' compile '+str(v),[cc,*flags,'-DPA_MUTANT='+str(0 if v=='stub' else v),src,CLOCK,SERIAL,LAB/'tests.c','-o',exe])
   if ok:run(tag+' execute '+str(v),[exe],0 if v==0 else 1,'PA_PASS' if v==0 else 'PA_CHECK:')
  for v in [0,1,2,3,4,'stub']:
   exe=build/(tag+'-target-'+str(v));src=LAB/'target.c' if v=='stub' else REF/'target.c';ok,_=run(tag+' target compile '+str(v),[cc,*flags,'-DPA_TARGET_MODEL','-DPA_TARGET_MUTANT='+str(0 if v=='stub' else v),src,REF/'target_test.c',REF/'adc.c',CLOCK,SERIAL,'-o',exe])
   if ok:run(tag+' target execute '+str(v),[exe],0 if v==0 else 1,'PA_TARGET_PASS' if v==0 else 'PA_TARGET_CHECK:')
  for v in [0,1,2,'stub']:
   exe=build/(tag+'-comp-'+str(v));src=LAB/'comparator.c' if v=='stub' else REF/'comparator.c';ok,_=run(tag+' comparator compile '+str(v),[cc,*flags,'-DPA_TARGET_MODEL','-DPA_COMP_MUTANT='+str(0 if v=='stub' else v),src,REF/'comparator_test.c','-o',exe])
   if ok:run(tag+' comparator execute '+str(v),[exe],0 if v==0 else 1,'PA_COMP_PASS' if v==0 else 'PA_COMP_CHECK:')
  for name,src,marker in [('composition',REF/'composition.c','PA_COMPOSE_PASS'),('oracle',REF/'probe.c',None)]:
   exe=build/(tag+'-'+name);ok,_=run(tag+' '+name+' compile',[cc,*flags,REF/'adc.c',CLOCK,SERIAL,src,*([FW,TIMER] if name=='composition' else []),'-o',exe])
   if ok:
    ok,_=run(tag+' '+name+' execute',[exe],marker=marker)
    if ok and name=='oracle':run(tag+' independent oracle',[sys.executable,REF/'oracle.py',ROOT/rows[-1]['raw']])
 for opt in ['-O0','-O2','-O3']:
  for v in [0,1,2,3,4,5,6,7,'stub']:
   label='big-endian'+opt+' '+str(v);exe=build/('be'+opt+str(v));src=LAB/'adc.c' if v=='stub' else REF/'adc.c'
   flags=['--target=powerpc64-linux-gnu','-mabi=elfv2','-std=c17',opt,'-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-stack-protector','-nostdlib','-static','--ld-path='+str(lld),'-Wl,-e,_start','-DPA_BIG_ENDIAN','-DPA_MUTANT='+str(0 if v=='stub' else v),*inc]
   ok,_=run(label+' compile',['clang',*flags,src,CLOCK,SERIAL,LAB/'tests.c',BE,BOOT,'-o',exe])
   if ok:
    run(label+' ELF',['readelf','-hW',exe],marker='big endian');run(label+' closure',['nm','-u',exe],empty=True);run(label+' execute',[qemu,exe],0 if v==0 else 1,'PA_PASS' if v==0 else 'PA_CHECK:')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['-O0','-O3']:
   tag=cc+opt;dest=build/tag;dest.mkdir();objs=[]
   flags=['-std=c17',opt,'-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections',*inc,'-isystem','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib']+(['--target=arm-none-eabi'] if cc=='clang' else [])
   for name,src in [('core',REF/'adc.c'),('target',REF/'target.c'),('clock',CLOCK),('serial',SERIAL),('firmware',FW),('timer',TIMER),('comparator',REF/'comparator.c'),('helpers',HELPERS)]:
    obj=dest/(name+'.o');objs.append(obj);ok,_=run(tag+' '+name+' compile',[cc,*flags,'-c',src,'-o',obj])
    if ok:run(tag+' '+name+' imports',['arm-none-eabi-nm','-u',obj])
   elf=dest/'audit.elf';keep=['pa_timing','pa_open','pa_ready','pa_begin','pa_step','pa_cancel','pa_calibrate','pa_ramp','pa_threshold','pa_f439_io','pa_f439_pins','pa_f439_timer','pa_f439_dac_open','pa_f439_dac_write','pa_f439_dac_read','pa_g474_comp_open','pa_g474_comp_poll','pa_g474_comp_close','pu_rates','fw_adc','pt_choose','pt_input'];args=[v for name in keep for v in ['-u',name]]
   ok,_=run(tag+' combined audit link',['arm-none-eabi-ld','--gc-sections','-e','pa_f439_io','-Ttext=0x08000000',*args,*objs,'-Map='+str(dest/'image.map'),'-o',elf])
   if ok:
    run(tag+' image closure',['arm-none-eabi-nm','-u',elf],empty=True);run(tag+' image symbols',['arm-none-eabi-nm',elf]);run(tag+' target disassembly',['arm-none-eabi-objdump','-dr',dest/'target.o']);run(tag+' comparator disassembly',['arm-none-eabi-objdump','-dr',dest/'comparator.o']);run(tag+' cost',['arm-none-eabi-size','-A',elf]);run(tag+' attributes',['arm-none-eabi-readelf','-A',elf])
 run('analog analysis',[sys.executable,REF/'analog_test.py'])
 rows.append(dict(label='sources stable',passed=all(sha(ROOT/p)==h for p,h in pins.items())))
 out=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tools_sha256={str(p):sha(p) for p in [qemu,lld]},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in build.rglob('*') if p.is_file()},limits=['No physical analog accuracy, trigger timing or comparator response observed.','M4 linked image audits both board adapters and original helpers; it is not booted firmware.','Serialized owner excludes ISR and thread mutation. ADC1/common clock and TIM3 ownership are exclusive.','PPC64 core experiment changes endian/data model, not STM32 operating mode.','Actual pu_rates/fw_adc/pt_choose/pt_input references are assessor artifacts.','Original trigger model has declared busy-event rules, not a cycle-accurate F439 peripheral.','G474 comparator selected fields backed by inspected official device/LL source and datasheet; full RM0440 fetch blocked separately.'])
 report=ROOT/a.report;report.parent.mkdir(parents=True,exist_ok=True);report.write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(dict(passed=out['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return int(not out['passed'])
if __name__=='__main__':raise SystemExit(main())
