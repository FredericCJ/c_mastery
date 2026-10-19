"""Image-program completion and finite epoch recovery models with negative controls."""
from pathlib import Path
import subprocess,tempfile,json,hashlib,datetime
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/firmware_lifecycle/targets';ref=ROOT/'assessor/firmware_lifecycle/targets';rows=[]
 with tempfile.TemporaryDirectory(prefix='b06-image-') as d:
  for cc,flags,tag in [('gcc',['-O0'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O1','-fsanitize=address,undefined'],'clang-sanitize')]:
   for m in [0,1,2,3,'stub']:
    exe=Path(d)/(tag+'-'+str(m));cmd=[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',*flags,'-I'+str(lab),str((lab if m=='stub' else ref)/'image_transfer.c'),str(ref/'lifecycle_extension.c'),str(lab/'image_transfer_test.c'),'-o',str(exe)]+([] if m=='stub' else ['-DFI_MUTANT='+str(m)]);p=subprocess.run(cmd,capture_output=True,text=True);rows.append(dict(label=tag+' build '+str(m),command=cmd,returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=p.returncode==0))
    if p.returncode:continue
    p=subprocess.run([str(exe)],capture_output=True,text=True,timeout=30);ok=p.returncode==0 and 'B06_IMAGE_PASS' in p.stdout if m==0 else p.returncode==1 and 'B06_IMAGE_ASSERT' in p.stderr;rows.append(dict(label=tag+' execute '+str(m),command=[str(exe)],returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=ok))
 files=[Path(__file__),*lab.glob('image_transfer*'),lab/'IMAGE_TRANSFER.md',ref/'image_transfer.c',ref/'lifecycle_extension.c',lab/'lifecycle_extension.h',ref/'IMAGE_TRANSFER_ANALYSIS.md'];report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['Digital flash program-unit completion model; no physical erase/program timing or controller register claims','CRC binds accidental corruption to image metadata, not adversarial authenticity; actual signature adapter is separate required composition','Trusted recovery authorization and atomic policy-store callback are explicit dependencies; six-field failure model does not establish a particular flash controller implementation','No independent learner, external review or H745/F439 physical validation']);(ROOT/'research/v5/expansion/b06/image_transfer_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
