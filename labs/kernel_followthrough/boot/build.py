"""Compile learner boot by default; an assessor directory must be explicit."""
import argparse,subprocess
from pathlib import Path
LAB=Path(__file__).resolve().parent;ROOT=LAB.parents[2]
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);p.add_argument('--cc',default='gcc');p.add_argument('--assessor-root',type=Path);a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
 boot=(a.assessor_root/'boot.S') if a.assessor_root else LAB/'boot.S';probe=(a.assessor_root/'probe.c') if a.assessor_root else LAB/'probe.c';objects=[]
 for name,source in [('boot',boot),('entry',LAB/'entry.S'),('probe',probe)]:
  obj=out/(name+'.o');subprocess.run([a.cc,'-std=c17','-O2','-g','-ffreestanding','-fno-builtin','-fno-pie','-fno-stack-protector','-fno-asynchronous-unwind-tables','-mno-red-zone','-mgeneral-regs-only','-I'+str(LAB),'-c',str(source),'-o',str(obj)],check=True);objects.append(str(obj))
 subprocess.run(['ld','-m','elf_x86_64','-nostdlib','-z','max-page-size=4096','-T',str(LAB/'link.ld'),'-o',str(out/'kernel.elf'),*objects],check=True)
if __name__=='__main__':main()
