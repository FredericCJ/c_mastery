"""Explicit original-provider inputs; no library or assessor fallback."""
import argparse,pathlib,subprocess
HERE=pathlib.Path(__file__).resolve().parent
def main():
 p=argparse.ArgumentParser();p.add_argument('--build',type=pathlib.Path,required=True);p.add_argument('--source',type=pathlib.Path,default=HERE/'learner.c')
 for n in ['codec','codec-include','depth','depth-include','boot','runtime','compare']:p.add_argument('--'+n,type=pathlib.Path,required=True)
 p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--define',action='append',default=[])
 a=p.parse_args();a.build.mkdir(parents=True);flags=['-std=c17','-O'+a.opt,'-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-Wall','-Wextra','-Werror','-I'+str(HERE),'-I'+str(a.codec_include),'-I'+str(a.depth_include),'-I'+str(HERE/'freestanding'),*['-D'+n for n in a.define]]
 if a.cc=='clang':flags+=['--target=arm-none-eabi']
 objects=[]
 for i,s in enumerate([a.source,a.codec,a.depth,a.compare,HERE/'tests.c',HERE/'guest.c',a.boot,a.runtime]):
  o=a.build/(str(i)+'.o');objects.append(o);cmd=[a.cc,*flags,'-c',str(s),'-o',str(o)];print(cmd,flush=True);subprocess.run(cmd,check=True)
 cmd=['arm-none-eabi-ld','--gc-sections','-T',str(HERE/'target.ld'),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(a.build/'image.elf')];print(cmd,flush=True);subprocess.run(cmd,check=True)
 assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(a.build/'image.elf')],text=True).strip();print('EF_TARGET_BUILD_PASS explicit-no-library-closure')
if __name__=='__main__':main()
