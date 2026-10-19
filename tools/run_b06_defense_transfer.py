"""Original-target changed-width/borrow/content-scan evidence, not constant-time proof."""
from pathlib import Path
import argparse,subprocess,json,hashlib,datetime,ctypes as C,re
ROOT=Path(__file__).resolve().parents[1]
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--rustc',default='rustc');a=ap.parse_args();lab=ROOT/'labs/memory_defense/targets';ref=ROOT/'assessor/memory_defense/targets/defense_transfer.c';tool=Path.home()/'cmastery-b06-tools/usr';w=ROOT/'build/b06-defense-transfer';w.mkdir(parents=True,exist_ok=True);rows=[]
 def run(label,cmd,code=0,marker=None):
  try:p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=90);row=dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=p.returncode==code and (not marker or marker in p.stdout+p.stderr))
  except OSError as e:row=dict(label=label,command=list(map(str,cmd)),passed=False,error=str(e))
  rows.append(row);print(label,row['passed'],flush=True);return row['passed']
 for cc,tag,flags in [('gcc','debug',['-O0']),('gcc','release',['-O3']),('clang','sanitize',['-O1','-g','-fsanitize=address,undefined'])]:
  for m in [0,1,2,3,'stub']:
   exe=w/f'{tag}-{m}';src=lab/'defense_transfer.c' if m=='stub' else ref;cmd=[cc,'-std=c17',*flags,'-I',lab,src,lab/'defense_transfer_test.c','-o',exe]
   if m!='stub':cmd+=['-DDT_MUTANT='+str(m)]
   if run(f'{tag} build {m}',cmd):run(f'{tag} execution {m}',[exe],0 if m==0 else 1,'DT_PASS' if m==0 else 'DT_CHECK')
 for cc in ['gcc','clang']:
  for vector in [True,False]:
   exe=w/f'{cc}-{vector}-measure';flags=[] if vector else ['-fno-tree-vectorize'] if cc=='gcc' else ['-fno-vectorize','-fno-slp-vectorize'];cmd=[cc,'-std=c17','-O3',*flags,'-I',lab,'-I',tool/'include','-I',tool/'include/x86_64-linux-gnu',ref,lab/'defense_transfer_measure.c','-L',tool/'lib/x86_64-linux-gnu','-Wl,-rpath,'+str(tool/'lib/x86_64-linux-gnu'),'-lcrypto','-o',exe]
   if run(f'{cc} vector={vector} measurement build',cmd):
    run(f'{cc} vector={vector} raw72samples',[exe],0,'DT_TIMING_DONE');p=subprocess.run(['objdump','-d','--disassemble=dt_equal',str(exe)],capture_output=True,text=True);rows.append(dict(label=f'{cc} vector={vector} generated code',command=p.args,passed=p.returncode==0 and '<dt_equal>:' in p.stdout,disassembly=p.stdout,branches=[x.strip() for x in p.stdout.splitlines() if re.search(r'\s+j[a-z]+\s',x)],scope='Actual instructions retained for causal branch/dataflow review; no automatic constant-time certification'))
 run('Rust compiler identity',[a.rustc,'--version']);exe=w/'borrow-positive'
 if run('safe borrow positive compile',[a.rustc,'--edition=2021',lab/'borrow_positive.rs','-o',exe]):run('safe borrow positive execute',[exe],0,'DT_RUST_PASS')
 run('safe borrow conflict diagnostic',[a.rustc,'--edition=2021',lab/'borrow_negative.rs','-o',w/'borrow-negative'],1,'E0502')
 files=[Path(__file__),ref,*[lab/n for n in ['defense_transfer.h','defense_transfer.c','defense_transfer_test.c','defense_transfer_measure.c','borrow_positive.rs','borrow_negative.rs']]];report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),source_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['C view counterexample uses valid live synthetic arrays, no undefined dangling access.','Rust comparison concerns safe language borrow checking; unsafe/FFI contracts require separate review.','Exact algorithmic scan count and timings do not prove constant-time machine behavior; full generated-code/dataflow review required.','CRYPTO_memcmp is an external comparison only; public length policy leaks lengths by design.']);(ROOT/'research/v5/expansion/b06/defense_transfer_validation.json').write_text(json.dumps(report,indent=2)+'\n');return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
