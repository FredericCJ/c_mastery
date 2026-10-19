"""Actual SSE2 exact dot, fixed-tree pthread execution and accuracy policies."""
from pathlib import Path
import subprocess,json,hashlib,datetime,platform
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/numeric_simd/targets';ref=ROOT/'assessor/numeric_simd/targets';build=ROOT/'build/b06-numeric-transfer';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(label,cmd,negative=False,marker=None):
  p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=120);text=p.stdout+p.stderr;ok=(p.returncode!=0 if negative else p.returncode==0) and (not marker or marker in text);rows.append(dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=text,passed=ok));print(label,ok,flush=True);return ok
 for cc,tag,flags in [('gcc','debug',['-O0']),('gcc','release',['-O3']),('clang','sanitize',['-O1','-g','-fsanitize=address,undefined'])]:
  for m in [0,1,2,'stub']:
   exe=build/f'{cc}-{tag}-{m}';src=lab/'numeric_transfer.c' if m=='stub' else ref/'numeric_transfer.c';cmd=[cc,'-std=c17',*flags,'-pthread','-ffp-contract=off','-fno-fast-math','-I',lab,src,ref/'numeric_extension.c',lab/'numeric_transfer_test.c','-lm','-o',exe]
   if m!='stub':cmd+=['-DNT_MUTANT='+str(m)]
   if run(f'{tag} build {m}',cmd):run(f'{tag} execute {m}',[exe],m!=0,'NT_CHECK' if m else 'NT_PASS')
  if tag=='release':
   p=subprocess.run(['objdump','-d',str(build/f'{cc}-{tag}-0')],capture_output=True,text=True);text=p.stdout;required=['pmullw','pmulhw','paddq'];rows.append(dict(label='actual SSE2 generated instruction audit',passed=p.returncode==0 and all(x in text for x in required),command=p.args,required={x:text.count(x) for x in required},disassembly_sha256=hashlib.sha256(text.encode()).hexdigest()));(build/'instructions.txt').write_text(text)
 files=[Path(__file__),ref/'numeric_transfer.c',ref/'numeric_extension.c',*[lab/n for n in ['numeric_transfer.h','numeric_transfer.c','numeric_transfer_test.c','numeric_extension.h']]];report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),system=platform.platform(),source_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['SSE2 is an x86-64 baseline assumption, not runtime AVX dispatch; P6 separately tests AVX OS feature dispatch.','Fixed top-level two-worker tree includes thread creation/join cost in raw timings; no speedup claim.','No NaN payload or infinity distance semantics; finite binary64 only.','Finite fixtures and host-thread runs are author evidence, not proof over arbitrary scheduling or learner mastery.']);(ROOT/'research/v5/expansion/b06/numeric_transfer_validation.json').write_text(json.dumps(report,indent=2)+'\n');return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
