"""Actual CLI publication, explicit assessor providers, compiled learner rejection."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 dest=ROOT/'build/layers-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');rows=[];errors=[]
 def run(label,cmd,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=90);ok=r.returncode==expected and (not marker or marker in r.stdout+r.stderr);rows.append(dict(label=label,command=cmd,returncode=r.returncode,expected=expected,marker=marker,stdout=r.stdout,stderr=r.stderr,passed=ok));return ok
 attachment=json.loads((ROOT/'research/v5/expansion/legacy_advanced_layers/attachment_PR-T28-06.json').read_text());ref=ROOT/'assessor/peripheral_depth/layers';uart=ROOT/'assessor/peripheral_depth/uart';serial=ROOT/'assessor/serial_depth/serial_depth.c'
 if not run('actual init',[sys.executable,'-m','cmastery','init','PR-T28-06',dest]):errors.append('init failed')
 else:
  for p in attachment['supplied_artifacts']:
   if not (dest/p).is_file() or sha(dest/p)!=sha(ROOT/p):errors.append('missing/changed public '+p)
  private={sha(p) for p in ref.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
  source=json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T28-06.json').read_text());export=json.loads((dest/'ASSESSMENT.json').read_text());
  if source['criteria']!=export['criteria']:errors.append('criteria changed')
  lab=dest/'labs/peripheral_depth/layers';includes=['-I'+str(dest/p) for p in ['labs/peripheral_depth/layers','labs/peripheral_depth/uart','labs/peripheral_depth/mmio','labs/serial_depth/include','labs/firmware_io/include']]
  cases=[('service','service.c',lab/'tests.c',[],[],'PL_PASS','PL_CHECK:'),('posted','posted.c',ref/'posted_test.c',[ROOT/'assessor/peripheral_depth/mmio/mmio.c'],[],'PL_POST_PASS','PL_POST_CHECK:'),('compare','compare.c',ref/'compare_test.c',[],['-DPL_NATIVE'],'PL_COMPARE_PASS','PL_COMPARE_CHECK:'),('uart','uart_adapter.c',ref/'uart_test.c',[ref/'service.c',uart/'uart.c',serial],[],'PL_UART_PASS','PU_CHECK:'),('target','target.c',ref/'target_test.c',[ref/'service.c',ref/'uart_adapter.c',uart/'uart.c',uart/'target.c',serial],['-DPU_TARGET_MODEL'],'PL_TARGET_PASS','PU_TARGET_CHECK:')]
  for name,file,test,extras,defs,good,bad in cases:
   for reference in [False,True]:
    label=name+(' explicit assessor' if reference else ' exported learner');exe=dest/(name+str(reference));selected=ref/file if reference else lab/file
    if run(label+' compile',['gcc','-std=c17','-O2','-Wall','-Wextra','-Werror',*includes,*defs,selected,test,*extras,'-o',exe]):run(label+' execute',[exe],0 if reference else 1,good if reference else bad)
 pins=[Path(__file__),ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T28-06.json',ROOT/'research/v5/expansion/legacy_advanced_layers/attachment_PR-T28-06.json',*[ROOT/p for p in attachment['supplied_artifacts']],*[p for p in ref.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
 from envelope_export_inputs import bind_project_exports
 pins=set(pins);bind_project_exports(ROOT,pins,rows,errors)
 out=dict(passed=not errors and len(rows)==21 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in pins},limits=['Reference candidates and prerequisite providers explicitly selected; no learner mastery/physical evidence.','Private transfer tests remain outside the learner export; assessor source is not copied.'])
 (ROOT/'reviews/v5/expansion/legacy_advanced_layers/exports.json').write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=out['passed'],commands=len(rows),errors=errors,failed=[r['label'] for r in rows if not r['passed']]));return int(not out['passed'])
if __name__=='__main__':raise SystemExit(main())
