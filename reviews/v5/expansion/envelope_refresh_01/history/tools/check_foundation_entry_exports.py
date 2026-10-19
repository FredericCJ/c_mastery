"""Execute the actual exported PR-T01-01 workspace with no hidden provider fallback."""
from pathlib import Path
import datetime,json,subprocess,sys
from run_foundation_entry import ROOT,REF,PROVIDER,MARKERS,STUB,sha,target_args,emulator
from close_foundation_entry import HERE,CARD,PROJECT
def main():
 w=ROOT/'build/foundation-entry-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);dest=w/PROJECT;rows=[];errors=[]
 attachment=HERE/('attachment_'+PROJECT+'.json');a=json.loads(attachment.read_text());pins={Path(__file__),ROOT/'tools/run_foundation_entry.py',ROOT/'assessor/execution/support.c',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',ROOT/CARD,attachment,PROVIDER,*[ROOT/f for f in a['supplied_artifacts']],*[p for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts]}
 def run(label,cmd,expected=0,markers=()):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=120);ok=r.returncode==expected and all(m in r.stdout+r.stderr for m in markers)
  rows.append(dict(label=label,command=cmd,expected=expected,returncode=r.returncode,markers=list(markers),stdout=r.stdout,stderr=r.stderr,passed=ok));return ok
 if run('actual init',[sys.executable,'-m','cmastery','init',PROJECT,dest]):
  for f in a['supplied_artifacts']:
   if not (dest/f).is_file() or sha(dest/f)!=sha(ROOT/f):errors.append('missing/changed '+f)
  private={sha(p) for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
  if json.loads((ROOT/CARD).read_text())['criteria']!=json.loads((dest/'ASSESSMENT.json').read_text())['criteria']:errors.append('changed criteria')
  lab=dest/'labs/foundation_entry';run('missing provider',[sys.executable,lab/'run.py','--build',w/'missing'],2,['--foundation'])
  for cc in ['gcc','clang']:
   for ref in [False,True]:
    cmd=[sys.executable,lab/'run.py','--cc',cc,'--foundation',PROVIDER,'--build',w/(cc+'-'+str(ref)),*(['--source',REF/'entry.c'] if ref else [])]
    run(cc+' '+str(ref),cmd,int(not ref),MARKERS if ref else [STUB])
  dep=w/'headers.d';run('header dependency identity',['gcc','-std=c17','-MM','-MF',dep,'-I'+str(lab),'-I'+str(dest/'labs/foundations/include'),lab/'learner.c'])
  t=dep.read_text() if dep.exists() else ''
  if str(dest/'labs/foundations/include/foundation.h') not in t or str(ROOT/'labs/foundations/include/foundation.h') in t:errors.append('header fallback')
  for ref in [False,True]:
   target=w/('target-'+str(ref));cmd=target_args(target,'arm-none-eabi-gcc','0',not ref);cmd[1]=lab/'target_build.py'
   if not ref:cmd[cmd.index('--source')+1]=lab/'learner.c'
   if run('target build '+str(ref),cmd,markers=['FE_TARGET_BUILD_PASS explicit_objects=5 no_archives']):
    run('target execute '+str(ref),emulator(target/'image.elf'),int(not ref),['FE_TARGET_PASS all_functions explicit_prefix_provider recursion16'] if ref else ['FE_TARGET_FAIL'])
    dependencies='\n'.join(p.read_text() for p in target.glob('*.d'))
    if str(dest/'labs/foundations/include/foundation.h') not in dependencies or str(ROOT/'labs/foundations/include/foundation.h') in dependencies:errors.append('target header fallback')
 else:errors.append('init failed')
 d=dict(passed=not errors and len(rows)==11 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)},limits=['Actual isolated public export; explicit author reference outside learner tree.','GCC/Clang and actual M4 stubs compile and reject semantically, references succeed; missing provider refused and exported header identity checked.','No physical, independent learner or external-review claim.'])
 (ROOT/'reviews/v5/expansion/legacy_foundation_entry/exports.json').write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(rows),errors=errors,failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
