"""Run the public contract against explicit sources; default is an unfinished learner."""
from pathlib import Path
import argparse,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('--cc',default='gcc');p.add_argument('--source',type=Path,action='append');p.add_argument('--build',type=Path,required=True);a=p.parse_args()
 lab=Path(__file__).resolve().parent;a.build.mkdir(parents=True,exist_ok=True);exe=a.build/'variadic-contract'
 subprocess.run([a.cc,'-std=c17','-O0','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(lab),*map(str,a.source or [lab/'learner.c']),str(lab/'scalars.c'),str(lab/'tests.c'),'-o',str(exe)],check=True)
 return subprocess.run([str(exe)]).returncode
if __name__=='__main__':raise SystemExit(main())
