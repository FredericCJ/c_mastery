"""Build an explicitly supplied solution and run only the original M4 guest."""
import argparse,hashlib,json,re,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent
def build(work,compiler,opt,sources,linker,variant,run):
 work.mkdir(parents=True,exist_ok=True);cc=['arm-none-eabi-gcc'] if compiler=='gcc' else ['clang','--target=arm-none-eabi']
 flags=['-std=c17',opt,'-Wall','-Wextra','-Werror','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fstack-usage','-DCS_SLEEP_MUTANT='+str(variant if isinstance(variant,int) else 0),'-I'+str(HERE)]
 if compiler=='gcc':flags+=['-fno-tree-loop-distribute-patterns']
 objects=[]
 for i,source in enumerate(sources):
  obj=work/(str(i)+'.o');run([*cc,*flags,'-c',source,'-o',obj]);objects.append(obj)
 image=work/'supervision.elf';mapping=work/'supervision.map';link=['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nostartfiles','-nodefaultlibs',*objects,'-T',linker,'-Wl,--gc-sections','-Wl,-u,cs_f439_stop,-u,cs_iwdg_start,-u,cs_iwdg_feed,-u,cs_stop_finish','-Wl,-Map='+str(mapping),'-o',image];run(link)
 data={name:run(cmd) for name,cmd in [('symbols.txt',['arm-none-eabi-nm','-n',image]),('undefined.txt',['arm-none-eabi-nm','-u',image]),('assembly.txt',['arm-none-eabi-objdump','-d',image]),('size.txt',['arm-none-eabi-size',image])]}
 if data['undefined.txt'].strip() or re.search(r'lib(?:gcc|c|atomic)\.a|compiler-rt|crt\w*\.o',mapping.read_text(encoding='utf-8')):raise ValueError('Undeclared target dependency')
 for name,text in data.items():(work/name).write_text(text,encoding='utf-8',newline='\n')
 return image,{'image_sha256':hashlib.sha256(image.read_bytes()).hexdigest(),'sources':list(map(str,sources)),'link':list(map(str,link)),'board_functions':'F439 binding retained/compiled only; MPS2 does not execute those peripheral accesses.'}

def command(image):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-nic','none','-semihosting-config','enable=on,target=native','-kernel',str(image)]
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--image',type=Path,required=True);a=p.parse_args();r=subprocess.run(command(a.image),capture_output=True,text=True,timeout=15);print(r.stdout+r.stderr,end='');raise SystemExit(r.returncode)
