"""Build reader/writer work with explicit completed CMX and adapter imports."""
import argparse
from pathlib import Path
import subprocess
import tempfile
HERE=Path(__file__).resolve().parent
CASES={'writer':('policy','cancel','bounds','starvation'),'phase':('phases','upgrade','cancel','boundaries'),'native':()}

def command(compiler,flags,execution,include,rw,phase,binary,kind,writer_mutant=0,phase_mutant=0):
    sources=[execution]
    if kind in ('writer','native'):sources.append(rw)
    if kind in ('phase','native'):sources.append(phase)
    sources.append(HERE/('rw_native_tests.c' if kind=='native' else 'rwlock_tests.c' if kind=='writer' else 'rwphase_tests.c'))
    return [compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(HERE),'-I'+str(include),
        *map(str,sources),'-DCDR_MUTANT='+str(writer_mutant),'-DCPF_MUTANT='+str(phase_mutant),'-o',str(binary)]+(['-pthread'] if kind=='native' else [])

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('execution-source','execution-include','rwlock-source','rwphase-source'):parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--compiler',default='cc');parser.add_argument('--kind',choices=CASES,required=True);parser.add_argument('--case')
    args=parser.parse_args()
    if args.kind!='native' and args.case not in CASES[args.kind]:parser.error('Choose a named case for the selected core')
    with tempfile.TemporaryDirectory(prefix='rw-depth-') as directory:
        binary=Path(directory)/'rw'
        result=subprocess.run(command(args.compiler,['-O0','-g'],args.execution_source,args.execution_include,args.rwlock_source,args.rwphase_source,binary,args.kind))
        if result.returncode:return result.returncode
        return subprocess.run([str(binary)]+([] if args.kind=='native' else [args.case])).returncode
if __name__=='__main__':raise SystemExit(main())
