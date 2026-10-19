"""Compile the finite cohort model with explicit completed per-dispatch producer imports."""
import argparse
from pathlib import Path
import subprocess,tempfile
HERE=Path(__file__).resolve().parent
def command(compiler,flags,source,server,advanced,execution_include,server_include,binary,mutant=0):
    paths=[source,server,advanced,HERE/'specimens.c',HERE/'tests.c']
    if any(not p.is_file() for p in paths):raise ValueError('Explicit completed or learner source files required')
    return [compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(HERE),'-I'+str(execution_include),'-I'+str(server_include),
            *map(str,paths),'-DSU_MUTANT='+str(mutant),'-o',str(binary)]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('source','server-source','advanced-source','execution-include','server-include'):parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--compiler',default='cc');parser.add_argument('--case',choices=('cohort','overrun','guard','comparison','stream'),required=True)
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='cohort-') as directory:
        binary=Path(directory)/'cohort'
        result=subprocess.run(command(args.compiler,['-O0'],args.source,args.server_source,args.advanced_source,args.execution_include,args.server_include,binary))
        return result.returncode if result.returncode else subprocess.run([binary,args.case]).returncode
if __name__=='__main__':raise SystemExit(main())
