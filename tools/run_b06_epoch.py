"""Finite epoch/generation semantic controls; never a replacement for kernel RCU."""
from pathlib import Path
import subprocess,tempfile,json,hashlib,datetime
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/kernel_lifecycle/guest';ref=ROOT/'assessor/kernel_lifecycle/guest';checks=[]
 files=[lab/'epoch.h',lab/'epoch.c',lab/'epoch_test.c',ref/'epoch.c',lab/'README.md',ref/'ANALYSIS.md',Path(__file__)]
 with tempfile.TemporaryDirectory(prefix='b06-epoch-') as d:
  for cc,flags in [('gcc',['-O0']),('gcc',['-O3']),('clang',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
   for name,src,defs in [('reference',ref/'epoch.c',[]),('wait_all',ref/'epoch.c',['-DKE_WAIT_ALL']),('stale_generation',ref/'epoch.c',['-DKE_STALE']),('learner_stub',lab/'epoch.c',[])]:
    exe=Path(d)/(cc+'-'+name);cmd=[cc,'-std=c17','-Wall','-Wextra','-Werror',*flags,*defs,'-I',str(lab),str(src),str(lab/'epoch_test.c'),'-o',str(exe)];p=subprocess.run(cmd,capture_output=True,text=True);checks.append(dict(label=cc+name+' build',command=cmd,returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=p.returncode==0))
    if p.returncode:continue
    p=subprocess.run([str(exe)],capture_output=True,text=True);good=p.returncode==0 and 'B06_EPOCH_PASS' in p.stdout if name=='reference' else p.returncode==1 and 'B06_EPOCH_ASSERT' in p.stderr;checks.append(dict(label=cc+name+' semantic',command=[str(exe)],returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=good))
 report=dict(passed=all(c['passed'] for c in checks),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=checks,limitations=['Serialized finite epoch model; not a C memory-model implementation or proof of Linux RCU internals','Reader epoch is distinct from a retained object reference','No external review, learner transfer, or physical observation']);(ROOT/'research/v5/expansion/b06/epoch_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(checks))));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
