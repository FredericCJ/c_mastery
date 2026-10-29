"""Build the unsolved guest adapter with the unchanged T43 workload."""
import argparse, subprocess, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];LAB=Path(__file__).resolve().parent
def main():
    p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);p.add_argument('--cc',choices=['gcc','clang'],default='gcc');p.add_argument('--source',type=Path,default=LAB/'learner.c');p.add_argument('--run',action='store_true');p.add_argument('--kernel',type=Path);p.add_argument('--busybox',type=Path);a=p.parse_args();a.work.mkdir(parents=True,exist_ok=False)
    common=[a.cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic','-O2','-static'];probe=a.work/'probe';workload=a.work/'workload'
    subprocess.run([*common,str(a.source),'-o',str(probe)],check=True)
    subprocess.run([*common,'-I'+str(ROOT/'labs/performance_followthrough'),'-I'+str(ROOT/'labs/algorithms/include'),str(ROOT/'labs/performance_followthrough/observations.c'),str(ROOT/'labs/performance_followthrough/pmu.c'),str(ROOT/'labs/performance_followthrough/pmu_linux.c'),str(ROOT/'labs/performance_followthrough/publication.c'),str(ROOT/'labs/algorithms/src/x86_64.S'),'-pthread','-o',str(workload)],check=True)
    if a.run:
        if not a.kernel or not a.busybox:p.error('--run requires explicit --kernel and --busybox')
        return subprocess.run([sys.executable,'-B',str(LAB/'guest.py'),'--work',str(a.work/'guest'),'--probe',str(probe),'--workload',str(workload),'--kernel',str(a.kernel),'--busybox',str(a.busybox)]).returncode
    return 0
if __name__=='__main__':raise SystemExit(main())
