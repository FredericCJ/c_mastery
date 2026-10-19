"""Portable learner build; reference opt-in is explicit."""
from pathlib import Path
import argparse,subprocess
HERE=Path(__file__).resolve().parent
def main():
 p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True);p.add_argument('--cc',default='gcc');p.add_argument('--source',type=Path,default=HERE/'transfers.c');p.add_argument('--atomic-source',type=Path,default=HERE/'atomics.c');p.add_argument('--base-source',type=Path,default=HERE.parent/'b03_depth/src/depth.c');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
 exe=a.build/'follow';cmd=[a.cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(HERE),'-I'+str(HERE.parent/'b03_depth/include'),str(a.source),str(a.atomic_source),str(a.base_source),str(HERE/'tests.c'),'-o',str(exe)]
 subprocess.run(cmd,check=True);return subprocess.run([str(exe)]).returncode
if __name__=='__main__':raise SystemExit(main())
