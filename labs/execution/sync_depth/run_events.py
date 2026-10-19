"""Run event/sequence cases with explicit completed CMX and event imports."""
import argparse
from pathlib import Path
import subprocess
import tempfile
HERE=Path(__file__).resolve().parent
def command(compiler,flags,execution,execution_include,extension,output,mutant=0):
    return [compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(HERE),'-I'+str(execution_include),
            str(execution),str(extension),str(HERE/'events_tests.c'),'-DCDE_MUTANT='+str(mutant),'-o',str(output)]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('execution-source','execution-include','events-source'):
        parser.add_argument('--'+name,required=True,type=Path)
    parser.add_argument('--compiler',default='cc')
    parser.add_argument('--case',required=True,choices=('truth','order','terminal','sequence'))
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='events-depth-') as directory:
        binary=Path(directory)/'events'
        result=subprocess.run(command(args.compiler,['-O0','-g'],args.execution_source,args.execution_include,args.events_source,binary))
        if result.returncode:return result.returncode
        return subprocess.run([str(binary),args.case]).returncode
if __name__=='__main__':raise SystemExit(main())
