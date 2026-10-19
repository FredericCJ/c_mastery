"""Changed-condition author evidence; bounded solver/fuzzer results stay distinct."""
from pathlib import Path
import subprocess,json,hashlib,datetime,platform,sys
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/verification_fuzzing/targets';ref=ROOT/'assessor/verification_fuzzing/targets/transfer.c';build=ROOT/'build/b06-verification-transfer';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(label,cmd,code=0,marker=None,timeout=120):
  try:
   p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=timeout);ok=p.returncode==code and (not marker or marker in p.stdout+p.stderr);row=dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=ok)
  except (OSError,subprocess.TimeoutExpired) as e:row=dict(label=label,command=list(map(str,cmd)),passed=False,error=str(e))
  rows.append(row);print(label,row['passed'],flush=True);return row['passed']
 for name,cc,flags in [('gcc-debug','gcc',['-O0']),('gcc-release','gcc',['-O3']),('clang-sanitize','clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
  for mutant in [0,1,2,3,4,5,'stub']:
   exe=build/f'{name}-{mutant}';src=lab/'transfer.c' if mutant=='stub' else ref
   if run(f'{name} build {mutant}',[cc,'-std=c17',*flags,'-I',lab,src,lab/'transfer_test.c',*([f'-DVT_MUTANT={mutant}'] if mutant!='stub' else []),'-o',exe]):run(f'{name} semantic {mutant}',[exe],0 if mutant==0 else 1,'VT_PASS' if mutant==0 else 'VT_CHECK')
 cbmc=Path.home()/'cmastery-b06-tools/usr/bin/cbmc'
 run('independent Euclidean multiplication threshold lemma',[Path.home()/'cmastery-b06-tools/usr/bin/z3',lab/'multiplication_lemma.smt2'],0,'unsat',30)
 for name,flags,expect in [('multiply conversion',[],0),('multiply mutant',['-DVT_MUTANT=1'],10),('descending record partition',['-DCHECK_SEARCH'],0),('descending equality mutant',['-DCHECK_SEARCH','-DVT_MUTANT=2'],10)]:
  smt=build/(name.replace(' ','-')+'.smt2');proof=smt.with_suffix('.proof.smt2')
  if run('CBMC generate '+name,[cbmc,lab/'transfer_cbmc.c',ref,'-I',lab,'--bounds-check','--pointer-check','--signed-overflow-check','--unwinding-assertions','--unwind','7',*flags,'--smt2','--outfile',smt],0,None,60):
   lines=smt.read_text().splitlines();queries=[x for x in lines if x.startswith('(get-value')];proof.write_text('\n'.join(x for x in lines if not x.startswith('(get-value'))+'\n');rows[-1]['smt_sha256']=hashlib.sha256(smt.read_bytes()).hexdigest();rows[-1]['proof_sha256']=hashlib.sha256(proof.read_bytes()).hexdigest();rows[-1]['transformation']='Removed only post-check-sat get-value queries, because unsat has no model; no declaration, assumption, property or constraint removed.';rows[-1]['removed_model_queries']=len(queries)
   run('Z3 C obligations '+name,[Path.home()/'cmastery-b06-tools/usr/bin/z3',proof],0,'unsat' if expect==0 else 'sat',90)
   rows[-1]['passed'] = rows[-1]['passed'] and rows[-1].get('stdout','').strip()==('unsat' if expect==0 else 'sat')
 corpus=build/'corpus';corpus.mkdir(exist_ok=True);(corpus/'valid-session').write_bytes(bytes([1,4,2,3,4,7,2,0,1,2,0,0,2]));(corpus/'split-session').write_bytes(bytes([1,1,2,1,2,3,4,1,1,7,2]))
 exe=build/'fuzz';
 if run('stateful fuzzer build',['clang','-std=c17','-O1','-g','-fsanitize=fuzzer,address,undefined','-I',lab,ref,lab/'session_fuzz.c','-o',exe]):run('stateful20000sessions',[exe,corpus,'-seed=6107','-runs=20000','-max_len=128'],0,'Done 20000 runs',180)
 run('finite atomic reads-from graph model',[sys.executable,ROOT/'assessor/verification_fuzzing/targets/publication_model.py'],0,'"passed": true')
 exe=build/'publication'
 if run('publication build',['gcc','-std=c17','-O2','-pthread',lab/'publication_litmus.c','-o',exe]):run('two actor publication',[exe],0,'VT_PUBLICATION_PASS')
 files=[Path(__file__),ref,ROOT/'assessor/verification_fuzzing/targets/publication_model.py',lab/'TRANSFER.md',*[lab/n for n in ['transfer.h','transfer.c','transfer_test.c','transfer_cbmc.c','multiplication_lemma.smt2','session_fuzz.c','publication_litmus.c']]]
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),system=platform.platform(),source_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['C obligations prove the exact division threshold, low32 product and unchanged failure; separate Euclidean integer lemma proves threshold equivalent to full mathematical product admission. int64 conversion unrestricted; search assumes descending records <=6.','Fuzzing is bounded20000sessions, not exhaustiveness.','Weak-order counterexample is an explicit allowed-observation model, not an observed hardware failure or complete C11 axiomatic proof. Release/acquire thread checks cover selected executions.','No independent learner or external proof audit.'])
 (ROOT/'research/v5/expansion/b06/verification_transfer_validation.json').write_text(json.dumps(report,indent=2)+'\n');return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
