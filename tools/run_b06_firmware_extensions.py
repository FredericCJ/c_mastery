"""Bounded flash cut and trusted-boundary models; not hardware validation."""
from pathlib import Path
import subprocess,json,hashlib,datetime
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/firmware_lifecycle/targets';ref=ROOT/'assessor/firmware_lifecycle/targets/lifecycle_extension.c';build=ROOT/'build/b06-firmware-extension';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(cmd,label,negative=False):
  r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=45);out=r.stdout+r.stderr;ok=r.returncode==0 if not negative else r.returncode!=0 and 'B06_FW_CHECK:' in out;rows.append(dict(label=label,command=list(map(str,cmd)),returncode=r.returncode,stdout=out,passed=ok));print(label,ok,flush=True);return ok
 for cc,flags,tag in [('gcc',['-O0','-g'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]:
  for m in [0,1,2,3,'stub']:
   exe=build/(tag+'-'+str(m));src=lab/'lifecycle_extension.c' if m=='stub' else ref
   cmd=[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',*flags,'-I'+str(lab),src,lab/'lifecycle_extension_test.c','-o',exe]
   if m!='stub':cmd+=['-DLCE_MUTANT='+str(m)]
   if run(cmd,tag+' build '+str(m)):run([exe],tag+' execute '+str(m),m!=0)
 files=[Path(__file__),ref,*lab.glob('lifecycle_extension*')];report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),source_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['294 specified partial-program schedules are a model, not physical power-cut measurements.','Injected verifier tests exact immutable image/key binding, not cryptographic soundness; target cryptographic integration remains separate.','Serialized joint-generation and nested-fault state machines do not establish cache coherence, DMA ordering, NMI atomicity or two-core hardware behavior.','Finite generation exhaustion fails closed; recovery requires a separately trusted epoch transition.']);(ROOT/'research/v5/expansion/b06/firmware_extension_validation.json').write_text(json.dumps(report,indent=2)+'\n');return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
