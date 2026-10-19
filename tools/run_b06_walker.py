"""Finite synthetic Sv39 walker gates, separate from real privileged guest evidence."""
from pathlib import Path
import subprocess,json,hashlib,datetime
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/architecture_ports/targets';ref=ROOT/'assessor/architecture_ports/targets/walker.c';build=ROOT/'build/b06-walker';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(label,cmd,expected=0,marker=None):
  p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=60);rows.append(dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=p.returncode==expected and (marker is None or marker in p.stdout+p.stderr)));return p
 for cc,flags in [('gcc',['-O0','-g']),('gcc',['-O3']),('clang',['-O2','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
  run(cc+' identity',[cc,'--version'])
  for mutation in range(5):
   name=cc+str(len(rows));exe=build/name;src=lab/'walker.c' if mutation==4 else ref
   if run(name+' build',[cc,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I',lab,'-DPW_MUTANT='+str(mutation),src,lab/'walker_test.c','-o',exe]).returncode==0:run(name+' semantic execution',[exe],expected=0 if mutation==0 else 1,marker='B06_WALK_'+('PASS' if mutation==0 else 'FAIL'))
 files=[Path(__file__),ref,*[lab/n for n in ['walker.h','walker.c','walker_test.c','WALKER.md']]];report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['Finite immutable synthetic tables, single byte access, Svade-style A/D fault policy, no optional PTE extensions','No model of TLB caches, concurrent PTE writes, hardware A/D updates, PMP/PMA data accesses or Arm descriptor permissions','Actual selected guest translations are separate source-bound ABI receipt observations, not inferred from hosted model success','Full Arm architecture manual source closure remains separate from inspected official introduction and actual guest behavior']);(ROOT/'research/v5/expansion/b06/walker_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
