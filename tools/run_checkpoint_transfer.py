"""Run the delayed checkpoint reference packages; never solve exported learner stubs."""
import argparse,hashlib,json,platform,re,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def run(report,openssl_include_root=None):
 destination=(ROOT/report).resolve();base=ROOT/'reviews/v5/expansion/legacy_completion'
 if not destination.is_relative_to(base.resolve()):raise ValueError('Use legacy completion evidence')
 stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ');logs=base/'transfer_runs'/stamp;logs.mkdir(parents=True)
 sources=[Path(__file__).resolve()]+[p for directory in ('labs/checkpoint_depth/transfer','assessor/checkpoint_depth/transfer') for p in (ROOT/directory).rglob('*') if p.is_file()]
 sources += [ROOT/p for p in ('assessor/ownership/ownership.c','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','assessor/errors/errors.c','labs/errors/include/errors.h','assessor/checkpoint_depth/checkpoint_writer.c','labs/checkpoint_depth/include/checkpoint_writer.h')]
 sources += [ROOT/p for p in ('assessor/checkpoint_depth/checkpoint_memory.c','labs/checkpoint_depth/include/checkpoint_memory.h')]
 sources += [ROOT/p for p in ('assessor/checkpoint_depth/checkpoint_depth.c','labs/checkpoint_depth/include/checkpoint_depth.h')]
 sources += [ROOT/p for p in ('labs/service_depth/adapters/receipt.h','assessor/service_depth/kernel/receipt.c','labs/storage/include/store.h','assessor/storage/store.c')]
 sources += [ROOT/p for p in ('assessor/checkpoint_depth/checkpoint_firmware.c','labs/checkpoint_depth/include/checkpoint_firmware.h')]
 pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}
 result={'schema_version':1,'created_utc':stamp,'inspected_paths':pins,'profiles':[],'environment':platform.uname()._asdict(),
         'scope':'Executable original delayed-task reference support and semantic rejection controls. No elapsed learner attempt or human review is manufactured.',
         'independent_learner':'not-assessed','external_review':'not-run','physical_observation':'not-run'}
 try:
  ssl_include=Path(openssl_include_root) if openssl_include_root else Path('/usr/include')
  if not (ssl_include/'openssl/evp.h').is_file():raise ValueError('OpenSSL development headers required; use --openssl-include-root for locally extracted libssl-dev/usr/include')
  result['openssl']={'version':subprocess.check_output(['openssl','version'],text=True).strip(),'header_root':str(ssl_include),
                     'headers_sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in ssl_include.rglob('*.h') if 'openssl' in p.parts}}
  with tempfile.TemporaryDirectory(prefix='checkpoint-transfer-') as tmp:
   for compiler in ('gcc','clang'):
    for mode,flags in [('debug',['-O0','-g']),('release',['-O2']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
     name=compiler+'-'+mode;outcomes=[]
     for case,mutants in [('memory',(1,2)),('adapters',(3,4)),('protocol',(5,6)),('protocol16',(5,6)),('auth',(7,8)),('durable',(9,10)),('halfduplex',(11,12))]:
      for variant in ('reference',*(f'mutant{i}' for i in mutants),'learner'):
       public=ROOT/'labs/checkpoint_depth/transfer';private=ROOT/'assessor/checkpoint_depth/transfer'
       source_case='protocol' if case.startswith('protocol') else case
       implementation=public/(source_case+'_stub.c') if variant=='learner' else private/(source_case+'.c');exe=Path(tmp)/(name+'-'+case+'-'+variant)
       extra=[]
       if case=='memory':extra=[str(ROOT/'assessor/checkpoint_depth/checkpoint_memory.c'),'-I'+str(ROOT/'labs/checkpoint_depth/include')]
       if case.startswith('protocol'):
        extra=[str(ROOT/'assessor/checkpoint_depth/checkpoint_depth.c'),'-I'+str(ROOT/'labs/checkpoint_depth/include')]
        if case=='protocol16':extra+=['-DCPX_MAX_PAYLOAD=16']
       if case=='auth':
        extra=[str(public/'openssl_mac.c'),str(ROOT/'assessor/checkpoint_depth/checkpoint_depth.c'),'-I'+str(ROOT/'labs/checkpoint_depth/include'),'-I'+str(ssl_include),'-I'+str(ssl_include/'x86_64-linux-gnu'),'-l:libcrypto.so.3']
       if case=='durable':
        extra=[str(ROOT/p) for p in ('assessor/checkpoint_depth/checkpoint_depth.c','assessor/service_depth/kernel/receipt.c','assessor/storage/store.c')]
        extra+=['-I'+str(ROOT/p) for p in ('labs/checkpoint_depth/include','labs/service_depth/adapters','labs/storage/include')]
       if case=='halfduplex':extra=[str(ROOT/p) for p in ('assessor/checkpoint_depth/checkpoint_depth.c','assessor/checkpoint_depth/checkpoint_firmware.c')]+['-I'+str(ROOT/'labs/checkpoint_depth/include')]
       if case=='adapters':
        extra=[str(ROOT/p) for p in ('assessor/ownership/ownership.c','assessor/errors/errors.c','assessor/checkpoint_depth/checkpoint_writer.c')]
        extra += ['-I'+str(ROOT/p) for p in ('labs/ownership/include','labs/errors/include','labs/checkpoint_depth/include')]
       command=[compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-I'+str(public),str(public/(source_case+'_tests.c')),str(implementation),*extra,'-o',str(exe)]
       if variant.startswith('mutant'):command.append('-DCPX_MUTANT='+variant.removeprefix('mutant'))
       build=subprocess.run(command,capture_output=True,text=True,timeout=60)
       logname=name+'-'+case+'-'+variant
       (logs/(logname+'-build.log')).write_text(build.stdout+build.stderr,encoding='utf-8',newline='\n')
       if build.returncode:raise RuntimeError(logname+' compile: '+build.stderr)
       execution=subprocess.run([str(exe)],capture_output=True,text=True,timeout=30)
       (logs/(logname+'.log')).write_text(execution.stdout+execution.stderr,encoding='utf-8',newline='\n')
       passed=execution.returncode==0 and 'CPX_PASS' in execution.stdout if variant=='reference' else execution.returncode==1 and 'CPX_CHECK:' in execution.stderr and 'Sanitizer' not in execution.stderr
       outcomes.append({'case':case,'variant':variant,'command':command,'build_exit':build.returncode,'execution_exit':execution.returncode,'passed':passed})
       if not passed:raise RuntimeError(logname+' semantic result: '+execution.stdout+execution.stderr)
       if case=='adapters' and variant=='reference':
        assembly=subprocess.check_output(['objdump','-d',str(exe)],text=True)
        (logs/(logname+'-disassembly.txt')).write_text(assembly,encoding='utf-8',newline='\n')
        shapes={}
        for symbol in ('cpx_progress_step','cpx_owner_close'):
         match=re.search(r'<'+symbol+r'>:\n(.*?)(?=\n\n|\Z)',assembly,re.S)
         if not match:raise ValueError('Missing inspected adapter symbol '+symbol)
         jumps=re.findall(r'\t(j[a-z]+)\s',match.group(1));shapes[symbol]={'static_conditional_branch_instructions':sum(j!='jmp' and j!='jmpq' for j in jumps),'scope':'Generated static branch sites, not runtime branch frequency or misprediction count.'}
        outcomes[-1]['code_shapes']=shapes
       if case.startswith('protocol') and variant=='reference':
        checked=subprocess.run(['python3',str(public/'check_protocol.py'),str(exe),'16' if case=='protocol16' else '64'],capture_output=True,text=True,timeout=120)
        (logs/(logname+'-oracle.json')).write_text(checked.stdout+checked.stderr,encoding='utf-8',newline='\n')
        if checked.returncode:raise RuntimeError(logname+' oracle: '+checked.stdout+checked.stderr)
        outcomes[-1]['differential']=json.loads(checked.stdout)
     result['profiles'].append({'name':name,'compiler':subprocess.check_output([compiler,'--version'],text=True).splitlines()[0],'outcomes':outcomes,'passed':True})
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:result['error']=str(error)
 result['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items())
 result['passed']='error' not in result and result['source_stable'] and len(result['profiles'])==6
 result['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
 payload=json.dumps(result,indent=2)+'\n';(logs/'report.json').write_text(payload,encoding='utf-8',newline='\n');destination.write_text(payload,encoding='utf-8',newline='\n')
 print(json.dumps({'passed':result['passed'],'profiles':len(result['profiles']),'error':result.get('error')}));return 0 if result['passed'] else 1

if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);parser.add_argument('--openssl-include-root',help='OpenSSL3 development include root; locally extracted package is supported without system installation.');args=parser.parse_args();raise SystemExit(run(args.report,args.openssl_include_root))
