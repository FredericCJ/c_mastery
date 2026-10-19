"""Explicit producer sources; no reference solutions are inferred or exported."""
import argparse,pathlib,subprocess
HERE=pathlib.Path(__file__).resolve().parent
def main():
 p=argparse.ArgumentParser();p.add_argument('--build',type=pathlib.Path,required=True)
 p.add_argument('--source',type=pathlib.Path,default=HERE/'learner.c');p.add_argument('--target',type=pathlib.Path,default=HERE/'target_learner.c');p.add_argument('--variants',type=pathlib.Path,default=HERE/'variants_learner.c')
 for n in ['phy','phy-public','ethernet','ethernet-include','bringup','bringup-include','boot','runtime']:p.add_argument('--'+n,type=pathlib.Path,required=True)
 p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--define',action='append',default=[]);p.add_argument('--bad-placement',action='store_true');a=p.parse_args();a.build.mkdir(parents=True)
 flags=['-std=c17','-O'+a.opt,'-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-Wall','-Wextra','-Werror','-DEN_TARGET_MODEL','-DEN_TARGET_IMAGE',*['-I'+str(n) for n in [HERE,a.phy_public,a.ethernet_include,a.bringup_include]],*['-D'+s for s in a.define]]
 if a.cc=='clang':flags+=['--target=arm-none-eabi']
 objects=[]
 for i,s in enumerate([a.source,a.target,a.variants,a.phy,a.ethernet,a.bringup,a.phy_public/'model.c',*[HERE/n for n in ['model.c','tests.c','mapped.c','variants_tests.c','guest.c']],a.boot,a.runtime]):
  o=a.build/(str(i)+'.o');objects.append(o);cmd=[a.cc,*flags,'-c',str(s),'-o',str(o)];print('BUILD',cmd,flush=True);subprocess.run(cmd,check=True)
 cmd=['arm-none-eabi-ld','--gc-sections',*(['--defsym','ETH_BASE=0x10001000'] if a.bad_placement else []),'-T',str(HERE/'target.ld'),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(a.build/'image.elf')];print('LINK',cmd,flush=True);subprocess.run(cmd,check=True)
 assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(a.build/'image.elf')],text=True).strip();print('EN_TARGET_BUILD_PASS explicit-no-library-closure')
if __name__=='__main__':main()
