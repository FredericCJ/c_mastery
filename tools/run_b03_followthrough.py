"""Exact original B03 mechanisms and changed-condition campaign, no global writes."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/b03_followthrough';REF=ROOT/'assessor/b03_followthrough';BASE=ROOT/'assessor/b03_depth/depth.c';HEADER=ROOT/'labs/b03_depth/include/depth.h'
GENMC=Path('/home/frede/cmastery-modern-tools/genmc/build/bin/genmc')
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O2','gcc-release'),('clang','-O0','clang-debug'),('clang','-O2','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 return [Path(__file__),BASE,HEADER,ROOT/'labs/b03_depth/src/depth.c',ROOT/'labs/b03_depth/CONTRACTS.md',*[p for folder in [LAB,REF] for p in folder.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);p.add_argument('--pilot',action='store_true');a=p.parse_args()
 work=ROOT/'build/b03-followthrough'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True);checks=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};external={}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1',TSAN_OPTIONS='halt_on_error=1')
 def run(label,argv,expected=0,marker=None,timeout=180):
  argv=list(map(str,argv));log=work/(str(len(checks))+'.log')
  with log.open('w') as f:
   try:rc=subprocess.run(argv,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,env=env,timeout=timeout).returncode
   except (OSError,subprocess.TimeoutExpired) as e:f.write(str(e));rc=None
  body=log.read_text(errors='replace');passed=rc==expected and (not marker or marker in body) and not any(x in body for x in ['runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer'])
  checks.append(dict(label=label,command=argv,expected=expected,returncode=rc,marker=marker,passed=bool(passed),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(passed),flush=True);return passed,log
 def compile_run(label,source,flags,cc='gcc',marker=None,expected=0,args=()):
  exe=work/label.replace(' ','-');ok,_=run(label+' compile',[cc,*flags,*source,'-o',exe]);
  if ok:return run(label+' execute',[exe,*args],expected,marker)
  return False,None
 inc=['-I'+str(LAB),'-I'+str(HEADER.parent)];strict=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*inc]
 for cc in ['gcc','clang']:
  path=Path(shutil.which(cc)).resolve();external[str(path)]=sha(path);run(cc+' version',[cc,'--version'])
 run('environment',['uname','-a'])
 sc=work/'sc_depth.c';run('SC exact source translation',[sys.executable,REF/'sc_translation.py',sc],marker='b3_store_order release -> seq_cst')
 for cc,opt,tag in (PROFILES[:1] if a.pilot else PROFILES):
  flags=[*strict,opt,*(['-O1','-g'] if 'asan' in tag else [])];exe=work/(tag+'-base')
  ok,_=run(tag+' baseline compile',[cc,*flags,BASE,REF/'base_cases.c','-o',exe])
  if ok:
   for kind,marker in [('io','BASE_IO_PASS scripts=351558'),('cancel','BASE_CANCEL_PASS words=16395'),('mmio','BASE_MMIO_PASS cases=66048'),('deadline','BASE_DEADLINE_PASS valid=4194304 ambiguous=4194304 invalid=8388608')]:run(tag+' base '+kind,[exe,kind],marker=marker)
  compile_run(tag+' public',[BASE,REF/'transfers.c',REF/'atomics.c',LAB/'tests.c'],flags,cc,'FOLLOW_PASS')
  exe=work/(tag+'-variant');ok,_=run(tag+' variants compile',[cc,*flags,BASE,REF/'transfers.c',REF/'variants.c','-o',exe])
  if ok:
   for kind in ['scheduler','cancel','message','device','clock']:
    passed,log=run(tag+' variant '+kind,[exe,kind],marker='VAR_')
    if passed and kind in ['cancel','message','device']:run(tag+' oracle '+kind,[sys.executable,REF/'oracle.py',kind,log],marker='ORACLE_PASS')
  for mode in [0,1]:
   label=tag+' threads '+str(mode);compile_run(label,[sc if mode else BASE,REF/'atomics.c',REF/'threads.c'],[*flags,'-pthread','-DBF_SC='+str(mode)],cc,'THREAD_PASS spsc=10000 claim=20000 producer_order=2')
   run(label+' undefined',['nm','-u',work/label.replace(' ','-')]);run(label+' disassembly',['objdump','-drwC',work/label.replace(' ','-')])
 if not a.pilot:
  for cc in ['gcc','clang']:
   flags=[*strict,'-O2']
   for name,kind in [('IO','io'),('CANCEL','cancel'),('MMIO','mmio'),('DEADLINE','deadline')]:compile_run(cc+' base mutant '+name,[BASE,REF/'base_cases.c'],[*flags,'-DB3_MUTATE_'+name],cc,'BASE_CHECK:',1,[kind])
   for mutant in range(1,6):compile_run(cc+' variant mutant '+str(mutant),[BASE,REF/'transfers.c',REF/'atomics.c',LAB/'tests.c'],[*flags,'-DBF_MUTANT='+str(mutant)],cc,'FOLLOW_CHECK:',1)
   for mode in [0,1]:compile_run(cc+' tsan '+str(mode),[sc if mode else BASE,REF/'atomics.c',REF/'threads.c'],[*strict,'-O1','-g','-pthread','-fsanitize=thread','-DBF_SC='+str(mode)],cc,'THREAD_PASS spsc=10000 claim=20000 producer_order=2')
  external[str(GENMC)]=sha(GENMC);run('GenMC version',[GENMC,'--version'],marker='GenMC v0.19.0')
  _,deps=run('GenMC linked dependencies',['ldd',GENMC])
  for line in deps.read_text().splitlines():
   for token in line.split():
    if token.startswith('/') and Path(token).is_file():external[str(Path(token).resolve())]=sha(Path(token).resolve())
  cases=[('spsc',1,0,0,0),('claim',0,0,0,0),('claim-sc',0,1,0,0),('spsc-relaxed',1,0,0,42),('claim-no-publication',0,0,1,42),('claim-no-reuse',0,0,2,42),('original-two-producers',2,0,0,42)]
  for label,model,mode,mutant,expected in cases:
   path=work/(label+'-model.c');path.write_text('\n'.join('#include "'+str(f)+'"' for f in [BASE,REF/'atomics.c',REF/'model.c'])+'\n')
   run('GenMC '+label,[GENMC,'-rc11','-mode=verify','-disable-mm-detector','-disable-estimation','--','-std=c17','-O0',*inc,'-DBF_MODEL='+str(model),'-DBF_SC='+str(mode),'-DBF_ATOMIC_MUTANT='+str(mutant),*(['-DB3_MUTATE_PUBLICATION'] if label=='spsc-relaxed' else []),path],expected,'Error: Non-atomic race!' if expected else 'No errors were detected.')
  # Public tree only: exact two sibling packages; no assessor directory copied.
  export=work/'export';shutil.copytree(LAB,export/'b03_followthrough',ignore=shutil.ignore_patterns('__pycache__'));(export/'b03_depth/include').mkdir(parents=True);(export/'b03_depth/src').mkdir();shutil.copy2(HEADER,export/'b03_depth/include/depth.h');shutil.copy2(ROOT/'labs/b03_depth/src/depth.c',export/'b03_depth/src/depth.c');shutil.copy2(ROOT/'labs/b03_depth/CONTRACTS.md',export/'b03_depth/CONTRACTS.md')
  for cc in ['gcc','clang']:
   runner=export/'b03_followthrough/run.py'
   run(cc+' export learner rejection',[sys.executable,runner,'--cc',cc,'--build',work/(cc+'-export-learner')],1,'FOLLOW_CHECK:')
   run(cc+' export explicit reference',[sys.executable,runner,'--cc',cc,'--build',work/(cc+'-export-reference'),'--source',REF/'transfers.c','--atomic-source',REF/'atomics.c','--base-source',BASE],marker='FOLLOW_PASS')
  checks.append(dict(label='export has no assessor source',passed=not any('assessor' in p.parts for p in export.rglob('*')) and sha(export/'b03_followthrough/transfers.c')==sha(LAB/'transfers.c') and sha(export/'b03_followthrough/atomics.c')==sha(LAB/'atomics.c')))
 checks.append(dict(label='sources stable',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 result=dict(schema_version=1,passed=all(x['passed'] for x in checks),pilot=a.pilot,checks=checks,input_sha256=pins,external_input_sha256=external,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},limits=['Author execution, not independent review, learner delay or physical observation.','GenMC selected RC11 bounded attempts only; native stress is not exhaustive weak-memory proof.','Host state protocols have the explicit callback, atomic-step, counter and quiescence premises in CONTRACTS.md.','No actual OS cancellation, physical MMIO, clock frequency, crash persistence or inaccessible published C17 evidence.'])
 target=ROOT/a.report;target.parent.mkdir(parents=True,exist_ok=True);target.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['label'] for r in checks if not r['passed']]}));return int(not result['passed'])
if __name__=='__main__':raise SystemExit(main())
