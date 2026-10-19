"""Explicit source/boot/runtime adapter: no reference fallback or implicit runtime."""
from pathlib import Path
import argparse,subprocess
HERE=Path(__file__).resolve().parent
def main():
 p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True);p.add_argument('--source',type=Path,default=HERE/'frame.c');p.add_argument('--driver',type=Path,default=HERE/'target.c');p.add_argument('--boot',type=Path,default=HERE/'target.S');p.add_argument('--runtime',type=Path,required=True);p.add_argument('--fp-callee',type=Path);p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--mutant',default='0');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
 flags=['-std=c17','-O'+a.opt,'-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-I'+str(HERE),'-DXF_MUTANT='+a.mutant,'-DXF_FP_CALLEE='+str(int(a.fp_callee is not None))]
 if a.cc=='clang':flags+=['--target=arm-none-eabi']
 sources=[a.source,a.driver,a.boot,a.runtime]+([a.fp_callee] if a.fp_callee else []);objects=[]
 for i,s in enumerate(sources):
  obj=a.build/(str(i)+'.o');objects.append(obj)
  subprocess.run([a.cc,*flags,*(['-mfloat-abi=softfp','-mfpu=fpv4-sp-d16'] if s==a.fp_callee else []),'-c',str(s),'-o',str(obj)],check=True)
  subprocess.run(['arm-none-eabi-nm','-u',str(obj)],check=True)
 elf=a.build/'image.elf'
 subprocess.run(['arm-none-eabi-ld','--gc-sections','-T',str(HERE/'target.ld'),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(elf)],check=True)
 imports=subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True);assert not imports.strip(),imports
 print('XF_TARGET_BUILD_PASS explicit_objects='+str(len(objects)))
if __name__=='__main__':main()
