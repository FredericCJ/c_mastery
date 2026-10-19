"""Compile the learner ceiling adapter with an explicitly completed CMX source."""
import argparse
from pathlib import Path
import subprocess,tempfile
HERE=Path(__file__).resolve().parent
def command(compiler,flags,source,execution,include,binary,mutant=0):
    if not source.is_file() or not execution.is_file() or not include.is_dir():raise ValueError('Explicit source files/include directory required')
    return [compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(HERE),'-I'+str(include),str(source),str(execution),str(HERE/'tests.c'),'-DSC_MUTANT='+str(mutant),'-o',str(binary)]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('source','execution-source','execution-include'):parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--compiler',default='cc');parser.add_argument('--case',choices=('table','nested','admission','workloads','stream'),required=True);args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='ceiling-') as directory:
        binary=Path(directory)/'ceiling';r=subprocess.run(command(args.compiler,['-O0'],args.source,args.execution_source,args.execution_include,binary))
        return r.returncode if r.returncode else subprocess.run([str(binary),args.case]).returncode
if __name__=='__main__':raise SystemExit(main())
