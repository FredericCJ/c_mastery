"""Public C17 core runner. Completed providers are explicit source inputs."""
import argparse,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent;ROOT=HERE.parents[1]
GROUPS=('admission','lifecycle','scheduling','faults','timers','timing','masks-sleep','helper')
def main():
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path,default=HERE/'core/learner.c');p.add_argument('--cmx',type=Path,default=ROOT/'labs/execution/src/cmx.c');p.add_argument('--cc',default='gcc');p.add_argument('--optimization',choices=['O0','O2'],default='O2');p.add_argument('--group',choices=GROUPS);p.add_argument('--build',type=Path,required=True);a=p.parse_args();a.build=a.build.resolve();a.build.mkdir(parents=True,exist_ok=False)
 exe=a.build/'core';subprocess.run([a.cc,'-std=c17','-'+a.optimization,'-g','-Wall','-Wextra','-Werror','-I'+str(HERE/'core'),'-I'+str(ROOT/'labs/execution/include'),str(HERE/'core/tests.c'),str(a.source),str(a.cmx),'-o',str(exe)],check=True)
 for group in (a.group,) if a.group else GROUPS:subprocess.run([str(exe),group],check=True)
if __name__=='__main__':main()
