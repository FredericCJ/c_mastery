"""Actual local Linux io_uring experiment; no external target or host changes."""
from pathlib import Path
import subprocess,json,hashlib,datetime,platform
ROOT=Path(__file__).resolve().parents[1]
def main():
 build=ROOT/'build/b06-io-completion';build.mkdir(parents=True,exist_ok=True);lab=ROOT/'labs/linux_io_depth/targets';ref=ROOT/'assessor/linux_io_depth/targets/raw_ring.c';rows=[]
 def run(cmd,label,expected=0,marker=None):
  try:r=subprocess.run(list(map(str,cmd)),cwd=ROOT,text=True,capture_output=True,timeout=45);out=r.stdout+r.stderr;code=r.returncode
  except subprocess.TimeoutExpired as e:out=str(e);code=124
  passed=(code==expected if expected==0 else code!=0) and (marker is None or marker in out)
  rows.append(dict(label=label,command=list(map(str,cmd)),returncode=code,stdout=out,passed=passed));print(label,passed,flush=True);return passed
 for cc,flags,tag in [('gcc',['-O0','-g'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'],'clang-sanitize')]:
  for mutant in [0,1,2,'stub']:
   exe=build/(tag+'-'+str(mutant));source=lab/'raw_ring.c' if mutant=='stub' else ref
   cmd=[cc,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(lab),str(source),str(lab/'completion_probe.c'),'-o',str(exe)]
   if mutant!='stub':cmd+=['-DCM_MUTANT='+str(mutant)]
   if run(cmd,tag+' build '+str(mutant)):run([exe],tag+' execute '+str(mutant),0 if mutant==0 else 1,None if mutant==0 else 'B06_IO_CHECK:')
 files=[Path(__file__),lab/'raw_ring.h',lab/'raw_ring.c',lab/'completion_probe.c',ref]
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),system=platform.uname()._asdict(),source_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['Single issuer no SQPOLL; no multi-producer adapter claim.','EALREADY is specified and separately modeled but not forced in this schedule.','No physical durability or zero-copy claim.','Finite actual schedules are not exhaustive race or lifetime proof.'])
 (ROOT/'research/v5/expansion/b06/io_completion_validation.json').write_text(json.dumps(report,indent=2)+'\n');return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
