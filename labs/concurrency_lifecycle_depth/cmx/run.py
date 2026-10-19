"""Build the queue/lifetime CMX adapter with explicit completed producer imports."""
import argparse
from pathlib import Path
import subprocess
import tempfile
HERE=Path(__file__).resolve().parent
def command(compiler,flags,execution,execution_include,lifecycle,lifecycle_include,adapter,binary,mutant=0,core_mutant=0):
    return [compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(HERE),'-I'+str(execution_include),'-I'+str(lifecycle_include),
        str(execution),str(lifecycle),str(adapter),str(HERE/'tests.c'),'-DCC_MUTANT='+str(mutant),'-DCL_MUTANT='+str(core_mutant),'-o',str(binary)]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('execution-source','execution-include','lifecycle-source','lifecycle-include','adapter-source'):parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--compiler',default='cc');parser.add_argument('--case',choices=('queue','terminal','condition','lifetime','dispatch'),required=True)
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='cmx-byte-queue-') as directory:
        binary=Path(directory)/'queue'
        built=subprocess.run(command(args.compiler,['-O0','-g'],args.execution_source,args.execution_include,args.lifecycle_source,args.lifecycle_include,args.adapter_source,binary))
        if built.returncode:return built.returncode
        return subprocess.run([str(binary),args.case]).returncode
if __name__=='__main__':raise SystemExit(main())
