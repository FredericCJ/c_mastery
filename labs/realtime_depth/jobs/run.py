"""Explicit-import learner runner: multiple-job scheduler, actual CMX and completed response analysis."""
import argparse
from pathlib import Path
import subprocess
import tempfile
HERE=Path(__file__).resolve().parent
def command(compiler,flags,source,advanced,execution,include,response,response_include,binary,mutant=0):
    if any(not p.is_file() for p in (source,advanced,execution,response,HERE/'tests.c')) or not include.is_dir() or not response_include.is_dir():
        raise ValueError('Explicit source inputs must be files and include inputs directories')
    return [compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,
      '-I'+str(HERE),'-I'+str(include),'-I'+str(response_include),str(source),str(advanced),str(execution),str(response),
      str(HERE/'tests.c'),'-DSJ_MUTANT='+str(mutant),'-o',str(binary)]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('source','advanced-source','execution-source','execution-include','response-source','response-include'):parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--compiler',default='cc');parser.add_argument('--case',choices=('order','edf','backlog','assumptions','refusal','stream'),required=True)
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='jobs-') as directory:
        binary=Path(directory)/'jobs';r=subprocess.run(command(args.compiler,['-O0'],args.source,args.advanced_source,args.execution_source,args.execution_include,args.response_source,args.response_include,binary))
        return r.returncode if r.returncode else subprocess.run([str(binary),args.case]).returncode
if __name__=='__main__':raise SystemExit(main())
