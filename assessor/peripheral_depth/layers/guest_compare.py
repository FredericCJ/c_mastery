"""Build/run actual vendor UART only in its ARM environment over an original RAM model."""
import argparse,hashlib,json,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
LAB=ROOT/'labs/peripheral_depth/layers';REF=ROOT/'assessor/peripheral_depth/layers'
VENDOR=Path('/home/frede/cmastery-architecture-tools')
def main():
 p=argparse.ArgumentParser();p.add_argument('--directory',required=True);p.add_argument('--cc',default='arm-none-eabi-gcc');p.add_argument('--opt',default='-O2');p.add_argument('--mutant',type=int,default=0);p.add_argument('--g474',action='store_true');p.add_argument('--suite',choices=['compare','callback','frozen_tick'],default='compare');a=p.parse_args();d=ROOT/a.directory;d.mkdir(parents=True,exist_ok=False);rows=[]
 def run(label,cmd,expected=0,marker=None,empty=False):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=45);out=r.stdout+r.stderr;(d/(label+'.txt')).write_text(out);ok=r.returncode==expected and (not marker or marker in out) and (not empty or not out.strip());rows.append(dict(label=label,command=cmd,returncode=r.returncode,expected=expected,marker=marker,passed=ok));print(label,ok,flush=True);return ok
 hal=VENDOR/'stm32f4xx-hal-driver-1.8.3';device=VENDOR/'cmsis-device-f4-2.6.10';cmsis=VENDOR/'CMSIS_5-5.9.0'
 if a.g474:
  assert a.suite=='compare'
  provision=json.loads((ROOT/'research/v5/expansion/legacy_advanced_layers/g4_provisioning.json').read_text());hal=Path(provision[0]['path']);device=Path(provision[1]['path'])
 flags=['-std=c17',a.opt,'-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-DSTM32G474xx' if a.g474 else '-DSTM32F439xx','-DPL_GUEST','-Dmain=pl_guest_main','-DPL_COMPARE_MUTANT='+str(a.mutant),'-I'+str(LAB),*['-I'+str(x) for x in [hal/'Inc',device/'Include',cmsis/'CMSIS/Core/Include']],'-isystem','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib']+(['--target=arm-none-eabi'] if a.cc=='clang' else [])+(['-DPL_G474'] if a.g474 else [])
 units=[REF/'compare.c',REF/(a.suite+'_test.c'),LAB/'thumb_start.c',ROOT/'assessor/execution/support.c',hal/('Src/stm32g4xx_hal_uart.c' if a.g474 else 'Src/stm32f4xx_hal_uart.c')]+([hal/'Src/stm32f4xx_hal_dma.c'] if a.suite=='callback' else []);objects=[];pins={}
 for i,source in enumerate(units):
  obj=d/(str(i)+'.o');objects.append(obj);pins[str(source)]=hashlib.sha256(source.read_bytes()).hexdigest()
  if not run('compile'+str(i),[a.cc,*flags,'-MMD','-MF',str(obj)+'.d','-c',source,'-o',obj]):break
  run('object-imports'+str(i),['arm-none-eabi-nm','-u',obj]);run('assembly'+str(i),['arm-none-eabi-objdump','-dr',obj])
  for word in Path(str(obj)+'.d').read_text().replace('\\\n',' ').split():
   f=Path(word)
   if f.is_file():pins[str(f)]=hashlib.sha256(f.read_bytes()).hexdigest()
 if all(x['passed'] for x in rows):
  elf=d/'guest.elf'
  if run('link',['arm-none-eabi-ld','--gc-sections','-T',LAB/'link.ld',*objects,'-Map='+str(d/'guest.map'),'-o',elf]):
   run('closure',['arm-none-eabi-nm','-u',elf],empty=True);run('size',['arm-none-eabi-size','-A',elf]);run('symbols',['arm-none-eabi-nm',elf]);run('execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf],42 if a.suite=='frozen_tick' else 1 if a.mutant and (a.mutant!=2 or a.g474) else 0,'PL_FROZEN_4096_WAIT_ITERATIONS' if a.suite=='frozen_tick' else 'PL_CALLBACK_PASS' if a.suite=='callback' else 'PL_COMPARE_CHECK:' if a.mutant and (a.mutant!=2 or a.g474) else 'PL_COMPARE_PASS')
 value=dict(passed=all(x['passed'] for x in rows),commands=rows,input_sha256=pins,limits=['Actual ARM vendor instruction execution over a RAM register model on MPS2, not STM32 peripheral/board behavior.','Unused HAL functions are discarded; the linked selected vendor path is explicit and is not a first-principles target dependency.']);(d/'report.json').write_text(json.dumps(value,indent=2)+'\n');return int(not value['passed'])
if __name__=='__main__':raise SystemExit(main())
