"""Compile the learner successor with an explicitly named CVM predecessor."""
from pathlib import Path
import argparse,subprocess
HERE=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--source',type=Path,default=HERE/'learner.c');p.add_argument('--verifier',type=Path,required=True);p.add_argument('--cvm',type=Path,required=True);p.add_argument('--build',type=Path,required=True);p.add_argument('--cc',default='gcc');a=p.parse_args()
a.build.mkdir(parents=True,exist_ok=True);exe=a.build/'execution'
cmd=[a.cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(HERE),'-I'+str(HERE.parents[1]/'runtime/include'),'-I'+str(HERE.parent/'verifier'),str(a.source),str(a.verifier),str(a.cvm),str(HERE/'tests.c'),'-o',str(exe)]
subprocess.run(cmd,check=True);raise SystemExit(subprocess.run([str(exe)]).returncode)
