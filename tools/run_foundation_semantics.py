"""Execute defined repairs and compile-only diagnostics under separate categories."""
from pathlib import Path
import argparse,datetime,json,os,shutil,subprocess,sys
from run_foundation_entry import sha,PROFILES
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/foundation_semantics';REF=ROOT/'assessor/foundation_semantics'
REPORT='reviews/v5/expansion/legacy_foundation_semantics/current_validation.json'
MARKERS=['FS_ARRAY_PASS metadata_cases=117 typed_rows actual_stride failure_atomic','FS_PATH_PASS both_branches invalid_preserved no_uninitialized_execution','FS_SEQUENCE_PASS observed_call_order=','permitted=12,21 comma=2,3 short_circuit=0','FS_PASS defined_repairs_and_transfer']
MUTANTS={1:'fs_index(indices[i], 3) == expected[i]',2:'fs_matrix(matrix, 2, &out) == FS_OK && out == 66',3:'fs_branch(0, &out) == FS_OK && out == 11',4:'fs_comma(&a, &b) == FS_OK && a == 2 && b == 3',5:'fs_calls(&result) == FS_OK && result.sum == 6'}
STUB=MUTANTS[1]
def inputs():return sorted({Path(__file__),ROOT/'tools/run_foundation_entry.py',ROOT/'tools/verify_language_fixtures.py',*[p for p in (ROOT/'fixtures/language').rglob('*.json')],*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]})
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args();w=ROOT/'build/foundation-semantics'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(label,cmd,expected=0,markers=(),category='defined-execution'):
  cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log')
  with log.open('w') as f:
   try:rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=120,env=env).returncode
   except (OSError,subprocess.TimeoutExpired) as e:f.write(str(e));rc=None
  t=log.read_text(errors='replace');ok=rc==expected and all(m in t for m in markers) and not any(m in t for m in ['runtime error:','ERROR: AddressSanitizer'])
  rows.append(dict(label=label,command=cmd,category=category,returncode=rc,expected=expected,markers=list(markers),passed=ok,raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,ok,flush=True);(w/'commands.json').write_text(json.dumps(rows,indent=2)+'\n');return ok
 tools=['gcc','clang','readelf','nm']
 for t in tools:run(t+' version',[t,'--version'],category='tool-identity')
 run('environment',['uname','-a'],category='tool-identity');run('original synthetic fixtures',[sys.executable,ROOT/'tools/verify_language_fixtures.py'],category='independent-fixture')
 for cc,opt,tag in PROFILES:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie','-I'+str(LAB)]+(['-O1','-g'] if 'asan' in tag else [])
  for v in [0,*MUTANTS,'stub']:
   exe=w/(tag+'-'+str(v));src=LAB/'learner.c' if v=='stub' else REF/'semantics.c'
   if run(tag+' '+str(v)+' compile',[cc,*flags,'-DFS_MUTANT='+str(0 if v=='stub' else v),src,LAB/'tests.c','-o',exe],category='defined-build'):
    run(tag+' '+str(v)+' execute',[exe],int(v!=0),MARKERS if v==0 else [STUB if v=='stub' else MUTANTS[v]])
  if tag in ['gcc-debug','clang-debug']:
   run(tag+' dynamic dependencies',['readelf','-dW',w/(tag+'-0')],markers=['libc.so.6'],category='hosted-dependency-audit')
   run(tag+' undefined symbols',['nm','-u',w/(tag+'-0')],markers=['__libc_start_main'],category='hosted-dependency-audit')
 for cc in ['gcc','clang']:
  for name in ['wrong_row','wrong_matrix']:
   run(cc+' '+name+' constraint',[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-pedantic-errors','-Werror','-I'+str(LAB),'-c',REF/(name+'.c'),'-o',w/(cc+'-'+name+'.o')],1,['incompatible'],'negative-translation')
  for opt in ['0','3']:
   run(cc+' O'+opt+' analysis-only',[cc,'-std=c17','-O'+opt,'-c',REF/'analysis_only.c','-o',w/(cc+'-analysis-'+opt+'.o')],category='analysis-only-translation')
  for ref in [False,True]:run(cc+' public '+str(ref),[sys.executable,LAB/'run.py','--cc',cc,'--build',w/(cc+'-public-'+str(ref)),*(['--source',REF/'semantics.c'] if ref else [])],int(not ref),MARKERS if ref else [STUB])
 rows.append(dict(label='source stability',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 d=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tool_sha256={t:sha(Path(shutil.which(t))) for t in tools},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Six hosted C17 profiles; standard CRT/libc permitted under profile A and actual dynamic/undefined dependencies inspected.','Undefined off-by-one, uninitialized and conflicting-expression specimens are compile-only, never linked/executed or assigned golden outputs.','Five wrong-but-defined compiled mutants and a compiling unfinished learner stub per profile; two incompatible-type negative translations per compiler are a distinct category.','117 finite metadata cases and explicit row/branch/sequence tests supplement manual object/initialization/event-graph arguments; no arbitrary-pointer or all-ordering proof from execution.','Author validation is not independent review, learner performance, seven-day assessment or physical observation; published C17 access remains separate.'])
 out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
