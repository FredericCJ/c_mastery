"""Actual host evidence plus honest licensed-target unavailability."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];BASE='research/v5/expansion/alternate_close_depth';LAB='labs/alternate_close_depth'
PROFILES={'gcc-debug':('gcc',['-O0','-g']),'gcc-release':('gcc',['-O3']),'gcc-asan':('gcc',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie']),'clang-debug':('clang',['-O0','-g']),'clang-release':('clang',['-O3']),'clang-asan':('clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 files=[p for base in [LAB,'assessor/alternate_close_depth'] for p in (ROOT/base).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
 files += [ROOT/'tools/run_alternate_close_depth.py',ROOT/'tools/verify_alternate_close_depth.py',ROOT/BASE/'resources.json',ROOT/BASE/'research_gate.json']
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=BASE+'/validation.json');a=p.parse_args()
 stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');out=ROOT/'reviews/v5/expansion/alternate_close_depth'/stamp;out.mkdir(parents=True)
 report=dict(schema_version=1,scope='C17 host reference/seam/cancellation plus licensed QNX8 build probe; no alternate OS execution',input_sha256=inputs(),commands=[],profiles=[],passed=False,target_execution='not-run',external_review='not-run',independent_learner='not-assessed',physical_observation='not-run')
 def run(label,argv,expect=0,marker=None):
  r=subprocess.run([str(s) for s in argv],cwd=ROOT,text=True,capture_output=True,timeout=120,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'})
  ok=r.returncode==expect and (not marker or marker in r.stdout+r.stderr)
  entry=dict(label=label,argv=[str(s) for s in argv],exit_code=r.returncode,expected=expect,marker=marker,stdout=r.stdout,stderr=r.stderr,passed=ok);report['commands'].append(entry)
  (out/(str(len(report['commands']))+'.json')).write_text(json.dumps(entry,indent=2)+'\n')
  if not ok:raise RuntimeError(label+' unexpected result: '+r.stdout+r.stderr)
  return r
 try:
  for name,(cc,flags) in PROFILES.items():
   run(name+' version',[cc,'--version']);common=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-pthread',*flags,'-I'+LAB]
   binary=out/(name+'-test');src=[LAB+'/tests.c','assessor/alternate_close_depth/close_owner.c']
   run(name+' reference build',[*common,*src,'-o',binary])
   for case,marker in [('core','QC_OK:core'),('native','QC_OK:native-seam-linux'),('cancel','QC_OK:cancellation-handoff')]:run(name+' '+case,[binary,case],marker=marker)
   probe=out/(name+'-probe');run(name+' oracle build',[*common,LAB+'/probe.c',src[1],'-o',probe])
   oracle=run(name+' oracle',[sys.executable,'tools/verify_alternate_close_depth.py','--driver',probe]);od=json.loads(oracle.stdout);assert od['passed'] and od['cases']==1305
   for m in range(1,7):
    mutant=out/(name+'-mutant'+str(m));run(name+' mutant'+str(m)+' build',[*common,'-DQC_MUTATION='+str(m),*src,'-o',mutant]);run(name+' mutant'+str(m)+' reject',[mutant,'core'],1,'QC_CHECK:')
   learner=out/(name+'-learner');run(name+' learner build',[*common,src[0],LAB+'/learner.c','-o',learner]);run(name+' learner reject',[learner,'core'],1,'QC_CHECK:')
   report['profiles'].append(dict(name=name,passed=True,oracle=od,mutants=6,learner_rejected=True))
  for source in ['qnx_probe.c','qnx_manager.c']:run(source+' host refusal',['gcc','-std=c17','-I'+LAB,'-fsyntax-only',LAB+'/'+source],1,'QNX_SDP8_TARGET_REQUIRED')
  target=out/'qnx';r=run('actual SDK availability/build',[sys.executable,LAB+'/qnx_build.py','--source','assessor/alternate_close_depth/close_owner.c','--out',target],2,'qcc unavailable')
  report['target_build']=dict(path=(target/'build.json').relative_to(ROOT).as_posix(),sha256=sha(target/'build.json'),status='not-built',reason=json.loads(r.stdout)['blocker'])
  exported=out/'public';exported.mkdir()
  for path in (ROOT/LAB).iterdir():
   if path.is_file():shutil.copyfile(path,exported/path.name)
  run('isolated learner export',[sys.executable,exported/'run.py','--out',out/'export-learner'],1,'QC_CHECK:')
  run('isolated explicit reference export',[sys.executable,exported/'run.py','--out',out/'export-reference','--source',ROOT/src[1]],marker='QC_OK:cancellation-handoff')
  report['source_stable']=inputs()==report['input_sha256'];assert report['source_stable'];report['passed']=True
 except (RuntimeError,AssertionError,OSError,subprocess.TimeoutExpired) as error:report['failure']=str(error)
 report['outputs_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in out.rglob('*') if p.is_file()};report['limits']=['Host injected EINTR is not a QNX kernel event.','QNX8 target sources remain uncompiled/unexecuted without authorized SDK/guest.','Controlled resource-manager error trigger is not actual signal interruption.','No independent review, delayed learner attempt, whole-domain closure or physical claim.']
 report['finished_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat();dest=ROOT/a.report;dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(report,indent=2)+'\n');(out/'report.json').write_bytes(dest.read_bytes());print({'passed':report['passed'],'commands':len(report['commands']),'profiles':len(report['profiles']),'target':report.get('target_build'),'failure':report.get('failure')});return int(not report['passed'])
if __name__=='__main__':raise SystemExit(main())
