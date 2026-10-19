"""Run Mesa condition cases with explicit completed CMX and extension imports."""
import argparse
from pathlib import Path
import subprocess
import tempfile
HERE=Path(__file__).resolve().parent


def command(compiler,flags,execution,execution_include,extension,output,mutant=0):
    return [compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(HERE),'-I'+str(execution_include),
            str(execution),str(extension),str(HERE/'condition_tests.c'),'-DCDQ_MUTANT='+str(mutant),'-o',str(output)]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('execution-source','execution-include','condition-source'):
        parser.add_argument('--'+name,required=True,type=Path)
    parser.add_argument('--compiler',default='cc')
    parser.add_argument('--case',required=True,choices=('mesa','broadcast','terminal','predicates','shutdown'))
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='condition-depth-') as directory:
        binary=Path(directory)/'condition'
        build=subprocess.run(command(args.compiler,['-O0','-g'],args.execution_source,args.execution_include,args.condition_source,binary))
        if build.returncode:return build.returncode
        return subprocess.run([str(binary),args.case]).returncode


if __name__=='__main__':raise SystemExit(main())
