"""Explicit learner core/header; no hidden repository implementation fallback."""
from pathlib import Path
import argparse,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--include',type=Path,required=True);p.add_argument('--build',type=Path,required=True);p.add_argument('--cc',default='gcc');a=p.parse_args();lab=Path(__file__).resolve().parent;a.build.mkdir(parents=True,exist_ok=True);exe=a.build/'p1-memory'
 subprocess.run([a.cc,'-std=c17','-O0','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(lab),'-I'+str(a.include),str(a.source),str(lab/'core_checks.c'),str(lab/'main.c'),'-o',str(exe)],check=True)
 return subprocess.run([str(exe)]).returncode
if __name__=='__main__':raise SystemExit(main())
