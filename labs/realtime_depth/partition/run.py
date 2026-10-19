"""Build explicit learner/completed placement and retained scheduling sources."""
import argparse
from pathlib import Path
import subprocess,tempfile
HERE=Path(__file__).resolve().parent
def command(compiler,flags,source,execution,include,binary,mutant=0):
    return [compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-DRP_MUTANT='+str(mutant),'-I'+str(HERE),'-I'+str(include),source,execution,HERE/'tests.c','-o',binary]
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('source','execution-source','execution-include'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--compiler',default='cc');p.add_argument('--case',default='packing');p.add_argument('--mutant',type=int,default=0);args=p.parse_args()
    with tempfile.TemporaryDirectory(prefix='partition-learner-') as d:
        binary=Path(d)/'tests';subprocess.run([str(v) for v in command(args.compiler,['-O0'],args.source,args.execution_source,args.execution_include,binary,args.mutant)],check=True)
        return subprocess.run([str(binary),args.case]).returncode
if __name__=='__main__':raise SystemExit(main())
