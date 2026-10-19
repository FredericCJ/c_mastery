"""Exact X43--55 additions, original anchors, target tasks and public exports."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shlex,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/systems_x_followthrough';REF=ROOT/'assessor/systems_x_followthrough'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O2','gcc-release'),('clang','-O0','clang-debug'),('clang','-O2','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
BASE={'b04':ROOT/'assessor/b04_depth/depth.c','b05':ROOT/'assessor/b05_depth/depth.c','systems':ROOT/'assessor/b045_systems/systems.c','bridge':ROOT/'assessor/modern_bridge/bridge.c','hashing':ROOT/'assessor/replicated/hashing/hashing.c'}
PUBLIC={'b04':'b04_depth/src/depth.c','b05':'b05_depth/src/depth.c','systems':'b045_systems/src/systems.c','bridge':'modern_bridge/src/bridge.c','hashing':'replicated/hashing/hashing.c'}
HEADERS={'b04':'b04_depth/include/b04.h','b05':'b05_depth/include/b05.h','systems':'b045_systems/include/systems.h','bridge':'modern_bridge/include/bridge.h','hashing':'replicated/hashing/hashing.h'}
PARTS={'transfers':('transfers.c',['b04','b05','systems'],'SX_PASS','SX_CHECK:'),'once':('once.c',['bridge'],'ONCE_X_PASS','ONCE_X_CHECK:'),'reclamation':('list.c',['bridge'],'LIST_X_PASS','LIST_X_CHECK:'),'response':('response.c',['b05'],'RESPONSE_X_PASS','RESPONSE_X_CHECK:'),'authentication':('auth.c',['b05','hashing'],'AUTH_X_PASS','AUTH_X_CHECK:')}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 paths={Path(__file__),*BASE.values(),ROOT/'labs/debugging_followthrough/replay.py'}
 for base in [LAB,REF,ROOT/'assessor/execution',ROOT/'labs/execution/include']:
  paths|={p for p in base.rglob('*') if p.is_file() and p.suffix in ['.c','.h','.py','.S','.ld'] and '__pycache__' not in p.parts}
 for d in [PUBLIC,HEADERS]:paths|={ROOT/'labs'/s for s in d.values()}
 paths|={ROOT/f'labs/{family}/tests/tests.c' for family in ['b04_depth','b05_depth','b045_systems','modern_bridge']}
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(paths)}
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);p.add_argument('--pilot',action='store_true');a=p.parse_args();work=ROOT/'build/systems-x-followthrough'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True);checks=[];before=inputs();external={}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1',TSAN_OPTIONS='halt_on_error=1')
 for key in ['CPATH','C_INCLUDE_PATH','LIBRARY_PATH','CFLAGS','CPPFLAGS','LDFLAGS']:env.pop(key,None)
 def run(label,argv,expected=0,marker=None,timeout=120):
  argv=list(map(str,argv));log=work/(str(len(checks))+'.log')
  with log.open('w') as f:
   try:rc=subprocess.run(argv,cwd=ROOT,env=env,stdout=f,stderr=subprocess.STDOUT,timeout=timeout).returncode
   except (OSError,subprocess.TimeoutExpired) as e:f.write(str(e));rc=None
  body=log.read_text(errors='replace');passed=rc==expected and (not marker or marker in body) and not any(s in body for s in ['runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer'])
  checks.append(dict(label=label,command=argv,expected=expected,returncode=rc,marker=marker,passed=bool(passed),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(passed),flush=True);return passed,log
 def compile_run(label,cc,flags,sources,marker=None,expected=0,args=()):
  exe=work/label.replace(' ','-');ok,_=run(label+' compile',[cc,*flags,*sources,'-o',exe]);return (run(label+' execute',[exe,*args],expected,marker) if ok else (False,None))
 for cc in ['gcc','clang']:
  path=Path(shutil.which(cc)).resolve();external[str(path)]=sha(path);run(cc+' version',[cc,'--version'])
 run('environment',['uname','-a']);port=work/'systems-port.c';run('registered baseline zero scratch translation',[sys.executable,REF/'port_registration.py',port],marker='8b5cfc7e7dff9b5b2c8ff68586068bea223ce11186322f47de765dad1b0f3978')
 providers=dict(BASE,systems=port);incs=[*(LAB/x for x in PARTS),LAB/'statistics',*(ROOT/'labs'/Path(v).parent for v in HEADERS.values())];strict=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*['-I'+str(x) for x in incs]]
 def part_sources(part,private=REF):
  name,deps,_,_=PARTS[part];return [private/part/name,*(providers[n] for n in deps),LAB/part/'tests.c']
 first_paired=None
 for cc,opt,tag in (PROFILES[:1] if a.pilot else PROFILES):
  flags=[*strict,opt,*(['-O1','-g'] if 'asan' in tag else [])]
  for family,provider,cases in [('b04_depth','b04',['memory']),('b05_depth','b05',['verify','recover','copy','timer','response','grace']),('b045_systems','systems',['paired','reduce','replace','wait','shoot','peer']),('modern_bridge','bridge',['once','hazard','epoch'])]:
   exe=work/(tag+'-'+family);ok,_=run(tag+' '+family+' compile',[cc,*flags,'-pthread',providers[provider],ROOT/f'labs/{family}/tests/tests.c','-o',exe])
   if ok:
    for case in cases:run(tag+' '+family+' '+case,[exe,case])
  for part in PARTS:
   ok,log=compile_run(tag+' '+part,cc,flags,part_sources(part),PARTS[part][2])
   if ok and part=='authentication':run(tag+' MAC oracle',[sys.executable,REF/'authentication/oracle.py',log],marker='AUTH_ORACLE_PASS')
  for part,marker in [('transfers','SX_ROWS_PASS'),('reclamation',None),('response',None)]:
   src=part_sources(part)[:-1]+[REF/part/'rows.c'];ok,log=compile_run(tag+' '+part+' rows',cc,flags,src,marker)
   if ok:run(tag+' '+part+' oracle',[sys.executable,REF/part/'oracle.py',log],marker='ORACLE_PASS')
  compile_run(tag+' threads',cc,[*flags,'-pthread'],[REF/'reclamation/list.c',BASE['bridge'],REF/'reclamation/threads.c'],'LIST_THREAD_PASS rounds=60')
  ok,log=compile_run(tag+' paired',cc,flags,[port,LAB/'statistics/kernel.c',LAB/'statistics/benchmark.c'],'PAIRED median=')
  if ok:
   first_paired=first_paired or log;run(tag+' bootstrap',[sys.executable,REF/'statistics/verify.py',log],marker='"passed": true');run(tag+' retained benchmark calls',['objdump','-drwC',work/(tag+'-paired')])
 if not a.pilot:
  for cc in ['gcc','clang']:
   for part,define,count in [('transfers','SX_MUTANT',4),('once','SX_ONCE_MUTANT',2),('reclamation','SX_LIST_MUTANT',3),('response','SX_RESPONSE_MUTANT',2),('authentication','SX_AUTH_MUTANT',2)]:
    for mutant in range(1,count+1):compile_run(cc+' '+part+' mutant '+str(mutant),cc,[*strict,'-O2','-D'+define+'='+str(mutant)],part_sources(part),PARTS[part][3],1)
   compile_run(cc+' tsan',cc,[*strict,'-O1','-g','-pthread','-fsanitize=thread'],[REF/'reclamation/list.c',BASE['bridge'],REF/'reclamation/threads.c'],'LIST_THREAD_PASS rounds=60')
  flaky=work/'flaky';ok,_=run('controlled mixed replay compile',['gcc',*strict,'-O2',REF/'replay/flaky.c','-o',flaky])
  if ok:run('actual repeated mixed replay',[sys.executable,REF/'replay/verify.py',flaky,work/'replay-state'],marker='"controlled_mixed_replay_pairs": 12')
  if first_paired:
   run('bootstrap compiling learner semantic rejection',[sys.executable,REF/'statistics/verify.py',first_paired,'--candidate',LAB/'statistics/bootstrap.py'],1,'BOOTSTRAP_CHECK: NotImplementedError')
   for name,old,new in [('upper-median','(len(values)-1)//2','len(values)//2'),('wrong-quantile','*975//1000','*900//1000')]:
    body=(REF/'statistics/bootstrap.py').read_text();assert body.count(old)==1;mutant=work/(name+'.py');mutant.write_text(body.replace(old,new));run(name+' bootstrap control',[sys.executable,REF/'statistics/verify.py',first_paired,'--candidate',mutant],1,'BOOTSTRAP_CHECK: AssertionError')
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','2']:
    target=work/(cc+'-m4-'+opt);ok,_=run(cc+' M4 '+opt,[sys.executable,LAB/'reclamation/target.py','--source',REF/'reclamation/list.c','--bridge',BASE['bridge'],'--runtime',ROOT/'assessor/execution','--build',target,'--cc',cc,'--opt',opt],marker='LIST_TARGET_PASS',timeout=180)
    if ok:
     obj=json.loads((target/'target.json').read_text());external.update({path:value for path,value in obj['source_dependencies'].items() if not Path(path).is_relative_to(ROOT)})
  # Export public paths only; explicit source choices do not copy assessor bytes.
  export=work/'export';shutil.copytree(LAB,export/'systems_x_followthrough',ignore=shutil.ignore_patterns('__pycache__'))
  for d in [PUBLIC,HEADERS]:
   for value in d.values():dest=export/value;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/'labs'/value,dest)
  for cc in ['gcc','clang']:
   for part in PARTS:
    args=[sys.executable,export/'systems_x_followthrough/run.py','--cc',cc,'--part',part,'--build',work/(cc+'-export-'+part)]
    run(cc+' export '+part+' stub',args,1,PARTS[part][3]);run(cc+' export '+part+' reference',[*args,'--source',REF,*sum(([ '--'+n,path] for n,path in providers.items()),[])],marker=PARTS[part][2])
  checks.append(dict(label='public export no assessor',passed=not any('assessor' in p.parts for p in export.rglob('*'))))
 checks.append(dict(label='sources unchanged during execution',passed=before==inputs()))
 value=dict(schema_version=1,passed=all(r['passed'] for r in checks),pilot=a.pilot,checks=checks,input_sha256=before,external_input_sha256=external,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},limits=['Original bounded author experiments, not independent review/retrieval or physical evidence.','M4 actual CMX task execution on AN386 is distinct from hardware or multi-core execution.','Explicit gate protects ordinary container data; no lock-free progress or universal weak-memory theorem.','Copy/hotplug/authentication contracts retain serialization/trusted-quiescence/key/rollback assumptions.','Actual paired times and resampling do not prove speedup,95% coverage or WCET.'])
 report=ROOT/a.report;report.parent.mkdir(parents=True,exist_ok=True);report.write_text(json.dumps(value,indent=2)+'\n');print(json.dumps({'passed':value['passed'],'checks':len(checks),'failed':[r['label'] for r in checks if not r['passed']]}));return int(not value['passed'])
if __name__=='__main__':raise SystemExit(main())
