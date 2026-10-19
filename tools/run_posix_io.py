"""Run resumable I/O and actual native/i386 offset contracts with explicit tools."""
import argparse,hashlib,json,platform,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def run(report,sysroot):
 destination=(ROOT/report).resolve();base=ROOT/'reviews/v5/expansion/legacy_posix_io'
 if not destination.is_relative_to(base.resolve()):raise ValueError('Use legacy POSIX I/O evidence directory')
 sysroot=Path(sysroot).resolve();loader=sysroot/'usr/lib32/ld-linux.so.2';libs=sysroot/'usr/lib32';gccdir=sysroot/'usr/lib/gcc/x86_64-linux-gnu/15/32'
 for p in (loader,libs/'libc.so.6',gccdir/'crtbeginS.o'):
  if not p.is_file():raise ValueError('Explicit private i386 toolchain missing '+str(p))
 stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ');logs=base/'runs'/stamp;logs.mkdir(parents=True)
 public=ROOT/'labs/posix_lifecycle_depth/io';private=ROOT/'assessor/posix_lifecycle_depth/io'
 paths=[Path(__file__).resolve()]+[p for d in (public,private) for p in d.rglob('*') if p.is_file()]
 pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
 result={'schema_version':1,'created_utc':stamp,'inspected_paths':pins,'profiles':[],'environment':platform.uname()._asdict(),
  'scope':'Owned Linux files/pipes/processes, scripted callbacks and explicit close ownership models; actual native, i386 off_t32 and i386 large-file ABI. No durable commit or external service claim.',
  'external_review':'not-run','independent_learner':'not-assessed','physical_observation':'not-applicable-to-declared-hosted-contract',
  'toolchain':{'sysroot':str(sysroot),'packages':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in (sysroot.parent/'packages').glob('*.deb')},
   'runtime_sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in (loader,libs/'libc.so.6',gccdir/'crtbeginS.o')}}}
 profiles=[(c+'-'+m,c,f,False,False) for c in ('gcc','clang') for m,f in [('debug',['-O0','-g']),('release',['-O3']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-pie','-no-pie'])]]
 target=['-m32','--sysroot='+str(sysroot),'-isystem',str(sysroot/'usr/include/x86_64-linux-gnu'),'-B'+str(gccdir)+'/', '-L'+str(libs),'-Wl,--dynamic-linker='+str(loader),'-O2']
 profiles += [('gcc-i386-off32','gcc',target,True,False),('gcc-i386-lfs64','gcc',target+['-D_FILE_OFFSET_BITS=64'],True,True)]
 try:
  with tempfile.TemporaryDirectory(prefix='posix-io-',dir='/var/tmp') as temporary:
   tmp=Path(temporary)
   for name,compiler,flags,i386,lfs in profiles:
    outcomes=[]
    cases=[('stream',(1,2),True),('extent',(3,4),True),('position',(5,6),True),('close_policy',(7,8),True)]
    if not i386 or lfs:cases.append(('sparse',(9,10),False))
    for case,mutants,learner in cases:
     for variant in ('reference',*(f'mutant{i}' for i in mutants),*(['learner'] if learner else [])):
      label=name+'-'+case+'-'+variant;exe=tmp/label;dep=tmp/(label+'.d')
      implementation=public/'io_stub.c' if variant=='learner' else private/'io.c'
      command=[compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-MD','-MF',str(dep),'-I'+str(public),str(public/'tests.c'),str(implementation),'-Wl,--wrap='+('pwrite64' if i386 and lfs else 'pwrite'),'-o',str(exe)]
      if variant.startswith('mutant'):command.append('-DPC_MUTANT='+variant.removeprefix('mutant'))
      build=subprocess.run(command,capture_output=True,text=True,timeout=60)
      (logs/(label+'-build.log')).write_text(build.stdout+build.stderr,encoding='utf-8',newline='\n')
      if build.returncode:raise RuntimeError(label+' compilation: '+build.stderr)
      actual_command=([str(loader),'--library-path',str(libs)] if i386 else [])+[str(exe),case]
      execution=subprocess.run(actual_command,cwd=tmp,capture_output=True,text=True,timeout=15)
      (logs/(label+'.log')).write_text(execution.stdout+execution.stderr,encoding='utf-8',newline='\n')
      passed=(execution.returncode==0 and 'PC_PASS' in execution.stdout) if variant=='reference' else (execution.returncode==1 and 'PL_CHECK:' in execution.stderr and 'Sanitizer' not in execution.stderr)
      observations=[line for line in execution.stdout.splitlines() if line.startswith('PC_') and line!='PC_PASS']
      if case=='extent' and variant=='reference':passed=passed and ('off_t_bytes=4' if i386 and not lfs else 'off_t_bytes=8') in execution.stdout
      dependencies=[Path(p) for p in dep.read_text(encoding='utf-8').replace('\\\n',' ').split(':',1)[1].split()]
      dependency_pins={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in dependencies}
      outcomes.append({'case':case,'variant':variant,'command':command,'execution_command':actual_command,'build_exit':build.returncode,'execution_exit':execution.returncode,'passed':passed,'observations':observations,'dependency_sha256':dependency_pins,
       'dependency_scope':'Compiler-emitted implementation translation-unit dependencies; public harness source separately pinned and package archives identify the private sysroot.',
       'role':'provided platform investigation, not learner implementation test' if case=='sparse' else 'reference implementation and learner/semantic controls'})
      if not passed:raise RuntimeError(label+' semantic result: '+execution.stdout+execution.stderr)
    result['profiles'].append({'name':name,'compiler':subprocess.check_output([compiler,'--version'],text=True).splitlines()[0],'outcomes':outcomes,'passed':True})
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:result['error']=str(error)
 result['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items());result['passed']='error' not in result and result['source_stable'] and len(result['profiles'])==8
 result['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
 payload=json.dumps(result,indent=2)+'\n';(logs/'report.json').write_text(payload,encoding='utf-8',newline='\n');destination.write_text(payload,encoding='utf-8',newline='\n')
 print(json.dumps({'passed':result['passed'],'profiles':len(result['profiles']),'error':result.get('error')}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--i386-sysroot',required=True,help='Explicit private sysroot described in labs/posix_lifecycle_depth/io/TOOLCHAIN.md; no global installs.');a=p.parse_args();raise SystemExit(run(a.report,a.i386_sysroot))
