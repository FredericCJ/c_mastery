"""Execute the exact type/scope/linkage and retained-context assessment package."""
from pathlib import Path
import argparse,datetime,importlib.util,json,shutil,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/type_interfaces';REF=ROOT/'assessor/type_interfaces';REPORT='reviews/v5/expansion/legacy_type_interfaces/current_validation.json'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
MARKERS=['TD_REP_PASS row_factory function_alias wrapper opaque_distinct exact_units','TD_CALL_PASS actual_row_stride admission_before_callbacks explicit_adapter','TD_JOB_PASS borrowed_until_quiescent cancel_suppresses_call running_busy','TD_PASS typed_interfaces']
MUTANTS={1:'a && b && a != b',2:'td_speed((struct td_meters){13}',3:'td_rows(rows, 2, td_adapter',4:'td_cancel(&job) == TD_OK'};STUB='row && (*row)[0] == 1'
def module(name,p):
 s=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
common=module('td_common',ROOT/'assessor/legacy_systems/toolchain/common.py');sha=common.sha
def inputs():return sorted({Path(__file__),ROOT/'assessor/legacy_systems/toolchain/common.py',*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]})
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args();w=ROOT/'build/type-interfaces'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');c=common.Campaign(w,ROOT/a.report);pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};error=None;tools=['gcc','clang','nm','readelf'];count=None
 try:
  c.group='identities'
  for t in tools:c.run([t,'--version'])
  for cc in ['gcc','clang']:c.run([cc,'-dumpmachine'])
  c.run(['uname','-a'])
  oracle,count=module('td_oracle',REF/'oracle.py').generate(c)
  for cc,opt,tag in PROFILES:
   flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie','-I'+str(LAB)]+(['-O1','-g'] if 'asan' in tag else [])
   env={'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
   for v in [0,*MUTANTS,'stub']:
    c.group=tag+':'+str(v);exe=w/(tag+'-'+str(v));src=LAB/'learner.c' if v=='stub' else REF/'types.c'
    c.run([cc,*flags,'-DTD_MUTANT='+str(0 if v=='stub' else v),src,LAB/'tests.c','-o',exe])
    out=c.run([exe],expected=int(v!=0),contains=MARKERS[-1] if v==0 else STUB if v=='stub' else MUTANTS[v],env=env)
    c.require(not any(x in out for x in ['runtime error:','ERROR: AddressSanitizer']),'defined semantic rejection or reference execution')
    if v==0:c.require(all(x in out for x in MARKERS),'all representation/callback/job mechanisms observed')
   c.group=tag+':oracle';exe=w/(tag+'-oracle');c.run([cc,*flags,REF/'types.c',oracle,'-o',exe]);c.run([exe],contains=f'TD_ORACLE_PASS equations={count}',env=env)
   if tag.endswith('debug'):
    c.group=tag+':dependencies';c.run(['readelf','-dW',w/(tag+'-0')],contains='libc.so.6');c.run(['nm','-u',w/(tag+'-0')],contains='__libc_start_main')
  c.group='public-runner'
  for cc in ['gcc','clang']:
   for ref in [False,True]:c.run([sys.executable,LAB/'run.py','--cc',cc,'--build',w/(cc+'-public-'+str(ref)),*(['--source',REF/'types.c'] if ref else [])],expected=int(not ref),contains=MARKERS[-1] if ref else STUB)
  experiments=module('td_experiments',REF/'experiments.py');experiments.run(c,ROOT);experiments.units(c,ROOT)
  c.group='final';c.require(pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()},'source stability')
 except Exception as e:error=str(e)
 d=c.finish(pins,error);d.update(external_tool_sha256={t:dict(path=shutil.which(t),sha256=sha(Path(shutil.which(t)))) for t in tools if shutil.which(t)},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},bounds=dict(unit_equations=count,profiles=6,mutants=4,opaque_owners=2,matrix_rows=4,row_width=4),limits=d['limits']+['Hosted C17 profile A permits CRT/libc/POSIX harness; actual native dependencies inspected. Core mechanisms use original code.','Conflicting cross-TU, hidden mixed linkage and cast-based calls are analysis/translation only; only repaired compatible programs execute.','C23 capability observations concern only the inspected N3096 empty-parameter-list change, not full standard conformance.','Deferred job is serialized single-thread ownership, not real OS cancellation or concurrent scheduling. POSIX worker is used only for TLS observation.','External review, independent learner and seven-day performance remain unassessed.'])
 (ROOT/a.report).write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(d['commands']),checks=len(d['checks']),error=error));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
