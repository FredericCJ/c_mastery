"""Execute bounded Linux mapping, process-death and isolated fault experiments."""
import argparse,hashlib,json,platform,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def run(report):
 destination=(ROOT/report).resolve();base=ROOT/'reviews/v5/expansion/legacy_posix_mapping'
 if not destination.is_relative_to(base.resolve()):raise ValueError('Use legacy POSIX mapping evidence directory')
 stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ');logs=base/'runs'/stamp;logs.mkdir(parents=True)
 public=ROOT/'labs/posix_lifecycle_depth/mapping';private=ROOT/'assessor/posix_lifecycle_depth/mapping'
 paths=[Path(__file__).resolve()]+[p for d in (public,private) for p in d.rglob('*') if p.is_file()]
 pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
 result={'schema_version':1,'created_utc':stamp,'inspected_paths':pins,'profiles':[],'environment':platform.uname()._asdict(),
  'scope':'Owned Linux mapping/record-lock/fork/exec experiments. Faulting helper intentionally unsanitized so wait status records the kernel signal. No disk durability or physical power-loss claim.',
  'external_review':'not-run','independent_learner':'not-assessed','physical_observation':'not-applicable-to-declared-hosted-contract'}
 try:
  with tempfile.TemporaryDirectory(prefix='posix-mapping-') as temporary:
   tmp=Path(temporary)
   for compiler in ('gcc','clang'):
    for mode,flags in [('debug',['-O0','-g']),('release',['-O2']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
     name=compiler+'-'+mode;outcomes=[]
     for case,mutants,learner in [('views',(1,2),True),('codec',(3,4),True),('publication',(5,6),True),('sharing',(7,8),False),('residency',(),False),('faults',(1,2),False)]:
      for variant in ('reference',*(f'mutant{i}' for i in mutants),*(['learner'] if learner else [])):
       label=name+'-'+case+'-'+variant;exe=tmp/label;helper=tmp/(label+'-fault-helper')
       helper_command=[compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-O2',str(public/'fault_helper.c'),'-o',str(helper)]
       if case=='faults' and variant.startswith('mutant'):helper_command.append('-DPM_FAULT_MUTANT='+variant.removeprefix('mutant'))
       helper_build=subprocess.run(helper_command,capture_output=True,text=True,timeout=60)
       (logs/(label+'-helper-build.log')).write_text(helper_build.stdout+helper_build.stderr,encoding='utf-8',newline='\n')
       if helper_build.returncode:raise RuntimeError(label+' helper compilation: '+helper_build.stderr)
       implementation=public/'map_stub.c' if variant=='learner' else private/'map.c'
       command=[compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-I'+str(public),str(public/'tests.c'),str(implementation),'-o',str(exe)]
       if case!='faults' and variant.startswith('mutant'):command.append('-DPM_MUTANT='+variant.removeprefix('mutant'))
       build=subprocess.run(command,capture_output=True,text=True,timeout=60)
       (logs/(label+'-build.log')).write_text(build.stdout+build.stderr,encoding='utf-8',newline='\n')
       if build.returncode:raise RuntimeError(label+' compilation: '+build.stderr)
       execution=subprocess.run([str(exe),case,str(helper)],cwd=tmp,capture_output=True,text=True,timeout=15)
       (logs/(label+'.log')).write_text(execution.stdout+execution.stderr,encoding='utf-8',newline='\n')
       passed=execution.returncode==0 and 'PLM_PASS' in execution.stdout if variant=='reference' else execution.returncode==1 and 'PL_CHECK:' in execution.stderr and 'Sanitizer' not in execution.stderr
       outcomes.append({'case':case,'variant':variant,'command':command,'helper_command':helper_command,'build_exit':build.returncode,'execution_exit':execution.returncode,'passed':passed,
        'role':'provided platform investigation, not a learner implementation test' if case in ('sharing','residency','faults') else 'reference implementation and learner/semantic controls'})
       if not passed:raise RuntimeError(label+' semantic result: '+execution.stdout+execution.stderr)
     result['profiles'].append({'name':name,'compiler':subprocess.check_output([compiler,'--version'],text=True).splitlines()[0],'outcomes':outcomes,'passed':True})
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:result['error']=str(error)
 result['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items());result['passed']='error' not in result and result['source_stable'] and len(result['profiles'])==6
 result['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
 payload=json.dumps(result,indent=2)+'\n';(logs/'report.json').write_text(payload,encoding='utf-8',newline='\n');destination.write_text(payload,encoding='utf-8',newline='\n')
 print(json.dumps({'passed':result['passed'],'profiles':len(result['profiles']),'error':result.get('error')}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(run(p.parse_args().report))
