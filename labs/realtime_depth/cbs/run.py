"""Run finite CBS tests using an explicit completed CMX producer source."""
import argparse
from pathlib import Path
import subprocess,tempfile
HERE=Path(__file__).resolve().parent
def command(compiler,flags,source,execution,include,binary,mutant=0):
    return [compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags,
      '-I'+str(HERE),'-I'+str(include),'-DCB_MUTANT='+str(mutant),source,execution,HERE/'tests.c','-o',binary]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('source','execution-source','execution-include'):parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--case',choices=('soft','equality','constrained','guards','workload','mutex','stream'),required=True)
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='cbs-') as directory:
        binary=Path(directory)/'tests';result=subprocess.run(command('cc',['-O0'],args.source,args.execution_source,args.execution_include,binary))
        return result.returncode if result.returncode else subprocess.run([binary,args.case]).returncode
if __name__=='__main__':raise SystemExit(main())
