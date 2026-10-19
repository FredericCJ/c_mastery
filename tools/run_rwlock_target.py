"""Actual freestanding CMX reader/writer and phase/upgrade Cortex-M4 observations with explicit helper audit."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
PUBLIC=ROOT/'labs/execution/sync_depth'
PRIVATE=ROOT/'assessor/legacy_concurrency_sync'
HELPERS={'memset','memcpy','memmove','memcmp','__aeabi_memclr','__aeabi_memclr4','__aeabi_memclr8',
         '__aeabi_memset','__aeabi_memset4','__aeabi_memset8','__aeabi_memcpy','__aeabi_memcpy4','__aeabi_memcpy8'}
VARIANTS=('reference','reader-barge','reader-bound','writer-phase','late-reader','second-upgrader','writer-stub','phase-stub','producer-stub')


def main(report):
    base=ROOT/'reviews/v5/expansion/legacy_concurrency_sync';destination=(ROOT/report).resolve()
    if not destination.is_relative_to(base.resolve()):raise ValueError('Use sync successor evidence directory')
    logs=base/'rwlock_target_runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
    paths=[Path(__file__).resolve(),*[PUBLIC/n for n in ('rwlock.h','rwlock.c','rwphase.h','rwphase.c','rw_m4.c','rwlock_tests.c','rwphase_tests.c','RWLOCK.md')],PRIVATE/'rwlock.c',PRIVATE/'rwphase.c',
           ROOT/'assessor/execution/cmx.c',ROOT/'assessor/execution/support.c',ROOT/'labs/execution/src/cmx.c',
           ROOT/'labs/execution/include/cmx.h',ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld']
    pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'outcomes':[],
           'scope':'Actual logical CMX tasks, writer-preference and phase/upgrade adapters on Cortex-M4 target ABI; original explicit memory helper sources.',
           'limits':['No physical or native context-switch observation, no timing bound; host analysis and target execution remain separate.']}
    def run(command,label,code=0,marker=None):
        command=[str(a) for a in command];result=subprocess.run(command,capture_output=True,text=True,timeout=45);output=result.stdout+result.stderr
        path=logs/(str(len(value['commands'])).zfill(3)+'-'+label+'.log');path.write_text(output,encoding='utf-8',newline='\n')
        passed=result.returncode==code and (marker is None or marker in output)
        value['commands'].append({'command':command,'exit_code':result.returncode,'log':path.relative_to(ROOT).as_posix(),'passed':passed})
        if not passed:raise ValueError(label+': '+output)
        return output
    try:
        value['toolchains']={name:run([name,'--version'],name).splitlines()[0] for name in ('arm-none-eabi-gcc','clang','qemu-system-arm')}
        with tempfile.TemporaryDirectory(prefix='rwlock-target-') as directory:
            build=Path(directory)
            for compiler,optimization in (('gcc','-O0'),('gcc','-O3'),('clang','-O3')):
                profile=compiler+optimization
                for variant in VARIANTS:
                    name=profile+'-'+variant;cc=['arm-none-eabi-gcc'] if compiler=='gcc' else ['clang','--target=arm-none-eabi']
                    flags=['-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-std=c17',optimization,'-ffreestanding','-fno-builtin',
                           '-fno-stack-protector','-ffunction-sections','-fdata-sections','-fno-unwind-tables','-fno-asynchronous-unwind-tables',
                           '-Wall','-Wextra','-Werror','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/execution/include'),
                           '-DCDR_MUTANT='+str({'reader-barge':1,'reader-bound':2}.get(variant,0)),
                           '-DCPF_MUTANT='+str({'writer-phase':1,'late-reader':2,'second-upgrader':3}.get(variant,0))]
                    if compiler=='gcc':flags+=['-fno-tree-loop-distribute-patterns']
                    if variant=='producer-stub':flags+=['-Wno-unused-parameter']
                    sources=[ROOT/('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c'),
                             ROOT/'assessor/execution/support.c',PUBLIC/'rwlock.c' if variant=='writer-stub' else PRIVATE/'rwlock.c',
                             PUBLIC/'rwphase.c' if variant=='phase-stub' else PRIVATE/'rwphase.c',PUBLIC/'rw_m4.c']
                    objects=[]
                    for index,source in enumerate(sources):
                        obj=build/(name+'-'+str(index)+'.o');run([*cc,*flags,'-c',source,'-o',obj],name+'-compile-'+str(index));objects.append(obj)
                    image=build/(name+'.elf');mapping=build/(name+'.map')
                    run(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nodefaultlibs','-nostartfiles',*objects,
                         '-T',ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld','-Wl,--gc-sections','-Wl,-Map='+str(mapping),'-o',image],name+'-link')
                    undefined=run(['arm-none-eabi-nm','-u',image],name+'-undefined')
                    symbols=run(['arm-none-eabi-nm','-n',image],name+'-symbols')
                    assembly=run(['arm-none-eabi-objdump','-d',image],name+'-assembly')
                    text=mapping.read_text(encoding='utf-8');(logs/(name+'.map')).write_text(text,encoding='utf-8',newline='\n')
                    names={line.split()[-1] for line in symbols.splitlines() if line.split()}
                    helpers=names & HELPERS
                    forbidden={s for s in names if s.startswith(('__aeabi_','__atomic_','__sync_','__stack_chk')) and s not in HELPERS}
                    if undefined.strip() or forbidden or re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',text) or 'bkpt' not in assembly:
                        raise ValueError('Undeclared runtime dependency or missing semihost instruction: '+name)
                    output=run(['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-nic','none',
                                '-semihosting-config','enable=on,target=native','-kernel',image],name+'-execute',0 if variant=='reference' else 1,
                               'RWD_M4_PASS' if variant=='reference' else 'RWD_M4_CHECK')
                    value['outcomes'].append({'profile':profile,'variant':variant,'passed':True,'output':output,
                        'image_sha256':hashlib.sha256(image.read_bytes()).hexdigest(),'undefined_symbols':[],
                        'linked_course_helpers':sorted(helpers),'forbidden_helpers':[]})
    except (OSError,ValueError,subprocess.SubprocessError) as error:value['error']=str(error)
    value['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items())
    value['passed']='error' not in value and value['source_stable'] and len(value['outcomes'])==27
    value['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
    for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':value['passed'],'outcomes':len(value['outcomes']),'error':value.get('error')}));return 0 if value['passed'] else 1


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    raise SystemExit(main(parser.parse_args().report))
