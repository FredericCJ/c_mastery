"""Actual F439 wrapper + retained MMIO provider relocatable closure, no execution."""
import argparse,json,pathlib,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[3]
def main():
 p=argparse.ArgumentParser();p.add_argument('build',type=pathlib.Path);p.add_argument('cc',choices=['arm-none-eabi-gcc','clang']);p.add_argument('opt',choices=['0','3']);a=p.parse_args();a.build.mkdir(parents=True)
 flags=['-std=c17','-O'+a.opt,'-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-ffunction-sections','-fdata-sections','-Wall','-Wextra','-Werror','-I'+str(ROOT/'labs/serial_followthrough/mcp'),'-I'+str(ROOT/'labs/peripheral_depth/spi'),'-I'+str(ROOT/'labs/peripheral_depth/uart'),'-I'+str(ROOT/'labs/serial_depth/include')]
 if a.cc=='clang':flags+=['--target=arm-none-eabi']
 objects=[]
 for i,n in enumerate(['assessor/serial_followthrough/mcp/target.c','assessor/peripheral_depth/spi/target.c','assessor/peripheral_depth/spi/spi.c','assessor/peripheral_depth/uart/uart.c','assessor/execution/support.c']):
  o=a.build/(str(i)+'.o');cmd=[a.cc,*flags,'-c',str(ROOT/n),'-o',str(o)];print(cmd,flush=True);subprocess.run(cmd,check=True);objects.append(o)
 out=a.build/'route.elf';subprocess.run(['arm-none-eabi-ld','--gc-sections','-e','mc_f439_open','-Ttext=0x10000',*map(str,objects),'-Map='+str(a.build/'route.map'),'-o',str(out)],check=True)
 assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(out)],text=True).strip()
 asm=subprocess.check_output(['arm-none-eabi-objdump','-dr',str(out)],text=True);(a.build/'route.asm').write_text(asm)
 symbols=subprocess.check_output(['arm-none-eabi-nm','--defined-only',str(out)],text=True)
 assert all(n in symbols for n in ['mc_f439_open','ps_f439_pins','ps_f439_io','ps_open','pu_rates'])
 print(json.dumps(dict(passed=True,target='F439 route cross-build, not physical execution',symbols=['mc_f439_open','ps_f439_pins','ps_f439_io','ps_open','pu_rates'],undefined=[])))
if __name__=='__main__':main()
