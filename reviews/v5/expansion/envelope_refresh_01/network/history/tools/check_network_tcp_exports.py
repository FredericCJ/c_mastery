"""Run exact original project exports with explicit author prerequisites."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys
from run_network_tcp_depth import PROVIDERS,target_args,emulator
ROOT=Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_network_tcp';REF=ROOT/'assessor/network_followthrough/tcp'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 base=ROOT/'build/network-tcp-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');rows=[];errors=[];pins={Path(__file__),ROOT/'tools/run_network_tcp_depth.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',*[ROOT/p for p in PROVIDERS.values()]}
 def run(label,cmd,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=180);ok=r.returncode==expected and (not marker or marker in r.stdout+r.stderr);rows.append(dict(label=label,command=cmd,returncode=r.returncode,expected=expected,marker=marker,stdout=r.stdout,stderr=r.stderr,passed=bool(ok)));return ok
 for project in ['PR-T41-06','PR-T41-07']:
  dest=base/project;attachment=HERE/('attachment_'+project+'.json');a=json.loads(attachment.read_text());source=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins|={attachment,source};pins.update(ROOT/p for p in a['supplied_artifacts'])
  if not run(project+' actual init',[sys.executable,'-m','cmastery','init',project,dest]):errors.append('init failed');continue
  for p in a['supplied_artifacts']:
   if not (dest/p).is_file() or sha(dest/p)!=sha(ROOT/p):errors.append('missing/changed public '+p)
  private={sha(p) for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
  if json.loads(source.read_text())['criteria']!=json.loads((dest/'ASSESSMENT.json').read_text())['criteria']:errors.append('criteria changed')
  lab=dest/'labs/network_followthrough/tcp'
  for reference in [False,True]:
   cmd=[sys.executable,lab/'run.py','--build',dest/('native-'+str(reference)),*[x for k in ['systems','ip','scope','stream','http'] for x in ['--'+k,ROOT/PROVIDERS[k]]]]
   if reference:cmd+=['--source',REF/'tcp.c']
   run(project+' native '+str(reference),cmd,0 if reference else 1,'NC_PASS' if reference else 'NC_CHECK')
   target=dest/('target-'+str(reference));cmd=target_args(target,'arm-none-eabi-gcc','0',stub=not reference);cmd[1]=lab/'target_build.py'
   if not reference:cmd[cmd.index('--source')+1]=lab/'tcp.c'
   ok=run(project+' target build '+str(reference),cmd,marker='NC_TARGET_BUILD_PASS explicit_objects=13 original_runtime')
   if ok:run(project+' target execute '+str(reference),emulator(target/'image.elf'),0 if reference else 1,'NC_TARGET_PASS' if reference else 'NC_CHECK')
  run(project+' capture checker',[sys.executable,dest/'tools/check_network_tcp_capture.py','--self-test'],marker='"physical_claim": false')
 pins.update(p for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 d=dict(passed=not errors and len(rows)==16 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)},limits=['Actual exported native/target tools and compiling incomplete TCP source; author prerequisites explicitly selected outside learner tree.','No implicit private fallback; reference imports are author execution, not learner integration.','No independent learner, physical observation or external review.'])
 (ROOT/'reviews/v5/expansion/legacy_network_tcp/exports.json').write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(rows),errors=errors,failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
