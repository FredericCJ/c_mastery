"""Explicit learner producer inputs; no reference fallback or target library."""
import argparse,pathlib,subprocess
HERE=pathlib.Path(__file__).resolve().parent
SOURCES=['serial','codec','fault','controller','boot','runtime','compare']
INCLUDES=['serial','codec']
def main():
 p=argparse.ArgumentParser();p.add_argument('--build',type=pathlib.Path,required=True);p.add_argument('--source',type=pathlib.Path,default=HERE/'learner.c')
 for n in SOURCES:p.add_argument('--'+n,type=pathlib.Path,required=True)
 for n in INCLUDES:p.add_argument('--'+n+'-include',type=pathlib.Path,required=True)
 p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--define',action='append',default=[]);a=p.parse_args();a.build.mkdir(parents=True)
 flags=['-std=c17','-O'+a.opt,'-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-Wall','-Wextra','-Werror','-I'+str(HERE),'-I'+str(HERE/'freestanding'),*['-I'+str(getattr(a,n+'_include')) for n in INCLUDES],*['-D'+n for n in a.define]]
 if a.cc=='clang':flags+=['--target=arm-none-eabi']
 objects=[]
 for i,s in enumerate([a.source,*[getattr(a,n) for n in SOURCES],HERE/'tests.c',HERE/'guest.c']):
  o=a.build/(str(i)+'.o');objects.append(o);cmd=[a.cc,*flags,'-c',str(s),'-o',str(o)];print(cmd,flush=True);subprocess.run(cmd,check=True)
 cmd=['arm-none-eabi-ld','--gc-sections','-T',str(HERE/'target.ld'),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(a.build/'image.elf')];print(cmd,flush=True);subprocess.run(cmd,check=True)
 assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(a.build/'image.elf')],text=True).strip();print('SV_TARGET_BUILD_PASS explicit_sources_no_library')
if __name__=='__main__':main()
