"""Initialize the actual project; reject stubs and target width faults."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 dest=ROOT/'build/mmio-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');rows=[];errors=[]
 def run(label,cmd,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=90);text=r.stdout+r.stderr;ok=r.returncode==expected and (not marker or marker in text);rows.append(dict(label=label,command=cmd,returncode=r.returncode,expected=expected,marker=marker,stdout=r.stdout,stderr=r.stderr,passed=ok));return ok
 if not run('actual PR-T28-01 init',[sys.executable,'-m','cmastery','init','PR-T28-01',dest]):errors.append('init failed')
 else:
  attachment=json.loads((ROOT/'research/v5/expansion/legacy_advanced_mmio/attachment_PR-T28-01.json').read_text())
  for p in attachment['supplied_artifacts']:
   if not (dest/p).is_file() or sha(dest/p)!=sha(ROOT/p):errors.append('missing public '+p)
  private={sha(p) for p in (ROOT/'assessor/peripheral_depth/mmio').rglob('*') if p.is_file() and '__pycache__' not in p.parts}
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
  source=json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T28-01.json').read_text());export=json.loads((dest/'ASSESSMENT.json').read_text());
  if source['criteria']!=export['criteria']:errors.append('criteria changed')
  runner=dest/'labs/peripheral_depth/mmio/run.py'
  run('compiled learner rejected',[sys.executable,runner,'--build',dest/'core-stub'],1,'PM_CHECK:')
  run('external core reference',[sys.executable,runner,'--source',ROOT/'assessor/peripheral_depth/mmio/mmio.c','--build',dest/'core-ref'],0,'PM_PASS')
  flags=['--target','--type-headers','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib']
  run('compiled target learner rejected',[sys.executable,runner,*flags,'--build',dest/'target-stub'],1,'PM_TARGET_REJECT')
  run('external target reference',[sys.executable,runner,*flags,'--source',ROOT/'assessor/peripheral_depth/mmio/target.c','--build',dest/'target-ref'],0,'PM_TARGET_PASS')
  bad=dest.parent/(dest.name+'-wrong_width.c');bad.write_text((ROOT/'assessor/peripheral_depth/mmio/target.c').read_text().replace('return p->idr;','uint32_t v; __asm__ volatile ("ldrh %0,[%1,#16]" : "=r"(v) : "r"(p) : "memory"); return v;'))
  run('compiled wrong-width rejected',[sys.executable,runner,*flags,'--source',bad,'--build',dest/'target-width'],1,'PM_TARGET_REJECT')
  # The temporary negative is assessor evaluation input after leakage checking,
  # never an exported learner artifact or a reference moved into the public tree.
 pins=[Path(__file__),ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T28-01.json',ROOT/'research/v5/expansion/legacy_advanced_mmio/attachment_PR-T28-01.json',*[ROOT/p for p in attachment['supplied_artifacts']]] if not errors else [Path(__file__)]
 from envelope_export_inputs import bind_project_exports
 pins=set(pins);bind_project_exports(ROOT,pins,rows,errors)
 out=dict(passed=not errors and len(rows)==6 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in pins},limits=['Author reference selected explicitly; no learner work inferred.','Target structural access audit is not full semantic proof or physical execution.'])
 (ROOT/'reviews/v5/expansion/legacy_advanced_mmio/exports.json').write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=out['passed'],commands=len(rows),errors=errors,failed=[r['label'] for r in rows if not r['passed']]));return int(not out['passed'])
if __name__=='__main__':raise SystemExit(main())
