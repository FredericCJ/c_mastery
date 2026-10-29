"""Build unsolved core by default. Assessor paths require explicit selection."""
import argparse,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--cc',default='gcc');p.add_argument('--cost-source',default=str(ROOT/'labs/performance_followthrough/cost.c'));p.add_argument('--foundation-source',default=str(ROOT/'labs/foundations/src/foundation.c'));p.add_argument('--container-source',default=str(ROOT/'labs/containers_depth/learner.c'));p.add_argument('--protocol',action='store_true');p.add_argument('--pmu-source',default=str(ROOT/'labs/performance_followthrough/pmu.c'));a=p.parse_args()
 if a.protocol:
  return subprocess.call([a.cc,'-std=c17','-O0','-g','-Wall','-Wextra','-Werror','-pedantic','-I'+str(ROOT/'labs/performance_followthrough'),str(ROOT/'labs/performance_followthrough/pmu_tests.c'),a.pmu_source,str(ROOT/'labs/performance_followthrough/publication.c'),'-o',a.output])
 return subprocess.call([a.cc,'-std=c17','-O2','-Wall','-Wextra','-Werror','-pedantic',*['-I'+str(ROOT/d) for d in ('labs/performance_followthrough','labs/foundations/include','labs/algorithms/include','labs/containers_depth')],str(ROOT/'labs/performance_followthrough/tests.c'),a.cost_source,a.foundation_source,a.container_source,'-o',a.output])
if __name__=='__main__':raise SystemExit(main())
