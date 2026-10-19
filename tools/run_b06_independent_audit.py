"""Withheld new-target reference, semantic counterexamples and bounded proof."""
from pathlib import Path
import subprocess,json,hashlib,datetime
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/verification_fuzzing/independent';ref=ROOT/'assessor/verification_fuzzing/independent';build=ROOT/'build/b06-independent-audit';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(label,cmd,expected=0,marker=None,timeout=120):
  p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=timeout);rows.append(dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=p.returncode==expected and (marker is None or marker in p.stdout+p.stderr)));return p
 for cc in ['gcc','clang']:
  run(cc+' identity',[cc,'--version'])
  for m in range(5):
   so=build/(cc+str(m)+'.so');src=lab/'audit_target.c' if m==4 else ref/'repair.c'
   if run(cc+str(m)+' build',[cc,'-std=c17','-O2','-shared','-fPIC','-I',lab,'-DAUDIT_MUTANT='+str(m),src,'-o',so]).returncode==0:run(cc+str(m)+' independent semantic oracle',['python3',ref/'oracle.py',so],expected=0 if m==0 else 1,marker='B06_AUDIT_'+('PASS' if m==0 else 'FAIL'))
 cbmc=Path.home()/'cmastery-b06-tools/usr/bin/cbmc';run('solver identity',[cbmc,'--version']);run('bounded22-byte safety and rollback proof',[cbmc,ref/'proof.c',ref/'repair.c','-I',lab,'--function','main','--unwind','24','--unwinding-assertions','--bounds-check','--pointer-check','--signed-overflow-check','--unsigned-overflow-check'],marker='VERIFICATION SUCCESSFUL');run('rollback mutant counterexample',[cbmc,ref/'proof.c',ref/'repair.c','-I',lab,'-DAUDIT_MUTANT=3','--function','main','--unwind','24','--unwinding-assertions','--bounds-check','--pointer-check'],expected=10,marker='VERIFICATION FAILED')
 files=[Path(__file__),*[p for p in lab.iterdir() if p.is_file()],*[p for p in ref.iterdir() if p.is_file()]];report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['22-byte bounded proof of safety/rollback, not full semantic acceptance or arbitrary packet lengths','Independent generated packet oracle plus10000 random bytes are finite tests; three semantic mutants and original target give causal negative controls','Author reference/method-selection evidence is not the unaided learner attempt or seven-day elapsed-time evidence']);(ROOT/'research/v5/expansion/b06/independent_audit_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
