"""Original local progress, scheduling and MPSC-to-io_uring composition."""
from pathlib import Path
import subprocess,tempfile,json,hashlib,datetime,platform
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/linux_io_depth/targets';ref=ROOT/'assessor/linux_io_depth/targets';rows=[]
 with tempfile.TemporaryDirectory(prefix='b06-io-transfer-') as d:
  for cc,flags,profile in [('gcc',['-O0'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer'],'clang-sanitize')]:
   for mutation in [0,1,2,3,4,'stub']:
    exe=Path(d)/(profile+'-'+str(mutation));cmd=[cc,'-std=c17','-Wall','-Wextra','-Werror','-pthread',*flags,'-I'+str(lab),str((lab if mutation=='stub' else ref)/'transfer.c'),str(ref/'raw_ring.c'),str(lab/'transfer_test.c'),'-o',str(exe)]+([] if mutation=='stub' else ['-DIT_MUTANT='+str(mutation)]);p=subprocess.run(cmd,capture_output=True,text=True);rows.append(dict(label=profile+' build '+str(mutation),command=cmd,returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=p.returncode==0))
    if p.returncode:continue
    try:p=subprocess.run([str(exe)],capture_output=True,text=True,timeout=40);ok=p.returncode==0 and 'B06_IO_TRANSFER_PASS' in p.stdout if mutation==0 else p.returncode==1 and 'B06_IO_TRANSFER_ASSERT' in p.stderr;rows.append(dict(label=profile+' run '+str(mutation),command=[str(exe)],returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=ok))
    except subprocess.TimeoutExpired as e:rows.append(dict(label=profile+' run '+str(mutation),passed=False,error=str(e)))
 files=[Path(__file__),lab/'transfer.h',lab/'transfer.c',lab/'transfer_test.c',lab/'TRANSFERS.md',ref/'transfer.c',ref/'TRANSFER_ANALYSIS.md',ref/'raw_ring.c',lab/'raw_ring.h'];report=dict(passed=all(v['passed'] for v in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),system=platform.uname()._asdict(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['EINTR/EAGAIN/error prefix schedule is injected at original callback boundary; actual pipe/readiness and kernel CQEs separately exercised','MPSC uses pthread mutex/conditions; no lock-free claim and thread cancellation prohibited','Measured response latency is an observation, not a real-time bound','No external review, independent learner or physical persistence claim']);(ROOT/'research/v5/expansion/b06/io_transfer_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[v for v in rows if not v['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
