"""Build/execute original lifecycle failure schedules; Linux, no host policy changes."""
import argparse,hashlib,json,platform,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def run(report):
 destination=(ROOT/report).resolve();base=ROOT/'reviews/v5/expansion/legacy_posix_followthrough'
 if not destination.is_relative_to(base.resolve()):raise ValueError('Use legacy POSIX followthrough evidence directory')
 stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ');logs=base/'runs'/stamp;logs.mkdir(parents=True)
 public=ROOT/'labs/posix_lifecycle_depth/followthrough';private=ROOT/'assessor/posix_lifecycle_depth/followthrough'
 sources=[Path(__file__).resolve(),ROOT/'labs/posix_lifecycle_depth/tests/helper.c']+[p for d in (public,private) for p in d.rglob('*') if p.is_file()]
 sources += [ROOT/p for p in ('assessor/posix_lifecycle_depth/posix_lifecycle.c','labs/posix_lifecycle_depth/src/posix_lifecycle.c','labs/posix_lifecycle_depth/include/posix_lifecycle.h')]
 pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}
 result={'schema_version':1,'created_utc':stamp,'inspected_paths':pins,'profiles':[],'environment':platform.uname()._asdict(),
         'scope':'Actual original fork/exec/dup/pipe/signal tests plus separately labelled deterministic syscall-boundary injection. No delayed learner attempt or external review.',
         'independent_learner':'not-assessed','external_review':'not-run','physical_observation':'not-applicable'}
 try:
  with tempfile.TemporaryDirectory(prefix='plx-reference-') as tmp:
   for compiler in ('gcc','clang'):
    for mode,flags in [('debug',['-O0','-g']),('release',['-O2']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
     name=compiler+'-'+mode;outcomes=[];helper=Path(tmp)/(name+'-helper')
     helper_command=[compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags,str(ROOT/'labs/posix_lifecycle_depth/tests/helper.c'),'-o',str(helper)]
     build=subprocess.run(helper_command,capture_output=True,text=True,timeout=60)
     (logs/(name+'-helper-build.log')).write_text(build.stdout+build.stderr,encoding='utf-8',newline='\n')
     if build.returncode:raise RuntimeError('helper compilation: '+build.stderr)
     for case,mutants in [('original',(1,2)),('channel',(1,2)),('channel_injected',(1,2)),('stages',(3,4)),('redirect',(5,6)),('inheritance',(7,8)),('interrupts',(9,10))]:
      for variant in ('reference',*(f'mutant{i}' for i in mutants),'learner'):
       implementation=public/'launch_stub.c' if variant=='learner' else private/'launch.c';exe=Path(tmp)/(name+'-'+case+'-'+variant)
       test=public/'tests.c'
       if case=='original':
        implementation=ROOT/('labs/posix_lifecycle_depth/src/posix_lifecycle.c' if variant=='learner' else 'assessor/posix_lifecycle_depth/posix_lifecycle.c');test=public/'original_tests.c'
       command=[compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-I'+str(public),str(test),str(implementation),'-o',str(exe)]
       if case in ('channel_injected','original'):command += ['-DPLX_INJECT',str(public/'io_inject.c'),'-Wl,--wrap=read','-Wl,--wrap=__read_chk','-Wl,--wrap=write','-Wl,--wrap=waitpid']
       if case=='original':command += ['-DPLX_ORIGINAL',str(public/'original_inject.c'),'-Wl,--wrap=execve','-I'+str(ROOT/'labs/posix_lifecycle_depth/include'),'-pthread']
       if variant.startswith('mutant'):command.append(('-DPL_MUTANT=' if case=='original' else '-DPLX_MUTANT=')+variant.removeprefix('mutant'))
       build=subprocess.run(command,capture_output=True,text=True,timeout=60);logname=name+'-'+case+'-'+variant
       (logs/(logname+'-build.log')).write_text(build.stdout+build.stderr,encoding='utf-8',newline='\n')
       if build.returncode:raise RuntimeError(logname+' compilation: '+build.stderr)
       actual='channel' if case=='channel_injected' else case
       execution=subprocess.run([str(exe),actual,str(helper)],capture_output=True,text=True,timeout=15)
       (logs/(logname+'.log')).write_text(execution.stdout+execution.stderr,encoding='utf-8',newline='\n')
       passed=execution.returncode==0 and 'PLX_PASS' in execution.stdout if variant=='reference' else execution.returncode==1 and 'PLX_CHECK:' in execution.stderr and 'Sanitizer' not in execution.stderr
       outcomes.append({'case':case,'variant':variant,'command':command,'build_exit':build.returncode,'execution_exit':execution.returncode,'passed':passed})
       if not passed:
        (logs/(logname+'-disassembly.txt')).write_text(subprocess.check_output(['objdump','-d',str(exe)],text=True),encoding='utf-8',newline='\n')
        raise RuntimeError(logname+' semantic result: '+execution.stdout+execution.stderr)
     result['profiles'].append({'name':name,'compiler':subprocess.check_output([compiler,'--version'],text=True).splitlines()[0],'helper_command':helper_command,'outcomes':outcomes,'passed':True})
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:result['error']=str(error)
 result['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items())
 result['passed']='error' not in result and result['source_stable'] and len(result['profiles'])==6
 result['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
 payload=json.dumps(result,indent=2)+'\n';(logs/'report.json').write_text(payload,encoding='utf-8',newline='\n');destination.write_text(payload,encoding='utf-8',newline='\n')
 print(json.dumps({'passed':result['passed'],'profiles':len(result['profiles']),'error':result.get('error')}));return 0 if result['passed'] else 1

if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);args=parser.parse_args();raise SystemExit(run(args.report))
