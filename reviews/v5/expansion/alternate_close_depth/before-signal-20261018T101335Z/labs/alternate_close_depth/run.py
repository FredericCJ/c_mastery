"""Run this exported laboratory with learner source or an explicit author reference."""
from pathlib import Path
import argparse,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path);p.add_argument('--cc',default='cc');p.add_argument('--out',type=Path,required=True);a=p.parse_args()
 here=Path(__file__).resolve().parent;source=(a.source or here/'learner.c').resolve();a.out.mkdir(parents=True,exist_ok=True)
 binary=a.out.resolve()/'close-owner'
 subprocess.run([a.cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-pthread','-I'+str(here),str(here/'tests.c'),str(source),'-o',str(binary)],check=True)
 for case in ['core','native','cancel']:subprocess.run([str(binary),case],check=True)
if __name__=='__main__':main()
