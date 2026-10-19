"""Run notification cases with explicit completed CMX and notification imports."""
import argparse
from pathlib import Path
import subprocess
import tempfile
HERE=Path(__file__).resolve().parent
def command(compiler,flags,execution,execution_include,extension,output,mutant=0):
    return [compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(HERE),'-I'+str(execution_include),
            str(execution),str(extension),str(HERE/'notifications_tests.c'),'-DCDN_MUTANT='+str(mutant),'-o',str(output)]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('execution-source','execution-include','notifications-source'):
        parser.add_argument('--'+name,required=True,type=Path)
    parser.add_argument('--compiler',default='cc')
    parser.add_argument('--case',required=True,choices=('ledger','handoff','lifetime','histories'))
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='notifications-depth-') as directory:
        binary=Path(directory)/'notifications'
        result=subprocess.run(command(args.compiler,['-O0','-g'],args.execution_source,args.execution_include,args.notifications_source,binary))
        if result.returncode:return result.returncode
        return subprocess.run([str(binary),args.case]).returncode
if __name__=='__main__':raise SystemExit(main())
