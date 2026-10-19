"""Owned PTYs/pipelines and explicit same-object parser transfer campaign."""
import argparse,hashlib,json,os,platform,signal,subprocess,tempfile,zlib
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def run(report):
 base=ROOT/'reviews/v5/expansion/legacy_posix_terminal';destination=(ROOT/report).resolve()
 if not destination.is_relative_to(base.resolve()):raise ValueError('Use legacy POSIX terminal evidence directory')
 public=ROOT/'labs/posix_lifecycle_depth/terminal';private=ROOT/'assessor/posix_lifecycle_depth/terminal'
 parser=ROOT/'assessor/checkpoint_depth/checkpoint_depth.c';include=ROOT/'labs/checkpoint_depth/include'
 paths=[Path(__file__).resolve(),parser,include/'checkpoint_depth.h']+[p for d in (public,private) for p in d.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
 pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
 stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ');logs=base/'runs'/stamp;logs.mkdir(parents=True)
 result={'schema_version':1,'created_utc':stamp,'inspected_paths':pins,'profiles':[],'environment':platform.uname()._asdict(),
  'scope':'Owned UNIX98 PTYs, forked teaching pipeline and local socketpair; one explicit profile-C parser object reused across three adapters. No controlling-terminal changes, physical baud, external device or network target.',
  'external_review':'not-run','independent_learner':'not-assessed','physical_observation':'not-run'}
 def execute(command,label,timeout=30):
  process=subprocess.Popen(command,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,start_new_session=True)
  try:stdout,stderr=process.communicate(timeout=timeout)
  except subprocess.TimeoutExpired:
   os.killpg(process.pid,signal.SIGKILL);stdout,stderr=process.communicate()
   (logs/(label+'.log')).write_text(stdout+stderr+'\nOWNED_PROCESS_GROUP_TIMEOUT\n',encoding='utf-8',newline='\n')
   raise RuntimeError(label+' deadline: only this campaign process group terminated')
  value=subprocess.CompletedProcess(command,process.returncode,stdout,stderr)
  (logs/(label+'.log')).write_text(value.stdout+value.stderr,encoding='utf-8',newline='\n');return value
 try:
  vectors=[bytes([165,1,52,18,3,0,65,0,10,31,137,151,109]),bytes([165,1,120,86,2,0,165,255,167,14,66,63])]
  result['independent_crc_vectors']=[{'hex':v.hex(),'passed':zlib.crc32(v[1:-4])==int.from_bytes(v[-4:],'little')} for v in vectors]
  if not all(v['passed'] for v in result['independent_crc_vectors']):raise RuntimeError('Independent CRC vector error')
  with tempfile.TemporaryDirectory(prefix='posix-terminal-',dir='/var/tmp') as directory:
   tmp=Path(directory)
   for compiler in ('gcc','clang'):
    for mode,flags in [('debug',['-O0','-g']),('release',['-O3']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
     name=compiler+'-'+mode;common=[compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags];outcomes=[]
     # This target object is uninstrumented; host adapters carry the selected
     # sanitizer. Audit its actual undefined symbols before importing it.
     obj=tmp/(name+'-parser.o');object_command=[compiler,'-std=c17','-O2','-ffreestanding','-fno-builtin','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(include),'-c',str(parser),'-o',str(obj)]
     compiled=execute(object_command,name+'-parser-build');audit=execute(['nm','-u',str(obj)],name+'-parser-undefined') if compiled.returncode==0 else None
     if compiled.returncode or audit.returncode or audit.stdout.strip():raise RuntimeError(name+' profile-C object closure failed')
     object_hash=hashlib.sha256(obj.read_bytes()).hexdigest()
     cases=[('guard',(1,2),False),('pipeline',(3,4),False),('bytes',(),False),('timers',(),False),('disconnect',(),False),('segmentation',(2,),True),('reconnect',(1,),True)]
     for case,mutants,stream in cases:
      for variant in ('reference',*(f'mutant{i}' for i in mutants),'learner'):
       label=name+'-'+case+'-'+variant;exe=tmp/label;implementation=(public/('stream_stub.c' if stream else 'terminal_stub.c')) if variant=='learner' else private/('stream.c' if stream else 'terminal.c')
       command=[*common,'-I'+str(public),str(public/('stream_tests.c' if stream else 'tests.c')),str(implementation)]
       if stream:command+=['-I'+str(include),str(obj)]
       else:command+=['-Wl,--wrap=tcgetattr,--wrap=tcsetattr,--wrap=fork,--wrap=pipe2,--wrap=clock_gettime']
       if variant.startswith('mutant'):command+=['-D'+('PS_MUTANT=' if stream else 'PT_MUTANT=')+variant.removeprefix('mutant')]
       command+=['-o',str(exe)];build=execute(command,label+'-build')
       if build.returncode:raise RuntimeError(label+' compilation failed: '+build.stderr)
       observed=execute([str(exe),case],label)
       passed=(observed.returncode==0 and ('PS_PASS' if stream else 'PT_PASS') in observed.stdout) if variant=='reference' else (observed.returncode==1 and 'PL_CHECK:' in observed.stderr and 'Sanitizer' not in observed.stderr)
       outcomes.append({'case':case,'variant':variant,'command':command,'execution_command':[str(exe),case],'build_exit':build.returncode,'execution_exit':observed.returncode,'passed':passed,
        'observations':[line for line in observed.stdout.splitlines() if line.startswith(('PT_','PS_')) and line not in ('PT_PASS','PS_PASS')],
        'role':'provided kernel-semantics investigation plus required guard' if case in ('bytes','timers','disconnect') else 'reference implementation and semantic/learner controls',
        'parser_object_sha256':object_hash if stream else None})
       if not passed:raise RuntimeError(label+' semantic result: '+observed.stdout+observed.stderr)
     result['profiles'].append({'name':name,'compiler':subprocess.check_output([compiler,'--version'],text=True).splitlines()[0],'parser_object':{'command':object_command,'sha256':object_hash,'undefined_symbols':audit.stdout,'scope':'Uninstrumented freestanding target object; hosted adapter sanitizer does not instrument parser internals.'},'outcomes':outcomes,'passed':True})
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:result['error']=str(error)
 result['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items());result['passed']='error' not in result and result['source_stable'] and len(result['profiles'])==6
 result['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
 payload=json.dumps(result,indent=2)+'\n';(logs/'report.json').write_text(payload,encoding='utf-8',newline='\n');destination.write_text(payload,encoding='utf-8',newline='\n')
 print(json.dumps({'passed':result['passed'],'profiles':len(result['profiles']),'error':result.get('error')}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();raise SystemExit(run(a.report))
