"""Public native scaffold runner; source selection is explicit."""
from pathlib import Path
import argparse,subprocess
HERE=Path(__file__).resolve().parent
def main():
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path,default=HERE/'frame.c');p.add_argument('--cc',default='cc');p.add_argument('--build',type=Path,required=True);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True);exe=a.build/'frame-tests'
 subprocess.run([a.cc,'-std=c17','-O2','-Wall','-Wextra','-Werror','-I'+str(HERE),str(a.source),str(HERE/'tests.c'),'-o',str(exe)],check=True)
 return subprocess.run([str(exe.resolve())]).returncode
if __name__=='__main__':raise SystemExit(main())
