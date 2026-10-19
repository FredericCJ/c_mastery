"""Real cryptographic primitive adapter, original synthetic image/key policy."""
from pathlib import Path
import subprocess,json,hashlib,datetime
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/firmware_lifecycle/targets';ref=ROOT/'assessor/firmware_lifecycle/targets';tool=Path.home()/'cmastery-b06-tools/usr';build=ROOT/'build/b06-signature';build.mkdir(parents=True,exist_ok=True);rows=[]
 for cc,tag,flags in [('gcc','debug',['-O0']),('gcc','release',['-O3']),('clang','sanitize',['-O1','-g','-fsanitize=address,undefined'])]:
  for m in [0,1,2,'stub']:
   exe=build/f'{tag}-{m}';src=lab/'signature_adapter.c' if m=='stub' else ref/'signature_adapter.c';cmd=[cc,'-std=c17',*flags,'-I',lab,'-I',tool/'include','-I',tool/'include/x86_64-linux-gnu',src,ref/'lifecycle_extension.c',ref/'image_transfer.c',lab/'signature_adapter_test.c','-L',tool/'lib/x86_64-linux-gnu','-Wl,-rpath,'+str(tool/'lib/x86_64-linux-gnu'),'-lcrypto','-o',exe];
   if m!='stub':cmd+=['-DSIGN_MUTANT='+str(m)]
   p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True);rows.append(dict(label=f'{tag} build {m}',command=list(map(str,cmd)),returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=p.returncode==0))
   if p.returncode==0:
    p=subprocess.run([str(exe)],capture_output=True,text=True,timeout=30);rows.append(dict(label=f'{tag} execution {m}',command=[str(exe)],returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=(p.returncode==0 and 'LCE_SIGNATURE_PASS' in p.stdout) if m==0 else (p.returncode==1 and 'LCE_SIGNATURE_CHECK' in p.stderr)))
 files=[Path(__file__),ref/'signature_adapter.c',ref/'lifecycle_extension.c',ref/'image_transfer.c',lab/'image_transfer.h',lab/'IMAGE_TRANSFER.md',*[lab/n for n in ['signature_adapter.h','signature_adapter.c','signature_adapter_test.c','lifecycle_extension.h']]];report=dict(passed=all(x['passed'] for x in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),source_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limits=['OpenSSL is the explicitly permitted external cryptographic primitive; image parsing/key policy are original first-principles code.','Synthetic public teaching seeds only; no production key or firmware target.','Signing and verification share the same provider, not independent cryptographic algorithm validation or secure-boot certification.','Hosted adapter execution does not establish freestanding firmware dependency closure or board boot behavior.']);(ROOT/'research/v5/expansion/b06/signature_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'passed':report['passed'],'checks':len(rows),'failed':[r['label'] for r in rows if not r['passed']]}));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
