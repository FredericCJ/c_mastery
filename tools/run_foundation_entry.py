"""Execute exact PR-T01-01 successor and compiled semantic rejection controls."""
from pathlib import Path
import argparse, datetime, hashlib, json, os, shutil, subprocess, sys
ROOT=Path(__file__).resolve().parents[1]
LAB=ROOT/'labs/foundation_entry'; REF=ROOT/'assessor/foundation_entry'
PROVIDER=ROOT/'assessor/foundations/foundation.c'
REPORT='reviews/v5/expansion/legacy_foundation_entry/current_validation.json'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
MUTANTS={1:'x.count == 8 && y.count == 8',2:'fe_two_pass(bad, 3, out, 3) == FE_RANGE',3:'pool.live == 0 && pool.releases == 1',4:'fe_maximum(NULL, 0, &out[0]) == FE_EMPTY',5:'fe_encode24(UINT32_C(0x1234ad), bytes, 4) == FE_OK',6:'fe_forbidden_clear((uint8_t)b, (uint8_t)m) == (present == 0)'}
MARKERS=['FE_TRACE_PASS normal8 empty2 bounded34 explicit_activation_correspondence','FE_PREFIX_PASS atomic_failure dynamic_rollback overlap17 maximum_empty','FE_VALUE_PASS BE24 masks65536 saturation_nonassociative unsigned_modulo','FE_ORACLE_PASS direct_integer_formula_cases=343','FE_PASS entry_algorithms_values']
STUB='fe_recursive(in, 3, &a, &x) == FE_OK'
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 return sorted({Path(__file__),PROVIDER,ROOT/'assessor/execution/support.c',ROOT/'labs/foundations/include/foundation.h',ROOT/'tools/verify_language_fixtures.py',*[p for p in (ROOT/'fixtures/language').rglob('*.json')],*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]})
def target_args(dest,cc,opt,stub=False):
 return [sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--source',LAB/'learner.c' if stub else REF/'entry.c','--foundation',PROVIDER,'--runtime',ROOT/'assessor/execution/support.c','--driver',REF/'target.c']
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args()
 w=ROOT/'build/foundation-entry'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True)
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};rows=[]
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(label,cmd,expected=0,markers=()):
  cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log')
  with log.open('w') as f:
   try:rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=120,env=env).returncode
   except (OSError,subprocess.TimeoutExpired) as e:f.write(str(e));rc=None
  t=log.read_text(errors='replace');ok=rc==expected and all(m in t for m in markers) and not any(m in t for m in ['runtime error:','ERROR: AddressSanitizer'])
  rows.append(dict(label=label,command=cmd,returncode=rc,expected=expected,markers=list(markers),passed=ok,raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)))
  print(label,ok,flush=True);(w/'commands.json').write_text(json.dumps(rows,indent=2)+'\n');return ok
 tools=['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']
 for cc in tools:run(cc+' version',[cc,'--version'])
 run('environment',['uname','-a'])
 run('original synthetic fixtures',[sys.executable,ROOT/'tools/verify_language_fixtures.py'])
 for cc,opt,tag in PROFILES:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie','-I'+str(LAB),'-I'+str(ROOT/'labs/foundations/include')]+(['-O1','-g'] if 'asan' in tag else [])
  for v in [0,*MUTANTS,'stub']:
   exe=w/(tag+'-'+str(v));src=LAB/'learner.c' if v=='stub' else REF/'entry.c'
   if run(tag+' '+str(v)+' compile',[cc,*flags,'-DFE_MUTANT='+str(0 if v=='stub' else v),src,PROVIDER,LAB/'tests.c','-o',exe]):
    run(tag+' '+str(v)+' execute',[exe],int(v!=0),MARKERS if v==0 else [STUB if v=='stub' else MUTANTS[v]])
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   tag=cc+'-O'+opt;dest=w/tag
   if run(tag+' build',target_args(dest,cc,opt),markers=['FE_TARGET_BUILD_PASS explicit_objects=5 no_archives']):
    run(tag+' execute',emulator(dest/'image.elf'),markers=['FE_TARGET_PASS all_functions explicit_prefix_provider recursion16'])
    run(tag+' resources',[sys.executable,REF/'resource_audit.py',dest],markers=['"passed": true'])
    run(tag+' assembly',['arm-none-eabi-objdump','-drwC',dest/'image.elf'])
 dest=w/'target-stub'
 if run('target stub build',target_args(dest,'arm-none-eabi-gcc','0',True),markers=['FE_TARGET_BUILD_PASS explicit_objects=5 no_archives']):run('target stub execute',emulator(dest/'image.elf'),1,['FE_TARGET_FAIL'])
 for ref in [False,True]:
  run('public '+str(ref),[sys.executable,LAB/'run.py','--foundation',PROVIDER,'--build',w/('public-'+str(ref)),*(['--source',REF/'entry.c'] if ref else [])],int(not ref),MARKERS if ref else [STUB])
 run('missing provider refused',[sys.executable,LAB/'run.py','--build',w/'missing'],2,['--foundation'])
 rows.append(dict(label='source stability',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 d=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tool_sha256={cc:sha(Path(shutil.which(cc))) for cc in tools},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Six hosted C17 profiles and four executed M4 cross profiles; object/image/map/generated-code audit uses explicit original helpers and no archive. Emulation is not physical observation.','343 direct-formula triples and65536 per-bit value/mask cases per native reference profile; M4 driver exercises all functions and recursion16, not the entire native corpus. Finite cases do not replace proofs.','Six separately compiled semantic mutants and one compiling unfinished learner stub per native profile; compiling M4 stub also rejected. Compile failure is not semantic success.','Actual original f_prefix provider imported without modification; reference artifacts are not learner implementations.','Published C17 text, external review, independent learner performance and actual delayed assessments remain separate.'])
 out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
