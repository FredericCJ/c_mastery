"""Explicit original Cortex-M4 handoff composition; never flash hardware."""
import argparse,hashlib,json,re,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent
def build(work,cc,opt,source,driver,handoff_c,handoff_s,boot_s,linker,provider_include,helpers,variant,run):
 work.mkdir(parents=True,exist_ok=True);compiler=['arm-none-eabi-gcc'] if cc=='gcc' else ['clang','--target=arm-none-eabi'];flags=['-std=c17',opt,'-g','-Wall','-Wextra','-Werror','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fstack-usage','-DFB_CODE_BASE=0','-DFB_FAULT_VARIANT=0','-DDU_MUTANT='+('2' if variant==2 else '0'),'-DDU_TARGET_MUTANT='+('1' if variant==1 else '0'),'-I'+str(HERE),'-I'+str(provider_include)]
 if cc=='gcc':flags+=['-fno-tree-loop-distribute-patterns']
 inputs=[HERE/'guest_main.c',source,driver,handoff_c,handoff_s,boot_s,helpers];objects=[]
 for index,p in enumerate(inputs):
  obj=work/(str(index)+'.o');run([*compiler,*(['-mcpu=cortex-m4','-mthumb','-g','-DFB_FAULT_VARIANT=0'] if p.suffix=='.S' else flags),'-c',p,'-o',obj]);objects.append(obj)
 image=work/'recovery.elf';mapping=work/'recovery.map';link=['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nostartfiles','-nodefaultlibs',*objects,'-T',linker,'-Wl,--gc-sections','-Wl,-Map='+str(mapping),'-o',image];run(link)
 outputs={n:run(cmd) for n,cmd in [('symbols.txt',['arm-none-eabi-nm','-n',image]),('undefined.txt',['arm-none-eabi-nm','-u',image]),('assembly.txt',['arm-none-eabi-objdump','-d','-S',image]),('headers.txt',['arm-none-eabi-readelf','-l',image]),('size.txt',['arm-none-eabi-size',image])]}
 if outputs['undefined.txt'].strip() or re.search(r'lib(?:gcc|c|atomic)\.a|compiler-rt|crt\w*\.o',mapping.read_text(encoding='utf-8')):raise ValueError('Undeclared runtime dependency')
 for n,out in outputs.items():(work/n).write_text(out,encoding='utf-8',newline='\n')
 return image,{'image_sha256':hashlib.sha256(image.read_bytes()).hexdigest(),'sources':list(map(str,inputs)),'link':list(map(str,link)),'scope':'Actual selected M4 handoff with frozen provider, not F439 Flash or clocks.'}
def command(image):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-nic','none','-semihosting-config','enable=on,target=native','-kernel',str(image)]
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 for n in ('output','source','driver','handoff-c','handoff-s','boot-s','linker','provider-include','helpers'):p.add_argument('--'+n,type=Path,required=True)
 p.add_argument('--compiler',choices=['gcc','clang'],default='gcc');p.add_argument('--opt',choices=['0','2'],default='0');p.add_argument('--run',action='store_true');a=p.parse_args()
 run=lambda cmd:subprocess.check_output(list(map(str,cmd)),stderr=subprocess.STDOUT,text=True,timeout=60)
 image,audit=build(a.output,a.compiler,'-O'+a.opt,a.source,a.driver,a.handoff_c,a.handoff_s,a.boot_s,a.linker,a.provider_include,a.helpers,0,run);result={'audit':audit}
 if a.run:result['output']=run(command(image))
 print(json.dumps(result,indent=2))
