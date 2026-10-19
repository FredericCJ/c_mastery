"""Execute isolated learner export with external explicit reference providers."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys
from run_vm_verifier import PROVIDERS,target_args,emulator,MARKER
ROOT=Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_vm_verifier';REF=ROOT/'assessor/runtime_followthrough/verifier'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 base=ROOT/'build/runtime-verifier-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');base.mkdir(parents=True);rows=[];errors=[];project='PR-T46-02';dest=base/project
 attachment=HERE/('attachment_'+project+'.json');a=json.loads(attachment.read_text());source=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json')
 pins={Path(__file__),ROOT/'tools/run_vm_verifier.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',attachment,source,*[ROOT/p for p in PROVIDERS.values()],*[ROOT/p for p in a['supplied_artifacts']]}
 def run(label,cmd,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=180);ok=r.returncode==expected and (not marker or marker in r.stdout+r.stderr);rows.append(dict(label=project+' '+label,command=cmd,returncode=r.returncode,expected=expected,marker=marker,stdout=r.stdout,stderr=r.stderr,passed=bool(ok)));return ok
 if run('actual init',[sys.executable,'-m','cmastery','init',project,dest]):
  for p in a['supplied_artifacts']:
   if not (dest/p).is_file() or sha(dest/p)!=sha(ROOT/p):errors.append('missing/changed '+p)
  private={sha(p) for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
  if json.loads(source.read_text())['criteria']!=json.loads((dest/'ASSESSMENT.json').read_text())['criteria']:errors.append('criteria changed')
  lab=dest/'labs/runtime_followthrough/verifier'
  run('missing provider',[sys.executable,lab/'run.py','--build',base/'missing'],2,'--cvm')
  for ref in [False,True]:
   cmd=[sys.executable,lab/'run.py','--build',base/('native-'+str(ref)),'--cvm',ROOT/PROVIDERS['cvm']]
   if ref:cmd+=['--source',REF/'verifier.c']
   run('native '+str(ref),cmd,int(not ref),MARKER if ref else 'VV_CHECK')
   target=base/('target-'+str(ref));cmd=target_args(target,'arm-none-eabi-gcc','0',0 if ref else 'stub');cmd[1]=lab/'target_build.py'
   if not ref:cmd[cmd.index('--source')+1]=lab/'learner.c'
   if run('target build '+str(ref),cmd,marker='VV_TARGET_BUILD_PASS explicit_objects=9 original_limb_division'):
    run('target execute '+str(ref),emulator(target/'image.elf'),int(not ref),'VV_TARGET_PASS' if ref else 'VV_CHECK')
    # Exact compiler dependency paths must include the exported header, never
    # silently select the repository header for this public build.
    text='\n'.join(p.read_text() for p in target.glob('*.d'))
    if str(dest/'labs/runtime/include/cvm.h') not in text or str(ROOT/'labs/runtime/include/cvm.h') in text:errors.append('header fallback')
 else:errors.append('init failed')
 pins.update(p for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 d=dict(passed=not errors and len(rows)==8 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)},limits=['Actual exported public header/source/tool identities, compiling learner stub rejection and explicit reference providers outside learner tree.','M4 final closure checks run inside exported builder; no root-header fallback.','No physical, independent learner or external-review claim.']);(ROOT/'reviews/v5/expansion/legacy_vm_verifier/exports.json').write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(rows),errors=errors,failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
