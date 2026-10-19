"""Standalone hosted harness; assessor implementation is an explicit external input."""
import argparse,pathlib,subprocess
p=argparse.ArgumentParser();p.add_argument('--unit',choices=['core','lifecycle'],default='core');p.add_argument('--source',type=pathlib.Path);p.add_argument('--cc',default='cc');p.add_argument('--build',type=pathlib.Path,required=True);a=p.parse_args()
root=pathlib.Path(__file__).resolve().parent;a.build.mkdir(parents=True,exist_ok=True)
name='bringup.c' if a.unit=='core' else 'lifecycle.c';test='tests.c' if a.unit=='core' else 'lifecycle_tests.c'
src=(a.source or root/name).resolve();exe=(a.build/a.unit).resolve()
subprocess.run([a.cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(root),str(src),str(root/test),'-o',str(exe)],check=True)
raise SystemExit(subprocess.run([str(exe)]).returncode)
