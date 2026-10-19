"""Hosted C17 explicit provider; the unfinished scaffold is the default."""
from pathlib import Path
import argparse,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('--cc',default='gcc');p.add_argument('--source',type=Path);p.add_argument('--build',type=Path,required=True);a=p.parse_args();lab=Path(__file__).resolve().parent;a.build.mkdir(parents=True,exist_ok=True);exe=a.build/'float-contract'
 subprocess.run([a.cc,'-std=c17','-O0','-Wall','-Wextra','-Wpedantic','-Werror','-frounding-math','-ffp-contract=off','-fno-fast-math','-I'+str(lab),str(a.source or lab/'learner.c'),str(lab/'tests.c'),'-lm','-o',str(exe)],check=True)
 return subprocess.run([str(exe)]).returncode
if __name__=='__main__':raise SystemExit(main())
