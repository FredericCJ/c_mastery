"""Original local synthetic security targets; CFI mismatches run instrumented only."""
from pathlib import Path
import subprocess,json,hashlib,datetime
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/memory_defense/targets';ref=ROOT/'assessor/memory_defense/targets/security_extension.c';build=ROOT/'build/b06-security-extension';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(cmd,label,negative=False,marker=None):
  r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30);out=r.stdout+r.stderr;ok=(r.returncode==0 if not negative else r.returncode!=0) and (marker is None or marker in out);rows.append(dict(label=label,command=list(map(str,cmd)),returncode=r.returncode,stdout=out,passed=ok));print(label,ok,flush=True);return ok
 for cc,flags,tag in [('gcc',['-O0','-g'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]:
  for m in [0,1,2,'stub']:
   exe=build/(tag+'-'+str(m));src=lab/'security_extension.c' if m=='stub' else ref;cmd=[cc,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(lab),src,lab/'security_extension_test.c','-o',exe]
   if m!='stub':cmd+=['-DMDE_MUTANT='+str(m)]
   if run(cmd,tag+' build '+str(m)):run([exe],tag+' execute '+str(m),m!=0,'B06_SECURITY_CHECK:' if m!=0 else None)
 exe=build/'cfi';linker=Path.home()/'cmastery-b06-tools/usr/bin/ld.lld-21'
 if run(['clang','-std=c17','-O2','-g','-flto','-fvisibility=hidden','-fsanitize=cfi-icall','-fno-sanitize-trap=cfi','-fuse-ld='+str(linker),lab/'dispatch_target.c',ROOT/'assessor/memory_defense/targets/dispatch_repair.c','-Wl,-z,relro,-z,now','-o',exe],'CFI build'):
  run([exe,'same-type'],'CFI does not authorize valid-type target',marker='sentinel=77');run([exe,'repaired'],'explicit authority repair',marker='DENIED sentinel=91');run([exe,'mismatch'],'CFI blocks incompatible indirect call',True,'control flow integrity check')
  for index,value in [(0,92),(1,77),(2,0)]:
   run([exe,'index',str(index)],'CFI read-only table corrupted index '+str(index),marker=f'index={index} sentinel={value} readonly=1')
   run([exe,'index-repaired',str(index)],'CFI explicit indexed authority '+str(index),marker='index=0 sentinel=92 readonly=1' if index==0 else 'INDEX_DENIED sentinel=91 readonly=1')
  run([exe,'index','3'],'out-of-range index rejected without access',marker='INDEX_DENIED sentinel=91 readonly=1')
 plain=build/'table-plain'
 if run(['gcc','-std=c17','-O2',lab/'dispatch_target.c',ROOT/'assessor/memory_defense/targets/dispatch_repair.c','-Wl,-z,relro,-z,now','-o',plain],'plain indexed target build'):
  for index,value in [(0,92),(1,77),(2,0)]:run([plain,'index',str(index)],'plain indexed callback '+str(index),marker=f'index={index} sentinel={value} readonly=1')
 stub=build/'table-stub'
 if run(['gcc','-std=c17','-O2',lab/'dispatch_target.c',lab/'dispatch_repair.c','-Wl,-z,relro,-z,now','-o',stub],'indexed authorization stub build'):
  # The public target exits normally: reject its claimed repair by the observed
  # unauthorized sentinel result, not by a fabricated process crash.
  run([stub,'index-repaired','1'],'authorization stub exposes original violation',marker='index=1 sentinel=77 readonly=1')

 files=[Path(__file__),ref,*lab.glob('security_extension*'),lab/'dispatch_target.c',lab/'dispatch_repair.c',ROOT/'assessor/memory_defense/targets/dispatch_repair.c',lab/'EXTENSIONS.md',ROOT/'assessor/memory_defense/targets/ANALYSIS.md'];report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),source_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['Only original local synthetic targets; no external targets, credentials or arbitrary-address write.','CFI type enforcement does not establish authorization or complete memory safety.','Serialized generation checks require live control storage and non-reused generation; no concurrent reclamation proof.','Binary escaping is a defined log encoding, not a universal context-sensitive sanitizer.']);(ROOT/'research/v5/expansion/b06/security_extension_validation.json').write_text(json.dumps(report,indent=2)+'\n');return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
