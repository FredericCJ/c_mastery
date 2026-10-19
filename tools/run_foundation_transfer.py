"""Execute withheld B01 changed-condition author references and compiled controls.

Never runs a learner assessment. The ordinary foundation producer calls this
extension; it can also be run alone during source development.
"""
from __future__ import annotations
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

ROOT=Path(__file__).resolve().parents[1]
CASES=('semantic','layout','cursor','codec','lifecycle','movement','registry','frame')
CONTROLS=(('FD_TRANSFER_LAYOUT','layout'),('FD_TRANSFER_EPOCH','registry'),
          ('FD_TRANSFER_AUTHORITY','frame'),('FD_MUTATE_CURSOR','cursor'),
          ('FD_MUTATE_LIFECYCLE','lifecycle'),('FD_MUTATE_EXTENT','layout'),
          ('FD_MUTATE_ZERO_OFFSET','layout'),('FD_MUTATE_EARLY_EXTENT','layout'),
          ('FD_MUTATE_FLAG_AUTHORITY','frame'),('FD_TRANSFER_TAG_COMMIT','cursor'),
          ('FD_TRANSFER_OWNER_DUPLICATE','registry'),('FD_TRANSFER_QUERY_AUTH','frame'))

def run_campaign(report_path, compilers=('gcc','clang')):
    folder=ROOT/'build/foundation_transfer'
    folder.mkdir(parents=True,exist_ok=True)
    source_paths=[p for p in (ROOT/'assessor/foundation_depth/transfer').rglob('*') if p.is_file()]
    source_paths += [ROOT/'assessor/foundation_depth/depth.c', ROOT/'labs/foundation_depth/include/depth.h',
                     ROOT/'tools/run_foundation_transfer.py', ROOT/'labs/foundation_depth/CONTRACTS.md']
    def pins():
        return {p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(source_paths)}
    report={'schema_version':1,'scope':'G-B01-EXPERIMENT-SCOPE author transfer support; no learner assessment',
            'started_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins(),
            'commands':[],'profiles':[],'passed':False,
            'external_review':'not-performed','independent_learner':'not-performed',
            'physical_observation':'not-performed'}
    def run(argv, expect=0, marker=None):
        proc=subprocess.run(argv,cwd=ROOT,text=True,capture_output=True,timeout=180,
            env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1',
                 'UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'})
        ok=proc.returncode==expect and (marker is None or marker in proc.stdout+proc.stderr)
        report['commands'].append({'command':argv,'cwd':str(ROOT),'exit_code':proc.returncode,
            'expected_exit_code':expect,'required_marker':marker,'stdout':proc.stdout,'stderr':proc.stderr,'passed':ok})
        if not ok: raise RuntimeError('Unexpected command result: '+str(argv)+'\n'+proc.stdout+proc.stderr)
        return proc.stdout
    sources=['assessor/foundation_depth/depth.c','assessor/foundation_depth/transfer/reference.c',
             'assessor/foundation_depth/transfer/tests.c']
    includes=['-Ilabs/foundation_depth/include','-Iassessor/foundation_depth/transfer']
    try:
        for cc in compilers:
            run([cc,'--version'])
            for profile,flags in [('debug',['-O0','-g']),('release',['-O2']),
                                  ('asan',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])]:
                common=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags,*includes]
                exe=folder/(cc+'-'+profile)
                run([*common,*sources,'-o',str(exe)])
                counts={}
                for case in CASES:
                    output=run([str(exe),case],marker='FD_TRANSFER_OK:'+case+':')
                    counts[case]=int(output.strip().split(':')[-1])
                    if counts[case]<=0: raise RuntimeError('Vacuous transfer case: '+case)
                controls=[]
                for macro,case in CONTROLS:
                    mutant=folder/(cc+'-'+profile+'-'+macro)
                    run([*common,'-D'+macro,*sources,'-o',str(mutant)])
                    run([str(mutant),case],expect=1,marker='FD_TRANSFER_CHECK:')
                    controls.append({'macro':macro,'case':case,'semantic_rejection':True})
                report['profiles'].append({'compiler':cc,'profile':profile,'observations':counts,'compiled_controls':controls,'passed':True})
            learner=folder/(cc+'-learner')
            run([cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-O0',*includes,
                 sources[0],'assessor/foundation_depth/transfer/learner.c',sources[2],'-o',str(learner)])
            for case in CASES:
                run([str(learner),case],expect=1,marker='FD_TRANSFER_CHECK:')
            for edition,defines in [('c17',[]),('c23',['-DFD_C23'])]:
                comparison=folder/(cc+'-'+edition+'-comparisons')
                run([cc,'-std='+edition,'-O2','-Wall','-Wextra','-Wpedantic','-Werror',*defines,*includes,
                     sources[0],'assessor/foundation_depth/transfer/comparisons.c',
                     'assessor/foundation_depth/transfer/unrestricted.c','-o',str(comparison)])
                run([str(comparison)],marker='FD_COMPARE_OK:65542')
                run(['objdump','-d',str(comparison)])
                run(['size',str(comparison)])
        cc='arm-none-eabi-gcc'
        run([cc,'--version'])
        flags=['-std=c17','-O2','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin',
               '-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fstack-usage',
               '-Wall','-Wextra','-Wpedantic','-Werror',*includes]
        objects=[]
        for source,name in [(sources[0],'depth'),(sources[1],'transfer')]:
            obj=folder/(name+'.o');objects.append(str(obj))
            run([cc,*flags,'-c',source,'-o',str(obj)])
            undefined=run(['arm-none-eabi-nm','-u',str(obj)])
            symbols={line.split()[-1] for line in undefined.splitlines() if line.strip()}
            allowed={'fd_from_signed','fd_extent','fd_take','fd_decode','fd_apply','fd_move','fd_pool_acquire','fd_pool_get'} if name=='transfer' else set()
            if not symbols<=allowed: raise RuntimeError('Undeclared implicit dependency: '+str(symbols-allowed))
        # No section garbage collection: every reference function remains in the
        # image so a successful link cannot hide an unreferenced helper call.
        image=folder/'transfer.elf'
        run([cc,'-mcpu=cortex-m4','-mthumb','-nostdlib','-nodefaultlibs','-nostartfiles',*objects,
             '-Wl,--no-undefined,-e,fd_step_value,-Ttext=0x08000000,-Map='+str(folder/'transfer.map'),'-o',str(image)])
        undefined=run(['arm-none-eabi-nm','-u',str(image)])
        if undefined.strip(): raise RuntimeError('Unresolved final symbols')
        symbols=run(['arm-none-eabi-nm','--defined-only',str(image)])
        if re.search(r'\b(?:memcpy|memmove|memset|malloc|free|__aeabi_\w+|__atomic_\w+)\b',symbols):
            raise RuntimeError('Forbidden runtime present')
        linkmap=(folder/'transfer.map').read_text()
        if re.search(r'\.a\b|\.so\b',linkmap): raise RuntimeError('Unexpected archive/shared input')
        run(['arm-none-eabi-objdump','-d',str(image)])
        run(['arm-none-eabi-size',str(image)])
        report['target']={'profile':'strict Cortex-M4 Thumb C17','linked_objects':objects,
            'undefined_symbols':[],'libraries':[],'execution':'not-run; link-only audit with no reset vector',
            'artifacts':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in folder.iterdir()
                         if p.suffix in {'.elf','.o','.su','.map'}}}
        report['scope_source_stable']=pins()==report['inspected_paths']
        if not report['scope_source_stable']: raise RuntimeError('Scoped sources changed during run')
        report['passed']=True
    except (RuntimeError,OSError,subprocess.TimeoutExpired) as error:
        report['failure']=str(error)
    report['finished_utc']=datetime.now(timezone.utc).isoformat()
    report_path=Path(report_path)
    report_path.parent.mkdir(parents=True,exist_ok=True)
    report_path.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    return report

if __name__=='__main__':
    parser=argparse.ArgumentParser()
    parser.add_argument('--report',type=Path,default=ROOT/'build/foundation_transfer/report.json')
    parser.add_argument('--compiler',action='append',choices=['gcc','clang'])
    args=parser.parse_args()
    result=run_campaign(args.report,args.compiler or ('gcc','clang'))
    print(json.dumps({'passed':result['passed'],'profiles':len(result['profiles']),
                      'failure':result.get('failure'),'report':str(args.report)},indent=2))
    raise SystemExit(0 if result['passed'] else 1)
