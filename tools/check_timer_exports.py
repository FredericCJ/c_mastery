"""Actual CLI init plus compiled core/dependency/target acceptance and rejection."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 dest=ROOT/'build/timer-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');rows=[];errors=[]
 def run(label,cmd,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=90);ok=r.returncode==expected and (not marker or marker in r.stdout+r.stderr);rows.append(dict(label=label,command=cmd,returncode=r.returncode,expected=expected,marker=marker,stdout=r.stdout,stderr=r.stderr,passed=ok));return ok
 attachment=json.loads((ROOT/'research/v5/expansion/legacy_advanced_timer/attachment_PR-T28-03.json').read_text())
 if not run('actual init',[sys.executable,'-m','cmastery','init','PR-T28-03',dest]):errors.append('init failed')
 else:
  for p in attachment['supplied_artifacts']:
   if not (dest/p).is_file() or sha(dest/p)!=sha(ROOT/p):errors.append('missing public '+p)
  private={sha(p) for p in (ROOT/'assessor/peripheral_depth/timer').rglob('*') if p.is_file() and '__pycache__' not in p.parts}
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
  source=json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T28-03.json').read_text());export=json.loads((dest/'ASSESSMENT.json').read_text());
  if source['criteria']!=export['criteria']:errors.append('criteria changed')
  runner=dest/'labs/peripheral_depth/timer/run.py';core=ROOT/'assessor/peripheral_depth/timer/timer.c';clock=ROOT/'assessor/peripheral_depth/uart/uart.c';serial=ROOT/'assessor/serial_depth/serial_depth.c'
  run('compiled core stub rejected',[sys.executable,runner,'--build',dest/'core-stub'],1,'PT_CHECK:')
  run('compiled clock stub rejected',[sys.executable,runner,'--source',core,'--build',dest/'clock-stub'],1,'PT_CHECK:')
  run('explicit external references',[sys.executable,runner,'--source',core,'--clock-source',clock,'--serial-source',serial,'--build',dest/'core-ref'],0,'PT_PASS')
  flags=['--target','--type-headers','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib']
  run('compiled target stub rejected',[sys.executable,runner,*flags,'--build',dest/'target-stub'],1,'PT_TARGET_REJECT')
  run('external target reference',[sys.executable,runner,*flags,'--source',ROOT/'assessor/peripheral_depth/timer/target.c','--build',dest/'target-ref'],0,'PT_TARGET_PASS')
  bad=dest.parent/(dest.name+'-wrong-base.c');bad.write_text((ROOT/'assessor/peripheral_depth/timer/target.c').read_text().replace('0x40000400u','0x40000c00u'))
  run('compiled wrong timer base rejected',[sys.executable,runner,*flags,'--source',bad,'--build',dest/'target-wrong'],1,'PT_TARGET_REJECT')
 pins=[Path(__file__),ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T28-03.json',ROOT/'research/v5/expansion/legacy_advanced_timer/attachment_PR-T28-03.json',*[ROOT/p for p in attachment['supplied_artifacts']]]
 from envelope_export_inputs import bind_project_exports
 pins=set(pins);bind_project_exports(ROOT,pins,rows,errors)
 out=dict(passed=not errors and len(rows)==7 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in pins},limits=['Assessor references explicitly selected; no learner or physical evidence.','Target structural rejection complements the private actual register mapping controls.'])
 (ROOT/'reviews/v5/expansion/legacy_advanced_timer/exports.json').write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=out['passed'],commands=len(rows),errors=errors,failed=[r['label'] for r in rows if not r['passed']]));return int(not out['passed'])
if __name__=='__main__':raise SystemExit(main())
