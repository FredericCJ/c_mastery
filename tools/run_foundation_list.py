"""Run exact T01 list, real build/loader/Git, and changed-condition experiments."""
from pathlib import Path
import argparse,datetime,importlib.util,json,shutil,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/foundation_list';REF=ROOT/'assessor/foundation_list'
REPORT='reviews/v5/expansion/legacy_foundation_list/current_validation.json'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
MARKERS=['FT_TOKEN_PASS grammar ranges capacity4 atomic_prefix collected10_stored8','FT_STREAM_PASS all7splits EOF_once embeddedNUL long_leading_zeros','FT_PASS bounded_list_and_changed_input']
MUTANTS={1:'ft_tokens(bad, 3, 0, &out, &e) == FT_PREFIX',2:'ft_stream_finish(&s, &r) == FT_OK && r.count == 3',3:'ft_tokens(bad, 3, 0, &out, &e) == FT_PREFIX',4:'ft_tokens(errors, 11, 1, &out, &e) == FT_MALFORMED',5:'ft_tokens(&spelling[i], 1, 0, &out, &e) == FT_MALFORMED'}
STUB='ft_tokens(normal, 3, 0, &out, &e) == FT_OK'
def module(name,p):
 spec=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
common=module('list_common',ROOT/'assessor/legacy_systems/toolchain/common.py');sha=common.sha
def inputs():return sorted({Path(__file__),ROOT/'assessor/legacy_systems/toolchain/common.py',*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]})
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args();w=ROOT/'build/foundation-list'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');c=common.Campaign(w,ROOT/a.report)
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};error=None;tools=['gcc','clang','readelf','nm','objdump','git','env'];count=None
 try:
  c.group='identities'
  for t in tools:c.run([t,'--version'])
  for cc in ['gcc','clang']:c.run([cc,'-dumpmachine'])
  c.run(['uname','-a'])
  oracle,count=module('list_oracle',REF/'oracle.py').generate(c)
  for cc,opt,tag in PROFILES:
   flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie','-I'+str(LAB)]+(['-O1','-g'] if 'asan' in tag else [])
   env={'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
   for v in [0,*MUTANTS,'stub']:
    c.group=tag+':'+str(v);exe=w/(tag+'-'+str(v));src=LAB/'learner.c' if v=='stub' else REF/'list.c'
    c.run([cc,*flags,'-DFT_MUTANT='+str(0 if v=='stub' else v),src,LAB/'tests.c','-o',exe])
    out=c.run([exe],expected=int(v!=0),contains=MARKERS[-1] if v==0 else STUB if v=='stub' else MUTANTS[v],env=env)
    c.require(not any(x in out for x in ['runtime error:','ERROR: AddressSanitizer']),'defined semantic rejection or reference execution')
    if v==0:c.require(all(x in out for x in MARKERS),'all token/stream mechanisms observed')
   c.group=tag+':oracle';exe=w/(tag+'-oracle');c.run([cc,*flags,REF/'list.c',oracle,'-o',exe]);c.run([exe],contains=f'FT_ORACLE_PASS cases={count} modes=2',env=env)
   c.group=tag+':cli';exe=w/(tag+'-cli');c.run([cc,*flags,REF/'list.c',LAB/'main.c','-o',exe]);c.run([exe],contains='empty count=0 sum=0',env=env);c.run([exe,'-32768','-1','1'],expected=5,contains='prefix',env=env)
   if tag.endswith('debug'):
    c.group=tag+':dependencies';c.run(['readelf','-dW',exe],contains='libc.so.6');c.run(['nm','-u',exe],contains='__libc_start_main')
  c.group='capacity-transfer'
  for cc in ['gcc','clang']:
   exe=w/(cc+'-capacity2');c.run([cc,'-std=c17','-O3','-DFT_CAPACITY=2','-I'+str(LAB),REF/'list.c',LAB/'main.c','-o',exe]);c.run([exe,'2','-5'],contains='ok count=2 sum=-3');c.run([exe,'2','-5','9'],expected=4,contains='capacity')
  c.group='public-runner'
  for cc in ['gcc','clang']:
   for ref in [False,True]:c.run([sys.executable,LAB/'run.py','--cc',cc,'--build',w/(cc+'-public-'+str(ref)),*(['--source',REF/'list.c'] if ref else [])],expected=int(not ref),contains=MARKERS[-1] if ref else STUB)
  wf=module('list_workflow',REF/'workflow.py');wf.stages(c,ROOT);wf.revisions(c,ROOT);wf.diagnosis(c,ROOT)
  for objective in ['T01.06.a','T01.06.b']:
   c.observe(objective,'Six native profiles, handwritten contract partitions and independent whole-token/bignum oracle; failure-atomic numeric result',['independent-oracle.json','native command groups'],False)
   c.observe(objective,'Chunked EOF/state and bounded collect-all diagnostics are separate implemented APIs with semantic controls',['labs/foundation_list/tests.c','independent-oracle.json'],True)
  c.group='final';c.require(pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()},'source stability')
 except Exception as e:error=str(e)
 d=c.finish(pins,error);d.update(external_tool_sha256={t:dict(path=shutil.which(t),sha256=sha(Path(shutil.which(t)))) for t in tools if shutil.which(t)},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file() and '.git' not in p.parts},bounds=dict(oracle_cases=count,oracle_modes=2,capacity=4,diagnostic_capacity=8,profiles=len(PROFILES),mutants=5,stream_splits=7),limits=d['limits']+['Hosted C17 profile A permits CRT/libc; actual dynamic/undefined dependencies inspected. Original parser uses no library parsing mechanism.','Actual INT_MAX+1 faulty check-after-add is compile-only; executable synthetic-range defects fit int32 and have no UB golden outputs.','ABI/loader experiments are Linux x86-64 System V observations; six profiles do not prove all toolchains, inputs or execution orders.','Seven-day packages are implemented; independent delayed performance remains unassessed.'])
 (ROOT/a.report).write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(d['commands']),checks=len(d['checks']),observations=len(d['observations']),error=error));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
