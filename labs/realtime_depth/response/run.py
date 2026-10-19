"""Response-analysis learner runner with explicit completed original CMX import."""
import argparse
from pathlib import Path
import subprocess
import tempfile
HERE=Path(__file__).resolve().parent
def command(compiler,flags,source,execution,include,binary,mutant=0):
    return [compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(HERE),'-I'+str(include),
      str(source),str(execution),str(HERE/'tests.c'),'-DRTA_MUTANT='+str(mutant),'-o',str(binary)]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('source','execution-source','execution-include'):parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--compiler',default='cc');parser.add_argument('--case',choices=('basic','jitter','arithmetic','limit','interference','stream'),required=True)
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='rta-') as directory:
        binary=Path(directory)/'analysis';result=subprocess.run(command(args.compiler,['-O0'],args.source,args.execution_source,args.execution_include,binary))
        return result.returncode if result.returncode else subprocess.run([str(binary),args.case]).returncode
if __name__=='__main__':raise SystemExit(main())
