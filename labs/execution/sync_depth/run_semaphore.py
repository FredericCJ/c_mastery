"""Run semaphore cases with explicit completed CMX, wait and semaphore imports."""
import argparse
from pathlib import Path
import subprocess
import tempfile
HERE=Path(__file__).resolve().parent


def command(compiler,flags,execution,execution_include,wait,wait_include,semaphore,output,mutant=0):
    return [compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(HERE),'-I'+str(execution_include),'-I'+str(wait_include),
            str(execution),str(wait),str(semaphore),str(HERE/'semaphore_tests.c'),'-DCDSEM_MUTANT='+str(mutant),'-o',str(output)]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('execution-source','execution-include','wait-source','wait-include','semaphore-source'):
        parser.add_argument('--'+name,required=True,type=Path)
    parser.add_argument('--compiler',default='cc')
    parser.add_argument('--case',required=True,choices=('counting','orders','priority','deferred'))
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='sem-depth-') as directory:
        binary=Path(directory)/'sem'
        build=subprocess.run(command(args.compiler,['-O0','-g'],args.execution_source,args.execution_include,args.wait_source,args.wait_include,args.semaphore_source,binary))
        if build.returncode:return build.returncode
        return subprocess.run([str(binary),args.case]).returncode


if __name__=='__main__':raise SystemExit(main())
