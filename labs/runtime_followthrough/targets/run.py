"""Build actual learner-owned core; require explicit provider, no reference fallback."""
from pathlib import Path
import argparse,subprocess
HERE=Path(__file__).resolve().parent;LABS=HERE.parents[1]
p=argparse.ArgumentParser();p.add_argument('--cvm',type=Path,required=True);p.add_argument('--probe',type=Path);p.add_argument('--build',type=Path,required=True);p.add_argument('--cc',default='gcc');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
cmd=[a.cc,'-std=c17','-O0','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(LABS/'runtime/include'),'-I'+str(LABS/'kernel_followthrough/runtime'),str(a.cvm),str(a.probe or LABS/'kernel_followthrough/runtime/probe.c'),str(HERE/'driver.c'),'-o',str(a.build/'probe')]
r=subprocess.run(cmd)
if r.returncode:raise SystemExit(r.returncode)
raise SystemExit(subprocess.run([str((a.build/'probe').resolve())]).returncode)
