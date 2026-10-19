"""Source-bound numerical contracts, independent exact oracles and raw timings."""
from pathlib import Path
import subprocess,json,hashlib,datetime,ctypes as C,platform
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/numeric_simd/targets';ref=ROOT/'assessor/numeric_simd/targets/numeric_extension.c';build=ROOT/'build/b06-numeric-extension';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(cmd,label,negative=False,marker=None):
  r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=90);out=r.stdout+r.stderr;ok=(r.returncode==0 if not negative else r.returncode!=0) and (marker is None or marker in out);rows.append(dict(label=label,command=list(map(str,cmd)),returncode=r.returncode,stdout=out,passed=ok));print(label,ok,flush=True);return ok
 def check(label,ok,detail):rows.append(dict(label=label,passed=bool(ok),details=detail));print(label,bool(ok),flush=True)
 for cc,flags,tag in [('gcc',['-O0','-g'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]:
  for m in [0,1,2,3,'stub']:
   exe=build/(tag+'-'+str(m));src=lab/'numeric_extension.c' if m=='stub' else ref;cmd=[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wno-unknown-pragmas','-Wno-misleading-indentation','-frounding-math','-ffp-contract=off','-fno-fast-math',*flags,'-I'+str(lab),src,lab/'numeric_extension_test.c','-lm','-o',exe]
   if m!='stub':cmd+=['-DND_MUTANT='+str(m)]
   if run(cmd,tag+' build '+str(m)):run([exe],tag+' execute '+str(m),m!=0,'B06_NUM_CHECK:' if m!=0 else 'B06_NUM_PASS')
 so=build/'oracle.so'
 if run(['gcc','-shared','-fPIC','-std=c17','-O2','-fno-fast-math','-ffp-contract=off','-I'+str(lab),ref,'-o',so],'independent oracle library build'):
  dll=C.CDLL(str(so));dll.nd_q312.argtypes=[C.c_int16,C.c_int16,C.c_uint,C.POINTER(C.c_int16)];dll.nd_matrix.argtypes=[C.POINTER(C.c_double),C.POINTER(C.c_double),C.c_size_t,C.c_size_t,C.c_size_t,C.POINTER(C.c_double),C.c_size_t,C.c_uint]
  vals=[-32768,-32767,-16385,-8193,-4097,-4096,-2049,-3,-1,0,1,3,2049,4096,4097,8193,16385,32767];ok=True;count=0
  for a in vals:
   for b in vals:
    p=a*b;q=abs(p)//4096*(1 if p>=0 else -1)
    for mode in range(3):
     out=C.c_int16(91);accepted=dll.nd_q312(a,b,mode,C.byref(out));want=max(-32768,min(32767,q)) if mode==1 else ((q+32768)%65536)-32768;error=mode==2 and not -32768<=q<=32767;ok &= (accepted==0 and out.value==91) if error else (accepted==1 and out.value==want);count+=1
  check('Python integer Q3.12 oracle',ok,dict(cases=count))
  ok=True;count=0
  for m in range(1,10):
   for n in range(1,12):
    for k in range(1,14):
     av=[i%15-7 for i in range(m*k)];bv=[(i*3)%15-7 for i in range(k*n)];a=(C.c_double*len(av))(*av);b=(C.c_double*len(bv))(*bv);o=(C.c_double*(m*n))();want=[sum(av[i*k+h]*bv[h*n+j] for h in range(k)) for i in range(m) for j in range(n)]
     for mode in range(3):ok &= dll.nd_matrix(a,b,m,n,k,o,m*n,mode)==1 and list(o)==want;count+=1
  check('Python exact integer matrix oracle',ok,dict(cases=count))
 for cc in ['gcc','clang']:
  exe=build/(cc+'-measure');asm=build/(cc+'-numeric.s')
  cmd=[cc,'-std=c17','-O3','-fno-fast-math','-ffp-contract=off','-I'+str(lab),ref]
  if run(cmd+[lab/'numeric_extension_measure.c','-o',exe],cc+' measurement build'):run([exe],cc+' raw216samples including transpose/staging',marker='size,mode')
  run(cmd+['-S','-o',asm],cc+' generated assembly');rows[-1]['assembly_sha256']=hashlib.sha256(asm.read_bytes()).hexdigest() if asm.exists() else None
 files=[Path(__file__),ref,*lab.glob('numeric_extension*')];report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),system=platform.uname()._asdict(),source_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['Raw timings include full call/transpose and bound checks; no universal speedup or physical machine isolation claim.','8MiB eviction sweep is an explicit changed working-set experiment, not proof every cache level was cold.','FP behavior is selected binary64 compiler/fenv/x86 MXCSR evidence, not an ISO-required format.','Pairwise tree is a fixed reproducible serial tree that may be scheduled in parallel only with identical joins; no parallel throughput claim here.']);(ROOT/'research/v5/expansion/b06/numeric_extension_validation.json').write_text(json.dumps(report,indent=2)+'\n');return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
