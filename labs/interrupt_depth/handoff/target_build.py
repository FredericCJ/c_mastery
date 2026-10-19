"""Explicit sources, actual target objects and closed link; no runtime fallback."""
from pathlib import Path
import argparse,subprocess
HERE=Path(__file__).resolve().parent
def main():
 p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True)
 for key,default in [('source','handoff.c'),('port','port.c'),('wide','wide.c'),('driver','target.c'),('boot','target.S')]:p.add_argument('--'+key,type=Path,default=HERE/default)
 p.add_argument('--frame',type=Path,default=HERE.parent/'frame/frame.c');p.add_argument('--runtime',type=Path,required=True);p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--port-mutant',choices=['0','1','2'],default='0');p.add_argument('--instrument',choices=['0','1'],default='1');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
 flags=['-std=c17','-O'+a.opt,'-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-I'+str(HERE),'-I'+str(HERE.parent/'frame'),'-DXI_PORT_MUTANT='+a.port_mutant,'-DXI_INSTRUMENT='+a.instrument]
 if a.cc=='clang':flags+=['--target=arm-none-eabi']
 objects=[]
 for i,s in enumerate([a.source,a.port,a.wide,a.frame,a.driver,a.boot,a.runtime]):
  obj=a.build/(str(i)+'.o');objects.append(obj);subprocess.run([a.cc,*flags,'-c',str(s),'-o',str(obj)],check=True);subprocess.run(['arm-none-eabi-nm','-u',str(obj)],check=True)
 elf=a.build/'image.elf';subprocess.run(['arm-none-eabi-ld','--gc-sections','-T',str(HERE.parent/'frame/target.ld'),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(elf)],check=True)
 imports=subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True);assert not imports.strip(),imports
 print('XI_TARGET_BUILD_PASS explicit_objects='+str(len(objects)))
if __name__=='__main__':main()
