"""Explicit learner/prerequisite inputs; no assessor implementation discovery."""
import argparse,pathlib,subprocess
HERE=pathlib.Path(__file__).resolve().parent
def main():
 p=argparse.ArgumentParser();p.add_argument('--build',type=pathlib.Path,required=True)
 p.add_argument('--source',type=pathlib.Path,default=HERE/'learner.c');p.add_argument('--port',type=pathlib.Path,default=HERE/'port_learner.c')
 for n in ['uart','byte','byte-port','serial','edu','uart-include','byte-include','serial-include','edu-include','boot','link','runtime']:p.add_argument('--'+n,type=pathlib.Path,required=True)
 p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--define',action='append',default=[]);p.add_argument('--bad-placement',action='store_true')
 a=p.parse_args();a.build.mkdir(parents=True)
 flags=['-std=c17','-O'+a.opt,'-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-Wall','-Wextra','-Werror','-DR5_TARGET_MODEL','-DPU_TARGET_MODEL',*['-I'+str(n) for n in [HERE,a.uart_include,a.byte_include,a.serial_include,a.edu_include]],*['-D'+n for n in a.define]]
 flags+=['-I'+str(HERE/'freestanding')]
 if a.cc=='clang':flags+=['--target=arm-none-eabi']
 objects=[]
 for i,s in enumerate([a.source,a.port,a.uart,a.byte,a.byte_port,a.serial,a.edu,HERE/'tests.c',HERE/'mapped.c',HERE/'guest.c',a.boot,a.runtime]):
  o=a.build/(str(i)+'.o');objects.append(o);c=[a.cc,*flags,'-c',str(s),'-o',str(o)];print(c,flush=True);subprocess.run(c,check=True)
 c=['arm-none-eabi-ld','--gc-sections',*(['--defsym','R5_DMA_BASE=0x10000000'] if a.bad_placement else []),'-T',str(a.link),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(a.build/'image.elf')];print(c,flush=True);subprocess.run(c,check=True)
 assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(a.build/'image.elf')],text=True).strip();print('R5_TARGET_BUILD_PASS explicit-no-library-closure')
if __name__=='__main__':main()
