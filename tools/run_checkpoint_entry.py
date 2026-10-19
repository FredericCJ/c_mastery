"""Build and inspect original custom-entry x86-64 Linux images, with local syscall tracing."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]

def audit(path):
    data=path.read_bytes()
    h=struct.unpack_from('<16sHHIQQQIHHHHHH',data)
    if h[0][:6]!=b'\x7fELF\x02\x01' or h[1] not in (2,3) or h[2]!=62:
        raise ValueError('Wrong selected ELF ABI')
    segments=[struct.unpack_from('<IIQQQQQQ',data,h[5]+i*h[9]) for i in range(h[10])]
    loads=[x for x in segments if x[0]==1]
    if any(x[0]==3 for x in segments):raise ValueError('PT_INTERP requires an undeclared loader')
    if not any(x[1]&1 and x[3]<=h[4]<x[3]+x[6] for x in loads):raise ValueError('Entry outside executable PT_LOAD')
    if any(x[6]<x[5] or x[2]+x[5]>len(data) or x[1]&3==3 for x in loads):raise ValueError('Bad extent or writable executable segment')
    if not any(x[6]>x[5] for x in loads):raise ValueError('Expected zero-filled BSS tail')
    sections=[struct.unpack_from('<IIQQQQIIQQ',data,h[6]+i*h[11]) for i in range(h[12])]
    if any(x[1] in (4,9) and x[5] for x in sections):raise ValueError('Relocations require an unimplemented relocation stage')
    for x in sections:
        if x[1]==6:
            dynamic=[struct.unpack_from('<qQ',data,at) for at in range(x[4],x[4]+x[5],16)]
            if any(tag==1 or tag in (8,18,35) and value for tag,value in dynamic):raise ValueError('Dynamic dependency or relocation requirement')
    undefined=subprocess.check_output(['nm','-u',str(path)],text=True)
    if undefined.strip():raise ValueError('Undefined helper: '+undefined.strip())
    symbols=subprocess.check_output(['nm','--defined-only',str(path)],text=True)
    if re.search(r'\b(?:malloc|calloc|realloc|free|printf|memmove|__.*di3|__stack_chk_fail)$',symbols,re.M):
        raise ValueError('Forbidden resolved helper/provider')
    return {'type':'ET_EXEC' if h[1]==2 else 'ET_DYN','entry':hex(h[4]),'file_bytes':len(data),
            'load_segments':[{'flags':x[1],'offset':x[2],'vaddr':hex(x[3]),'filesz':x[5],'memsz':x[6]} for x in loads],
            'defined_symbols':symbols,'undefined_symbols':undefined,'relocations':0,
            'allowed_byte_helpers':'memcpy and memset supplied only by the pinned byte_helpers.c object; no default runtime libraries'}

def run(report,tracer,library_dir=None):
    base=ROOT/'reviews/v5/expansion/legacy_completion';destination=(ROOT/report).resolve()
    if not destination.is_relative_to(base.resolve()):raise ValueError('Use a new legacy_completion report')
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ');logs=base/'entry_runs'/stamp;logs.mkdir(parents=True)
    files=[Path(__file__).resolve(),ROOT/'assessor/checkpoint_depth/checkpoint_writer.c',ROOT/'labs/checkpoint_depth/include/checkpoint_writer.h',
        *[ROOT/'assessor/checkpoint_depth/targets'/name for name in ('entry.S','image.c','byte_helpers.c')],
        ROOT/'labs/checkpoint_depth/tests/helper_probe.c',ROOT/'labs/checkpoint_depth/tests/resolved_forbidden.c']
    pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files}
    result={'schema_version':1,'created_utc':stamp,'inspected_paths':pins,'profiles':[],'negative_controls':[],
            'scope':'Original local Linux child images; no host setting changes or external targets. Source inspection plus actual fixed/PIE execution, not physical firmware evidence.',
            'learner_mastery':'not-assessed','independent_review':'not-performed'}
    def command(argv):
        environment=os.environ.copy()
        if argv[0]==tracer and library_dir:environment['LD_LIBRARY_PATH']=library_dir
        p=subprocess.run(argv,capture_output=True,text=True,timeout=30,env=environment)
        if p.returncode:raise RuntimeError('Command failed: '+repr(argv)+'\n'+p.stdout+p.stderr)
        return p
    def reject(path,label):
        try:audit(path)
        except ValueError as error:result['negative_controls'].append({'case':label,'passed':True,'reason':str(error)});return
        raise RuntimeError('Audit accepted negative '+label)
    try:
        result['tracer']={'path':tracer,'version':command([tracer,'--version']).stdout.splitlines()[0],
                          'sha256':hashlib.sha256(Path(tracer).read_bytes()).hexdigest()}
        if library_dir:result['tracer']['local_library_sha256']={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in Path(library_dir).glob('*.so*') if p.is_file()}
        with tempfile.TemporaryDirectory(prefix='checkpoint-entry-') as temporary:
            tmp=Path(temporary)
            for compiler in ('gcc','clang'):
                for opt in ('-O0','-O2'):
                    for pie in (False,True):
                        name=compiler+opt+('-pie' if pie else '-fixed');image=tmp/name
                        flags=['-std=c17',opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-asynchronous-unwind-tables',
                               '-fno-unwind-tables','-nostdlib','-nostartfiles','-nodefaultlibs',
                               '-fPIE' if pie else '-fno-pie','-static-pie' if pie else '-no-pie']
                        if not pie:flags+=['-static']
                        argv=[compiler,*flags,'-I'+str(ROOT/'labs/checkpoint_depth/include'),
                            str(ROOT/'assessor/checkpoint_depth/targets/entry.S'),str(ROOT/'assessor/checkpoint_depth/targets/image.c'),
                            str(ROOT/'assessor/checkpoint_depth/targets/byte_helpers.c'),
                            str(ROOT/'assessor/checkpoint_depth/checkpoint_writer.c'),'-Wl,-e,_start,-z,noexecstack,--no-dynamic-linker',
                            '-Wl,-Map,'+str(logs/(name+'.map')),'-o',str(image)]
                        build=command(argv);observed=audit(image)
                        (logs/(name+'-readelf.txt')).write_text(command(['readelf','-aW',str(image)]).stdout,encoding='utf-8',newline='\n')
                        assembly=command(['objdump','-d',str(image)]).stdout
                        (logs/(name+'-disassembly.txt')).write_text(assembly,encoding='utf-8',newline='\n')
                        invocations=[]
                        for index,args in enumerate((['ABCDEFGHI','normal'],['ABCDEFGHI','changed-argument'])):
                            trace=logs/(name+'-'+str(index)+'-trace.log')
                            ran=command([tracer,'-qq','-e','trace=write,exit,exit_group','-o',str(trace),str(image),*args])
                            trace_text=trace.read_text(encoding='utf-8')
                            if ran.stdout!='ABCDEFGHI' or not re.search(r'CP_ENTRY_OK [0-9a-f]{16}\n',ran.stderr):raise RuntimeError('Wrong image output')
                            if not re.search(r'write\(1, "ABCDEFGHI", 9\)\s+= 9',trace_text) or 'EBADF' not in trace_text or 'exit(0)' not in trace_text:raise RuntimeError('Actual syscall trace mismatch')
                            invocations.append({'argv':args,'stdout':ran.stdout,'stderr':ran.stderr,'trace':trace.relative_to(ROOT).as_posix()})
                        entry=re.search(r'<_start>:\n(.*?)(?=\n\n)',assembly,re.S)
                        if not entry or not all(token in entry.group(1) for token in ('%rsp,%rdi','call','syscall')):raise RuntimeError('Missing entry sequence')
                        result['profiles'].append({'name':name,'command':argv,'audit':observed,'invocations':invocations,'passed':True})
                        if compiler=='gcc' and opt=='-O2':
                            corrupt=tmp/(name+'-bad-entry');content=bytearray(image.read_bytes());struct.pack_into('<Q',content,24,0x123);corrupt.write_bytes(content);reject(corrupt,'entry outside segment '+name)
                            extra='-DCPW_BAD_RELOCATION' if pie else str(ROOT/'labs/checkpoint_depth/tests/resolved_forbidden.c')
                            bad=tmp/(name+'-bad');bad_argv=argv[:-2]+[extra,'-o',str(bad)]
                            command(bad_argv);reject(bad,'unhandled relocation' if pie else 'resolved forbidden malloc despite empty nm -u')
                            if not pie:
                                removed=tmp/'without-optional-helpers'
                                without=[x for x in argv[:-2] if not x.endswith('/targets/byte_helpers.c')]+['-DCPW_NO_HELPERS','-o',str(removed)]
                                command(without);removed_audit=audit(removed)
                                ran=command([str(removed),'ABCDEFGHI','no-helpers'])
                                result['optional_helper_removed']={'command':without,'audit':removed_audit,
                                    'passed':ran.stdout=='ABCDEFGHI' and not re.search(r'\b(?:memcpy|memset)$',removed_audit['defined_symbols'],re.M),
                                    'scope':'GCC O2 only; explicit helper tests omitted and actual compiler closure rechecked. Clang O0 still requires the declared providers.'}
                helper=tmp/(compiler+'-helper.o')
                command([compiler,'-std=c17','-O2','-ffreestanding','-fno-builtin','-c',str(ROOT/'labs/checkpoint_depth/tests/helper_probe.c'),'-o',str(helper)])
                undefined=command(['nm','-u',str(helper)]).stdout
                result['negative_controls'].append({'case':compiler+' aggregate helper probe','undefined_symbols':undefined,
                    'passed':bool(re.search(r'\bmemcpy\b',undefined)),'scope':'Actual aggregate-copy compiler helper requires the explicitly audited local byte_helpers.c provider; undefined-symbol emptiness alone is insufficient.'})
    except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:result['error']=str(error)
    result['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==v for p,v in pins.items())
    result['passed']='error' not in result and result['source_stable'] and len(result['profiles'])==8 and all(x['passed'] for x in result['negative_controls']) and result.get('optional_helper_removed',{}).get('passed',False)
    result['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
    payload=json.dumps(result,indent=2)+'\n';(logs/'report.json').write_text(payload,encoding='utf-8',newline='\n');destination.write_text(payload,encoding='utf-8',newline='\n')
    print(json.dumps({'passed':result['passed'],'profiles':len(result['profiles']),'negative_controls':len(result['negative_controls']),'error':result.get('error')}))
    return 0 if result['passed'] else 1

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--strace',required=True)
    p.add_argument('--strace-library-dir',help='Optional extracted local library directory, applied only to the tracer child environment')
    args=p.parse_args();raise SystemExit(run(args.report,args.strace,args.strace_library_dir))
