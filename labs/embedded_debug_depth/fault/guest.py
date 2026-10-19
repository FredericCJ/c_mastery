"""Build an explicitly supplied fault/MPU teaching target and retained frame import."""
import argparse,hashlib,json,re,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent
def build(work,compiler,opt,source,driver,frame,frame_include,helpers,linker,variant,run):
 work.mkdir(parents=True,exist_ok=True);cc=['arm-none-eabi-gcc'] if compiler=='gcc' else ['clang','--target=arm-none-eabi']
 flags=['-std=c17',opt,'-g','-Wall','-Wextra','-Werror','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fstack-usage','-DDF_TARGET_MUTANT='+str(variant if isinstance(variant,int) else 0),'-I'+str(HERE),'-I'+str(frame_include)]
 if compiler=='gcc':flags+=['-fno-tree-loop-distribute-patterns']
 sources=[HERE/'m4.c',source,driver,frame,helpers,HERE/'entry.S'];objects=[]
 for i,p in enumerate(sources):
  obj=work/(str(i)+'.o');run([*cc,*(['-mcpu=cortex-m4','-mthumb','-g'] if p.suffix=='.S' else flags),'-c',p,'-o',obj]);objects.append(obj)
 image=work/'fault.elf';mapping=work/'fault.map';link=['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nostartfiles','-nodefaultlibs',*objects,'-T',linker,'-Wl,--gc-sections','-Wl,-Map='+str(mapping),'-o',image];run(link)
 outputs={name:run(cmd) for name,cmd in [('symbols.txt',['arm-none-eabi-nm','-n',image]),('undefined.txt',['arm-none-eabi-nm','-u',image]),('assembly.txt',['arm-none-eabi-objdump','-d','-S',image]),('lines.txt',['arm-none-eabi-objdump','--dwarf=decodedline',image]),('size.txt',['arm-none-eabi-size',image])]}
 if outputs['undefined.txt'].strip() or re.search(r'lib(?:gcc|c|atomic)\.a|compiler-rt|crt\w*\.o',mapping.read_text(encoding='utf-8')):raise ValueError('Undeclared runtime dependency')
 for name,text in outputs.items():(work/name).write_text(text,encoding='utf-8',newline='\n')
 return image,{'image_sha256':hashlib.sha256(image.read_bytes()).hexdigest(),'sources':list(map(str,sources)),'link':list(map(str,link))}
def command(image):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-nic','none','-semihosting-config','enable=on,target=native','-kernel',str(image)]
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,required=True);p.add_argument('--source',type=Path,required=True);p.add_argument('--driver',type=Path,required=True);p.add_argument('--frame',type=Path,required=True);p.add_argument('--frame-include',type=Path,required=True);p.add_argument('--helpers',type=Path,required=True);p.add_argument('--linker',type=Path,required=True);p.add_argument('--cc',choices=['gcc','clang'],default='gcc');p.add_argument('--opt',default='-O3');p.add_argument('--variant',type=int,default=0);a=p.parse_args()
 def run(c):
  r=subprocess.run(list(map(str,c)),capture_output=True,text=True,timeout=30)
  if r.returncode:raise ValueError(r.stdout+r.stderr)
  return r.stdout+r.stderr
 image,audit=build(a.build.resolve(),a.cc,a.opt,a.source.resolve(),a.driver.resolve(),a.frame.resolve(),a.frame_include.resolve(),a.helpers.resolve(),a.linker.resolve(),a.variant,run);print(run(command(image)));print(json.dumps(audit,indent=2))
