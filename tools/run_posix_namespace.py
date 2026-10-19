"""Run local pathname, metadata and permission-model successors; no external paths."""
import argparse,hashlib,json,platform,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def run(report):
 destination=(ROOT/report).resolve();base=ROOT/'reviews/v5/expansion/legacy_posix_namespace'
 if not destination.is_relative_to(base.resolve()):raise ValueError('Use legacy POSIX namespace evidence directory')
 stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ');logs=base/'runs'/stamp;logs.mkdir(parents=True)
 public=ROOT/'labs/posix_lifecycle_depth/namespace';private=ROOT/'assessor/posix_lifecycle_depth/namespace'
 paths=[Path(__file__).resolve()]+[p for d in (public,private) for p in d.rglob('*') if p.is_file()]
 pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
 result={'schema_version':1,'created_utc':stamp,'inspected_paths':pins,'profiles':[],'environment':platform.uname()._asdict(),
  'scope':'Owned temporary Linux files/processes only. Demonstrated wrong synthetic-file read, full-resolution openat2 repair, stat observations on temporary storage and tmpfs, actual child umask and restricted ACL/authorization models.',
  'external_review':'not-run','independent_learner':'not-assessed','physical_observation':'not-applicable-to-declared-hosted-contract'}
 try:
  with tempfile.TemporaryDirectory(prefix='posix-namespace-',dir='/var/tmp') as temporary,tempfile.TemporaryDirectory(prefix='posix-namespace-',dir='/dev/shm') as shared:
   tmp=Path(temporary);shm=Path(shared)
   for compiler in ('gcc','clang'):
    for mode,flags in [('debug',['-O0','-g']),('release',['-O3']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
     name=compiler+'-'+mode;outcomes=[]
     for case,mutants,learner in [('confined',(1,2),True),('metadata',(3,4),True),('policy',(5,6),True),('identity',(7,8),False),('anchored',(9,10),False),('metadata_tmpfs',(3,4),True)]:
      for variant in ('reference',*(f'mutant{i}' for i in mutants),*(['learner'] if learner else [])):
       label=name+'-'+case+'-'+variant;exe=tmp/label
       implementation=public/'path_stub.c' if variant=='learner' else private/'path.c'
       command=[compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-I'+str(public),str(public/'tests.c'),str(implementation),'-Wl,--wrap=syscall','-o',str(exe)]
       if variant.startswith('mutant'):command.append('-DPN_MUTANT='+variant.removeprefix('mutant'))
       build=subprocess.run(command,capture_output=True,text=True,timeout=60)
       (logs/(label+'-build.log')).write_text(build.stdout+build.stderr,encoding='utf-8',newline='\n')
       if build.returncode:raise RuntimeError(label+' compilation: '+build.stderr)
       execution=subprocess.run([str(exe),case.removesuffix('_tmpfs')],cwd=shm if case.endswith('_tmpfs') else tmp,capture_output=True,text=True,timeout=15)
       (logs/(label+'.log')).write_text(execution.stdout+execution.stderr,encoding='utf-8',newline='\n')
       passed=(execution.returncode==0 and 'PN_PASS' in execution.stdout) if variant=='reference' else (execution.returncode==1 and 'PL_CHECK:' in execution.stderr and 'Sanitizer' not in execution.stderr)
       observations=[line for line in execution.stdout.splitlines() if line.startswith(('PN_META','PN_FS','PN_AUTH'))]
       outcomes.append({'case':case,'variant':variant,'command':command,'build_exit':build.returncode,'execution_exit':execution.returncode,'passed':passed,'observations':observations,
        'role':'provided platform investigation, not a learner implementation test' if case in ('identity','anchored') else 'reference implementation and learner/semantic controls'})
       if not passed:raise RuntimeError(label+' semantic result: '+execution.stdout+execution.stderr)
     fs={o['case']:next((s.split()[1] for s in o['observations'] if s.startswith('PN_FS')),None) for o in outcomes if o['variant']=='reference' and o['case'].startswith('metadata')}
     if len(fs)!=2 or not all(fs.values()) or len(set(fs.values()))!=2:raise RuntimeError('Changed-filesystem experiment needs two observed distinct filesystem types: '+str(fs))
     result['profiles'].append({'name':name,'compiler':subprocess.check_output([compiler,'--version'],text=True).splitlines()[0],'filesystem_types':fs,'outcomes':outcomes,'passed':True})
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:result['error']=str(error)
 result['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items());result['passed']='error' not in result and result['source_stable'] and len(result['profiles'])==6
 result['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
 payload=json.dumps(result,indent=2)+'\n';(logs/'report.json').write_text(payload,encoding='utf-8',newline='\n');destination.write_text(payload,encoding='utf-8',newline='\n')
 print(json.dumps({'passed':result['passed'],'profiles':len(result['profiles']),'error':result.get('error')}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(run(p.parse_args().report))
