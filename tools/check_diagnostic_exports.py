"""Exercise actual CLI publication and exported public builders without private fallback."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 dest=ROOT/'build/diagnostic-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');rows=[];errors=[]
 def run(label,cmd,expected=0,marker=None,timeout=120):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=timeout);ok=r.returncode==expected and (not marker or marker in r.stdout+r.stderr);rows.append(dict(label=label,command=cmd,returncode=r.returncode,expected=expected,marker=marker,stdout=r.stdout,stderr=r.stderr,passed=ok));return ok
 attachment=json.loads((ROOT/'research/v5/expansion/legacy_advanced_diagnostic/attachment_PR-T28-07.json').read_text());ref=ROOT/'assessor/peripheral_depth/diagnostic'
 if not run('actual init',[sys.executable,'-m','cmastery','init','PR-T28-07',dest]):errors.append('init failed')
 else:
  for p in attachment['supplied_artifacts']:
   if not (dest/p).is_file() or sha(dest/p)!=sha(ROOT/p):errors.append('missing/changed public '+p)
  # These exact shared type declarations and build rules contain no solution.
  shared={ref/'guest/Makefile',*[p for p in (ref/'guest/headers').glob('*.h')]}
  for p in shared:
   public=ROOT/'labs/peripheral_depth/diagnostic'/p.relative_to(ref)
   if not public.is_file() or sha(public)!=sha(p):errors.append('shared utility differs '+str(p))
  private={sha(p) for p in ref.rglob('*') if p.is_file() and '__pycache__' not in p.parts and p not in shared}
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
  source=json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T28-07.json').read_text());export=json.loads((dest/'ASSESSMENT.json').read_text())
  if source['criteria']!=export['criteria']:errors.append('criteria changed')
  lab=dest/'labs/peripheral_depth/diagnostic'
  for kind in ['core','usb','host']:
   for reference in [False,True]:
    label=kind+(' explicit assessor' if reference else ' exported learner');cmd=[sys.executable,lab/'run.py','--kind',kind,'--build',dest/('build-'+kind+str(reference))]
    if reference:
     cmd+=['--source',ref/('diagnostic.c' if kind=='core' else kind+'.c'),'--foundation',ROOT/'assessor/foundations/foundation.c']
     cmd+=(['--adapter',ref/'ethernet.c','--ethernet',ROOT/'assessor/ethernet_depth/ethernet_depth.c'] if kind=='core' else ['--payload',ref/'diagnostic.c'])
    marker='DG' if kind=='core' else 'DG_PUBLIC_'+kind.upper()
    run(label,cmd,0 if reference else 1,marker+('_PASS' if reference else '_CHECK:'))
  for reference in [False,True]:
   work=Path('/home/frede/cmastery-diagnostic-tools')/('export-'+dest.name+'-'+str(reference))
   cmd=[sys.executable,lab/'guest/build.py','--kernel-headers','/home/frede/cmastery-b06-tools/usr/src/linux-headers-7.0.0-38-generic','--dummy-source','/home/frede/cmastery-b06-tools/debug-kernel/linux-source-7.0.0/drivers/usb/gadget/udc/dummy_hcd.c','--directory',work]
   if reference:cmd+=['--source',ref/'guest/dg_gadget.c','--payload',ref/'diagnostic.c','--foundation',ROOT/'assessor/foundations/foundation.c']
   run('exported guest '+('explicit assessor' if reference else 'learner compilation only'),cmd,timeout=180)
   if (work/'build.json').is_file():
    build=json.loads((work/'build.json').read_text());rows[-1]['build']=build
    chosen=ref/'guest/dg_gadget.c' if reference else lab/'guest/dg_gadget.c'
    if str(chosen) not in build['input_sha256']:errors.append('guest provider silently substituted')
 pins=[Path(__file__),ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T28-07.json',ROOT/'research/v5/expansion/legacy_advanced_diagnostic/attachment_PR-T28-07.json',*[ROOT/p for p in attachment['supplied_artifacts']],*[p for p in ref.rglob('*') if p.is_file() and '__pycache__' not in p.parts],ROOT/'assessor/foundations/foundation.c',ROOT/'assessor/ethernet_depth/ethernet_depth.c']
 from envelope_export_inputs import bind_project_exports
 pins=set(pins);bind_project_exports(ROOT,pins,rows,errors)
 out=dict(passed=not errors and len(rows)==9 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in pins},limits=['Reference providers explicitly selected; no learner mastery or physical evidence.','Exported guest builds are compilation checks. Separate campaign exercises runtime binding and traffic.','Only exact shared type-header/build-rule utilities are allowed identical public/private bytes; solutions and withheld tests are excluded.'])
 (ROOT/'reviews/v5/expansion/legacy_advanced_diagnostic/exports.json').write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=out['passed'],commands=len(rows),errors=errors,failed=[r['label'] for r in rows if not r['passed']]));return int(not out['passed'])
if __name__=='__main__':raise SystemExit(main())
