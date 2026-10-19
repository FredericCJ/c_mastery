"""Explicit cumulative learner components; no assessor fallback."""
from pathlib import Path
import argparse,subprocess
HERE=Path(__file__).resolve().parent;LABS=HERE.parents[1]
p=argparse.ArgumentParser();p.add_argument('--source',type=Path,default=HERE/'learner.c')
for key in ['cvm','execution','verifier','objects','registrations','pool']:p.add_argument('--'+key,type=Path,required=True)
p.add_argument('--build',type=Path,required=True);p.add_argument('--cc',default='gcc');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
inc=['-I'+str(f) for f in [HERE,LABS/'runtime_followthrough/execution',LABS/'runtime_followthrough/verifier',LABS/'runtime/include',LABS/'object_runtime/include',LABS/'algorithms/include']]
exe=a.build/'budgets'
subprocess.run([a.cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror',*inc,str(a.source),*[str(getattr(a,k)) for k in ['cvm','execution','verifier','objects','registrations','pool']],str(HERE/'tests.c'),'-o',str(exe)],check=True)
raise SystemExit(subprocess.run([str(exe)]).returncode)
