"""Build a ledger preflight around an explicitly supplied existing IRQ driver.

No vector table or frame decoder is generated. All five implementation/runtime
inputs are explicit or default to unsolved exported learner sources.
"""
import argparse,pathlib,subprocess
HERE=pathlib.Path(__file__).resolve().parent;FRAME=HERE.parent/'interrupt_depth/frame'
def main():
 p=argparse.ArgumentParser();p.add_argument('--build',type=pathlib.Path,required=True);p.add_argument('--source',type=pathlib.Path,default=HERE/'learner.c');p.add_argument('--frame',type=pathlib.Path,default=FRAME/'frame.c');p.add_argument('--driver',type=pathlib.Path,default=FRAME/'target.c');p.add_argument('--boot',type=pathlib.Path,default=FRAME/'target.S');p.add_argument('--runtime',type=pathlib.Path,required=True);p.add_argument('--fp-callee',type=pathlib.Path);p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--define',action='append',default=[]);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
 flags=['-std=c17','-O'+a.opt,'-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-I'+str(HERE),'-I'+str(FRAME),'-DXF_FP_CALLEE='+str(int(a.fp_callee is not None)),*['-D'+s for s in a.define]]
 if a.cc=='clang':flags+=['--target=arm-none-eabi']
 sources=[a.source,a.frame,HERE/'tests.c',HERE/'target_prefix.c',a.driver,a.boot,a.runtime]+([a.fp_callee] if a.fp_callee else []);objects=[]
 for i,s in enumerate(sources):
  obj=a.build/(str(i)+'.o');objects.append(obj);extra=['-Dxf_target_main=mf_original_target_main'] if s==a.driver else []
  if s==a.fp_callee:extra+=['-mfloat-abi=softfp','-mfpu=fpv4-sp-d16']
  subprocess.run([a.cc,*flags,*extra,'-c',str(s),'-o',str(obj)],check=True);subprocess.run(['arm-none-eabi-nm','-u',str(obj)],check=True)
 subprocess.run(['arm-none-eabi-ld','--gc-sections','-T',str(FRAME/'target.ld'),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(a.build/'image.elf')],check=True)
 assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(a.build/'image.elf')],text=True).strip();print('MF_TARGET_BUILD_PASS explicit_objects='+str(len(objects)))
if __name__=='__main__':main()
