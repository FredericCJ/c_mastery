"""Build the original debug subject with explicit learner/runtime sources."""
import hashlib,re
from pathlib import Path
HERE=Path(__file__).resolve().parent
def build(work,compiler,opt,lto,source,helpers,linker,run,variant=0):
 work.mkdir(parents=True,exist_ok=True);cc=['arm-none-eabi-gcc'] if compiler=='gcc' else ['clang','--target=arm-none-eabi']
 flags=['-std=c17',opt,'-g','-Wall','-Wextra','-Werror','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fstack-usage','-DDC_INLINE='+str(int(lto)),'-I'+str(HERE)]
 flags+=['-DDC_ROUTINE_MUTANT='+str(variant if isinstance(variant,int) else 0)]
 if compiler=='gcc':flags+=['-fno-tree-loop-distribute-patterns']
 if lto:flags+=['-flto']
 sources=[HERE/'m4.c',source,helpers];objects=[]
 for i,p in enumerate(sources):
  obj=work/(str(i)+'.o');run([*cc,*flags,'-c',p,'-o',obj]);objects.append(obj)
 image=work/'debug.elf';mapping=work/'debug.map';link=['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-g',*(['-flto',opt,'-fstack-usage'] if lto else []),'-nostdlib','-nostartfiles','-nodefaultlibs',*objects,'-T',linker,'-Wl,--gc-sections','-Wl,-Map='+str(mapping),'-o',image];run(link)
 outputs={name:run(cmd) for name,cmd in [('symbols.txt',['arm-none-eabi-nm','-n',image]),('undefined.txt',['arm-none-eabi-nm','-u',image]),('assembly.txt',['arm-none-eabi-objdump','-d','-S',image]),('lines.txt',['arm-none-eabi-objdump','--dwarf=decodedline',image]),('dwarf.txt',['arm-none-eabi-objdump','--dwarf=info',image]),('size.txt',['arm-none-eabi-size',image])]}
 if outputs['undefined.txt'].strip() or re.search(r'lib(?:gcc|c|atomic)\.a|compiler-rt|crt\w*\.o',mapping.read_text(encoding='utf-8')):raise ValueError('Undeclared target dependency')
 for name,text in outputs.items():(work/name).write_text(text,encoding='utf-8',newline='\n')
 return image,{'sources':list(map(str,sources)),'image_sha256':hashlib.sha256(image.read_bytes()).hexdigest(),'link':list(map(str,link)),'lto':lto}
