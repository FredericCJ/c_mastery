"""Execute exact variadic protocol, promotion, sequencing and transfer mechanisms."""
from pathlib import Path
import argparse,datetime,importlib.util,json,shutil,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/variadic_expressions';REF=ROOT/'assessor/variadic_expressions';REPORT='reviews/v5/expansion/legacy_variadic_expressions/current_validation.json'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
MARKERS=['TF_PROMOTION_PASS char short float double exact_protocol','TF_LIFETIME_PASS two_independent_copies helper_borrow early_exit exact_once_cleanup','TF_ARRAY_PASS explicit_available_count tags borrow no_active_union_claim','TF_STEPS_PASS checked_arithmetic ordered_callbacks fallible_gate partial_progress','TF_NARROW_PASS admission_before_conversion negative_provenance_lost','TF_PASS variadic_expressions']
MUTANTS={1:'required==11',2:'m.releases==1 && m.live==0',3:'unchanged(out,sizeof out)',4:'p.result==7',5:'==TF_NOMEM',6:'tf_narrow_signed(-1,&byte)'};STUB='status==(capacity<=11?TF_FULL:TF_OK)'
def module(name,p):
 s=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
common=module('tf_common',ROOT/'assessor/legacy_systems/toolchain/common.py');sha=common.sha
def inputs():return sorted({Path(__file__),ROOT/'assessor/legacy_systems/toolchain/common.py',*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]})
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args();w=ROOT/'build/variadic-expressions'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');c=common.Campaign(w,ROOT/a.report);pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};error=None;tools=['gcc','clang','nm','readelf'];bounds={}
 try:
  c.group='identities'
  for t in tools:c.run([t,'--version'])
  for cc in ['gcc','clang']:c.run([cc,'-dumpmachine'])
  c.run(['uname','-a'])
  oracle_mod=module('tf_oracle',REF/'oracle.py');oracle,bounds=oracle_mod.generate(c)
  c.group='promotion-models';models=oracle_mod.models(c)
  for cc,opt,tag in PROFILES:
   flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie','-I'+str(LAB)]+(['-O1','-g'] if 'asan' in tag else [])
   env={'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
   for v in [0,*MUTANTS,'stub']:
    c.group=tag+':'+str(v);exe=w/(tag+'-'+str(v));src=[LAB/'learner.c'] if v=='stub' else [REF/'format.c',REF/'expressions.c']
    c.run([cc,*flags,'-DTF_MUTANT='+str(0 if v=='stub' else v),*src,LAB/'scalars.c',LAB/'tests.c','-o',exe])
    out=c.run([exe],expected=int(v!=0),contains=MARKERS[-1] if v==0 else STUB if v=='stub' else MUTANTS[v],env=env)
    c.require(not any(x in out for x in ['runtime error:','ERROR: AddressSanitizer']),'defined semantic rejection or reference execution')
    if v==0:c.require(all(x in out for x in MARKERS),'all protocol/step mechanisms observed')
   c.group=tag+':oracle';exe=w/(tag+'-oracle');c.run([cc,*flags,REF/'format.c',REF/'expressions.c',LAB/'scalars.c',oracle,'-o',exe]);c.run([exe],contains='TF_ORACLE_PASS text=259 equations=9261 narrow=2049',env=env)
   if tag.endswith('debug'):
    c.group=tag+':dependencies';c.run(['readelf','-dW',w/(tag+'-0')],contains='libc.so.6');c.run(['nm','-u',w/(tag+'-0')],contains='__libc_start_main')
  c.group='public-runner'
  for cc in ['gcc','clang']:
   for ref in [False,True]:c.run([sys.executable,LAB/'run.py','--cc',cc,'--build',w/(cc+'-public-'+str(ref)),*(['--source',REF/'format.c','--source',REF/'expressions.c'] if ref else [])],expected=int(not ref),contains=MARKERS[-1] if ref else STUB)
  module('tf_experiments',REF/'experiments.py').run(c,ROOT,PROFILES)
  c.group='final';c.require(pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()},'source stability')
 except Exception as e:error=str(e)
 d=c.finish(pins,error);d.update(external_tool_sha256={t:dict(path=shutil.which(t),sha256=sha(Path(shutil.which(t)))) for t in tools if shutil.which(t)},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},bounds=dict(**bounds,profiles=6,mutants=6,max_pairs=4),limits=d['limits']+['Hosted C17 profile A permits scalar snprintf, CRT/libc and allocation; core traversal/transaction is original. Native ELF dependencies inspected.','Wrong-type/missing varargs and conflicting modifications are analysis/translation only, never golden-output executions.','16/64-bit promotion models are mathematical changed assumptions, not actual target runs. Native recorded int32/short16/char8 ABI only.','Formatter size overflow is unreachable here: at most4 nonnegative int counts plus NUL fit native size_t64; branch retained for narrower size_t models and reviewed algebraically.','Stable conforming scalar callbacks and nonaliasing live caller buffers are preconditions. Runtime checks cannot prove their provenance.','External review, independent learner and seven-day performance remain unassessed.'])
 (ROOT/a.report).write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(d['commands']),checks=len(d['checks']),error=error));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
