"""Source-bound T46.12 cumulative host assessment and actual asynchronous store."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/runtime_followthrough/assessment';REF=ROOT/'assessor/runtime_followthrough/assessment'
PROVIDERS={'cvm':'assessor/runtime/cvm.c','compiler':'assessor/runtime/compiler.c','execution':'assessor/runtime_followthrough/execution/execution.c','verifier':'assessor/runtime_followthrough/verifier/verifier.c','services':'assessor/runtime_followthrough/services/services.c','store':'assessor/storage/store.c','adapter':'labs/runtime/adapters/store_host.c'}
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan'),('gcc','-fsanitize=thread','gcc-tsan'),('clang','-fsanitize=thread','clang-tsan')]
ASYNC_MUTANTS={1:'observed == VA_BUSY',2:'status == CVM_HOST_ERROR',3:'status == VS_RUNNING'}
DIAG_MUTANTS={'verifier':'cvm_verify(&b) == CVM_VERIFY','roots':'cvm_field(&vm, a, 0, &x) == CVM_OK','fuel':'status == (fuel == 1 ? CVM_FUEL : CVM_OK)'}
ASYNC_MARKER='VA_PASS real_file_commit_pending_cancel_quiescence';DIAG_MARKER='VP_DIAG_PASS three_minimal_pairs_host_lifetime_boundary';CORE=[k for k in PROVIDERS if k!='compiler']
HEADERS=['labs/runtime/include/cvm.h','labs/runtime/adapters/store_host.h','labs/runtime_followthrough/execution/execution.h','labs/runtime_followthrough/verifier/verifier.h','labs/runtime_followthrough/services/services.h','labs/storage/include/store.h']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():return sorted({Path(__file__),*[ROOT/p for p in PROVIDERS.values()],*[ROOT/p for p in HEADERS],ROOT/'labs/runtime/SPEC.md',*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]})
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/runtime-assessment'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1',TSAN_OPTIONS='halt_on_error=1')
 def run(label,cmd,expected=0,marker=None):
  cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log')
  with log.open('w') as f:
   try:rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,env=env,timeout=180).returncode
   except (OSError,subprocess.TimeoutExpired) as e:f.write(str(e));rc=None
  t=log.read_text(errors='replace');ok=rc==expected and (not marker or marker in t) and not any(x in t for x in ['runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer','FATAL: ThreadSanitizer']);rows.append(dict(label=label,command=cmd,returncode=rc,expected=expected,marker=marker,passed=bool(ok),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(ok),flush=True);return ok
 for cc in ['gcc','clang']:run(cc+' version',[cc,'--version'])
 run('environment',['uname','-a'])
 for kind in ['verifier','roots']:run('construct '+kind+' control',[sys.executable,REF/'mutate.py',ROOT/PROVIDERS['cvm'],w/(kind+'.c'),kind])
 run('construct H8 profile',[sys.executable,LAB/'port_capacity.py',ROOT/HEADERS[0],w/'h8/cvm.h'])
 inc=['-I'+str(p) for p in [LAB,*[ROOT/Path(p).parent for p in HEADERS]]]
 for cc,opt,tag in PROFILES:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*inc]+(['-O1','-g'] if 'san' in tag else [])
  for v in [0,*ASYNC_MUTANTS,'stub']:
   exe=w/(tag+'-async-'+str(v));src=LAB/'learner.c' if v=='stub' else REF/'async_store.c'
   if run(tag+' async '+str(v)+' compile',[cc,*flags,'-pthread','-DVA_MUTANT='+str(0 if v=='stub' else v),src,*[ROOT/PROVIDERS[k] for k in CORE],LAB/'async_tests.c','-o',exe]):run(tag+' async '+str(v)+' execute',[exe],int(v!=0),ASYNC_MARKER if v==0 else 'va_init(&task) == VA_READY' if v=='stub' else ASYNC_MUTANTS[v])
  for v in ['original',*DIAG_MUTANTS]:
   exe=w/(tag+'-diag-'+v);src=w/(v+'.c') if v in ['verifier','roots'] else ROOT/PROVIDERS['cvm'];defines=['-DCVM_MUTATE_NO_FUEL'] if v=='fuel' else []
   if run(tag+' diag '+v+' compile',[cc,*flags,*defines,src,REF/'diagnosis.c','-o',exe]):run(tag+' diag '+v+' execute',[exe],int(v!='original'),DIAG_MARKER if v=='original' else DIAG_MUTANTS[v])
  exe=w/(tag+'-create-failure')
  if run(tag+' create failure compile',[cc,*flags,'-pthread','-Wl,--wrap=pthread_create',REF/'async_store.c',*[ROOT/PROVIDERS[k] for k in CORE],REF/'create_failure.c','-o',exe]):run(tag+' create failure execute',[exe],marker='VA_CREATE_FAILURE_PASS')
  for heap in [16,8]:
   exe=w/(tag+'-capacity-'+str(heap));prefix=['-I'+str(w/'h8')] if heap==8 else []
   if run(tag+' capacity '+str(heap)+' compile',[cc,*prefix,*flags,ROOT/PROVIDERS['cvm'],LAB/'capacity_tests.c','-o',exe]):run(tag+' capacity '+str(heap)+' execute',[exe],marker='VP_CAP_PASS heap='+str(heap)+' root2_fields4_sweep='+str(heap+6))
  compiler=w/(tag+'-compiler');vm=w/(tag+'-vm');build_rows=[]
  for kind,exe in [('compiler',compiler),('vm',vm)]:
   run(tag+' '+kind+' build',[cc,*flags,*(['-DVP_COMPILER',ROOT/PROVIDERS['compiler']] if kind=='compiler' else []),ROOT/PROVIDERS['cvm'],LAB/'chain_driver.c','-o',exe]);r=rows[-1];build_rows.append(dict(command=r['command'],returncode=r['returncode'],stdout=(ROOT/r['raw']).read_text(),stderr=''))
  build=w/(tag+'-build.json');build.write_text(json.dumps(dict(passed=all(r['returncode']==0 for r in build_rows),commands=build_rows,inputs={str(p):sha(p) for p in [ROOT/PROVIDERS['compiler'],ROOT/PROVIDERS['cvm'],LAB/'chain_driver.c',ROOT/HEADERS[0],Path(shutil.which(cc))]},outputs={str(p):sha(p) for p in [compiler,vm] if p.exists()}),indent=2)+'\n')
  chain=w/(tag+'-chain')
  if run(tag+' chain record',[sys.executable,LAB/'provenance.py','--source',LAB/'source.cvm','--compiler',compiler,'--vm',vm,'--build-record',build,'--output',chain],marker='"value": 42'):
   # Keep each profile's controls separate; they deliberately create alternate files.
   run(tag+' chain controls',[sys.executable,REF/'chain_tests.py','--record',chain/'chain.json'],marker='VP_CHAIN_TEST_PASS seven_rejections_exact19bytes_adapter42_to43')
 for stub in [False,True]:run('public '+str(stub),[sys.executable,LAB/'run.py',*[x for k,p in PROVIDERS.items() for x in ['--'+k,ROOT/p]],'--build',w/('public-'+str(stub)),*([] if stub else ['--source',REF/'async_store.c'])],int(stub),'VA_CHECK' if stub else 'VP_CHAIN_PASS')
 run('missing provider refused',[sys.executable,LAB/'run.py','--build',w/'missing'],2,'--cvm')
 rows.append(dict(label='source stability',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 d=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},external_tool_sha256={cc:sha(Path(shutil.which(cc))) for cc in ['gcc','clang']},limits=['Actual Cvm/compiler/Vs/Vv/Vh/store providers; original files unchanged.','Finite original and explicit H8 profiles; strict target closure belongs to separate T46.11 and predecessor campaigns.','POSIX worker executes real pread/pwrite/fsync on owned temporary files; controlled gate/reply loss, not power-cut or device durability measurement.','Cancellation suppresses guest completion after join; does not stop arbitrary kernel I/O or reverse committed effects.','Trusted-binary source recompilation and VM replay; hashes alone are not derivation or compiler correctness.','External reader/review, actual minimum-seven-day learner attempt and physical observations remain not-run.'])
 out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
